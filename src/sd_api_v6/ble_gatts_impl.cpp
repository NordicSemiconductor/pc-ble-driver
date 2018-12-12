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

#include <stdint.h>

// C++ code
#include "adapter.h"
#include "adapter_internal.h"
#include "transport.h"


// C code
#include "ble_gatts.h"
#include "ble_gatts_app.h" // Encoder/decoder functions

#include "ble_common.h"

uint32_t sd_ble_gatts_service_add(adapter_t *adapter, uint8_t type, ble_uuid_t const *p_uuid, uint16_t *p_handle)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_service_add_req_enc(type, p_uuid, p_handle, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gatts_service_add_rsp_dec(buffer, length, p_handle, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_include_add(adapter_t *adapter, uint16_t service_handle, uint16_t inc_srvc_handle, uint16_t *p_include_handle)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_include_add_req_enc(service_handle, inc_srvc_handle, p_include_handle, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gatts_include_add_rsp_dec(buffer, length, p_include_handle, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_characteristic_add(adapter_t *adapter, uint16_t service_handle, ble_gatts_char_md_t const *p_char_md, ble_gatts_attr_t const *p_attr_char_value, ble_gatts_char_handles_t *p_handles)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_characteristic_add_req_enc(service_handle, p_char_md, p_attr_char_value, p_handles, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        uint16_t * handles = &p_handles->value_handle;
        return ble_gatts_characteristic_add_rsp_dec(buffer, length, &handles, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_descriptor_add(adapter_t *adapter, uint16_t char_handle, ble_gatts_attr_t const *p_attr, uint16_t *p_handle)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_descriptor_add_req_enc(char_handle, p_attr, p_handle, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gatts_descriptor_add_rsp_dec(buffer, length, p_handle, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_value_set(adapter_t *adapter, uint16_t conn_handle, uint16_t handle, ble_gatts_value_t *p_value)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_value_set_req_enc(conn_handle, handle, p_value, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gatts_value_set_rsp_dec(buffer, length, p_value, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_value_get(adapter_t *adapter, uint16_t conn_handle, uint16_t handle, ble_gatts_value_t *p_value)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_value_get_req_enc(conn_handle, handle, p_value, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gatts_value_get_rsp_dec(buffer, length, p_value, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_hvx(adapter_t *adapter, uint16_t conn_handle, ble_gatts_hvx_params_t const *p_hvx_params)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_hvx_req_enc(conn_handle, p_hvx_params, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        uint16_t *out_length = p_hvx_params->p_len;
        return ble_gatts_hvx_rsp_dec(buffer, length, result, &out_length);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_service_changed(adapter_t *adapter, uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_service_changed_req_enc(conn_handle, start_handle, end_handle, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gatts_service_changed_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_rw_authorize_reply(adapter_t *adapter, uint16_t conn_handle, ble_gatts_rw_authorize_reply_params_t const *p_rw_authorize_reply_params)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_rw_authorize_reply_req_enc(conn_handle, p_rw_authorize_reply_params, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gatts_rw_authorize_reply_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_sys_attr_set(adapter_t *adapter, uint16_t conn_handle, uint8_t const *p_sys_attr_data, uint16_t len, uint32_t flags)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_sys_attr_set_req_enc(conn_handle, p_sys_attr_data, len, flags, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gatts_sys_attr_set_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_sys_attr_get(adapter_t *adapter, uint16_t conn_handle, uint8_t *p_sys_attr_data, uint16_t *p_len, uint32_t flags)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_sys_attr_get_req_enc(conn_handle, p_sys_attr_data, p_len, flags, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
#if NRF_SD_BLE_API_VERSION == 2
        return ble_gatts_sys_attr_get_rsp_dec(buffer, length, p_sys_attr_data, p_len, result);
#else
        return ble_gatts_sys_attr_get_rsp_dec(buffer, length, &p_sys_attr_data, &p_len, result);
#endif
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_initial_user_handle_get(adapter_t *adapter, uint16_t *p_handle)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_initial_user_handle_get_req_enc(p_handle, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gatts_initial_user_handle_get_rsp_dec(buffer, length, &p_handle, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gatts_attr_get(adapter_t *adapter, uint16_t handle, ble_uuid_t * p_uuid, ble_gatts_attr_md_t * p_md)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_attr_get_req_enc(handle, p_uuid, p_md, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gatts_attr_get_rsp_dec(buffer, length, &p_uuid, &p_md, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}     

#if NRF_SD_BLE_API_VERSION >= 3
uint32_t sd_ble_gatts_exchange_mtu_reply(adapter_t *adapter, uint16_t conn_handle, uint16_t server_rx_mtu)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gatts_exchange_mtu_reply_req_enc(conn_handle, server_rx_mtu, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gatts_exchange_mtu_reply_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}
#endif
