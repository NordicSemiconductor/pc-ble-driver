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

typedef enum
{
    STATE_START,
    STATE_RESET,
    STATE_UNINITIALIZED,
    STATE_INITIALIZED,
    STATE_ACTIVE,
    STATE_FAILED,
    STATE_UNKNOWN
} h5_state_t;

typedef std::function<h5_state_t()> state_action_t;

class ExitCriterias
{
public:
    ExitCriterias() : ioResourceError(false), close(false) {}
    virtual ~ExitCriterias() {}

    bool ioResourceError;
    bool close;

    virtual bool isFullfilled() const = 0;

    virtual void reset()
    {
        ioResourceError = false; close = false;
    }
};

class StartExitCriterias : public ExitCriterias
{
public:
    bool isOpened;

    StartExitCriterias()
        : ExitCriterias(),
        isOpened(false) {}

    bool isFullfilled() const override
    {
        return (isOpened || ioResourceError);
    }

    void reset() override
    {
        ExitCriterias::reset();
        isOpened = false;
    }
};

class UninitializedExitCriterias : public ExitCriterias
{
public:
    bool syncSent;
    bool syncRspReceived;

    UninitializedExitCriterias()
        : ExitCriterias(),
        syncSent(false),
        syncRspReceived(false) {}

    bool isFullfilled() const override
    {
        return (syncSent && syncRspReceived) || ioResourceError || close;
    }

    void reset() override
    {
        ExitCriterias::reset();
        syncSent = false;
        syncRspReceived = false;
    }
};

class InitializedExitCriterias : public ExitCriterias
{
public:
    bool syncConfigSent;
    bool syncConfigRspReceived;

    InitializedExitCriterias()
        : ExitCriterias(),
        syncConfigSent(false),
        syncConfigRspReceived(false) {}

    bool isFullfilled() const override
    {
        return ioResourceError || close || (syncConfigSent && syncConfigRspReceived);
    }

    void reset() override
    {
        ExitCriterias::reset();
        syncConfigSent = false;
        syncConfigRspReceived = false;
    };

};

class ActiveExitCriterias : public ExitCriterias
{
public:
    bool irrecoverableSyncError;
    bool syncReceived;

    ActiveExitCriterias()
        : ExitCriterias(), irrecoverableSyncError(false),
        syncReceived(false) {}

    bool isFullfilled() const override {
        return ioResourceError || syncReceived || close || irrecoverableSyncError;
    }

    void reset() override
    {
        ExitCriterias::reset();
        irrecoverableSyncError = false;
        syncReceived = false;
        close = false;
    }
};

class ResetExitCriterias : public ExitCriterias
{
public:
    bool resetSent;
    bool resetWait;

    ResetExitCriterias()
        : ExitCriterias(), resetSent(false), resetWait(false)
    {}

    bool isFullfilled() const override
    {
        return ioResourceError || close || (resetSent && resetWait);
    }

    void reset() override
    {
        ExitCriterias::reset();
        resetSent = false;
        resetWait = false;
    }
};

class H5Transport : public Transport {
public:
    H5Transport(Transport *nextTransportLayer, uint32_t retransmission_interval);
    ~H5Transport();
    uint32_t open(status_cb_t status_callback, data_cb_t data_callback, log_cb_t log_callback) override;
    uint32_t close() override;
    uint32_t send(std::vector<uint8_t> &data) override;

private:
    void dataHandler(uint8_t *data, size_t length);
    void statusHandler(sd_rpc_app_status_t code, const char * error);
    void processPacket(std::vector<uint8_t>& packet);

    void sendControlPacket(control_pkt_type type);

    void incrementSeqNum();
    void incrementAckNum();

    Transport *nextTransportLayer;
    std::vector<uint8_t> lastPacket;

    // Variables used for reliable packets
    uint8_t seqNum;
    uint8_t ackNum;

    bool c0Found;
    std::vector<uint8_t> unprocessedData;

    // Variables used in state RESET/UNINITIALIZED/INITIALIZED
    std::mutex syncMutex; // TODO: evaluate a new name for syncMutex
    std::condition_variable syncWaitCondition; // TODO: evaluate a new name for syncWaitCondition

    // Variables used in state ACTIVE
    std::chrono::milliseconds retransmissionInterval;
    std::mutex ackMutex;
    std::condition_variable ackWaitCondition;

    // Debugging related
    uint32_t incomingPacketCount;
    uint32_t outgoingPacketCount;
    uint32_t errorPacketCount;

    void logPacket(bool outgoing, std::vector<uint8_t> &packet);
    void log(std::string &logLine) const;
    void log(char const *logLine) const;
    void logStateTransition(h5_state_t from, h5_state_t to) const;
    static std::string stateToString(h5_state_t state);
    std::string asHex(std::vector<uint8_t> &packet) const;
    std::string hciPacketLinkControlToString(std::vector<uint8_t> payload) const;
    std::string h5PktToString(bool out, std::vector<uint8_t> &h5Packet) const;
    static std::string pktTypeToString(h5_pkt_type_t pktType);

    // State machine related
    h5_state_t currentState;

    std::map<h5_state_t, state_action_t> stateActions;
    void setupStateMachine();
    void startStateMachine();
    void stopStateMachine();

    std::map<h5_state_t, ExitCriterias*> exitCriterias;

    std::thread *stateMachineThread;
    void stateMachineWorker();
    bool runStateMachine;

    std::mutex stateMutex; // Mutex that allows threads to wait for a given state in the state machine
    bool waitForState(h5_state_t state, std::chrono::milliseconds timeout);
    std::condition_variable stateWaitCondition;

    static std::map<control_pkt_type, std::vector<uint8_t>> pkt_pattern;
    static std::map<h5_state_t, std::string> stateString;
    static std::map<h5_pkt_type_t, std::string> pktTypeString;

    static const uint8_t syncFirstByte = 0x01;
    static const uint8_t syncSecondByte = 0x7E;
    static const uint8_t syncRspFirstByte = 0x02;
    static const uint8_t syncRspSecondByte = 0x7D;
    static const uint8_t syncConfigFirstByte = 0x03;
    static const uint8_t syncConfigSecondByte = 0xFC;
    static const uint8_t syncConfigRspFirstByte = 0x04;
    static const uint8_t syncConfigRspSecondByte = 0x7B;
    static const uint8_t syncConfigField = 0x11;
};

#endif //H5_TRANSPORT_H
