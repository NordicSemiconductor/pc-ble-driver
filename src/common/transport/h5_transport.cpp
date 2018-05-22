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
#include "sd_rpc_types.h"
#include "nrf_error.h"

#include "h5.h"
#include "slip.h"

#include <stdint.h>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <thread>
#include <map>

#include <exception>

// Constants use for state machine states UNINITIALIZED and INITIALIZED
const auto NON_ACTIVE_STATE_TIMEOUT = std::chrono::milliseconds(250);  // Duration to wait until resending a packet
const uint8_t PACKET_RETRANSMISSIONS = 6;                              // Number of times to send reliable packets before giving in

// Other constants
const auto OPEN_WAIT_TIMEOUT = std::chrono::milliseconds(2000);   // Duration to wait for state ACTIVE after open is called
const auto RESET_WAIT_DURATION = std::chrono::milliseconds(300);  // Duration to wait before continuing UART communication after reset is sent to target

#pragma region Public methods
H5Transport::H5Transport(Transport *_nextTransportLayer, uint32_t retransmission_interval)
    : Transport(),
    seqNum(0), ackNum(0), c0Found(false),
    unprocessedData(), incomingPacketCount(0), outgoingPacketCount(0),
    errorPacketCount(0), currentState(STATE_START), stateMachineThread(nullptr)
{
    this->nextTransportLayer = _nextTransportLayer;
    retransmissionInterval = std::chrono::milliseconds(retransmission_interval);

    setupStateMachine();
}

H5Transport::~H5Transport()
{
    delete nextTransportLayer;
}

#pragma region Static initializers

std::map<h5_state_t, std::string> H5Transport::stateString{
    { STATE_UNKNOWN, "STATE_UNKNOWN" },
    { STATE_START, "STATE_START" },
    { STATE_UNINITIALIZED, "STATE_UNINITIALIZED" },
    { STATE_ACTIVE, "STATE_ACTIVE" },
    { STATE_FAILED, "STATE_FAILED" },
    { STATE_RESET, "STATE_RESET" },
    { STATE_INITIALIZED, "STATE_INITIALIZED" }
};

std::map<control_pkt_type, std::vector<uint8_t>> H5Transport::pkt_pattern = {
    { CONTROL_PKT_RESET, {} },
    { CONTROL_PKT_SYNC, { syncFirstByte, syncSecondByte } },
    { CONTROL_PKT_SYNC_RESPONSE, { syncRspFirstByte, syncRspSecondByte } },
    { CONTROL_PKT_SYNC_CONFIG, { syncConfigFirstByte, syncConfigSecondByte, syncConfigField } },
    { CONTROL_PKT_SYNC_CONFIG_RESPONSE, { syncConfigRspFirstByte, syncConfigRspSecondByte, syncConfigField } }
};

std::map<h5_pkt_type_t, std::string> H5Transport::pktTypeString{
    { ACK_PACKET, "ACK" },
    { HCI_COMMAND_PACKET, "HCI_COMMAND_PACKET" },
    { ACL_DATA_PACKET, "ACL_DATA_PACKET" },
    { SYNC_DATA_PACKET, "SYNC_DATA_PACKET" },
    { HCI_EVENT_PACKET, "HCI_EVENT_PACKET" },
    { RESET_PACKET, "RESERVED_5" },
    { VENDOR_SPECIFIC_PACKET, "VENDOR_SPECIFIC" },
    { LINK_CONTROL_PACKET, "LINK_CONTROL_PACKET" },
};

#pragma endregion

