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
#ifndef _APP_BLE_GAP_SEC_KEYS_H
#define _APP_BLE_GAP_SEC_KEYS_H

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */

/**
 * @addtogroup ser_app_s130_codecs Application s130 codecs
 * @ingroup ser_codecs
 */

 /**@file
 *
 * @defgroup app_ble_gap_sec_keys GAP Functions for managing memory for security keys in application device.
 * @{
 * @ingroup  ser_app_s130_codecs
 *
 * @brief    GAP Application auxiliary functions for synchronizing GAP security keys with the ones stored in the connectivity device. 
 */

#include "ble_gap.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**@brief GAP connection - keyset mapping structure.
 *
 * @note  This structure is used to map keysets to connection instances, and will be stored in a static table.
 */
typedef struct
{
  uint8_t                conn_active;    /**< Indication that keys for this connection are used by soft device. 0: keys used; 1: keys not used*/
  ble_gap_sec_keyset_t   keyset;         /**< Keyset structure, see @ref ble_gap_sec_keyset_t.*/
} ser_ble_gap_app_keyset_t;


/**@brief Sets root context for calls to *_context_create, *_context_destroy, *_context_find
*
* @param[in]     context             root pointer to use as key for finding keysets
*/
void app_ble_gap_sec_context_root_set(void *context);


/**@brief Release root context for calls to *_context_create, *_context_destroy, *_context_find
*
*/
void app_ble_gap_sec_context_root_release();


/**@brief allocates instance for storage of encryption keys.
 *
 * @param[in]     adapter             adapter
 * @param[in]     conn_handle         conn_handle
 * @param[out]    **pp_gap_app_keyset Pointer to the keyset corresponding to the given conn_handle
 *
 * @retval NRF_SUCCESS                Context allocated.
 * @retval NRF_ERROR_NO_MEM           No free instance available.
 */
uint32_t app_ble_gap_sec_context_create(uint16_t conn_handle, ser_ble_gap_app_keyset_t **pp_gap_app_keyset);

/**@brief release instance identified by a connection handle.
 *
 * @param[in]     adapter             adapter
 * @param[in]     conn_handle         conn_handle
 *
 * @retval NRF_SUCCESS                Context released.
 * @retval NRF_ERROR_NOT_FOUND        instance with conn_handle not found
 */
uint32_t app_ble_gap_sec_context_destroy(uint16_t conn_handle);

/**@brief finds index of instance identified by a connection handle in m_app_keys_table[].
 *
 * @param[in]     adapter             adapter
 * @param[in]     conn_handle         conn_handle
 *
 * @param[out]    **pp_gap_app_keyset Pointer to the keyset corresponding to the given conn_handle
 *
 * @retval NRF_SUCCESS                Context found
 * @retval NRF_ERROR_NOT_FOUND        instance with conn_handle not found
 */
uint32_t app_ble_gap_sec_context_find(uint16_t conn_handle, ser_ble_gap_app_keyset_t **pp_gap_app_keyset);
/** @} */

#ifdef __cplusplus
}
#endif
#endif //_APP_BLE_GAP_SEC_KEYS_H
