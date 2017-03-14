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

// C++ code
#include "adapter.h"
#include "ble_common.h"
#include "adapter_internal.h"

#include <cstring>

// C code
#include "ble_gap.h"
#include "ble_gap_app.h" // Encoder/decoder functions

//TODO: Find a way to support multiple adapters
#include "app_ble_gap_sec_keys.h" // m_app_keys_table and app_ble_gap_sec_context_create 

#include <stdint.h>

uint32_t sd_ble_gap_adv_start(adapter_t *adapter, ble_gap_adv_params_t const * const p_adv_params)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_adv_start_req_enc(
            p_adv_params,
            buffer,
            length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_adv_start_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t sd_ble_gap_device_name_get(adapter_t *adapter, uint8_t * const p_dev_name, uint16_t * const p_len)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_device_name_get_req_enc(
            p_dev_name,
            p_len,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_device_name_get_rsp_dec(
            buffer,
            length,
            p_dev_name,
            p_len,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_appearance_get(adapter_t *adapter, uint16_t * const p_appearance)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_appearance_get_req_enc(
            p_appearance,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_appearance_get_rsp_dec(
            buffer,
            length,
            p_appearance,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_device_name_set(adapter_t *adapter, ble_gap_conn_sec_mode_t const * const p_write_perm,
    uint8_t const * const                 p_dev_name,
    uint16_t                              len)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_device_name_set_req_enc(p_write_perm,
            p_dev_name,
            len,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_device_name_set_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_appearance_set(adapter_t *adapter, uint16_t appearance)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_appearance_set_req_enc(
            appearance,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_appearance_set_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t sd_ble_gap_ppcp_set(adapter_t *adapter, ble_gap_conn_params_t const * const p_conn_params)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_ppcp_set_req_enc(
            p_conn_params,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_ppcp_set_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t sd_ble_gap_adv_data_set(adapter_t *adapter, uint8_t const * const p_data,
    uint8_t               dlen,
    uint8_t const * const p_sr_data,
    uint8_t               srdlen)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_adv_data_set_req_enc(p_data,
            dlen,
            p_sr_data,
            srdlen,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_adv_data_set_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t sd_ble_gap_conn_param_update(adapter_t *adapter, uint16_t conn_handle, ble_gap_conn_params_t const * const p_conn_params)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_conn_param_update_req_enc(
            conn_handle,
            p_conn_params,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_conn_param_update_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t sd_ble_gap_disconnect(adapter_t *adapter, uint16_t conn_handle, uint8_t hci_status_code)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_disconnect_req_enc(
            conn_handle,
            hci_status_code,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_disconnect_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_sec_info_reply(adapter_t *adapter, uint16_t conn_handle,
    ble_gap_enc_info_t  const * p_enc_info,
    ble_gap_irk_t       const * p_id_info,
    ble_gap_sign_info_t const * p_sign_info)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_sec_info_reply_req_enc(
            conn_handle,
            p_enc_info,
            p_id_info,
            p_sign_info,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_sec_info_reply_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t sd_ble_gap_ppcp_get(adapter_t *adapter, ble_gap_conn_params_t * const p_conn_params)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_ppcp_get_req_enc(
            p_conn_params,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_ppcp_get_rsp_dec(
            buffer,
            length,
            p_conn_params,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

#if NRF_SD_BLE_API_VERSION <= 2
uint32_t sd_ble_gap_address_get(adapter_t *adapter, ble_gap_addr_t * const p_addr)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_address_get_req_enc(
            p_addr,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_address_get_rsp_dec(
            buffer,
            length,
            (ble_gap_addr_t *)p_addr,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_address_set(adapter_t *adapter, uint8_t addr_cycle_mode, ble_gap_addr_t const * const p_addr)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_address_set_req_enc(
            addr_cycle_mode,
            p_addr,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_address_set_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}
#endif

#if NRF_SD_BLE_API_VERSION >= 3
uint32_t sd_ble_gap_addr_get(adapter_t *adapter, ble_gap_addr_t * const p_addr)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_addr_get_req_enc(
            p_addr,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_addr_get_rsp_dec(
            buffer,
            length,
            (ble_gap_addr_t *)p_addr,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_addr_set(adapter_t *adapter, ble_gap_addr_t const * const p_addr)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_addr_set_req_enc(
            p_addr,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_addr_set_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_whitelist_set(adapter_t *adapter, ble_gap_addr_t const * const * pp_wl_addrs, uint8_t len)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_whitelist_set_req_enc(
            pp_wl_addrs,
            len,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_whitelist_set_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_device_identities_set(adapter_t *adapter, ble_gap_id_key_t const * const * pp_id_keys, ble_gap_irk_t const * const * pp_local_irks, uint8_t len)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_device_identities_set_req_enc(
            pp_id_keys,
            pp_local_irks,
            len,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_device_identities_set_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_privacy_set(adapter_t *adapter, ble_gap_privacy_params_t const *p_privacy_params)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_privacy_set_req_enc(
            p_privacy_params,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_privacy_set_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);

}

uint32_t sd_ble_gap_privacy_get(adapter_t *adapter, ble_gap_privacy_params_t *p_privacy_params)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_privacy_get_req_enc(
            p_privacy_params,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_privacy_get_rsp_dec(
            buffer,
            length,
            p_privacy_params,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}
#endif

uint32_t sd_ble_gap_adv_stop(adapter_t *adapter)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_adv_stop_req_enc(
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_adv_stop_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t sd_ble_gap_auth_key_reply(adapter_t *adapter, uint16_t conn_handle,
    uint8_t               key_type,
    uint8_t const * const key)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_auth_key_reply_req_enc(
            conn_handle,
            key_type,
            key,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_auth_key_reply_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_authenticate(adapter_t *adapter, uint16_t conn_handle, ble_gap_sec_params_t const * const p_sec_params)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_authenticate_req_enc(
            conn_handle,
            p_sec_params,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_authenticate_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_conn_sec_get(adapter_t *adapter, uint16_t conn_handle, ble_gap_conn_sec_t * const p_conn_sec)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_conn_sec_get_req_enc(
            conn_handle,
            p_conn_sec,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_conn_sec_get_rsp_dec(
            buffer,
            length,
            (ble_gap_conn_sec_t * * const)&p_conn_sec,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_rssi_start(adapter_t *adapter, uint16_t conn_handle, uint8_t threshold_dbm, uint8_t skip_count)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_rssi_start_req_enc(
            conn_handle,
            threshold_dbm,
            skip_count,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_rssi_start_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_rssi_stop(adapter_t *adapter, uint16_t conn_handle)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_rssi_stop_req_enc(
            conn_handle,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_rssi_stop_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_tx_power_set(adapter_t *adapter, int8_t tx_power)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_tx_power_set_req_enc(
            tx_power,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_tx_power_set_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_scan_stop(adapter_t *adapter)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_scan_stop_req_enc(
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_scan_stop_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_connect(adapter_t *adapter,
    ble_gap_addr_t const * const        p_addr,
    ble_gap_scan_params_t const * const p_scan_params,
    ble_gap_conn_params_t const * const p_conn_params)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_connect_req_enc(
            p_addr,
            p_scan_params,
            p_conn_params,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_connect_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t sd_ble_gap_connect_cancel(adapter_t *adapter)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_connect_cancel_req_enc(
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_connect_cancel_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t sd_ble_gap_scan_start(adapter_t *adapter, ble_gap_scan_params_t const * const p_scan_params)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_scan_start_req_enc(
            p_scan_params,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_scan_start_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_encrypt(adapter_t *adapter,
    uint16_t conn_handle,
    ble_gap_master_id_t const * p_master_id,
    ble_gap_enc_info_t  const * p_enc_info)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_encrypt_req_enc(
            conn_handle,
            p_master_id,
            p_enc_info,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_encrypt_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t sd_ble_gap_rssi_get(adapter_t *adapter, uint16_t  conn_handle,
    int8_t  * p_rssi)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_rssi_get_req_enc(
            conn_handle,
            p_rssi,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_rssi_get_rsp_dec(
            buffer,
            length,
            (int8_t *)p_rssi,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t sd_ble_gap_sec_params_reply(adapter_t *adapter,
                                     uint16_t conn_handle,
                                     uint8_t sec_status,
                                     ble_gap_sec_params_t const *p_sec_params,
                                     ble_gap_sec_keyset_t const *p_sec_keyset)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_sec_params_reply_req_enc(
            conn_handle,
            sec_status,
            p_sec_params,
            p_sec_keyset,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_sec_params_reply_rsp_dec(
            buffer,
            length,
            p_sec_keyset,
            result);
    };

    uint32_t err_code = NRF_SUCCESS;
    ser_ble_gap_app_keyset_t *keyset = nullptr;

    // First allocate security context for serialization. We add the a security context for the 
    // connection even if the developer has not provided a p_sec_keyset since the same structure
    // will be used for storing keys received from the peer.
    auto adapterInternal = static_cast<AdapterInternal*>(adapter->internal);
    BLESecurityContext context(adapterInternal->transport);

    err_code = app_ble_gap_sec_context_create(conn_handle, &keyset);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    if (p_sec_keyset)
    {
        std::memcpy(&keyset->keyset, p_sec_keyset, sizeof(ble_gap_sec_keyset_t));
    }

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_lesc_oob_data_get(adapter_t *adapter, uint16_t conn_handle, ble_gap_lesc_p256_pk_t const *p_pk_own, ble_gap_lesc_oob_data_t *p_oobd_own)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_lesc_oob_data_get_req_enc(
            conn_handle,
            p_pk_own,
            p_oobd_own,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_lesc_oob_data_get_rsp_dec(
            buffer,
            length,
            &p_oobd_own,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_lesc_oob_data_set(adapter_t *adapter, uint16_t conn_handle, ble_gap_lesc_oob_data_t const *p_oobd_own, ble_gap_lesc_oob_data_t const *p_oobd_peer)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_lesc_oob_data_set_req_enc(
            conn_handle,
            p_oobd_own,
            p_oobd_peer,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_lesc_oob_data_set_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_lesc_dhkey_reply(adapter_t *adapter, uint16_t conn_handle, ble_gap_lesc_dhkey_t const *p_dhkey)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_lesc_dhkey_reply_req_enc(
            conn_handle,
            p_dhkey,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_lesc_dhkey_reply_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gap_keypress_notify(adapter_t *adapter, uint16_t conn_handle, uint8_t kp_not)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gap_keypress_notify_req_enc(
            conn_handle,
            kp_not,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gap_keypress_notify_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}
