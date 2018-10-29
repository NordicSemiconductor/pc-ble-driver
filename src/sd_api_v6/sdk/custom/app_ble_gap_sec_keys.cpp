/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "app_ble_gap_sec_keys.h"
#include "nrf_error.h"
#include "ser_config.h"

#include <cstring>
#include <map>
#include <mutex>
#include <sd_rpc_types.h>

#if NRF_SD_BLE_API_VERSION >= 6
typedef struct
{
    bool active;
    uint8_t adv_handle;
    uint8_t *p_adv_data;
    uint8_t *p_scan_rsp_data;
} adv_set_t;
#endif

typedef struct
{
    ser_ble_gap_app_keyset_t app_keys_table[SER_MAX_CONNECTIONS]{};
#if NRF_SD_BLE_API_VERSION >= 6
    adv_set_t adv_sets[BLE_GAP_ADV_SET_COUNT_MAX]{};
    ble_data_t m_scan_data = {nullptr, 0};
#endif
} adapter_ble_gap_state_t;

static std::map<void *, std::shared_ptr<adapter_ble_gap_state_t>> adapters_gap_state;

void *current_context = nullptr;
std::mutex current_context_mutex;

uint32_t app_ble_gap_state_create(void *key)
{
    std::lock_guard<std::mutex> lck(current_context_mutex);

    if (adapters_gap_state.count(key) == 1)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    adapters_gap_state[key] = std::make_shared<adapter_ble_gap_state_t>();

    return NRF_SUCCESS;
}

uint32_t app_ble_gap_state_release(void *key)
{
    std::lock_guard<std::mutex> lck(current_context_mutex);

    if (adapters_gap_state.erase(key) != 1)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    return NRF_SUCCESS;
}

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

uint32_t app_ble_gap_sec_context_create(uint16_t conn_handle, uint32_t *p_index)
{
    // Find current context
    std::lock_guard<std::mutex> lck(current_context_mutex);

    if (current_context == nullptr)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(current_context);

        for (auto i = 0; i < SER_MAX_CONNECTIONS; i++)
        {
            auto &keys = gap_state->app_keys_table[i];

            if (!keys.conn_active)
            {
                keys.conn_active = 1;
                keys.conn_handle = conn_handle;
                *p_index         = i;
                return NRF_SUCCESS;
            }
        }
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    return NRF_ERROR_NO_MEM;
}

uint32_t app_ble_gap_sec_context_destroy(uint16_t conn_handle)
{
    std::lock_guard<std::mutex> lck(current_context_mutex);

    if (current_context == nullptr)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(current_context);

        uint32_t err_code = NRF_ERROR_NO_MEM;

        for (auto &keys : gap_state->app_keys_table)
        {
            if (keys.conn_handle == conn_handle)
            {
                keys.conn_active = 0;
                err_code         = NRF_SUCCESS;
                break;
            }
        }

        return err_code;
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }
}

uint32_t app_ble_gap_sec_context_find(uint16_t conn_handle, uint32_t *p_index)
{
    std::lock_guard<std::mutex> lck(current_context_mutex);

    if (current_context == nullptr)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(current_context);

        uint32_t err_code = NRF_ERROR_NO_MEM;

        for (auto &keys : gap_state->app_keys_table)
        {
            if ((keys.conn_handle == conn_handle) && (keys.conn_handle == 1))
            {
                keys.conn_active = 0;
                err_code         = NRF_SUCCESS;
                break;
            }
        }

        return err_code;
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }
}

#if NRF_SD_BLE_API_VERSION >= 6
uint32_t app_ble_gap_scan_data_set(ble_data_t const *p_data)
{
    std::lock_guard<std::mutex> lck(current_context_mutex);

    if (current_context == nullptr)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(current_context);

        if (gap_state->m_scan_data.p_data != nullptr)
        {
            return NRF_ERROR_BUSY;
        }

        memcpy(&(gap_state->m_scan_data), p_data, sizeof(ble_data_t));
        return NRF_SUCCESS;
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }
}

uint32_t app_ble_gap_scan_data_fetch_clear(ble_data_t *p_data)
{
    std::lock_guard<std::mutex> lck(current_context_mutex);

    if (current_context == nullptr)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(current_context);

        if (gap_state->m_scan_data.p_data != nullptr)
        {
            return NRF_ERROR_BUSY;
        }

        memcpy(p_data, &(gap_state->m_scan_data), sizeof(ble_data_t));

        if (gap_state->m_scan_data.p_data != nullptr)
        {
            gap_state->m_scan_data.p_data = nullptr;
            return NRF_SUCCESS;
        }

        return NRF_ERROR_NOT_FOUND;
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }
}

uint32_t app_ble_gap_adv_set_register(uint8_t adv_handle, uint8_t *p_adv_data,
                                      uint8_t *p_scan_rsp_data)
{
    std::lock_guard<std::mutex> lck(current_context_mutex);

    if (current_context == nullptr)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(current_context);

        for (auto &m_adv_set : gap_state->adv_sets)
        {
            if (!m_adv_set.active)
            {
                m_adv_set.active          = true;
                m_adv_set.adv_handle      = adv_handle;
                m_adv_set.p_adv_data      = p_adv_data;
                m_adv_set.p_scan_rsp_data = p_scan_rsp_data;
                return NRF_SUCCESS;
            }
        }

        return NRF_ERROR_NOT_FOUND;
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }
}

uint32_t app_ble_gap_adv_set_unregister(uint8_t adv_handle, uint8_t **pp_adv_data,
                                        uint8_t **pp_scan_rsp_data)
{
    std::lock_guard<std::mutex> lck(current_context_mutex);

    if (current_context == nullptr)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(current_context);

        for (auto &m_adv_set : gap_state->adv_sets)
        {
            if (m_adv_set.active && (m_adv_set.adv_handle == adv_handle))
            {
                m_adv_set.active  = false;
                *pp_adv_data      = m_adv_set.p_adv_data;
                *pp_scan_rsp_data = m_adv_set.p_scan_rsp_data;
                return NRF_SUCCESS;
            }
        }

        return NRF_ERROR_NOT_FOUND;
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }
}

uint32_t app_ble_gap_reset()
{
    std::lock_guard<std::mutex> lck(current_context_mutex);

    if (current_context == nullptr)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(current_context);

        for (auto &adv_set : gap_state->adv_sets)
        {
            adv_set.active = false;
        }

        for (auto &keyset : gap_state->app_keys_table)
        {
            keyset.conn_active = false;
        }

        gap_state->m_scan_data = {nullptr, 0};

        current_context = nullptr;
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    return NRF_SUCCESS;
}

#endif