uint32_t H5Transport::open(status_cb_t status_callback, data_cb_t data_callback, log_cb_t log_callback)
{
    if (currentState != STATE_START)
    {
        log("Not able to open, current state is not valid");
        return NRF_ERROR_INTERNAL;
    }

    startStateMachine();
    auto _exitCriterias = dynamic_cast<StartExitCriterias*>(exitCriterias[STATE_START]);

    auto errorCode = Transport::open(status_callback, data_callback, log_callback);
    lastPacket.clear();

    if (errorCode != NRF_SUCCESS)
    {
        _exitCriterias->ioResourceError = true;
        syncWaitCondition.notify_all();
        return errorCode;
    }

    status_callback = std::bind(&H5Transport::statusHandler, this, std::placeholders::_1, std::placeholders::_2);
    data_callback = std::bind(&H5Transport::dataHandler, this, std::placeholders::_1, std::placeholders::_2);
    errorCode = nextTransportLayer->open(status_callback, data_callback, log_callback);

    if (errorCode != NRF_SUCCESS)
    {
        _exitCriterias->ioResourceError = true;
        syncWaitCondition.notify_all();
        return NRF_ERROR_INTERNAL;
    }

    _exitCriterias->isOpened = true;
    syncWaitCondition.notify_all();

    if (waitForState(STATE_ACTIVE, OPEN_WAIT_TIMEOUT))
    {
        return NRF_SUCCESS;
    }
    else
    {
        return NRF_ERROR_TIMEOUT;
    }
}

uint32_t H5Transport::close()
{
    auto exitCriteria = exitCriterias[currentState];
    
    if (exitCriteria != nullptr)
    {
        exitCriteria->close = true;
    }

    stopStateMachine();

    auto errorCode1 = nextTransportLayer->close();
    auto errorCode2 = Transport::close();

    if (errorCode1 != NRF_SUCCESS)
    {
        return errorCode1;
    }
    else
    {
        return errorCode2;
    }
}

uint32_t H5Transport::send(std::vector<uint8_t> &data)
{
    if (currentState != STATE_ACTIVE)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    // max theoretical length of encoded packet, aditional 6 bytes h5 encoding and all bytes escaped + 2 packet encapsuling
    std::vector<uint8_t> h5EncodedPacket;

    h5_encode(data,
              h5EncodedPacket,
              seqNum,
              ackNum,
              true,
              true,
              VENDOR_SPECIFIC_PACKET);

    std::vector<uint8_t> encodedPacket;
    slip_encode(h5EncodedPacket, encodedPacket);

    auto remainingRetransmissions = PACKET_RETRANSMISSIONS;

    lastPacket.clear();
    lastPacket = encodedPacket;

    std::unique_lock<std::mutex> ackGuard(ackMutex);

    while (remainingRetransmissions--)
    {
        logPacket(true, h5EncodedPacket);
        nextTransportLayer->send(lastPacket);

        const uint8_t seqNumBefore = seqNum;

        // Checking for timeout. Also checking against spurios wakeup by making sure the sequence
        // number has  actually increased. If the sequence number has not increased, we have not
        // received an ACK packet, and should not exit the loop (unless timeout).
        // Ref. spurious wakeup: 
        // http://en.cppreference.com/w/cpp/thread/condition_variable
        // https://en.wikipedia.org/wiki/Spurious_wakeup
        if (ackWaitCondition.wait_for(
            ackGuard,
            std::chrono::milliseconds(retransmissionInterval),
            [&] { return seqNum != seqNumBefore; }))
         {
             lastPacket.clear();
             return NRF_SUCCESS;
         }
    }

    lastPacket.clear();
    return NRF_ERROR_TIMEOUT;
}
#pragma endregion Public methods

