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
#include <memory>

#include <sd_rpc_types.h>

typedef struct
{
    bool active;
    uint8_t adv_handle;
    uint8_t *p_adv_data;
    uint8_t *p_scan_rsp_data;
} adv_set_t;

/**
 * @brief This structure keeps GAP states for one adapter
 */
typedef struct
{
    // GAP connection - BLE security keys table for storage.
    ser_ble_gap_app_keyset_t app_keys_table[SER_MAX_CONNECTIONS]{};
    // Advertisement sets - BLE advertisement sets
    adv_set_t adv_sets[BLE_GAP_ADV_SET_COUNT_MAX]{};
    // Buffer for scan data received
    ble_data_t scan_data = {nullptr, 0};
} adapter_ble_gap_state_t;

/**
 * @brief This map keeps the GAP states (see @ref adapter_ble_gap_state_t) for adapters used
 */
static std::map<void *, std::shared_ptr<adapter_ble_gap_state_t>> adapters_gap_state;

/**
 * @brief This structure keeps information related to an adapter key
 *
 * An adapter key is used to tell the codecs what adapter_ble_gap_state_t to
 * use during encoding and decoding.
 *
 */
typedef struct
{
    /**
     * @brief Key that points to the @ref adapter_ble_gap_state_t [adapter GAP state] currently used be codecs
     */
    void *key{nullptr};
    
    /**
     * @brief Mutex that protects changing the context while encoding/decoding is in progress
     */
    std::mutex key_mutex;
} adapter_key_t;

/**
 * @brief Adapter key used for encoding/decoding request reply commands
 */
static adapter_key_t request_reply_key;

/**
 * @brief Adapter key used for decoding events
 */
static adapter_key_t event_key;

uint32_t app_ble_gap_state_create(void *key)
{
    if (adapters_gap_state.count(key) == 1)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    adapters_gap_state[key] = std::make_shared<adapter_ble_gap_state_t>();

    return NRF_SUCCESS;
}

uint32_t app_ble_gap_state_delete(void *key)
{
    if (adapters_gap_state.erase(key) != 1)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    // TODO: if this key is used in request_reply_key
    // TODO: or event_key, remove them from the keys
    // TODO: and unlock any keys

    return NRF_SUCCESS;
}

void app_ble_gap_adapter_key_lock(void *key, const app_ble_gap_adapter_key_t key_type)
{
    if (key_type == EVENT_ADAPTER_KEY)
    {
        event_key.key_mutex.lock();
        event_key.key = key;
    }
    else if (key_type == REQUEST_REPLY_ADAPTER_KEY)
    {
        request_reply_key.key_mutex.lock();
        request_reply_key.key = key;
    }
}

void app_ble_gap_adapter_key_unlock(const app_ble_gap_adapter_key_t key_type)
{
    // Do not clear the current_context until
    // the context_lock_count is zero
    if (key_type == EVENT_ADAPTER_KEY)
    {
        event_key.key_mutex.unlock();
        event_key.key = nullptr;
    }
    else if (key_type == REQUEST_REPLY_ADAPTER_KEY)
    {
        request_reply_key.key_mutex.unlock();
        request_reply_key.key = nullptr;
    }
}

bool app_ble_gap_adapter_key_check_assigned(const app_ble_gap_adapter_key_t key_type)
{
    if (key_type == EVENT_ADAPTER_KEY)
    {
        return event_key.key != nullptr;
    }
    else if (key_type == REQUEST_REPLY_ADAPTER_KEY)
    {
        return request_reply_key.key != nullptr;
    }

    return false;
}

uint32_t app_ble_gap_sec_context_create(uint16_t conn_handle, uint32_t *p_index)
{
    if (!app_ble_gap_adapter_key_check_assigned(REQUEST_REPLY_ADAPTER_KEY))
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(request_reply_key.key);

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
    if (!app_ble_gap_adapter_key_check_assigned(EVENT_ADAPTER_KEY))
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(event_key.key);

        for (auto &keys : gap_state->app_keys_table)
        {
            if (keys.conn_handle == conn_handle)
            {
                keys.conn_active = 0;
                return NRF_SUCCESS;
            }
        }

        return NRF_ERROR_NO_MEM;
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }
}

uint32_t app_ble_gap_sec_context_find(uint16_t conn_handle, uint32_t *p_index)
{
    if (!app_ble_gap_adapter_key_check_assigned(EVENT_ADAPTER_KEY))
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(event_key.key);

        for (auto i = 0; i < SER_MAX_CONNECTIONS; i++)
        {
            auto &keys = gap_state->app_keys_table[i];
            if ((keys.conn_handle == conn_handle) && (keys.conn_active == 1))
            {
                *p_index = i;
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

uint32_t app_ble_gap_sec_keys_update(const uint32_t index, const ble_gap_sec_keyset_t *keyset)
{
    if (!app_ble_gap_adapter_key_check_assigned(REQUEST_REPLY_ADAPTER_KEY))
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(request_reply_key.key);
        std::memcpy(&(gap_state->app_keys_table[index].keyset), keyset,
                    sizeof(ble_gap_sec_keyset_t));
        return NRF_SUCCESS;
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }
}

uint32_t app_ble_gap_sec_keys_get(const uint32_t index, ble_gap_sec_keyset_t **keyset)
{
    if (!app_ble_gap_adapter_key_check_assigned(EVENT_ADAPTER_KEY))
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(event_key.key);
        *keyset              = &(gap_state->app_keys_table[index].keyset);
        return NRF_SUCCESS;
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }
}

uint32_t app_ble_gap_reset()
{
    if (!app_ble_gap_adapter_key_check_assigned(REQUEST_REPLY_ADAPTER_KEY))
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(request_reply_key.key);

        for (auto &adv_set : gap_state->adv_sets)
        {
            adv_set.active = false;
        }

        for (auto &keyset : gap_state->app_keys_table)
        {
            keyset.conn_active = false;
        }

        gap_state->scan_data = {nullptr, 0};
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    return NRF_SUCCESS;
}

uint32_t app_ble_gap_scan_data_set(ble_data_t const *p_data)
{
    if (!app_ble_gap_adapter_key_check_assigned(REQUEST_REPLY_ADAPTER_KEY))
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(request_reply_key.key);

        if (gap_state->scan_data.p_data != nullptr)
        {
            return NRF_ERROR_BUSY;
        }

        memcpy(&(gap_state->scan_data), p_data, sizeof(ble_data_t));
        return NRF_SUCCESS;
    }
    catch (const std::out_of_range &)
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }
}

uint32_t app_ble_gap_scan_data_fetch_clear(ble_data_t *p_data)
{
    if (!app_ble_gap_adapter_key_check_assigned(EVENT_ADAPTER_KEY))
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(event_key.key);
        std::memcpy(p_data, &(gap_state->scan_data), sizeof(ble_data_t));

        if (gap_state->scan_data.p_data != nullptr)
        {
            gap_state->scan_data.p_data = nullptr;
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
    if (!app_ble_gap_adapter_key_check_assigned(REQUEST_REPLY_ADAPTER_KEY))
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(request_reply_key.key);

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
    if (!app_ble_gap_adapter_key_check_assigned(EVENT_ADAPTER_KEY))
    {
        return NRF_ERROR_SD_RPC_INVALID_STATE;
    }

    try
    {
        const auto gap_state = adapters_gap_state.at(event_key.key);

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
