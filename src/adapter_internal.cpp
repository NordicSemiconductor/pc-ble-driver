/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

#include "adapter_internal.h"

#include "adapter.h"
#include "nrf_error.h"
#include "serialization_transport.h"

#include <string>

AdapterInternal::AdapterInternal(SerializationTransport *_transport): 
    eventCallback(nullptr),
    statusCallback(nullptr),
    logCallback(nullptr)
{
    this->transport = _transport;
}
                        
AdapterInternal::~AdapterInternal()
{
    delete transport;
}

uint32_t AdapterInternal::open(const sd_rpc_status_handler_t status_callback, const sd_rpc_evt_handler_t event_callback, const sd_rpc_log_handler_t log_callback)
{
    statusCallback = status_callback;
    eventCallback = event_callback;
    logCallback = log_callback;

    auto boundStatusHandler = std::bind(&AdapterInternal::statusHandler, this, std::placeholders::_1, std::placeholders::_2);
    auto boundEventHandler = std::bind(&AdapterInternal::eventHandler, this, std::placeholders::_1);
    auto boundLogHandler = std::bind(&AdapterInternal::logHandler, this, std::placeholders::_1, std::placeholders::_2);
    return transport->open(boundStatusHandler, boundEventHandler, boundLogHandler);
}

uint32_t AdapterInternal::close() const
{
    return transport->close();
}

void AdapterInternal::statusHandler(sd_rpc_app_status_t code, const char * message)
{
    adapter_t adapter;
    adapter.internal = static_cast<void *>(this);
    statusCallback(&adapter, code, message);
}

void AdapterInternal::eventHandler(ble_evt_t *event)
{
    // Event Thread
    adapter_t adapter;
    adapter.internal = static_cast<void *>(this);
    eventCallback(&adapter, event);
}

void AdapterInternal::logHandler(sd_rpc_log_severity_t severity, std::string log_message)
{
    adapter_t adapter;
    adapter.internal = static_cast<void *>(this);
    logCallback(&adapter, severity, log_message.c_str());
}

bool AdapterInternal::isInternalError(const uint32_t error_code) {
    if (error_code != NRF_SUCCESS) {
        return true;
    }
    else
    {
        return false;
    }
}