#pragma region Processing incoming data from UART
void H5Transport::processPacket(std::vector<uint8_t> &packet)
{
    uint8_t seq_num;
    uint8_t ack_num;
    bool reliable_packet;
    h5_pkt_type_t packet_type;

    std::vector<uint8_t> slipPayload;
    auto err_code = slip_decode(packet, slipPayload);

    if (err_code != NRF_SUCCESS)
    {
        errorPacketCount++;
        return;
    }

    logPacket(false, slipPayload);

    std::vector<uint8_t> h5Payload;

    err_code = h5_decode(
        slipPayload, 
        h5Payload,
        &seq_num,
        &ack_num,
        nullptr,
        nullptr,
        nullptr,
        &reliable_packet,
        &packet_type);

    if (err_code != NRF_SUCCESS)
    {
        errorPacketCount++;
        return;
    }

    if (currentState == STATE_RESET)
    {
        // Ignore packets packets received in this state.
        syncWaitCondition.notify_all();
        return;
    }

    if (packet_type == LINK_CONTROL_PACKET)
    {
        auto isSyncPacket = h5Payload[0] == syncFirstByte && h5Payload[1] == syncSecondByte;
        auto isSyncResponsePacket = h5Payload[0] == syncRspFirstByte && h5Payload[1] == syncRspSecondByte;
        auto isSyncConfigPacket = h5Payload[0] == syncConfigFirstByte && h5Payload[1] == syncConfigSecondByte;
        auto isSyncConfigResponsePacket = h5Payload[0] == syncConfigRspFirstByte && h5Payload[1] == syncConfigRspSecondByte;

        if (currentState == STATE_UNINITIALIZED)
        {
            if (isSyncResponsePacket) {
                dynamic_cast<UninitializedExitCriterias*>(exitCriterias[currentState])->syncRspReceived = true;
                syncWaitCondition.notify_all();
            }

            if (isSyncPacket) {
                sendControlPacket(CONTROL_PKT_SYNC_RESPONSE);
            }
        }
        else if (currentState == STATE_INITIALIZED)
        {
            auto exit = dynamic_cast<InitializedExitCriterias*>(exitCriterias[currentState]);

            if (isSyncConfigResponsePacket) {
                exit->syncConfigRspReceived = true;
                syncWaitCondition.notify_all();
            }

            if (isSyncConfigPacket)
            {
                sendControlPacket(CONTROL_PKT_SYNC_CONFIG_RESPONSE);
                syncWaitCondition.notify_all();
            }

            if (isSyncPacket)
            {
                sendControlPacket(CONTROL_PKT_SYNC_RESPONSE);
                syncWaitCondition.notify_all();
            }
        }
        else if (currentState == STATE_ACTIVE)
        {
            auto exit = dynamic_cast<ActiveExitCriterias*>(exitCriterias[currentState]);

            if (isSyncPacket)
            {
                exit->syncReceived = true;
                syncWaitCondition.notify_all();
            }

            if (isSyncConfigPacket)
            {
                sendControlPacket(CONTROL_PKT_SYNC_CONFIG_RESPONSE);
            }
        }
    }
    else if (packet_type == VENDOR_SPECIFIC_PACKET)
    {
        if (currentState == STATE_ACTIVE)
        {
            if (reliable_packet)
            {
                if (seq_num == ackNum)
                {
                    incrementAckNum();
                    sendControlPacket(CONTROL_PKT_ACK);
                    dataCallback(h5Payload.data(), h5Payload.size());
                }
                else
                {
                    sendControlPacket(CONTROL_PKT_ACK);
                }
            }
        }
    }
    else if (packet_type == ACK_PACKET)
    {
        if (ack_num == ((seqNum + 1) & 0x07))
        {
            // Received a packet with valid ack_num, inform threads that wait the command is received on the other end
            std::lock_guard<std::mutex> ackGuard(ackMutex);
            incrementSeqNum();
            ackWaitCondition.notify_all();
        }
        else if (ack_num == seqNum)
        {
            // Discard packet, we assume that we have received a reply from a previous packet
        }
        else
        {
            dynamic_cast<ActiveExitCriterias*>(exitCriterias[currentState])->irrecoverableSyncError = true;
            syncWaitCondition.notify_all();
        }
    }
}

void H5Transport::statusHandler(sd_rpc_app_status_t code, const char * error)
{
    if (code == IO_RESOURCES_UNAVAILABLE)
    {
        auto exitCriteria = exitCriterias[currentState];

        if (exitCriteria != nullptr)
        {
            exitCriteria->ioResourceError = true;
        }
        
        syncWaitCondition.notify_all();
    }

    statusCallback(code, error);
}

