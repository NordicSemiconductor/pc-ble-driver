/*
 * Copyright (c) 2016-2019 Nordic Semiconductor ASA
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

/**
    Three Wire Packet types (From BLUETOOTH SPECIFICATION V4.2 [Vol 4, Part D], 8.X

    |Type  | Name            | Byte pattern
    -------+-----------------+--------------------------------
    | 15   | SYNC MESSAGE    | 0x01 0x7e
    | 15   | SYNC RESPONSE   | 0x02 0x7d
    | 15   | CONFIG MESSAGE  | 0x03 0xfc CONFIGURATION_FIELD
    | 15   | CONFIG RESPONSE | 0x04 0x7b CONFIGURATION_FIELD
    | 15   | WAKEUP MESSAGE  | 0x05 0xfa
    | 15   | WOKEN MESSAGE   | 0x06 0xf9
    | 15   | SLEEP MESSAGE   | 0x07 0x78

*/

#include <iostream>

#include "h5_transport.h"
#include "nrf_error.h"
#include "sd_rpc_types.h"

#include "h5.h"
#include "slip.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>
#include <uart_transport.h>

const uint8_t PACKET_RETRANSMISSIONS =
    6; // Number of times to send reliable packets before giving in

// Other constants

// Duration to wait for state ACTIVE after open is called
const auto OPEN_WAIT_TIMEOUT = std::chrono::milliseconds(3000);
// Duration to wait before continuing UART communication after reset is sent to target
const auto RESET_WAIT_DURATION = std::chrono::milliseconds(300);

#pragma region Public methods
H5Transport::H5Transport(UartTransport *_nextTransportLayer, const uint32_t retransmission_interval)
    : nextTransportLayer(_nextTransportLayer)
    , seqNum(0)
    , ackNum(0)
    , c0Found(false)
    , retransmissionInterval(std::chrono::milliseconds(retransmission_interval))
    , incomingPacketCount(0)
    , outgoingPacketCount(0)
    , errorPacketCount(0)
    , currentState(STATE_START)
    , stateMachineReady(false)
    , isOpen(false)
{}

H5Transport::~H5Transport() noexcept
{
    stopStateMachine();
    delete nextTransportLayer;
}

uint32_t H5Transport::open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                           const log_cb_t &log_callback) noexcept
{
    std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

    if (isOpen)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_ALREADY_OPEN;
    }

    isOpen = true;

    auto errorCode = Transport::open(status_callback, data_callback, log_callback);

    if (errorCode != NRF_SUCCESS)
    {
        return errorCode;
    }

    try
    {
        std::unique_lock<std::mutex> lck(currentStateMutex);

        if (!(currentState == STATE_START || currentState == STATE_CLOSED))
        {
            std::stringstream ss;
            ss << "Not able to open, current state is not valid (" << stateToString(currentState)
               << ")";
            log(SD_RPC_LOG_FATAL, ss.str());
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
        }

        // State machine starts in a separate thread.
        // Wait for the state machine to be ready
        setupStateMachine();
        startStateMachine();

        lastPacket.clear();

        statusCallback = std::bind(&H5Transport::statusHandler, this, std::placeholders::_1,
                                   std::placeholders::_2);
        dataCallback   = std::bind(&H5Transport::dataHandler, this, std::placeholders::_1,
                                 std::placeholders::_2);

        errorCode = nextTransportLayer->open(statusCallback, dataCallback, upperLogCallback);

        std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);

        const auto currentExitCriteria = exitCriterias.at(currentState);
        const auto exitCriteria = dynamic_cast<StartExitCriterias *>(currentExitCriteria.get());

        if (exitCriteria == nullptr)
        {
            std::stringstream ss;
            ss << "h5_transport is in state " << stateToString(currentState)
               << " but should be in STATE_START. This state is not valid.";
            log(SD_RPC_LOG_WARNING, ss.str());
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
        }

        if (errorCode != NRF_SUCCESS)
        {
            exitCriteria->ioResourceError = true;
        }
        else
        {
            exitCriteria->isOpened = true;
        }

        stateMachineLock.unlock();
        stateMachineChange.notify_all();
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
    }
    catch (const std::exception &ex)
    {
        log(SD_RPC_LOG_ERROR, "Unexpected exception when opening adapter", ex);
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_INTERNAL_ERROR;
    }

    if (waitForState(STATE_ACTIVE, OPEN_WAIT_TIMEOUT))
    {
        return NRF_SUCCESS;
    }

    switch (state())
    {
        case STATE_START:
        case STATE_RESET:
        case STATE_UNINITIALIZED:
        case STATE_INITIALIZED:
        case STATE_NO_RESPONSE:
            // There are two situations on can get timeout:
            // 1) there is no response from the device
            // 2) non failing state transitions from STATE_START to STATE_ACTIVE did not happen
            // in time period OPEN_WAIT_TIMEOUT
            return NRF_ERROR_TIMEOUT;
        case STATE_FAILED:
        case STATE_CLOSED:
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
        case STATE_ACTIVE:
            return NRF_SUCCESS;
        default:
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
    }
}

