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

typedef enum
{
    SERIALIZATION_COMMAND = 0,
    SERIALIZATION_RESPONSE = 1,
    SERIALIZATION_EVENT = 2
} serialization_pkt_type_t;

class SerializationTransport {
public:
    SerializationTransport(Transport *dataLinkLayer, uint32_t response_timeout);
    ~SerializationTransport();
    uint32_t open(status_cb_t status_callback, evt_cb_t event_callback, log_cb_t log_callback);
    uint32_t close();
    uint32_t send(uint8_t *cmdBuffer, uint32_t cmdLength, uint8_t *rspBuffer, uint32_t *rspLength);

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
