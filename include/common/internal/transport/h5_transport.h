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

#ifndef H5_TRANSPORT_H
#define H5_TRANSPORT_H

#include "transport.h"

#include <mutex>
#include <condition_variable>

#include <vector>

#include <stdint.h>
#include <map>
#include <thread>
#include "h5.h"
#include "h5_transport_exit_criterias.h"

typedef enum
{
    STATE_START,
    STATE_RESET,
    STATE_UNINITIALIZED,
    STATE_INITIALIZED,
    STATE_ACTIVE,
    STATE_FAILED,
    STATE_CLOSED,
    STATE_NO_RESPONSE,
    STATE_UNKNOWN
} h5_state_t;

using state_action_t = std::function<h5_state_t()>;
using payload_t = std::vector<uint8_t>;

class H5Transport : public Transport {
public:
    H5Transport() = delete;
    H5Transport(Transport *nextTransportLayer, uint32_t retransmission_interval);
    ~H5Transport();
    
    uint32_t open(status_cb_t status_callback, data_cb_t data_callback, log_cb_t log_callback) override;
    uint32_t close() override;
    uint32_t send(const std::vector<uint8_t> &data) override;

    h5_state_t state() const;

    static bool isSyncPacket(const payload_t &packet, const uint8_t offset = 0);
    static bool isSyncResponsePacket(const payload_t &packet, const uint8_t offset = 0);
    static bool isSyncConfigPacket(const payload_t &packet, const uint8_t offset = 0);
    static bool isSyncConfigResponsePacket(const payload_t &packet, const uint8_t offset = 0);
    static bool isResetPacket(const payload_t &packet, const uint8_t offset = 0);
    static bool checkPattern(const payload_t  &packet, const uint8_t offset, const payload_t &pattern);

private:
    void dataHandler(uint8_t *data, size_t length);
    void statusHandler(sd_rpc_app_status_t code, const char * error);
    void processPacket(payload_t &packet);

    void sendControlPacket(control_pkt_type type);

    void incrementSeqNum();
    void incrementAckNum();

    Transport *nextTransportLayer;
    std::vector<uint8_t> lastPacket;

    // Callbacks used by lower transports
    // These callbacks invoke upper callbacks
    status_cb_t statusCallback;
    data_cb_t dataCallback;

    // Variables used for reliable packets
    uint8_t seqNum;
    uint8_t ackNum;

    bool c0Found;
    std::vector<uint8_t> unprocessedData;

    std::mutex stateMachineMutex; // Mutex controlling access to state machine variables
    std::condition_variable stateMachineChange; // Condition variable to communicate changes to state machine

    // Variables used in state ACTIVE
    std::chrono::milliseconds retransmissionInterval;
    std::mutex ackMutex;
    std::condition_variable ackWaitCondition;

    // Debugging related
    uint32_t incomingPacketCount;
    uint32_t outgoingPacketCount;
    uint32_t errorPacketCount;

    void logPacket(bool outgoing, payload_t &packet);
    void log(std::string &logLine) const;
    void log(char const *logLine) const;
    void logStateTransition(h5_state_t from, h5_state_t to) const;
    static std::string stateToString(h5_state_t state);
    std::string asHex(payload_t &packet) const;
    std::string hciPacketLinkControlToString(payload_t  payload) const;
    std::string h5PktToString(bool out, payload_t  &h5Packet) const;
    static std::string pktTypeToString(h5_pkt_type_t pktType);

    // State machine related
    h5_state_t currentState;
    std::thread stateMachineThread;

    bool stateMachineReady;

    std::map<h5_state_t, state_action_t> stateActions;
    void setupStateMachine();
    void startStateMachine();
    void stopStateMachine();

    std::map<h5_state_t, ExitCriterias*> exitCriterias;

    void stateMachineWorker();

    std::mutex stateMutex; // Mutex that allows threads to wait for a given state in the state machine
    bool waitForState(h5_state_t state, std::chrono::milliseconds timeout);
    std::condition_variable stateWaitCondition;

    static std::map<control_pkt_type, payload_t> pkt_pattern;
    static std::map<h5_state_t, std::string> stateString;
    static std::map<h5_pkt_type_t, std::string> pktTypeString;

    static const uint8_t SyncFirstByte = 0x01;
    static const uint8_t SyncSecondByte = 0x7E;
    static const uint8_t SyncRspFirstByte = 0x02;
    static const uint8_t SyncRspSecondByte = 0x7D;
    static const uint8_t SyncConfigFirstByte = 0x03;
    static const uint8_t SyncConfigSecondByte = 0xFC;
    static const uint8_t SyncConfigRspFirstByte = 0x04;
    static const uint8_t SyncConfigRspSecondByte = 0x7B;
    static const uint8_t SyncConfigField = 0x11;
};

#endif //H5_TRANSPORT_H