uint32_t H5Transport::close() noexcept
{
    std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

    if (!isOpen)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_ALREADY_CLOSED;
    }

    isOpen = false;

    {
        std::unique_lock<std::mutex> currentStateLck(currentStateMutex);

        try
        {
            const auto currentExitCriteria = exitCriterias.at(currentState);
            const auto exitCriteria        = currentExitCriteria.get();

            if (exitCriteria != nullptr)
            {
                std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
                exitCriteria->close = true;
            }

            stateMachineChange.notify_all();
        }
        catch (const std::out_of_range &)
        {
            std::stringstream ss;
            ss << "State " << stateToString(currentState)
               << " does not have exit criteria associated with it. Will continue to close the "
                  "H5Transport.";
            log(SD_RPC_LOG_WARNING, ss.str());
        }
        catch (const std::exception &)
        {
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
        }
    }

    try
    {
        stopStateMachine();
    }
    catch (const std::exception &)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
    }

    return nextTransportLayer->close();
}

uint32_t H5Transport::send(const std::vector<uint8_t> &data) noexcept
{
    std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

    if (!isOpen)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
    }

    if (currentState != STATE_ACTIVE)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE;
    }

    try
    {
        // max theoretical length of encoded packet, aditional 6 bytes h5 encoding and all bytes
        // escaped
        // + 2 packet encapsuling

        payload_t h5EncodedPacket;

        {
            std::unique_lock<std::recursive_mutex> seqNumLck(seqNumMutex);
            std::unique_lock<std::recursive_mutex> ackNumLck(ackNumMutex);
            h5_encode(data, h5EncodedPacket, seqNum, ackNum, true, true, VENDOR_SPECIFIC_PACKET);
        }

        payload_t encodedPacket;
        slip_encode(h5EncodedPacket, encodedPacket);

        auto remainingRetransmissions = PACKET_RETRANSMISSIONS;

        lastPacket.clear();
        lastPacket = encodedPacket;

        std::unique_lock<std::mutex> ackLock(ackMutex);

        while (remainingRetransmissions--)
        {
            logPacket(true, h5EncodedPacket);
            const auto err_code = nextTransportLayer->send(lastPacket);

            if (err_code != NRF_SUCCESS)
                return err_code;

            uint8_t seqNumBefore;

            {
                std::unique_lock<std::recursive_mutex> seqNumLck(seqNumMutex);
                seqNumBefore = seqNum;
            }

            // Checking for timeout. Also checking against spurios wakeup by making sure the
            // sequence number has actually increased. If the sequence number has not increased, we
            // have not received an ACK packet, and should not exit the loop (unless timeout). Ref.
            // spurious wakeup: http://en.cppreference.com/w/cpp/thread/condition_variable
            // https://en.wikipedia.org/wiki/Spurious_wakeup
            if (ackReceived.wait_for(
                    ackLock, std::chrono::milliseconds(retransmissionInterval), [&] {
                        std::unique_lock<std::recursive_mutex> seqNumLck(seqNumMutex);
                        return seqNum != seqNumBefore;
                    }))
            {
                lastPacket.clear();
                return NRF_SUCCESS;
            }
        }

        lastPacket.clear();
    }
    catch (const std::exception &)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_INTERNAL_ERROR;
    }

    return NRF_ERROR_SD_RPC_H5_TRANSPORT_NO_RESPONSE;
}

h5_state_t H5Transport::state() const
{
    return currentState;
}

#pragma endregion Public methods

