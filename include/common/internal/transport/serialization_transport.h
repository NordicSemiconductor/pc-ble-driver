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

#ifndef SERIALIZATION_TRANSPORT_H
#define SERIALIZATION_TRANSPORT_H

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

#endif // SERIALIZATION_TRANSPORT_H
