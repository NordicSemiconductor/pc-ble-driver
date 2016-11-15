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

#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "ble_gatt_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"
#include "ble_gatt.h"
#include <string.h>

uint32_t ble_gatt_char_props_t_enc(void const * const p_void_struct,
                              uint8_t * const    p_buf,
                              uint32_t           buf_len,
                              uint32_t * const   p_index)
{
    SER_STRUCT_ENC_BEGIN(ble_gatt_char_props_t);

    uint8_t ser_data = (p_struct->broadcast         & 0x01)
                       | ((p_struct->read           & 0x01) << 1)
                       | ((p_struct->write_wo_resp  & 0x01) << 2)
                       | ((p_struct->write          & 0x01) << 3)
                       | ((p_struct->notify         & 0x01) << 4)
                       | ((p_struct->indicate       & 0x01) << 5)
                       | ((p_struct->auth_signed_wr & 0x01) << 6);
    SER_PUSH_uint8(&ser_data);

    SER_STRUCT_ENC_END;
}

uint32_t ble_gatt_char_props_t_dec(uint8_t const * const p_buf,
                              uint32_t              buf_len,
                              uint32_t * const      p_index,
                              void * const          p_void_struct)
{
    SER_STRUCT_DEC_BEGIN(ble_gatt_char_props_t);

    uint8_t ser_data;
    SER_PULL_uint8(&ser_data);
    p_struct->broadcast      = ser_data & 0x01;
    p_struct->read           = (ser_data >> 1) & 0x01;
    p_struct->write_wo_resp  = (ser_data >> 2) & 0x01;
    p_struct->write          = (ser_data >> 3) & 0x01;
    p_struct->notify         = (ser_data >> 4) & 0x01;
    p_struct->indicate       = (ser_data >> 5) & 0x01;
    p_struct->auth_signed_wr = (ser_data >> 6) & 0x01;

    SER_STRUCT_DEC_END;
}

uint32_t ble_gatt_char_ext_props_t_enc(void const * const p_void_struct,
                              uint8_t * const    p_buf,
                              uint32_t           buf_len,
                              uint32_t * const   p_index)
{
    SER_STRUCT_ENC_BEGIN(ble_gatt_char_ext_props_t);

    uint8_t ser_data = (p_struct->reliable_wr & 0x01)
                       | ((p_struct->wr_aux   & 0x01) << 1);
    SER_PUSH_uint8(&ser_data);

    SER_STRUCT_ENC_END;
}

uint32_t ble_gatt_char_ext_props_t_dec(uint8_t const * const p_buf,
                              uint32_t              buf_len,
                              uint32_t * const      p_index,
                              void * const          p_void_struct)
{
    SER_STRUCT_DEC_BEGIN(ble_gatt_char_ext_props_t);

    uint8_t ser_data;
    SER_PULL_uint8(&ser_data);
    p_struct->reliable_wr = ser_data & 0x01;
    p_struct->wr_aux      = (ser_data >> 1) & 0x01;

    SER_STRUCT_DEC_END;
}

uint32_t ble_gatt_enable_params_t_enc(void const * const p_void_struct,
                                      uint8_t * const    p_buf,
                                      uint32_t           buf_len,
                                      uint32_t * const   p_index)
{
    SER_STRUCT_ENC_BEGIN(ble_gatt_enable_params_t);
    SER_PUSH_uint16(&p_struct->att_mtu);
    SER_STRUCT_ENC_END;
}


uint32_t ble_gatt_enable_params_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_struct)
{
    SER_STRUCT_DEC_BEGIN(ble_gatt_enable_params_t);
    SER_PULL_uint16(&p_struct->att_mtu);
    SER_STRUCT_DEC_END;
}
