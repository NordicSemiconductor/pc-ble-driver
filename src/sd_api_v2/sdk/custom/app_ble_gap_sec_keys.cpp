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

/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#include "app_ble_gap_sec_keys.h"
#include "nrf_error.h"
#include <stddef.h>

// C++ code
#include "adapter.h"

#include <map>
#include <vector>
#include <thread>
#include <mutex>

#include <boost/thread/tss.hpp>

typedef std::map<uint16_t, ser_ble_gap_app_keyset_t*> connhandle_keyset_t;

// Map with context, each with a set of conn_handle and each conn_handle a ser_ble_gap_app_keyset_t*
std::map<void*, connhandle_keyset_t*> m_app_keys_table;
void *current_context = nullptr;
std::mutex current_context_mutex;

void app_ble_gap_sec_context_root_set(void *context)
{
    current_context_mutex.lock();
    current_context = context;
}

void app_ble_gap_sec_context_root_release()
{
    current_context = nullptr;
    current_context_mutex.unlock();
}

uint32_t app_ble_gap_sec_context_create(uint16_t conn_handle, ser_ble_gap_app_keyset_t **pp_gap_app_keyset)
{
    if (current_context == nullptr) return NRF_ERROR_INVALID_DATA;

    auto tempRootContext = m_app_keys_table.find(current_context);

    auto keyset = new ser_ble_gap_app_keyset_t();

    // If current context is not found we have to add it
    if (tempRootContext == m_app_keys_table.end()) 
    {
        auto connectionAndKeyset = new std::map<uint16_t, ser_ble_gap_app_keyset_t*>{
            { conn_handle, keyset }
        };

        m_app_keys_table.insert(std::make_pair(current_context, connectionAndKeyset));
    }
    else
    {
        auto connHandleMap = tempRootContext->second;
        auto connHandle = connHandleMap->find(conn_handle);

        // If connection handle does not exist from before
        // we create a new ser_ble_gap_app_keyset_t for the connnection handle
        if (connHandle != connHandleMap->end())
        {
            delete connHandle->second;
            connHandleMap->erase(conn_handle);
        }

        connHandleMap->insert(std::make_pair(conn_handle, keyset));
    }

    *pp_gap_app_keyset = keyset;
    return NRF_SUCCESS;
}

uint32_t app_ble_gap_sec_context_destroy(uint16_t conn_handle)
{
    auto tempAdapter = m_app_keys_table.find(current_context);
    if (tempAdapter == m_app_keys_table.end()) return NRF_ERROR_NOT_FOUND;

    auto connHandleMap = tempAdapter->second;
    auto connHandle = connHandleMap->find(conn_handle);

    if (connHandle == connHandleMap->end()) return NRF_ERROR_NOT_FOUND;
    delete connHandle->second; // Delete the ser_ble_gap_app_keyset_t 
    connHandleMap->erase(conn_handle);

    return NRF_SUCCESS;
}

uint32_t app_ble_gap_sec_context_find(uint16_t conn_handle, ser_ble_gap_app_keyset_t **pp_gap_app_keyset)
{
    auto tempAdapter = m_app_keys_table.find(current_context);
    if (tempAdapter == m_app_keys_table.end()) return NRF_ERROR_NOT_FOUND;

    auto connHandleMap = tempAdapter->second;
    auto connHandle = connHandleMap->find(conn_handle);

    if (connHandle == connHandleMap->end()) return NRF_ERROR_NOT_FOUND;
    *pp_gap_app_keyset = connHandle->second;
    return NRF_SUCCESS;
}
