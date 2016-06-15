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
        return ble_gatts_sys_attr_get_rsp_dec(buffer, length, p_sys_attr_data, p_len, result);
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
