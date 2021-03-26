#pragma once

#include "transport.h"
#include "uart_transport.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <vector>

#include "h5.h"
#include "h5_transport_exit_criterias.h"

#include <map>
#include <stdint.h>
#include <thread>

enum class h5_state : uint8_t {
    STATE_START,
    STATE_RESET,
    STATE_UNINITIALIZED,
    STATE_INITIALIZED,
    STATE_ACTIVE,
    STATE_FAILED,
    STATE_CLOSED,
    STATE_NO_RESPONSE,
    STATE_UNKNOWN
};

constexpr uint8_t SyncFirstByte           = 0x01;
constexpr uint8_t SyncSecondByte          = 0x7E;
constexpr uint8_t SyncRspFirstByte        = 0x02;
constexpr uint8_t SyncRspSecondByte       = 0x7D;
constexpr uint8_t SyncConfigFirstByte     = 0x03;
constexpr uint8_t SyncConfigSecondByte    = 0xFC;
constexpr uint8_t SyncConfigRspFirstByte  = 0x04;
constexpr uint8_t SyncConfigRspSecondByte = 0x7B;
constexpr uint8_t SyncConfigField         = 0x11;

using state_action_t = std::function<h5_state()>;
using h5_payload_t   = std::vector<uint8_t>;

class H5Transport : public Transport
{
  public:
    H5Transport() = delete;
    H5Transport(UartTransport *nextTransportLayer, const uint32_t retransmission_interval);
    ~H5Transport() noexcept override;

    uint32_t open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                  const log_cb_t &log_callback) noexcept override;
    uint32_t close() noexcept override;
    uint32_t send(const std::vector<uint8_t> &data) noexcept override;

    h5_state state() const;

    static bool isSyncPacket(const h5_payload_t &packet, const uint8_t offset = 0);
    static bool isSyncResponsePacket(const h5_payload_t &packet, const uint8_t offset = 0);
    static bool isSyncConfigPacket(const h5_payload_t &packet, const uint8_t offset = 0);
    static bool isSyncConfigResponsePacket(const h5_payload_t &packet, const uint8_t offset = 0);
    static bool isResetPacket(const h5_payload_t &packet, const uint8_t offset = 0);
    static bool checkPattern(const h5_payload_t &packet, const uint8_t offset,
                             const h5_payload_t &pattern);
    static h5_payload_t getPktPattern(const control_pkt_type);
    static std::string stateToString(const h5_state state) noexcept;
    static std::string pktTypeToString(const h5_pkt_type pktType);

  private:
    void dataHandler(const uint8_t *data, const size_t length) noexcept;
    void statusHandler(const sd_rpc_app_status_t code, const std::string &error) noexcept;
    void processPacket(const h5_payload_t &packet);

    void sendControlPacket(control_pkt_type type, const uint8_t ackNum = 0xff);

    void incrementSeqNum();
    void incrementAckNum();

    Transport *nextTransportLayer;
    std::vector<uint8_t> lastPacket;

    // Callbacks used by lower transports
    // These callbacks invoke upper callbacks
    status_cb_t statusCallback;
    data_cb_t dataCallback;

    // Variables used for reliable packets
    std::recursive_mutex seqNumMutex;
    uint8_t seqNum;

    std::recursive_mutex ackNumMutex;
    uint8_t ackNum;

    bool c0Found;
    std::vector<uint8_t> unprocessedData;

    // Mutex controlling access to state machine variables in
    // the different state machine states
    std::mutex stateMachineMutex;

    // Condition variable to communicate changes to state machine
    std::condition_variable stateMachineChange;

    // Variables used in state ACTIVE
    std::chrono::milliseconds retransmissionInterval;
    std::mutex ackMutex;
    std::condition_variable ackReceived;

    // Debugging related
    std::atomic<uint32_t> incomingPacketCount;
    std::atomic<uint32_t> outgoingPacketCount;
    std::atomic<uint32_t> errorPacketCount;

    void logPacket(const bool outgoing, const h5_payload_t &packet);
    void logStateTransition(const h5_state from, const h5_state to) const;
    static std::string asHex(const h5_payload_t &packet);
    static std::string hciPacketLinkControlToString(const h5_payload_t &payload);
    std::string h5PktToString(const bool out, const h5_payload_t &h5Packet) const;

    // State machine related
    h5_state currentState;
    std::thread stateMachineThread;

    bool stateMachineReady;

    std::map<h5_state, state_action_t> stateActions;
    void setupStateMachine();
    void startStateMachine();
    void stopStateMachine();

    std::map<h5_state, std::shared_ptr<ExitCriterias>> exitCriterias;

    void stateMachineWorker() noexcept;

    // Mutex that allows threads to wait for a given state in the state machine
    std::mutex currentStateMutex;
    bool waitForState(h5_state state, std::chrono::milliseconds timeout);
    std::condition_variable currentStateChange;

    std::recursive_mutex isOpenMutex;
    bool isOpen;

    // Actions associated with each state
    h5_state stateActionStart();
    h5_state stateActionReset();
    h5_state stateActionUninitialized();
    h5_state stateActionInitialized();
    h5_state stateActionActive();
    h5_state stateActionFailed();
    h5_state stateActionClosed();
    h5_state stateActionNoResponse();
};
