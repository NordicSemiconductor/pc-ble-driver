/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

