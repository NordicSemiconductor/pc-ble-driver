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
#include "ble_gattc.h"
#include "ble_gattc_app.h" // Encoder/decoder functions

#include "ble_common.h"

uint32_t sd_ble_gattc_primary_services_discover(adapter_t *adapter, uint16_t conn_handle, uint16_t start_handle, ble_uuid_t const *p_srvc_uuid)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gattc_primary_services_discover_req_enc(conn_handle, start_handle, p_srvc_uuid, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gattc_primary_services_discover_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gattc_relationships_discover(adapter_t *adapter, uint16_t conn_handle, ble_gattc_handle_range_t const *p_handle_range)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gattc_relationships_discover_req_enc(conn_handle, p_handle_range, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gattc_relationships_discover_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gattc_characteristics_discover(adapter_t *adapter, uint16_t conn_handle, ble_gattc_handle_range_t const *p_handle_range)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gattc_characteristics_discover_req_enc(conn_handle, p_handle_range, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gattc_characteristics_discover_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gattc_descriptors_discover(adapter_t *adapter, uint16_t conn_handle, ble_gattc_handle_range_t const *p_handle_range)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gattc_descriptors_discover_req_enc(conn_handle, p_handle_range, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gattc_descriptors_discover_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gattc_char_value_by_uuid_read(adapter_t *adapter, uint16_t conn_handle, ble_uuid_t const *p_uuid, ble_gattc_handle_range_t const *p_handle_range)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gattc_char_value_by_uuid_read_req_enc(conn_handle, p_uuid, p_handle_range, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gattc_char_value_by_uuid_read_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gattc_read(adapter_t *adapter, uint16_t conn_handle, uint16_t handle, uint16_t offset)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gattc_read_req_enc(conn_handle, handle, offset, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gattc_read_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gattc_char_values_read(adapter_t *adapter, uint16_t conn_handle, uint16_t const *p_handles, uint16_t handle_count)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gattc_char_values_read_req_enc(conn_handle, p_handles, handle_count, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gattc_char_values_read_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gattc_write(adapter_t *adapter, uint16_t conn_handle, ble_gattc_write_params_t const *p_write_params)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gattc_write_req_enc(conn_handle, p_write_params, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gattc_write_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gattc_hv_confirm(adapter_t *adapter, uint16_t conn_handle, uint16_t handle)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gattc_hv_confirm_req_enc(conn_handle, handle, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gattc_hv_confirm_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_gattc_attr_info_discover(adapter_t *adapter, uint16_t conn_handle, ble_gattc_handle_range_t const * p_handle_range)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gattc_attr_info_discover_req_enc(conn_handle, p_handle_range, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gattc_attr_info_discover_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

#if NRF_SD_BLE_API_VERSION >= 3
uint32_t sd_ble_gattc_exchange_mtu_request(adapter_t *adapter, uint16_t conn_handle, uint16_t client_rx_mtu)
{
    encode_function_t encode_function = [&] (uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_gattc_exchange_mtu_request_req_enc(conn_handle, client_rx_mtu, buffer, length);
    };

    decode_function_t decode_function = [&] (uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_gattc_exchange_mtu_request_rsp_dec(buffer, length, result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}
#endif