#pragma region Processing incoming data from UART
void H5Transport::processPacket(const payload_t &packet)
{
    uint8_t decodedSeqNum;
    uint8_t decodedAckNum;
    bool decodedReliablePacket;
    h5_pkt_type_t decodedPacketType;

    payload_t slipPayload;
    auto err_code = slip_decode(packet, slipPayload);

    if (err_code != NRF_SUCCESS)
    {
        ++errorPacketCount;

        std::stringstream ss;
        ss << "slip_decode error, code: 0x" << std::hex << static_cast<uint32_t>(err_code);
        ss << ", H5 error count: " << static_cast<uint32_t>(errorPacketCount)
           << ". raw packet: " << asHex(packet);
        log(SD_RPC_LOG_ERROR, ss.str());

        return;
    }

    logPacket(false, slipPayload);

    payload_t h5Payload;

    err_code = h5_decode(slipPayload, h5Payload, &decodedSeqNum, &decodedAckNum, nullptr, nullptr,
                         nullptr, &decodedReliablePacket, &decodedPacketType);

    if (err_code != NRF_SUCCESS)
    {
        ++errorPacketCount;

        std::stringstream ss;
        ss << "h5_decode error, code: 0x" << std::hex << static_cast<uint32_t>(err_code);
        ss << ", H5 error count: " << static_cast<uint32_t>(errorPacketCount)
           << ". raw packet: " << asHex(packet);
        log(SD_RPC_LOG_ERROR, ss.str());

        return;
    }

    std::unique_lock<std::mutex> currentStateLock(currentStateMutex);

    if (currentState == STATE_RESET)
    {
        // Ignore packets packets received in this state.
        stateMachineChange.notify_all();
        return;
    }

    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);

    if (decodedPacketType == LINK_CONTROL_PACKET)
    {
        if (currentState == STATE_UNINITIALIZED)
        {
            if (H5Transport::isSyncResponsePacket(h5Payload))
            {
                const auto stateCriteria =
                    dynamic_cast<UninitializedExitCriterias *>(exitCriterias[currentState].get());

                if (stateCriteria != nullptr)
                {
                    stateCriteria->syncRspReceived = true;
                }
            }
            else if (H5Transport::isSyncPacket(h5Payload))
            {
                sendControlPacket(CONTROL_PKT_SYNC_RESPONSE);
            }
        }
        else if (currentState == STATE_INITIALIZED)
        {
            const auto exit =
                dynamic_cast<InitializedExitCriterias *>(exitCriterias[currentState].get());

            if (H5Transport::isSyncConfigResponsePacket(h5Payload))
            {
                exit->syncConfigRspReceived = true;
            }
            else if (H5Transport::isSyncConfigPacket(h5Payload))
            {
                sendControlPacket(CONTROL_PKT_SYNC_CONFIG_RESPONSE);
            }
            else if (H5Transport::isSyncPacket(h5Payload))
            {
                sendControlPacket(CONTROL_PKT_SYNC_RESPONSE);
            }
        }
        else if (currentState == STATE_ACTIVE)
        {
            const auto exit =
                dynamic_cast<ActiveExitCriterias *>(exitCriterias[currentState].get());

            if (H5Transport::isSyncPacket(h5Payload))
            {
                exit->syncReceived = true;
            }
            else if (H5Transport::isSyncConfigPacket(h5Payload))
            {
                sendControlPacket(CONTROL_PKT_SYNC_CONFIG_RESPONSE);
            }
        }
    }
    else if (decodedPacketType == VENDOR_SPECIFIC_PACKET)
    {
        if (currentState == STATE_ACTIVE)
        {
            if (decodedReliablePacket)
            {
                std::lock_guard<std::recursive_mutex> lck(ackNumMutex);

                if (decodedSeqNum == ackNum)
                {
                    incrementAckNum();
                    sendControlPacket(CONTROL_PKT_ACK, ackNum);
                    upperDataCallback(h5Payload.data(), h5Payload.size());
                }
                else
                {
                    sendControlPacket(CONTROL_PKT_ACK, ackNum);
                }
            }
        }
    }
    else if (decodedPacketType == ACK_PACKET)
    {
        std::lock_guard<std::recursive_mutex> lck(seqNumMutex);

        if (decodedAckNum == ((seqNum + 1) & 0x07))
        {
            // Received a packet with valid ack_num, inform threads that wait the command is
            // received on the other end
            incrementSeqNum();
            ackReceived.notify_all();
        }
        else if (decodedAckNum == seqNum)
        {
            // Discard packet, we assume that we have received a reply from a previous packet
        }
        else
        {
            if (currentState == STATE_ACTIVE)
            {
                const auto exit =
                    dynamic_cast<ActiveExitCriterias *>(exitCriterias[currentState].get());

                if (exit != nullptr)
                {
                    exit->irrecoverableSyncError = true;
                }
            }
            else
            {
                std::stringstream ss;
                ss << "h5_transport received ack packet in state " << stateToString(currentState)
                   << ". ack_num is: " << std::hex << +decodedAckNum << " seq_num is: " << std::hex
                   << +decodedSeqNum << ". Ignoring the packet.";
                log(SD_RPC_LOG_WARNING, ss.str());
            }
        }
    }

    stateMachineLock.unlock();
    stateMachineChange.notify_all();
}

