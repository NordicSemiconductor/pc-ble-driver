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

#include "adapter.h"
#include "ble_common.h"

#include "ble.h"
#include "ble_app.h"
#include "conn_systemreset.h"

#include <stdint.h>

uint32_t sd_ble_uuid_encode(adapter_t* adapter, ble_uuid_t const * const p_uuid,
    uint8_t * const          p_uuid_le_len,
    uint8_t * const          p_uuid_le)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_uuid_encode_req_enc(
            p_uuid,
            p_uuid_le_len,
            p_uuid_le,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_uuid_encode_rsp_dec(
            buffer,
            length,
            p_uuid_le_len,
            p_uuid_le,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}
 // ble_tx_packet_count_get_req_enc
uint32_t sd_ble_tx_packet_count_get(adapter_t *adapter, uint16_t conn_handle, uint8_t * p_count)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_tx_packet_count_get_req_enc(
            conn_handle,
            p_count,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_tx_packet_count_get_rsp_dec(
            buffer,
            length,
            reinterpret_cast<uint8_t * *>(&p_count),
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_uuid_vs_add(adapter_t *adapter, ble_uuid128_t const * const p_vs_uuid, uint8_t * const p_uuid_type)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_uuid_vs_add_req_enc(
            p_vs_uuid,
            p_uuid_type,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_uuid_vs_add_rsp_dec(
            buffer,
            length,
            const_cast<uint8_t**>(&p_uuid_type),
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_uuid_decode(adapter_t *adapter, uint8_t uuid_le_len, uint8_t const * const p_uuid_le, ble_uuid_t * const p_uuid)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_uuid_decode_req_enc(
            uuid_le_len,
            p_uuid_le,
            p_uuid,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_uuid_decode_rsp_dec(
            buffer,
            length,
            const_cast<ble_uuid_t * *>(&p_uuid),
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_version_get(adapter_t *adapter, ble_version_t * p_version)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_version_get_req_enc(
            p_version,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_version_get_rsp_dec(
            buffer,
            length,
            p_version,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_opt_get(adapter_t *adapter, uint32_t opt_id, ble_opt_t *p_opt)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_opt_get_req_enc(
            opt_id,
            p_opt,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_opt_get_rsp_dec(
            buffer,
            length,
            &opt_id,
            p_opt,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_opt_set(adapter_t *adapter, uint32_t opt_id, ble_opt_t const *p_opt)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_opt_set_req_enc(
            opt_id,
            p_opt,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_opt_set_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_enable(adapter_t *adapter, ble_enable_params_t * p_params, uint32_t *p_app_ram_base)
{
    (void)p_app_ram_base;

    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_enable_req_enc(
            p_params,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_enable_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}

uint32_t sd_ble_user_mem_reply(adapter_t *adapter, uint16_t conn_handle, ble_user_mem_block_t const *p_block)
{
    if (p_block != nullptr)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return ble_user_mem_reply_req_enc(
            conn_handle,
            p_block,
            buffer,
            length);
    };

    decode_function_t decode_function = [&](uint8_t *buffer, uint32_t length, uint32_t *result) -> uint32_t {
        return ble_user_mem_reply_rsp_dec(
            buffer,
            length,
            result);
    };

    return encode_decode(adapter, encode_function, decode_function);
}


uint32_t conn_systemreset(adapter_t *adapter)
{
    encode_function_t encode_function = [&](uint8_t *buffer, uint32_t *length) -> uint32_t {
        return 0;
    };

    return encode_decode(adapter, encode_function, nullptr);
}
