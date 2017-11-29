/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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
#include "conn_systemreset_app.h"

uint32_t conn_systemreset_enc(uint8_t * const  p_buf,
                              uint32_t * const p_buf_len)
{
    SER_REQ_ENC_BEGIN(CONN_SYSTEMRESET);
    SER_REQ_ENC_END;
}