void H5Transport::statusHandler(const sd_rpc_app_status_t code, const std::string &message) noexcept
{
    if (code == IO_RESOURCES_UNAVAILABLE)
    {
        try
        {
            std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
            const auto currentExitCriteria = exitCriterias.at(currentState);
            const auto exitCriteria        = currentExitCriteria.get();

            if (exitCriteria != nullptr)
            {
                exitCriteria->ioResourceError = true;
            }

            stateMachineLock.unlock();
            stateMachineChange.notify_all();
        }
        catch (std::out_of_range &)
        {
            try
            {
                std::stringstream ss;
                ss << "State " << stateToString(currentState)
                   << " does not have criteria associated with it.";
                log(SD_RPC_LOG_WARNING, ss.str());
            }
            catch (const std::exception &)
            {
                log(SD_RPC_LOG_ERROR, "Error creating string describing error status");
            }
        }
        catch (std::exception &e)
        {
            try
            {
                std::stringstream ss;
                ss << "Unexpected exception received in state " << stateToString(currentState)
                   << ", " << e.what();
                log(SD_RPC_LOG_ERROR, ss.str());
            }
            catch (const std::exception &)
            {
                log(SD_RPC_LOG_ERROR,
                    "Error creating string describing error status for unexpected exception");
            }
        }
    }

    status(code, message);
}

void H5Transport::dataHandler(const uint8_t *data, const size_t length) noexcept
{
    payload_t packet;

    try
    {
        // Check if we have any data from before that has not been processed.
        // If so add the remaining data from previous callback(s) to this packet
        if (!unprocessedData.empty())
        {
            packet.insert(packet.begin(), unprocessedData.begin(), unprocessedData.end());
        }

        for (size_t i = 0; i < length; i++)
        {
            packet.push_back(data[i]);

            if (data[i] == 0xC0)
            {
                if (c0Found)
                {
                    // End of packet found

                    // If we have two 0xC0 after another we assume it is the beginning of a new
                    // packet, and not the end
                    if (packet.size() == 2)
                    {
                        packet.clear();
                        packet.push_back(0xc0);
                        continue;
                    }

                    processPacket(packet);

                    packet.clear();
                    unprocessedData.clear();
                    c0Found = false;
                }
                else
                {
                    // Start of packet found
                    c0Found = true;

                    // Clear previous data from packet since data before the start of packet is
                    // irrelevant.
                    packet.clear();
                    packet.push_back(0xC0);
                }
            }
        }

        if (!packet.empty())
        {
            unprocessedData.clear();
            unprocessedData.insert(unprocessedData.begin(), packet.begin(), packet.end());
        }
    }
    catch (const std::exception &e)
    {
        try
        {
            std::stringstream ss;
            ss << "Error processing incoming packet, " << e.what();
            log(SD_RPC_LOG_ERROR, ss.str());
        }
        catch (const std::exception &)
        {
            log(SD_RPC_LOG_ERROR, "Error creating error message for incoming packet error");
        }
    }
}

void H5Transport::incrementSeqNum()
{
    std::unique_lock<std::recursive_mutex> lck(seqNumMutex);
    seqNum++;
    seqNum = seqNum & 0x07;
}

void H5Transport::incrementAckNum()
{
    std::unique_lock<std::recursive_mutex> lck(ackNumMutex);
    ackNum++;
    ackNum = ackNum & 0x07;
}

#pragma endregion Processing of incoming packets from UART

#pragma region State machine

h5_state_t H5Transport::stateActionStart()
{
    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
    auto exit = dynamic_cast<StartExitCriterias *>(exitCriterias[STATE_START].get());

    stateMachineReady = true;

    // Notify other threads that the state machine is ready
    stateMachineLock.unlock();
    stateMachineChange.notify_all();
    stateMachineLock.lock();

    // Wait for notification of a stateMachineChange that exits the state
    stateMachineChange.wait(stateMachineLock, [&exit] { return exit->isFullfilled(); });

    // Order is of importance when returning state
    if (exit->ioResourceError)
    {
        return STATE_FAILED;
    }

    if (exit->close)
    {
        return STATE_CLOSED;
    }

    if (exit->isOpened)
    {
        return STATE_RESET;
    }

    return STATE_FAILED;
}

h5_state_t H5Transport::stateActionReset()
{
    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
    auto exit = dynamic_cast<ResetExitCriterias *>(exitCriterias[STATE_RESET].get());

    // Send the reset packet, and wait for the device to reboot and ready for receiving commands
    sendControlPacket(CONTROL_PKT_RESET);

    if (statusCallback)
    {
        statusCallback(RESET_PERFORMED, "Target Reset performed");
    }

    exit->resetSent = true;
    stateMachineChange.wait_for(stateMachineLock, RESET_WAIT_DURATION,
                                [&exit] { return exit->isFullfilled(); });
    exit->resetWait = true;

    // Order is of importance when returning state
    if (exit->ioResourceError)
    {
        return STATE_FAILED;
    }

    if (exit->close)
    {
        return STATE_CLOSED;
    }

    if (exit->resetSent && exit->resetWait)
    {
        return STATE_UNINITIALIZED;
    }

    return STATE_FAILED;
};

