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
#include "ble_struct_serialization.h"
#include "app_util.h"
#include "ble_l2cap_evt_app.h"

uint32_t ble_l2cap_evt_rx_dec(uint8_t const * const p_buf,
                              uint32_t              packet_len,
                              ble_evt_t * const     p_event,
                              uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_L2CAP_EVT_RX, l2cap, rx);

    SER_PULL_uint16(&p_event->evt.l2cap_evt.conn_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.l2cap_evt.params.rx, ble_l2cap_evt_rx_t_dec);

    SER_EVT_DEC_END;
}
