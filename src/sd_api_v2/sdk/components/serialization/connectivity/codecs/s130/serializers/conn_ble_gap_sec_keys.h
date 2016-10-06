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
#ifndef _CONN_BLE_GAP_SEC_KEYS_H
#define _CONN_BLE_GAP_SEC_KEYS_H

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */
 
/**
 * @addtogroup ser_conn_s130_codecs Connectivity s130 codecs
 * @ingroup ser_codecs
 */

/**@file
 *
 * @defgroup conn_ble_gap_sec_keys GAP Functions for managing memory for security keys on connectivity device.
 * @{
 * @ingroup  ser_conn_s130_codecs
 *
 * @brief    GAP Connectivity auxiliary functions for providing static memory required by Soft Device. This memory is used to store GAP security keys. 
 */
 
#include "ble_gap.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SER_MAX_CONNECTIONS 2

/**@brief GAP connection - keyset mapping structure.
 *
 * @note  This structure is used to map keysets to connection instances, and will be stored in a static table.
 */
typedef struct
{
  uint16_t             conn_handle;        /**< Connection handle.*/
  uint8_t              conn_active;        /**< Indication that keys for this connection are used by soft device. 0: keys used; 1: keys not used*/
  ble_gap_sec_keyset_t keyset;             /**< Keyset structure see @ref ble_gap_sec_keyset_t.*/
  ble_gap_enc_key_t    enc_key_periph;     /**< Peripheral Encryption Key, see @ref ble_gap_enc_key_t.*/
  ble_gap_id_key_t     id_key_periph;      /**< Peripheral Identity Key, see @ref ble_gap_id_key_t.*/
  ble_gap_sign_info_t  sign_key_periph;    /**< Peripheral Signing Information, see @ref ble_gap_sign_info_t.*/
  ble_gap_enc_key_t    enc_key_central;    /**< Central Encryption Key, see @ref ble_gap_enc_key_t.*/
  ble_gap_id_key_t     id_key_central;     /**< Central Identity Key, see @ref ble_gap_id_key_t.*/
  ble_gap_sign_info_t  sign_key_central;   /**< Central Signing Information, see @ref ble_gap_sign_info_t.*/
} ser_ble_gap_conn_keyset_t;

/**@brief allocates instance in m_conn_keys_table[] for storage of encryption keys.
 *
 * @param[out]    p_index             pointer to the index of allocated instance
 *
 * @retval NRF_SUCCESS                great success.
 * @retval NRF_ERROR_NO_MEM           no free instance available.
 */
uint32_t conn_ble_gap_sec_context_create(uint32_t *p_index);

/**@brief release instance identified by a connection handle.
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @retval NRF_SUCCESS                Context released.
 * @retval NRF_ERROR_NOT_FOUND        instance with conn_handle not found
 */
uint32_t conn_ble_gap_sec_context_destroy(uint16_t conn_handle);

/**@brief finds index of instance identified by a connection handle in m_conn_keys_table[].
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @param[out]    p_index             Pointer to the index of entry in the context table corresponding to the given conn_handle
 *
 * @retval NRF_SUCCESS                Context table entry found
 * @retval NRF_ERROR_NOT_FOUND        instance with conn_handle not found
 */
uint32_t conn_ble_gap_sec_context_find(uint16_t conn_handle, uint32_t *p_index);
/** @} */

#ifdef __cplusplus
}
#endif
#endif //_CONN_BLE_GAP_SEC_KEYS_H