h5_state_t H5Transport::stateActionUninitialized()
{
    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
    auto exit =
        dynamic_cast<UninitializedExitCriterias *>(exitCriterias[STATE_UNINITIALIZED].get());
    auto syncRetransmission = PACKET_RETRANSMISSIONS;

    while (!exit->isFullfilled() && syncRetransmission > 0)
    {
        sendControlPacket(CONTROL_PKT_SYNC);
        exit->syncSent = true;
        stateMachineChange.wait_for(stateMachineLock, retransmissionInterval,
                                    [&exit] { return exit->isFullfilled(); });
        syncRetransmission--;
    }

    // Order is of importance when returning state
    if (exit->ioResourceError)
    {
        return STATE_FAILED;
    }

    if (exit->close)
    {
        return STATE_CLOSED;
    }

    if (exit->syncSent && exit->syncRspReceived)
    {
        return STATE_INITIALIZED;
    }

    if (syncRetransmission == 0)
    {
        std::stringstream status;
        status << "No response from device. Tried to send packet "
               << std::to_string(PACKET_RETRANSMISSIONS) << " times.";
        statusHandler(PKT_SEND_MAX_RETRIES_REACHED, status.str());
        return STATE_NO_RESPONSE;
    }

    return STATE_FAILED;
};

h5_state_t H5Transport::stateActionInitialized()
{
    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
    auto exit = dynamic_cast<InitializedExitCriterias *>(exitCriterias[STATE_INITIALIZED].get());
    auto syncRetransmission = PACKET_RETRANSMISSIONS;

    // Send a package immediately
    while (!exit->isFullfilled() && syncRetransmission > 0)
    {
        sendControlPacket(CONTROL_PKT_SYNC_CONFIG);
        exit->syncConfigSent = true;

        stateMachineChange.wait_for(stateMachineLock, retransmissionInterval,
                                    [&exit] { return exit->isFullfilled(); });

        syncRetransmission--;
    }

    // Order is of importance when returning state
    if (exit->ioResourceError)
    {
        return STATE_FAILED;
    }

    if (exit->close)
    {
        return STATE_CLOSED;
    }

    if (exit->syncConfigSent && exit->syncConfigRspReceived)
    {
        return STATE_ACTIVE;
    }

    if (syncRetransmission == 0)
    {
        std::stringstream status;
        status << "No response from device. Tried to send packet "
               << std::to_string(PACKET_RETRANSMISSIONS) << " times.";
        statusHandler(PKT_SEND_MAX_RETRIES_REACHED, status.str());
        return STATE_NO_RESPONSE;
    }

    return STATE_FAILED;
};
h5_state_t H5Transport::stateActionActive()
{
    std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
    auto exit = dynamic_cast<ActiveExitCriterias *>(exitCriterias[STATE_ACTIVE].get());

    {
        std::unique_lock<std::recursive_mutex> seqNumLck(seqNumMutex);
        std::unique_lock<std::recursive_mutex> ackNumLck(ackNumMutex);

        seqNum = 0;
        ackNum = 0;
    }

    statusHandler(CONNECTION_ACTIVE, "Connection active");
    stateMachineChange.wait(stateMachineLock, [&exit] { return exit->isFullfilled(); }); // T#2

    if (exit->ioResourceError)
    {
        return STATE_FAILED;
    }

    if (exit->close)
    {
        return STATE_CLOSED;
    }

    if (exit->syncReceived || exit->irrecoverableSyncError)
    {
        return STATE_RESET;
    }

    return STATE_FAILED;
};

h5_state_t H5Transport::stateActionFailed()
{
    std::lock_guard<std::mutex> stateMachineLock(stateMachineMutex);
    log(SD_RPC_LOG_FATAL, "Entered state failed. No exit exists from this state.");
    return STATE_FAILED;
};

h5_state_t H5Transport::stateActionClosed()
{
    std::lock_guard<std::mutex> stateMachineLock(stateMachineMutex);
    log(SD_RPC_LOG_DEBUG, "Entered state closed.");
    return STATE_CLOSED;
};
h5_state_t H5Transport::stateActionNoResponse()
{
    std::lock_guard<std::mutex> stateMachineLock(stateMachineMutex);
    log(SD_RPC_LOG_DEBUG, "No response to data sent to device.");
    return STATE_NO_RESPONSE;
};

