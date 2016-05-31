/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
    bool syncConfigReceived;
    bool syncConfigRspSent;

    InitializedExitCriterias()
        : ExitCriterias(),
        syncConfigSent(false),
        syncConfigRspReceived(false),
        syncConfigReceived(false),
        syncConfigRspSent(false) {}

    bool isFullfilled() const override
    {
        return ioResourceError || close || (syncConfigSent && syncConfigRspReceived && syncConfigReceived && syncConfigRspSent);
    }

    void reset() override
    {
        ExitCriterias::reset();
        syncConfigSent = false;
        syncConfigRspSent = false;
        syncConfigReceived = false;
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

    ResetExitCriterias()
        : ExitCriterias(), resetSent(false)
    {}

    bool isFullfilled() const override
    {
        return ioResourceError || close || resetSent;
    }

    void reset() override
    {
        ExitCriterias::reset();
        resetSent = false;
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