void H5Transport::dataHandler(uint8_t *data, size_t length)
{
    std::vector<uint8_t> packet;

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

                // If we have two 0xC0 after another we assume it is the beginning of a new packet, and not the end
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

                // Clear previous data from packet since data before the start of packet is irrelevant.
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

void H5Transport::incrementSeqNum()
{
    seqNum++;
    seqNum = seqNum & 0x07;
}

void H5Transport::incrementAckNum()
{
    ackNum++;
    ackNum = ackNum & 0x07;
}

#pragma endregion Processing of incoming packets from UART

#pragma region  State machine
void H5Transport::setupStateMachine()
{
    stateActions[STATE_START] = [&]() -> h5_state_t {
        auto exit = dynamic_cast<StartExitCriterias*>(exitCriterias[STATE_START]);
        exit->reset();

        std::unique_lock<std::mutex> syncGuard(syncMutex);

        while (!exit->isFullfilled())
        {
            syncWaitCondition.wait(syncGuard);
        }

        if (exit->ioResourceError)
        {
            return STATE_FAILED;
        }
        else if (exit->isOpened)
        {
            return STATE_RESET;
        }
        else
        {
            return STATE_FAILED;
        }
    };

    stateActions[STATE_RESET] = [&]() -> h5_state_t {
        auto exit = dynamic_cast<ResetExitCriterias*>(exitCriterias[STATE_RESET]);
        exit->reset();

        std::unique_lock<std::mutex> syncGuard(syncMutex);

        // Send the reset packet, and wait for the device to reboot and ready for receiving commands
        sendControlPacket(CONTROL_PKT_RESET);
        statusCallback(RESET_PERFORMED, "Target Reset performed");
        exit->resetSent = true;
        syncWaitCondition.wait_for(syncGuard, RESET_WAIT_DURATION, [&] { return exit->isFullfilled(); });
        exit->resetWait = true;

        // Order is of importance
        if (exit->ioResourceError)
        {
            return STATE_FAILED;
        }
        else if (exit->close)
        {
            return STATE_FAILED;
        }
        else if (exit->resetSent && exit->resetWait)
        {
            return STATE_UNINITIALIZED;
        }
        else
        {
            return STATE_FAILED;
        }
    };

    stateActions[STATE_UNINITIALIZED] = [&]() -> h5_state_t
    {
        auto exit = dynamic_cast<UninitializedExitCriterias*>(exitCriterias[STATE_UNINITIALIZED]);
        exit->reset();

        uint8_t syncRetransmission = PACKET_RETRANSMISSIONS;
        std::unique_lock<std::mutex> syncGuard(syncMutex);

        while (!exit->isFullfilled() && syncRetransmission > 0)
        {
            sendControlPacket(CONTROL_PKT_SYNC);
            exit->syncSent = true;
            syncWaitCondition.wait_for(syncGuard, NON_ACTIVE_STATE_TIMEOUT, [&] { return exit->isFullfilled(); });
            syncRetransmission--;
        }

        if (exit->isFullfilled())
        {
            return STATE_INITIALIZED;
        }
        else
        {
            return STATE_FAILED;
        }
    };

    stateActions[STATE_INITIALIZED] = [&]() -> h5_state_t
    {
        auto exit = dynamic_cast<InitializedExitCriterias*>(exitCriterias[STATE_INITIALIZED]);
        exit->reset();

        uint8_t syncRetransmission = PACKET_RETRANSMISSIONS;
        std::unique_lock<std::mutex> syncGuard(syncMutex);

        // Send a package immediately
        while (!exit->isFullfilled() && syncRetransmission > 0)
        {
            sendControlPacket(CONTROL_PKT_SYNC_CONFIG);
            exit->syncConfigSent = true;

            syncWaitCondition.wait_for(
                syncGuard,
                NON_ACTIVE_STATE_TIMEOUT,
                [&]{ return exit->isFullfilled(); }
            );

            syncRetransmission--;
        }

        if (exit->syncConfigSent && exit->syncConfigRspReceived)
        {
            return STATE_ACTIVE;
        }
        else
        {
            return STATE_FAILED;
        }
    };

    stateActions[STATE_ACTIVE] = [&]() -> h5_state_t
    {
        seqNum = 0;
        ackNum = 0;

        std::unique_lock<std::mutex> syncGuard(syncMutex);
        auto exit = dynamic_cast<ActiveExitCriterias*>(exitCriterias[STATE_ACTIVE]);
        exit->reset();

        statusHandler(CONNECTION_ACTIVE, "Connection active");

        while (!exit->isFullfilled())
        {
            syncWaitCondition.wait(syncGuard);
        }

        if (exit->syncReceived || exit->irrecoverableSyncError)
        {
            return STATE_RESET;
        }
        else if (exit->close)
        {
            return STATE_START;
        }
        else if (exit->ioResourceError)
        {
            return STATE_FAILED;
        }
        else
        {
            return STATE_FAILED;
        }
    };

    stateActions[STATE_FAILED] = [&]() -> h5_state_t
    {
        log("Giving up! I can not provide you a way of your failed state!");
        return STATE_FAILED;
    };

    // Setup exit criterias
    exitCriterias[STATE_START] = new StartExitCriterias();
    exitCriterias[STATE_RESET] = new ResetExitCriterias();
    exitCriterias[STATE_UNINITIALIZED] = new UninitializedExitCriterias();
    exitCriterias[STATE_INITIALIZED] = new InitializedExitCriterias();
    exitCriterias[STATE_ACTIVE] = new ActiveExitCriterias();
}

void H5Transport::startStateMachine()
{
    runStateMachine = true;
    currentState = STATE_START;

    if (stateMachineThread == nullptr)
    {
        stateMachineThread = new std::thread(std::bind(&H5Transport::stateMachineWorker, this));
    }
    else
    {
        // Terminate if the state machine already exists, this should not happen.
        std::cerr << __FILE__ << ":" << __LINE__ << " stateMachineThread exists, this should not happen. Terminating." << std::endl;
        std::terminate();
    }
}

void H5Transport::stopStateMachine()
{
    runStateMachine = false;
    syncWaitCondition.notify_all(); // Notify state machine thread

    if (stateMachineThread != nullptr)
    {
        // Check if stateMachineThread is stopping itself
        if (std::this_thread::get_id() == stateMachineThread->get_id())
        {
            stateMachineThread = nullptr;
            return;
        }

        stateMachineThread->join();
        delete stateMachineThread;
        stateMachineThread = nullptr;
    }
}

// Event Thread
void H5Transport::stateMachineWorker()
{
    h5_state_t nextState;

    while (currentState != STATE_FAILED && runStateMachine == true)
    {
        nextState = stateActions[currentState]();
        logStateTransition(currentState, nextState);

        currentState = nextState;

        // Inform interested parties that new state is about to be entered
        stateWaitCondition.notify_all();
    }
}

bool H5Transport::waitForState(h5_state_t state, std::chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(stateMutex);
    return stateWaitCondition.wait_for(lock, timeout, [&] { return currentState == state; });
}

#pragma endregion State machine related methods

#pragma region Sending packet types

void H5Transport::sendControlPacket(control_pkt_type type)
{
    h5_pkt_type_t h5_packet;

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

    auto payload = pkt_pattern[type];
    std::vector<uint8_t> h5Packet;

    h5_encode(payload,
        h5Packet,
        0,
        type == CONTROL_PKT_ACK ? ackNum : 0,
        false,
        false,
        h5_packet);

    std::vector<uint8_t> slipPacket;
    slip_encode(h5Packet, slipPacket);

    logPacket(true, h5Packet);

    nextTransportLayer->send(slipPacket);
}

#pragma endregion Methods related to sending packet types defined in the Three Wire Standard

#pragma region Debugging
std::string H5Transport::stateToString(h5_state_t state)
{
    return stateString[state];
}

std::string H5Transport::pktTypeToString(h5_pkt_type_t pktType)
{
    return pktTypeString[pktType];
}

std::string H5Transport::asHex(std::vector<uint8_t> &packet) const
{
    std::stringstream hex;

    for_each(packet.begin(), packet.end(), [&](uint8_t byte){
        hex << std::setfill('0') << std::setw(2) << std::hex << +byte << " ";
    });

    return hex.str();
}

std::string H5Transport::hciPacketLinkControlToString(std::vector<uint8_t> payload) const
{
    std::stringstream retval;

    auto configToString = [](uint8_t config)
    {
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

        if (payload[0] == syncFirstByte && payload[1] == syncSecondByte) retval << "SYNC";
        if (payload[0] == syncRspFirstByte && payload[1] == syncRspSecondByte) retval << "SYNC_RESP";

        if (payload[0] == syncConfigFirstByte && payload[1] == syncConfigSecondByte && payload.size() == 3)
        {
            retval << "CONFIG [" << configToString(payload[2]) << "]";
        }
        if (payload[0] == syncConfigRspFirstByte && payload[1] == syncConfigRspSecondByte)
        {
            retval << "CONFIG_RESP [" << configToString(payload[2]) << "]";
        }

        if (payload[0] == 0x05 && payload[1] == 0xfa) retval << "WAKEUP";
        if (payload[0] == 0x06 && payload[1] == 0xf9) retval << "WOKEN";
        if (payload[0] == 0x07 && payload[1] == 0x78) retval << "SLEEP";

        retval << "]";
    }

    return retval.str();
}

std::string H5Transport::h5PktToString(bool out, std::vector<uint8_t> &h5Packet) const
{
    std::vector<uint8_t> payload;

    uint8_t seq_num;
    uint8_t ack_num;
    bool reliable_packet;
    h5_pkt_type_t packet_type;
    bool data_integrity;
    uint16_t payload_length;
    uint8_t header_checksum;

    auto err_code = h5_decode(
        h5Packet, 
        payload,
        &seq_num,
        &ack_num,
        &data_integrity,
        &payload_length,
        &header_checksum,
        &reliable_packet,
        &packet_type);

    std::stringstream count;

    if (out)
    {
        count << std::setw(8) << outgoingPacketCount << " -> ";
    }
    else
    {
        count << std::setw(5) << incomingPacketCount << "/" << std::setw(2) << errorPacketCount << " <- ";
    }

    std::stringstream retval;
    retval
        << count.str()
        << " [" << asHex(payload) << "]" << std::endl
        << std::setw(20) << "type:" << std::setw(20) << pktTypeToString(packet_type)
        << " reliable:" << std::setw(3) << (reliable_packet ? "yes" : "no")
        << " seq#:" << std::hex << +seq_num << " ack#:" << std::hex << +ack_num
        << " payload_length:" << payload_length
        << " data_integrity:" << data_integrity;

    if (data_integrity)
    {
        retval << " header_checksum:" << std::hex << +header_checksum;
    }

    retval << " err_code:" << err_code;

    if (packet_type == LINK_CONTROL_PACKET)
    {
        retval << std::endl << std::setw(15) << "" << hciPacketLinkControlToString(payload);
    }

    return retval.str();
}

void H5Transport::logPacket(bool outgoing, std::vector<uint8_t> &packet)
{
    if (outgoing)
    {
        outgoingPacketCount++;
    }
    else
    {
        incomingPacketCount++;
    }

    std::string logLine = h5PktToString(outgoing, packet).c_str();

    if (this->logCallback != nullptr)
    {
        this->logCallback(SD_RPC_LOG_DEBUG, logLine);
    }
    else
    {
        std::clog << logLine << std::endl;
    }
}

void H5Transport::log(std::string &logLine) const
{
    if (this->logCallback != nullptr)
    {
        this->logCallback(SD_RPC_LOG_DEBUG, logLine);
    }
    else
    {
        std::clog << logLine << std::endl;
    }
}

void H5Transport::log(char const *logLine) const
{
    auto _logLine = std::string(logLine);
    log(_logLine);
}

void H5Transport::logStateTransition(h5_state_t from, h5_state_t to) const
{
    std::stringstream logLine;
    logLine << "State change: " << stateToString(from) << " -> " << stateToString(to) << std::endl;

    if (this->logCallback != nullptr)
    {
        this->logCallback(SD_RPC_LOG_DEBUG, logLine.str());
    }
    else
    {
        std::clog << logLine.str() << std::endl;
    }
}

#pragma endregion Debugging related methods