void H5Transport::setupStateMachine()
{
    // Setup state actions
    //
    // The reason for using lambdas calling data member functions is to be able to easier debug when
    // compiling the project as a release build Doing it this way allows us to see the symbol name
    // when debugging
    stateActions[STATE_START]         = [this] { return stateActionStart(); };
    stateActions[STATE_RESET]         = [this] { return stateActionReset(); };
    stateActions[STATE_UNINITIALIZED] = [this] { return stateActionUninitialized(); };
    stateActions[STATE_INITIALIZED]   = [this] { return stateActionInitialized(); };
    stateActions[STATE_ACTIVE]        = [this] { return stateActionActive(); };
    stateActions[STATE_FAILED]        = [this] { return stateActionFailed(); };
    stateActions[STATE_CLOSED]        = [this] { return stateActionClosed(); };
    stateActions[STATE_NO_RESPONSE]   = [this] { return stateActionNoResponse(); };

    // Setup exit criteria
    exitCriterias[STATE_START] = std::shared_ptr<ExitCriterias>(new StartExitCriterias());
    exitCriterias[STATE_RESET] = std::shared_ptr<ExitCriterias>(new ResetExitCriterias());
    exitCriterias[STATE_UNINITIALIZED] =
        std::shared_ptr<ExitCriterias>(new UninitializedExitCriterias());
    exitCriterias[STATE_INITIALIZED] =
        std::shared_ptr<ExitCriterias>(new InitializedExitCriterias());
    exitCriterias[STATE_ACTIVE] = std::shared_ptr<ExitCriterias>(new ActiveExitCriterias());
}

void H5Transport::startStateMachine()
{
    if (!stateMachineThread.joinable())
    {
        currentState = STATE_START;

        // Lock the stateMachineMutex and let stateMachineThread notify
        // when the state machine is ready to process states
        std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
        stateMachineThread = std::thread([this] { stateMachineWorker(); });

        // Wait for the state machine to be ready
        stateMachineChange.wait(stateMachineLock, [this] { return stateMachineReady; });
    }
    else
    {
        // Terminate if the state machine thread is joinable (already runs), this should not happen.
        std::cerr << __FILE__ << ":" << __LINE__
                  << " stateMachineThread exists, this should not happen. Terminating."
                  << std::endl;
        std::terminate();
    }
}

void H5Transport::stopStateMachine()
{
    if (stateMachineThread.joinable())
    {
        stateMachineThread.join();
    }
}

// State machine thread
void H5Transport::stateMachineWorker() noexcept
{
    try
    {
        auto doRun = true;

        while (doRun)
        {
            // Run next state actions
            const auto nextState = stateActions[currentState]();

            // After returning from current state action, lock the current state
            std::unique_lock<std::mutex> currentStateLck(currentStateMutex);

            // Make sure that state is not changed when assigning a new current state
            {
                std::unique_lock<std::mutex> stateMachineLock(stateMachineMutex);
                logStateTransition(currentState, nextState);

                // Reset the next states variables before starting to use them.
                switch (nextState)
                {
                    case STATE_START:
                        dynamic_cast<StartExitCriterias *>(exitCriterias[STATE_START].get())
                            ->reset();
                        break;
                    case STATE_RESET:
                        dynamic_cast<ResetExitCriterias *>(exitCriterias[STATE_RESET].get())
                            ->reset();
                        break;
                    case STATE_UNINITIALIZED:
                        dynamic_cast<UninitializedExitCriterias *>(
                            exitCriterias[STATE_UNINITIALIZED].get())
                            ->reset();
                        break;
                    case STATE_INITIALIZED:
                        dynamic_cast<InitializedExitCriterias *>(
                            exitCriterias[STATE_INITIALIZED].get())
                            ->reset();
                        break;
                    case STATE_ACTIVE:
                        dynamic_cast<ActiveExitCriterias *>(exitCriterias[STATE_ACTIVE].get())
                            ->reset();
                        break;
                    case STATE_FAILED:
                    case STATE_CLOSED:
                    case STATE_NO_RESPONSE:
                        // These are terminal states that do not have exit criteria associated with
                        // them
                        break;
                    case STATE_UNKNOWN:
                        // Not used
                        break;
                }

                currentState = nextState;

                // Check if current state give any reason to continue running this thread
                if (currentState == STATE_FAILED || currentState == STATE_CLOSED ||
                    currentState == STATE_NO_RESPONSE)
                {
                    doRun = false;
                }

                // Inform interested parties that new current state is set and ready
                currentStateChange.notify_all();
            }
        }

        stateMachineReady = false;
    }
    catch (const std::exception &e)
    {
        log(SD_RPC_LOG_FATAL, "Error in state machine thread", e);
    }
}

bool H5Transport::waitForState(h5_state_t state, std::chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(currentStateMutex);
    return currentStateChange.wait_for(lock, timeout,
                                       [&state, this] { return currentState == state; });
}

#pragma endregion State machine related methods

#pragma region Sending packet types

