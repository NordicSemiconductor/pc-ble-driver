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

#include "ble_gattc_evt_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gattc_struct_serialization.h"
#include "app_util.h"

uint32_t ble_gattc_evt_attr_info_disc_rsp_dec(uint8_t const * const p_buf,
                                              uint32_t              packet_len,
                                              ble_evt_t * const     p_event,
                                              uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_ATTR_INFO_DISC_RSP, gattc, attr_info_disc_rsp);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gattc_evt.params.attr_info_disc_rsp,
                            ble_gattc_evt_attr_info_disc_rsp_t_dec);

    SER_EVT_DEC_END;
}

uint32_t ble_gattc_evt_char_disc_rsp_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         ble_evt_t * const     p_event,
                                         uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_CHAR_DISC_RSP, gattc, char_disc_rsp);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gattc_evt.params.char_disc_rsp,
                            ble_gattc_evt_char_disc_rsp_t_dec);

    SER_EVT_DEC_END;
}



uint32_t ble_gattc_evt_char_val_by_uuid_read_rsp_dec(uint8_t const * const p_buf,
                                                     uint32_t              packet_len,
                                                     ble_evt_t * const     p_event,
                                                     uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_CHAR_VAL_BY_UUID_READ_RSP, gattc, char_val_by_uuid_read_rsp);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gattc_evt.params.char_val_by_uuid_read_rsp,
                            ble_gattc_evt_char_val_by_uuid_read_rsp_t_dec);

    SER_EVT_DEC_END;
}


uint32_t ble_gattc_evt_char_vals_read_rsp_dec(uint8_t const * const p_buf,
                                              uint32_t              packet_len,
                                              ble_evt_t * const     p_event,
                                              uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_CHAR_VALS_READ_RSP, gattc, char_vals_read_rsp);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gattc_evt.params.char_vals_read_rsp,
                            ble_gattc_evt_char_vals_read_rsp_t_dec);

    SER_EVT_DEC_END;
}


uint32_t ble_gattc_evt_desc_disc_rsp_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         ble_evt_t * const     p_event,
                                         uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_DESC_DISC_RSP, gattc, desc_disc_rsp);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gattc_evt.params.desc_disc_rsp,
                            ble_gattc_evt_desc_disc_rsp_t_dec);

    SER_EVT_DEC_END;
}

uint32_t ble_gattc_evt_hvx_dec(uint8_t const * const p_buf,
                               uint32_t              packet_len,
                               ble_evt_t * const     p_event,
                               uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_HVX, gattc, hvx);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gattc_evt.params.hvx,
                            ble_gattc_evt_hvx_t_dec);

    SER_EVT_DEC_END;
}


uint32_t ble_gattc_evt_prim_srvc_disc_rsp_dec(uint8_t const * const p_buf,
                                              uint32_t              packet_len,
                                              ble_evt_t * const     p_event,
                                              uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP, gattc, prim_srvc_disc_rsp);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gattc_evt.params.prim_srvc_disc_rsp,
                            ble_gattc_evt_prim_srvc_disc_rsp_t_dec);

    SER_EVT_DEC_END;
}

uint32_t ble_gattc_evt_read_rsp_dec(uint8_t const * const p_buf,
                                    uint32_t              packet_len,
                                    ble_evt_t * const     p_event,
                                    uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_READ_RSP, gattc, read_rsp);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gattc_evt.params.read_rsp,
                            ble_gattc_evt_read_rsp_t_dec);

    SER_EVT_DEC_END;
}


#define BLE_GATTC_EVT_REL_DISC_RSP_COUNT_POSITION 6


uint32_t ble_gattc_evt_rel_disc_rsp_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        ble_evt_t * const     p_event,
                                        uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_READ_RSP, gattc, rel_disc_rsp);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gattc_evt.params.rel_disc_rsp,
                            ble_gattc_evt_rel_disc_rsp_t_dec);

    SER_EVT_DEC_END;
}


uint32_t ble_gattc_evt_timeout_dec(uint8_t const * const p_buf,
                                   uint32_t              packet_len,
                                   ble_evt_t * const     p_event,
                                   uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_TIMEOUT, gattc, timeout);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD(&p_event->evt.gattc_evt.params.timeout,
                   ble_gattc_evt_timeout_t_dec);

    SER_EVT_DEC_END;
}

uint32_t ble_gattc_evt_write_rsp_dec(uint8_t const * const p_buf,
                                     uint32_t              packet_len,
                                     ble_evt_t * const     p_event,
                                     uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_WRITE_RSP, gattc, write_rsp);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gattc_evt.params.write_rsp,
                            ble_gattc_evt_write_rsp_t_dec);

    SER_EVT_DEC_END;
}

uint32_t ble_gattc_evt_exchange_mtu_rsp_dec(uint8_t const * const p_buf,
                                            uint32_t              packet_len,
                                            ble_evt_t * const     p_event,
                                            uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTC_EVT_EXCHANGE_MTU_RSP, gattc, exchange_mtu_rsp);

    SER_PULL_uint16(&p_event->evt.gattc_evt.conn_handle);
    SER_PULL_uint16(&p_event->evt.gattc_evt.gatt_status);
    SER_PULL_uint16(&p_event->evt.gattc_evt.error_handle);
    SER_PULL_FIELD(&p_event->evt.gattc_evt.params.exchange_mtu_rsp,
                            ble_gattc_evt_exchange_mtu_rsp_t_dec);

    SER_EVT_DEC_END;
}
