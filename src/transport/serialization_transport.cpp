/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "serialization_transport.h"

#include "ble.h"
#include "ble_app.h"
#include "nrf_error.h"

#include "ble_common.h"

#include <memory>
#include <iostream>
#include <sstream>
#include <cstring> // Do not remove! Required by gcc.

SerializationTransport::SerializationTransport(Transport *dataLinkLayer, uint32_t response_timeout)
    : statusCallback(nullptr), eventCallback(nullptr),
    logCallback(nullptr), rspReceived(false),
    responseBuffer(nullptr), responseLength(nullptr),
    runEventThread(false)
{
    eventThread = nullptr;
    nextTransportLayer = dataLinkLayer;
    responseTimeout = response_timeout;
}


SerializationTransport::SerializationTransport(): nextTransportLayer(nullptr), responseTimeout(0), rspReceived(false), responseBuffer(nullptr), responseLength(nullptr), runEventThread(false), eventThread(nullptr)
{}

SerializationTransport::~SerializationTransport()
{
    delete nextTransportLayer;
}

uint32_t SerializationTransport::open(status_cb_t status_callback, evt_cb_t event_callback, log_cb_t log_callback)
{
    statusCallback = status_callback;
    eventCallback = event_callback;
    logCallback = log_callback;

    data_cb_t dataCallback = std::bind(&SerializationTransport::readHandler, this, std::placeholders::_1, std::placeholders::_2);

    uint32_t errorCode = nextTransportLayer->open(status_callback, dataCallback, log_callback);

    if (errorCode != NRF_SUCCESS)
    {
        return errorCode;
    }

    runEventThread = true;

    if (eventThread == nullptr)
    {
        eventThread = new std::thread(std::bind(&SerializationTransport::eventHandlingRunner, this));
    }

    return NRF_SUCCESS;
}

uint32_t SerializationTransport::close()
{
    eventMutex.lock();
    runEventThread = false;
    eventWaitCondition.notify_one();
    eventMutex.unlock();

    if (eventThread != nullptr)
    {
        if (std::this_thread::get_id() == eventThread->get_id())
        {
            //log "ser_app_hal_pc_event_handling_stop was called from an event callback, causing the event thread to stop itself. This will cause a resource leak."
            eventThread = nullptr;
            return NRF_ERROR_INTERNAL;
        }

        eventThread->join();
        delete eventThread;
        eventThread = nullptr;
    }

    return nextTransportLayer->close();
}

uint32_t SerializationTransport::send(uint8_t *cmdBuffer, uint32_t cmdLength, uint8_t *rspBuffer, uint32_t *rspLength)
{
    // Mutex to avoid multiple threads sending commands at the same time.
    std::unique_lock<std::mutex> sendGuard(sendMutex);
    rspReceived = false;
    responseBuffer = rspBuffer;
    responseLength = rspLength;

    std::vector<uint8_t> commandBuffer(cmdLength + 1);
    commandBuffer[0] = SERIALIZATION_COMMAND;
    memcpy(&commandBuffer[1], cmdBuffer, cmdLength * sizeof(uint8_t));

    auto errCode = nextTransportLayer->send(commandBuffer);

    if (errCode != NRF_SUCCESS) {
        return errCode;
    }
    else if (rspBuffer == nullptr)
    {
        return NRF_SUCCESS;
    }

    std::unique_lock<std::mutex> responseGuard(responseMutex);

    if (!rspReceived)
    {
        std::chrono::milliseconds timeout(responseTimeout);
        std::chrono::system_clock::time_point wakeupTime = std::chrono::system_clock::now() + timeout;
        std::cv_status status = responseWaitCondition.wait_until(responseGuard, wakeupTime);

        if (status == std::cv_status::timeout)
        {
            logCallback(SD_RPC_LOG_WARNING, "Failed to receive response for command");
            return NRF_ERROR_INTERNAL;
        }
    }

    return NRF_SUCCESS;
}

// Event Thread
void SerializationTransport::eventHandlingRunner()
{
    while (runEventThread) {

        while (!eventQueue.empty())
        {
            eventData_t eventData = eventQueue.front();
            eventQueue.pop();
            // Allocate memory to store decoded event including an unknown quantity of padding

            // Set security context
            BLESecurityContext context(this);

            uint32_t possibleEventLength = 512;
            std::unique_ptr<ble_evt_t> event(static_cast<ble_evt_t*>(std::malloc(possibleEventLength)));
            uint32_t errCode = ble_event_dec(eventData.data, eventData.dataLength, event.get(), &possibleEventLength);

            if (eventCallback != nullptr && errCode == NRF_SUCCESS)
            {
                eventCallback(event.get());
            }

            if (errCode != NRF_SUCCESS)
            {
                std::stringstream logMessage;
                logMessage << "Failed to decode event, error code is " << errCode << "." << std::endl;
                logCallback(SD_RPC_LOG_ERROR, logMessage.str().c_str());
            }

            free(eventData.data);
        }

        std::unique_lock<std::mutex> eventLock(eventMutex);

        if (!runEventThread)
        {
            break;
        }

        if (!eventQueue.empty())
        {
            continue;
        }

        eventWaitCondition.wait(eventLock);
    }
}

// Read Thread
void SerializationTransport::readHandler(uint8_t *data, size_t length)
{
    auto eventType = static_cast<serialization_pkt_type_t>(data[0]);
    data += 1;
    length -= 1;

    if (eventType == SERIALIZATION_RESPONSE) {
        memcpy(responseBuffer, data, length);
        *responseLength = length;

        std::lock_guard<std::mutex> responseGuard(responseMutex);
        rspReceived = true;
        responseWaitCondition.notify_one();
    }
    else if (eventType == SERIALIZATION_EVENT)
    {
        eventData_t eventData;
        eventData.data = static_cast<uint8_t *>(malloc(length));
        memcpy(eventData.data, data, length);
        eventData.dataLength = length;

        std::lock_guard<std::mutex> eventLock(eventMutex);
        eventQueue.push(eventData);
        eventWaitCondition.notify_one();
    }
    else
    {
        logCallback(SD_RPC_LOG_WARNING, "Unknown Nordic Semiconductor vendor specific packet received");
    }
}