void H5Transport::sendControlPacket(const control_pkt_type type, const uint8_t ackNum)
{
    h5_pkt_type_t h5_packet;

    if (ackNum == 0xff && type == CONTROL_PKT_ACK)
    {
        throw std::invalid_argument("Argument ackNum must be set for CONTROL_PKT_ACK");
    }

    switch (type)
    {
        case CONTROL_PKT_RESET:
            h5_packet = RESET_PACKET;
            break;
        case CONTROL_PKT_SYNC:
        case CONTROL_PKT_SYNC_RESPONSE:
        case CONTROL_PKT_SYNC_CONFIG:
        case CONTROL_PKT_SYNC_CONFIG_RESPONSE:
            h5_packet = LINK_CONTROL_PACKET;
            break;
        case CONTROL_PKT_ACK:
            h5_packet = ACK_PACKET;
            break;
        default:
            h5_packet = LINK_CONTROL_PACKET;
    }

    payload_t h5Packet;

    try
    {
        const auto payload = getPktPattern(type);
        h5_encode(payload, h5Packet, 0, type == CONTROL_PKT_ACK ? ackNum : 0, false, false,
                  h5_packet);
    }
    catch (const std::out_of_range &e)
    {
        std::stringstream logLine;
        log(SD_RPC_LOG_FATAL, "Trying to send unknown control packet to device, aborting", e);
        std::terminate();
    }

    payload_t slipPacket;
    slip_encode(h5Packet, slipPacket);

    logPacket(true, h5Packet);

    nextTransportLayer->send(slipPacket);
}

#pragma endregion Methods related to sending packet types defined in the Three Wire Standard

#pragma region Debugging
std::string H5Transport::stateToString(const h5_state_t state) noexcept
{
    switch (state)
    {
        case STATE_START:
            return "STATE_START";
        case STATE_RESET:
            return "STATE_RESET";
        case STATE_UNINITIALIZED:
            return "STATE_UNINITIALIZED";
        case STATE_INITIALIZED:
            return "STATE_INITIALIZED";
        case STATE_ACTIVE:
            return "STATE_ACTIVE";
        case STATE_FAILED:
            return "STATE_FAILED";
        case STATE_CLOSED:
            return "STATE_CLOSED";
        case STATE_NO_RESPONSE:
            return "STATE_NO_RESPONSE";
        case STATE_UNKNOWN:
            return "STATE_UNKNOWN";
        default:
            try
            {
                std::stringstream ss;
                ss << "UNKNOWN[0x" << std::hex << static_cast<uint32_t>(state) << "]";
                return ss.str();
            }
            catch (const std::exception &)
            {
                return "ERROR creating string describing state";
            }
    }
}

std::string H5Transport::pktTypeToString(const h5_pkt_type_t pktType)
{
    switch (pktType)
    {
        case ACK_PACKET:
            return "ACK";
        case HCI_COMMAND_PACKET:
            return "HCI_COMMAND_PACKET";
        case ACL_DATA_PACKET:
            return "ACL_DATA_PACKET";
        case SYNC_DATA_PACKET:
            return "SYNC_DATA_PACKET";
        case HCI_EVENT_PACKET:
            return "HCI_EVENT_PACKET";
        case RESET_PACKET:
            return "RESERVED_5";
        case VENDOR_SPECIFIC_PACKET:
            return "VENDOR_SPECIFIC";
        case LINK_CONTROL_PACKET:
            return "LINK_CONTROL_PACKET";
        default:
            std::stringstream ss;
            ss << "UNKNOWN[0x" << std::hex << static_cast<uint32_t>(pktType) << "]";
            return ss.str();
    }
}

std::string H5Transport::asHex(const payload_t &packet)
{
    std::stringstream hex;

    if (packet.empty())
    {
        return "N/A";
    }

    for_each(packet.begin(), packet.end(), [&](uint8_t byte) {
        hex << std::setfill('0') << std::setw(2) << std::hex << +byte << " ";
    });

    return hex.str();
}

std::string H5Transport::hciPacketLinkControlToString(const payload_t &payload)
{
    std::stringstream retval;

    const auto configToString = [](uint8_t config) {
        std::stringstream info;
        info << " sliding-window-size:" << (config & 0x07);
        info << " out-of-frame:" << ((config & 0x08) ? "1" : "0");
        info << " data-integrity-check-type:" << ((config & 0x0f) ? "1" : "0");
        info << " version-number:" << ((config & 0x0e) >> 5) << " ";
        return info.str();
    };

    if (payload.size() >= 2)
    {
        retval << "[";

        if (isSyncPacket(payload))
        {
            retval << "SYNC";
        }
        else if (isSyncResponsePacket(payload))
        {
            retval << "SYNC_RESP";
        }
        else if (isSyncConfigPacket(payload))
        {
            retval << "CONFIG [" << configToString(payload[2]) << "]";
        }
        else if (isSyncConfigResponsePacket(payload))
        {
            retval << "CONFIG_RESP [" << configToString(payload[2]) << "]";
        }
        else
        {
            if (payload[0] == 0x05 && payload[1] == 0xfa)
            {
                retval << "WAKEUP";
            }

            if (payload[0] == 0x06 && payload[1] == 0xf9)
            {
                retval << "WOKEN";
            }

            if (payload[0] == 0x07 && payload[1] == 0x78)
            {
                retval << "SLEEP";
            }
        }

        retval << "]";
    }

    return retval.str();
}

