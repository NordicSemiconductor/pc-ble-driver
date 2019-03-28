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
#ifndef _APP_BLE_GAP_H
#define _APP_BLE_GAP_H

/**@file
 *
 * @defgroup app_ble_gap_sec_keys GAP Functions for managing memory for security keys in the
 * application device.
 * @{
 * @ingroup  ser_app_s130_codecs
 *
 * @brief    GAP Application auxiliary functions for synchronizing the GAP security keys with the
 * ones stored in the connectivity device.
 */

#include "ble_gap.h"
#include "ble_types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SER_MAX_CONNECTIONS
#define SER_MAX_CONNECTIONS 8
#endif

#ifndef APP_BLE_GAP_ADV_BUF_COUNT
#define APP_BLE_GAP_ADV_BUF_COUNT 8 // Keep in sync with connectivity in SDK
#endif

/**@brief GAP connection - keyset mapping structure.
 *
 * @note  This structure is used to map keysets to connection instances and store them in a static
 * table.
 */
typedef struct
{
    uint16_t conn_handle; /**< Connection handle.*/
    uint8_t conn_active;  /**< Indication that keys for this connection are used by the SoftDevice.
                             0: keys used; 1: keys not used. */
    ble_gap_sec_keyset_t keyset; /**< Keyset structure, see @ref ble_gap_sec_keyset_t.*/
} ser_ble_gap_app_keyset_t;

typedef enum {
    REQUEST_REPLY_CODEC_CONTEXT,
    EVENT_CODEC_CONTEXT
} app_ble_gap_adapter_codec_context_t;

/**
 * @brief Create GAP states for a given adapter
 *
 * The purpose of this function is to create a GAP state for a given adapter.
 *
 * @param[in] key Key that represents the adapter, used by other functions to select GAP state for a
 * adapter
 */
uint32_t app_ble_gap_state_create(void *key);

/**
 * @brief Delete GAP states for a given adapter
 *
 * The purpose of this function is to delete the GAP state for a given adapter.
 *
 * @param[in] key Key that represents the adapter
 */
uint32_t app_ble_gap_state_delete(void *key);

/**@brief Set the current GAP adapter to be used by codecs
 *
 * @param[in]     adapter_id    Adapter to be used by codecs
 * @param[in]     codec_context Codec context
 */
void app_ble_gap_set_current_adapter_id(void *adapter_id,
                                        const app_ble_gap_adapter_codec_context_t codec_context);

/**@brief Unset the current GAP adapter used by codecs
 * @param[in]     codec_context Unset the given codec context
 */
void app_ble_gap_unset_current_adapter_id(const app_ble_gap_adapter_codec_context_t codec_context);

/**@brief Check if current adapter is set
 * @param[in] codec_context       Check if adapter GAP state is set for codec context
 */
uint32_t
app_ble_gap_check_current_adapter_set(const app_ble_gap_adapter_codec_context_t codec_context);

/**@brief Allocates the instance in m_app_keys_table[] for storage of encryption keys.
 *
 * @param[in]     conn_handle         conn_handle
 * @param[out]    p_index             Pointer to the index of the allocated instance.
 *
 * @retval NRF_SUCCESS                Key storage allocated.
 * @retval NRF_ERROR_NO_MEM           No free instance available.
 */
uint32_t app_ble_gap_sec_keys_storage_create(uint16_t conn_handle, uint32_t *p_index);

/**@brief Release the instance identified by a connection handle.
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @retval NRF_SUCCESS                Context released.
 * @retval NRF_ERROR_NOT_FOUND        Instance with conn_handle not found.
 */
uint32_t app_ble_gap_sec_keys_storage_destroy(const uint16_t conn_handle);

/**@brief Finds index of instance identified by a connection handle.
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @param[out]    p_index             Pointer to the index of the entry in the keys table
 * corresponding to the given conn_handle.
 *
 * @retval NRF_SUCCESS                Context found.
 * @retval NRF_ERROR_NOT_FOUND        Instance with conn_handle not found.
 */
uint32_t app_ble_gap_sec_keys_find(const uint16_t conn_handle, uint32_t *p_index);

/**@brief Gets key for given adapter and connection.
 *
 * @param[in] index key index
 * @param[out] Double pointer to keyset for given key index
 */
uint32_t app_ble_gap_sec_keys_get(const uint32_t index, ble_gap_sec_keyset_t **keyset);

/**@brief Updates key in given index. This function is used in REQUEST_REPLY_CODEC_CONTEXT.
 *
 * @param[in] index key index
 * @param[out] Pointer to keyset for given key index
 */
uint32_t app_ble_gap_sec_keys_update(const uint32_t index, const ble_gap_sec_keyset_t *keyset);

/**
 * @brief Reset internal values in app_ble_gap
 *
 * The purpose of this function is to clear internal values in this file
 * when starting to use the adapter
 */
uint32_t app_ble_gap_state_reset();

#if NRF_SD_BLE_API_VERSION >= 6

/**
 * @brief Data structure to store advertising set data
 */
typedef struct
{
    uint8_t adv_handle;
    uint8_t *buf1;
    uint8_t *buf2;
} adv_set_data_t;

void app_ble_gap_set_adv_data_set(uint8_t adv_handle, uint8_t *buf1, uint8_t *buf2);

/**
 * @brief Stores buffer for adv report data.
 *
 * @param p_data Pointer to the buffer.
 *
 */
void app_ble_gap_scan_data_set(const uint8_t *p_scan_data);

/**
 * @brief Returns pointer to the buffer for storing report data. Returns error if not paired with
 *        @ref app_ble_gap_scan_data_set call.
 *
 * @param[out] p_data Stored data.
 * @return NRF_SUCCESS or error in case pointer is already cleared.
 */
uint32_t app_ble_gap_scan_data_fetch_clear(ble_data_t *p_data);

/**
 * @brief Register an advertisement buffer pointer
 *
 * @param[in] p_buf Advertisement buffer to create a pointer ID from
 * @return -1 if there is no space left in buffer table, 0 of p_buf is nullptr, >0 with buffer
 * location in buffer table
 */

int app_ble_gap_adv_buf_register(void *p_buf);
int app_ble_gap_adv_buf_addr_unregister(void *p_buf);
/**
 * @brief Unregister a buffer from advertisement buffer table
 *
 * Unregister a buffer from the buffer table (ble_gap_adv_buf_addr)
 *
 * @param[in] id buffer ID in the ble_gap_adv_buf_addr table
 * @param[in] event_context true if EVENT context, false if it is in REQUEST_REPLOY context
 *
 * @return Buffer pointer from advertisement buffer table, except nullptr if id == 0 or if the
 * context for the current adapter is not set
 */
void *app_ble_gap_adv_buf_unregister(const int id, bool event_context);

void app_ble_gap_scan_data_unset(bool free);

#endif // NRF_SD_BLE_API_VERSION >= 6
/** @} */
#ifdef __cplusplus
}
#endif

#endif //_APP_BLE_GAP_H
