/*
 * Copyright (c) 2016 Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of other
 *   contributors to this software may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *   4. This software must only be used in or with a processor manufactured by Nordic
 *   Semiconductor ASA, or in or with a processor manufactured by a third party that
 *   is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *   5. Any software provided in binary or object form under this license must not be
 *   reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "serialization_transport.h"

#include "ble.h"
#include "ble_app.h"
#include "nrf_error.h"

#include "ble_common.h"

#include <iterator>
#include <memory>
#include <sstream>

SerializationTransport::SerializationTransport(H5Transport *dataLinkLayer,
                                               uint32_t response_timeout)
    : statusCallback(nullptr)
    , eventCallback(nullptr)
    , logCallback(nullptr)
    , responseReceived(false)
    , responseBuffer(nullptr)
    , processEvents(false)
    , isOpen(false)
{
    // SerializationTransport takes ownership of dataLinkLayer provided object
    nextTransportLayer = std::shared_ptr<H5Transport>(dataLinkLayer);
    responseTimeout    = response_timeout;
}

SerializationTransport::~SerializationTransport()
{
    if (eventThread.joinable())
    {
        eventThread.join();
    }
}

uint32_t SerializationTransport::open(const status_cb_t &status_callback,
                                      const evt_cb_t &event_callback,
                                      const log_cb_t &log_callback) noexcept
{
    std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

    if (isOpen)
    {
        return NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT_ALREADY_OPEN;
    }

    statusCallback = status_callback;
    eventCallback  = event_callback;
    logCallback    = log_callback;

    const auto dataCallback = std::bind(&SerializationTransport::readHandler, this,
                                        std::placeholders::_1, std::placeholders::_2);

    const auto errorCode = nextTransportLayer->open(status_callback, dataCallback, log_callback);

    if (errorCode != NRF_SUCCESS)
    {
        return errorCode;
    }

    isOpen = true;

    // Thread should not be running from before when calling this
    if (!eventThread.joinable())
    {
        // If ::close is called when this method (::open) returns
        // and eventThread is executing somewhere between while (isOpen)
        // and eventWaitCondition notification we could get a deadlock.
        //
        // To prevent this, lock eventMutex in this thread, let eventThread
        // wait until eventMutex is unlocked (.wait in this thread).
        // When eventThread is started and outside critical region,
        // let eventThread notify eventWaitCondition, making .wait
        // return
        std::unique_lock<std::mutex> eventLock(eventMutex);
        processEvents = true;
        eventThread   = std::thread([this] { eventHandlingRunner(); });
        eventWaitCondition.wait(eventLock);
    }
    else
    {
        return NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT;
    }

    return NRF_SUCCESS;
}

uint32_t SerializationTransport::close() noexcept
{
    // Stop event processing thread before closing since
    // event callbacks may in application space invoke new calls to SerializationTransport
    {
        std::unique_lock<std::mutex> eventLock(eventMutex);
        processEvents = false;
        eventWaitCondition.notify_all();
    }

    if (eventThread.joinable())
    {
        if (std::this_thread::get_id() == eventThread.get_id())
        {
            // log "ser_app_hal_pc_event_handling_stop was called from an event callback, causing
            // the event thread to stop itself. This will cause a resource leak."
            return NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT;
        }

        try
        {
            eventThread.join();
        }
        catch (const std::system_error &)
        {
            return NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT_INVALID_STATE;
        }
    }

    // Close this and the underlying transport
    std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

    if (!isOpen)
    {
        return NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT_ALREADY_CLOSED;
    }

    isOpen = false;

    return nextTransportLayer->close();
}

uint32_t SerializationTransport::send(const std::vector<uint8_t> &cmdBuffer,
                                      std::shared_ptr<std::vector<uint8_t>> rspBuffer,
                                      serialization_pkt_type_t pktType) noexcept
{
    std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

    if (!isOpen)
    {
        return NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT_INVALID_STATE;
    }

    // Mutex to avoid multiple threads sending commands at the same time.
    std::lock_guard<std::mutex> sendGuard(sendMutex);
    responseReceived = false;
    responseBuffer   = rspBuffer;

    std::vector<uint8_t> commandBuffer(cmdBuffer.size() + 1);
    commandBuffer[0] = pktType;
    std::copy(cmdBuffer.begin(), cmdBuffer.end(), commandBuffer.begin() + 1);

    const auto errCode = nextTransportLayer->send(commandBuffer);

    if (errCode != NRF_SUCCESS)
    {
        return errCode;
    }

    if (!rspBuffer)
    {
        return NRF_SUCCESS;
    }

    std::unique_lock<std::mutex> responseGuard(responseMutex);

    const std::chrono::milliseconds timeout(responseTimeout);
    const auto wakeupTime = std::chrono::system_clock::now() + timeout;

    responseWaitCondition.wait_until(responseGuard, wakeupTime, [&] { return responseReceived; });

    if (!responseReceived)
    {
        logCallback(SD_RPC_LOG_WARNING, "Failed to receive response for command");
        return NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT_NO_RESPONSE;
    }

    return NRF_SUCCESS;
}

void SerializationTransport::drainEventQueue()
{
    std::unique_lock<std::mutex> eventLock(eventMutex);

    // Drain the queue for any old events
    try
    {
        while (!eventQueue.empty())
        {
            eventQueue.pop();
        }
    }
    catch (const std::exception &)
    {
        logCallback(SD_RPC_LOG_ERROR, "Error in SerializationTransport::drainEventQueue");
    }
}

// Event Thread
void SerializationTransport::eventHandlingRunner() noexcept
{
    try
    {
        drainEventQueue();

        std::unique_lock<std::mutex> eventLock(eventMutex);

        while (processEvents)
        {
            // Suspend this thread until event wait condition
            // is notified. This can happen from ::close and
            // ::readHandler (thread in H5Transport)
            eventWaitCondition.notify_all();
            eventWaitCondition.wait(eventLock);

            while (!eventQueue.empty() && processEvents)
            {
                // Get oldest event received from UART thread
                const auto eventData     = eventQueue.front();
                const auto eventDataSize = static_cast<uint32_t>(eventData.size());

                // Remove oldest event received from H5Transport thread
                eventQueue.pop();

                // Let UART thread add events to eventQueue
                // while popped event is processed
                eventLock.unlock();

                // Set codec context
                EventCodecContext context(this);

                // Allocate memory to store decoded event including an unknown quantity of padding
                auto possibleEventLength = static_cast<uint32_t>(MaxPossibleEventLength);
                std::vector<uint8_t> eventDecodeBuffer;
                eventDecodeBuffer.resize(MaxPossibleEventLength);
                const auto event = reinterpret_cast<ble_evt_t *>(eventDecodeBuffer.data());

                // Decode event
                const auto errCode =
                    ble_event_dec(eventData.data(), eventDataSize, event, &possibleEventLength);

                if (eventCallback && errCode == NRF_SUCCESS)
                {
                    eventCallback(event);
                }

                if (errCode != NRF_SUCCESS)
                {
                    std::stringstream logMessage;
                    logMessage << "Failed to decode event, error code is " << std::dec << errCode
                               << "/0x" << std::hex << errCode << ".";
                    logCallback(SD_RPC_LOG_ERROR, logMessage.str());
                    statusCallback(PKT_DECODE_ERROR, logMessage.str());
                }

                // Prevent UART from adding events to eventQueue
                eventLock.lock();
            }
        }

        eventWaitCondition.notify_all();
    }
    catch (const std::exception &e)
    {
        std::stringstream ss;
        ss << "Error in SerializationTransport::eventHandlingRunner, " << e.what() << std::endl;
        logCallback(SD_RPC_LOG_FATAL, ss.str());
    }
}

void SerializationTransport::readHandler(const uint8_t *data, const size_t length)
{
    const auto eventType = static_cast<serialization_pkt_type_t>(data[0]);

    const auto startOfData  = data + 1;
    const size_t dataLength = length - 1;

    if (eventType == SERIALIZATION_RESPONSE)
    {
        if (responseBuffer && !responseBuffer->empty())
        {
            if (responseBuffer->size() >= dataLength)
            {
                std::copy(startOfData, startOfData + dataLength, responseBuffer->begin());
                responseBuffer->resize(dataLength);
            }
            else
            {
                logCallback(SD_RPC_LOG_ERROR, "Received SERIALIZATION_RESPONSE with a packet that "
                                              "is larger than the allocated buffer.");
            }
        }
        else
        {
            logCallback(SD_RPC_LOG_ERROR, "Received SERIALIZATION_RESPONSE but command did not "
                                          "provide a buffer for the reply.");
        }

        std::lock_guard<std::mutex> responseGuard(responseMutex);
        responseReceived = true;
        responseWaitCondition.notify_one();
    }
    else if (eventType == SERIALIZATION_EVENT)
    {
        std::vector<uint8_t> event;
        event.reserve(dataLength);
        std::copy(startOfData, startOfData + dataLength, std::back_inserter(event));
        std::lock_guard<std::mutex> eventLock(eventMutex);
        eventQueue.push(std::move(event));
        eventWaitCondition.notify_one();
    }
    else
    {
        logCallback(SD_RPC_LOG_WARNING,
                    "Unknown Nordic Semiconductor vendor specific packet received");
    }
}