std::string H5Transport::h5PktToString(const bool out, const payload_t &h5Packet) const
{
    payload_t payload;

    uint8_t seq_num;
    uint8_t decodedAckNum;
    bool decodedReliablePacket;
    h5_pkt_type_t decodedPacketType;
    bool data_integrity;
    uint16_t payload_length;
    uint8_t header_checksum;

    const auto err_code =
        h5_decode(h5Packet, payload, &seq_num, &decodedAckNum, &data_integrity, &payload_length,
                  &header_checksum, &decodedReliablePacket, &decodedPacketType);

    std::stringstream count;

    if (out)
    {
        count << std::setw(8) << outgoingPacketCount << " -> ";
    }
    else
    {
        count << std::setw(5) << incomingPacketCount << "/" << std::setw(2) << errorPacketCount
              << " <- ";
    }

    std::stringstream retval;
    retval << count.str() << " [" << asHex(payload) << "] "
           << "type:" << std::setw(20) << pktTypeToString(decodedPacketType)
           << " reliable:" << std::setw(3) << (decodedReliablePacket ? "yes" : "no")
           << " seq#:" << std::hex << +seq_num << " ack#:" << std::hex << +decodedAckNum
           << " payload_length:" << +payload_length << " data_integrity:" << data_integrity;

    if (data_integrity)
    {
        retval << " header_checksum:" << std::hex << +header_checksum;
    }

    retval << " err_code:0x" << std::hex << err_code;

    if (decodedPacketType == LINK_CONTROL_PACKET)
    {
        retval << " " << hciPacketLinkControlToString(payload);
    }

    return retval.str();
}

void H5Transport::logPacket(const bool outgoing, const payload_t &packet)
{
    if (outgoing)
    {
        ++outgoingPacketCount;
    }
    else
    {
        ++incomingPacketCount;
    }

    const std::string logLine = h5PktToString(outgoing, packet);
    log(SD_RPC_LOG_DEBUG, logLine);
}

void H5Transport::logStateTransition(h5_state_t from, h5_state_t to) const
{
    std::stringstream logLine;
    logLine << "State change: " << stateToString(from) << " -> " << stateToString(to);

    log(SD_RPC_LOG_DEBUG, logLine.str());
}

#pragma endregion Debugging related methods

#pragma region Test related methods
bool H5Transport::checkPattern(const payload_t &packet, const uint8_t offset,
                               const payload_t &pattern)
{
    if (offset >= packet.size())
        return false;

    auto packetItr = packet.begin() + offset;

    for (auto patternEntry : pattern)
    {
        if (packetItr == packet.end())
        {
            return false;
        }

        if (*packetItr++ != patternEntry)
        {
            return false;
        }
    }

    return true;
}

payload_t H5Transport::getPktPattern(const control_pkt_type type)
{
    switch (type)
    {
        case CONTROL_PKT_RESET:
            return {};
        case CONTROL_PKT_ACK:
            return {};
        case CONTROL_PKT_SYNC:
            return {SyncFirstByte, SyncSecondByte};
        case CONTROL_PKT_SYNC_RESPONSE:
            return {SyncRspFirstByte, SyncRspSecondByte};
        case CONTROL_PKT_SYNC_CONFIG:
            return {SyncConfigFirstByte, SyncConfigSecondByte, SyncConfigField};
        case CONTROL_PKT_SYNC_CONFIG_RESPONSE:
            return {SyncConfigRspFirstByte, SyncConfigRspSecondByte, SyncConfigField};
        case CONTROL_PKT_LAST:
            return {};
        default:
            std::stringstream ss;
            ss << "unknown CONTROL packet type 0x" << std::hex << static_cast<uint32_t>(type);
            throw std::out_of_range(ss.str());
    }
}

bool H5Transport::isSyncPacket(const payload_t &packet, const uint8_t offset)
{
    return checkPattern(packet, offset, payload_t{SyncFirstByte, SyncSecondByte});
}

bool H5Transport::isSyncResponsePacket(const payload_t &packet, const uint8_t offset)
{
    return checkPattern(packet, offset, payload_t{SyncRspFirstByte, SyncRspSecondByte});
}

bool H5Transport::isSyncConfigPacket(const payload_t &packet, const uint8_t offset)
{
    return checkPattern(packet, offset, payload_t{SyncConfigFirstByte, SyncConfigSecondByte});
}

bool H5Transport::isSyncConfigResponsePacket(const payload_t &packet, const uint8_t offset)
{
    return checkPattern(packet, offset, payload_t{SyncConfigRspFirstByte, SyncConfigRspSecondByte});
}

bool H5Transport::isResetPacket(const payload_t &packet, const uint8_t offset)
{
    return checkPattern(packet, offset, payload_t{0x05});
}
#pragma endregion Test related methods
