#pragma once

#include "h5_transport.h"
#include "transport.h"

#include "ble.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <cstdint>
#include <queue>

typedef uint32_t (*transport_rsp_handler_t)(const uint8_t *p_buffer, uint16_t length);
typedef std::function<void(ble_evt_t *p_ble_evt)> evt_cb_t;

constexpr size_t MaxPossibleEventLength = 700;

struct eventData_t
{
    uint8_t *data;
    uint32_t dataLength;
};

typedef enum {
    SERIALIZATION_COMMAND   = 0,
    SERIALIZATION_RESPONSE  = 1,
    SERIALIZATION_EVENT     = 2,
    SERIALIZATION_DTM_CMD   = 3, // Direct test mode command
    SERIALIZATION_DTM_RESP  = 4, // Direct test mode response
    SERIALIZATION_RESET_CMD = 5
} serialization_pkt_type_t;

class SerializationTransport
{
  public:
    SerializationTransport(const SerializationTransport &) = delete;
    SerializationTransport &operator=(const SerializationTransport &) = delete;
    SerializationTransport(SerializationTransport &&)                 = delete;
    SerializationTransport &operator=(SerializationTransport &&) = delete;

    SerializationTransport(H5Transport *dataLinkLayer, uint32_t response_timeout);
    ~SerializationTransport();

    uint32_t open(const status_cb_t &status_callback, const evt_cb_t &event_callback,
                  const log_cb_t &log_callback) noexcept;
    uint32_t close() noexcept;
    uint32_t send(const std::vector<uint8_t> &cmdBuffer,
                  std::shared_ptr<std::vector<uint8_t>> rspBuffer,
                  serialization_pkt_type_t pktType = SERIALIZATION_COMMAND) noexcept;

  private:
    void readHandler(const uint8_t *data, const size_t length);
    void eventHandlingRunner() noexcept;

    status_cb_t statusCallback;
    evt_cb_t eventCallback;
    log_cb_t logCallback;

    data_cb_t dataCallback;

    std::shared_ptr<H5Transport> nextTransportLayer;
    uint32_t responseTimeout;

    bool responseReceived;
    std::shared_ptr<std::vector<uint8_t>> responseBuffer;

    std::mutex sendMutex;

    std::mutex responseMutex;
    std::condition_variable responseWaitCondition;

    std::mutex eventMutex;
    std::condition_variable eventWaitCondition;
    std::thread eventThread;
    std::queue<std::vector<uint8_t>> eventQueue;
    void drainEventQueue();
    bool processEvents;

    // Use recursive mutex since the mutex may be acquired recursively in the same thread
    // in the case of ::eventHandlingRunner thread calling a application callback that
    // again invoke a function that call ::send
    std::recursive_mutex isOpenMutex;
    bool isOpen;
};
