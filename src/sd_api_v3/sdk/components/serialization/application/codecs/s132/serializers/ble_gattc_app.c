/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_gattc_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gattc_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"
#include "ble_struct_serialization.h"
#include "ble_types.h"

uint32_t ble_gattc_attr_info_discover_req_enc(uint16_t                               conn_handle,
                                              ble_gattc_handle_range_t const * const p_handle_range,
                                              uint8_t * const                        p_buf,
                                              uint32_t *                             p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_GATTC_ATTR_INFO_DISCOVER);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_COND(p_handle_range, ble_gattc_handle_range_t_enc);

    SER_REQ_ENC_END;
}


uint32_t ble_gattc_attr_info_discover_rsp_dec(uint8_t const * const p_buf,
                                              uint32_t              packet_len,
                                              uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_GATTC_ATTR_INFO_DISCOVER);
}

uint32_t ble_gattc_char_value_by_uuid_read_req_enc(uint16_t                              conn_handle,
                                                   ble_uuid_t const * const               p_uuid,
                                                   ble_gattc_handle_range_t const * const p_handle_range,
                                                   uint8_t * const                        p_buf,
                                                   uint32_t * const                       p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_GATTC_CHAR_VALUE_BY_UUID_READ);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_COND(p_uuid, ble_uuid_t_enc);
    SER_PUSH_COND(p_handle_range, ble_gattc_handle_range_t_enc);

    SER_REQ_ENC_END;
}

uint32_t ble_gattc_char_value_by_uuid_read_rsp_dec(uint8_t const * const p_buf,
                                                   uint32_t              packet_len,
                                                   uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_GATTC_CHAR_VALUE_BY_UUID_READ);
}

uint32_t ble_gattc_char_values_read_req_enc(uint16_t               conn_handle,
                                            uint16_t const * const p_handles,
                                            uint16_t               handle_count,
                                            uint8_t * const        p_buf,
                                            uint32_t *             p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_GATTC_CHAR_VALUES_READ);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_len16data16(p_handles, handle_count);

    SER_REQ_ENC_END;
}


uint32_t ble_gattc_char_values_read_rsp_dec(uint8_t const * const p_buf,
                                            uint32_t              packet_len,
                                            uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_GATTC_CHAR_VALUES_READ);
}

uint32_t ble_gattc_characteristics_discover_req_enc(
    uint16_t                               conn_handle,
    ble_gattc_handle_range_t const * const p_handle_range,
    uint8_t * const                        p_buf,
    uint32_t *                             p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_GATTC_CHARACTERISTICS_DISCOVER);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_COND(p_handle_range, ble_gattc_handle_range_t_enc);

    SER_REQ_ENC_END;
}


uint32_t ble_gattc_characteristics_discover_rsp_dec(uint8_t const * const p_buf,
                                                    uint32_t              packet_len,
                                                    uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_GATTC_CHARACTERISTICS_DISCOVER);
}

uint32_t ble_gattc_descriptors_discover_req_enc(
    uint16_t                               conn_handle,
    ble_gattc_handle_range_t const * const p_handle_range,
    uint8_t * const                        p_buf,
    uint32_t *                             p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_GATTC_DESCRIPTORS_DISCOVER);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_COND(p_handle_range, ble_gattc_handle_range_t_enc);

    SER_REQ_ENC_END;
}


uint32_t ble_gattc_descriptors_discover_rsp_dec(uint8_t const * const p_buf,
                                                uint32_t              packet_len,
                                                uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_GATTC_DESCRIPTORS_DISCOVER);
}

uint32_t ble_gattc_hv_confirm_req_enc(uint16_t         conn_handle,
                                      uint16_t         handle,
                                      uint8_t * const  p_buf,
                                      uint32_t * const p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_GATTC_HV_CONFIRM);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_uint16(&handle);

    SER_REQ_ENC_END;
}

uint32_t ble_gattc_hv_confirm_rsp_dec(uint8_t const * const p_buf,
                                      uint32_t              packet_len,
                                      uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_GATTC_HV_CONFIRM);
}

uint32_t ble_gattc_primary_services_discover_req_enc(uint16_t                 conn_handle,
                                                     uint16_t                 start_handle,
                                                     ble_uuid_t const * const p_srvc_uuid,
                                                     uint8_t * const          p_buf,
                                                     uint32_t *               p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_GATTC_PRIMARY_SERVICES_DISCOVER);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_uint16(&start_handle);
    SER_PUSH_COND(p_srvc_uuid, ble_uuid_t_enc);

    SER_REQ_ENC_END;
}


uint32_t ble_gattc_primary_services_discover_rsp_dec(uint8_t const * const p_buf,
                                                     uint32_t              packet_len,
                                                     uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_GATTC_PRIMARY_SERVICES_DISCOVER);
}

uint32_t ble_gattc_read_req_enc(uint16_t        conn_handle,
                                uint16_t        handle,
                                uint16_t        offset,
                                uint8_t * const p_buf,
                                uint32_t *      p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_GATTC_READ);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_uint16(&handle);
    SER_PUSH_uint16(&offset);

    SER_REQ_ENC_END;
}


uint32_t ble_gattc_read_rsp_dec(uint8_t const * const p_buf,
                                uint32_t              packet_len,
                                uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_GATTC_READ);
}

uint32_t ble_gattc_relationships_discover_req_enc(
    uint16_t                               conn_handle,
    ble_gattc_handle_range_t const * const p_handle_range,
    uint8_t * const                        p_buf,
    uint32_t *                             p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_GATTC_RELATIONSHIPS_DISCOVER);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_COND(p_handle_range, ble_gattc_handle_range_t_enc);

    SER_REQ_ENC_END;
}


uint32_t ble_gattc_relationships_discover_rsp_dec(uint8_t const * const p_buf,
                                                  uint32_t              packet_len,
                                                  uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_GATTC_RELATIONSHIPS_DISCOVER);
}

uint32_t ble_gattc_write_req_enc(uint16_t                               conn_handle,
                                 ble_gattc_write_params_t const * const p_write_params,
                                 uint8_t * const                        p_buf,
                                 uint32_t *                             p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_GATTC_WRITE);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_COND(p_write_params, ble_gattc_write_params_t_enc);

    SER_REQ_ENC_END;
}


uint32_t ble_gattc_write_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_GATTC_WRITE);
}

uint32_t ble_gattc_exchange_mtu_request_req_enc(uint16_t          conn_handle,
                                                uint16_t          client_rx_mtu,
                                                uint8_t * const   p_buf,
                                                uint32_t * const  p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_GATTC_EXCHANGE_MTU_REQUEST);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_uint16(&client_rx_mtu);

    SER_REQ_ENC_END;
}


uint32_t ble_gattc_exchange_mtu_request_rsp_dec(uint8_t const * const p_buf,
                                                uint32_t              packet_len,
                                                uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_GATTC_EXCHANGE_MTU_REQUEST);
}
