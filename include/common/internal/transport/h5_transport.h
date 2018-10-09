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

#include <condition_variable>
#include <mutex>

#include <vector>

#include "h5.h"
#include "h5_transport_exit_criterias.h"
#include <map>
#include <stdint.h>
#include <thread>

typedef enum {
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

constexpr uint8_t SyncFirstByte           = 0x01;
constexpr uint8_t SyncSecondByte          = 0x7E;
constexpr uint8_t SyncRspFirstByte        = 0x02;
constexpr uint8_t SyncRspSecondByte       = 0x7D;
constexpr uint8_t SyncConfigFirstByte     = 0x03;
constexpr uint8_t SyncConfigSecondByte    = 0xFC;
constexpr uint8_t SyncConfigRspFirstByte  = 0x04;
constexpr uint8_t SyncConfigRspSecondByte = 0x7B;
constexpr uint8_t SyncConfigField         = 0x11;

using state_action_t = std::function<h5_state_t()>;
using payload_t      = std::vector<uint8_t>;

class H5Transport : public Transport
{
  public:
    H5Transport() = delete;
    H5Transport(Transport *nextTransportLayer, const uint32_t retransmission_interval);
    ~H5Transport() noexcept;

    uint32_t open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                  const log_cb_t &log_callback) override;
    uint32_t close() override;
    uint32_t send(const std::vector<uint8_t> &data) override;

    h5_state_t state() const;

    static bool isSyncPacket(const payload_t &packet, const uint8_t offset = 0);
    static bool isSyncResponsePacket(const payload_t &packet, const uint8_t offset = 0);
    static bool isSyncConfigPacket(const payload_t &packet, const uint8_t offset = 0);
    static bool isSyncConfigResponsePacket(const payload_t &packet, const uint8_t offset = 0);
    static bool isResetPacket(const payload_t &packet, const uint8_t offset = 0);
    static bool checkPattern(const payload_t &packet, const uint8_t offset,
                             const payload_t &pattern);
    static payload_t getPktPattern(const control_pkt_type);
    static std::string stateToString(const h5_state_t state);
    static std::string pktTypeToString(const h5_pkt_type_t pktType);

  private:
    void dataHandler(const uint8_t *data, const size_t length);
    void statusHandler(const sd_rpc_app_status_t code, const std::string &error);
    void processPacket(const payload_t &packet);

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
    std::condition_variable
        stateMachineChange; // Condition variable to communicate changes to state machine

    // Variables used in state ACTIVE
    std::chrono::milliseconds retransmissionInterval;
    std::mutex ackMutex;
    std::condition_variable ackWaitCondition;

    // Debugging related
    uint32_t incomingPacketCount;
    uint32_t outgoingPacketCount;
    uint32_t errorPacketCount;

    void logPacket(const bool outgoing, const payload_t &packet);
    void logStateTransition(const h5_state_t from, const h5_state_t to) const;
    static std::string asHex(const payload_t &packet);
    static std::string hciPacketLinkControlToString(const payload_t &payload);
    std::string h5PktToString(const bool out, const payload_t &h5Packet) const;

    // State machine related
    h5_state_t currentState;
    std::thread stateMachineThread;

    bool stateMachineReady;

    std::map<h5_state_t, state_action_t> stateActions;
    void setupStateMachine();
    void startStateMachine();
    void stopStateMachine();

    std::map<h5_state_t, std::shared_ptr<ExitCriterias>> exitCriterias;

    void stateMachineWorker();

    // Mutex that allows threads to wait for a given state in the state machine
    std::mutex stateMutex;
    bool waitForState(h5_state_t state, std::chrono::milliseconds timeout);
    std::condition_variable stateWaitCondition;

    bool isOpen;
    std::mutex publicMethodMutex;
};

#endif // H5_TRANSPORT_H
