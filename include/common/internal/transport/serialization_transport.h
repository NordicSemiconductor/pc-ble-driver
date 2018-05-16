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

#include "transport.h"

#include "ble.h"

#include <thread>
#include <mutex>
#include <condition_variable>

#include <queue>
#include <stdint.h>

typedef uint32_t(*transport_rsp_handler_t)(const uint8_t *p_buffer, uint16_t length);
typedef std::function<void(ble_evt_t * p_ble_evt)> evt_cb_t;

struct eventData_t
{
    uint8_t *data;
    uint32_t dataLength;
};

/**
 * @brief Serialization packet types
 */
typedef enum
{
    SERIALIZATION_COMMAND = 0,
    SERIALIZATION_RESPONSE = 1,
    SERIALIZATION_EVENT = 2,
    SERIALIZATION_DTM_CMD = 3,      // Direct test mode command
    SERIALIZATION_DTM_RESP = 4,     // Direct test mode response
    SERIALIZATION_RESET_CMD = 5
} serialization_pkt_type_t;

class SerializationTransport {
public:
    SerializationTransport(Transport *dataLinkLayer, uint32_t response_timeout);
    ~SerializationTransport();
    uint32_t open(status_cb_t status_callback, evt_cb_t event_callback, log_cb_t log_callback);
    uint32_t close();
    uint32_t send(uint8_t *cmdBuffer, uint32_t cmdLength, uint8_t *rspBuffer, uint32_t *rspLength,
        serialization_pkt_type_t pktType=SERIALIZATION_COMMAND);

private:
    SerializationTransport();
    void readHandler(uint8_t *data, size_t length);
    void eventHandlingRunner();

    status_cb_t statusCallback;
    evt_cb_t eventCallback;
    log_cb_t logCallback;

    Transport *nextTransportLayer;
    uint32_t responseTimeout;

    bool rspReceived;
    uint8_t *responseBuffer;
    uint32_t *responseLength;

    std::mutex sendMutex;

    std::mutex responseMutex;
    std::condition_variable responseWaitCondition;

    bool runEventThread; // Variable to control if thread shall run, used in thread to exit/keep running inthread
    std::mutex eventMutex;
    std::condition_variable eventWaitCondition;
    std::thread * eventThread;
    std::queue<eventData_t> eventQueue;
};

#endif //SERIALIZATION_TRANSPORT_H
