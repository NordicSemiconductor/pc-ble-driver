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

#include "adapter_internal.h"

#include "adapter.h"
#include "nrf_error.h"
#include "serialization_transport.h"

#include <string>

AdapterInternal::AdapterInternal(SerializationTransport *_transport): 
    eventCallback(nullptr),
    statusCallback(nullptr),
    logCallback(nullptr),
    logSeverityFilter(SD_RPC_LOG_TRACE)
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

    if((uint32_t) severity >= (uint32_t) logSeverityFilter)
    {
        logCallback(&adapter, severity, log_message.c_str());
    }
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

uint32_t AdapterInternal::logSeverityFilterSet(sd_rpc_log_severity_t severity_filter)
{
    logSeverityFilter = severity_filter;
    return NRF_SUCCESS;
}

