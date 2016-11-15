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
#include "ble_app.h"
#include "ble_evt_app.h"
#include "ble_gap_evt_app.h"
#include "ble_gattc_evt_app.h"
#include "ble_gatts_evt_app.h"
#include "ble_l2cap_evt_app.h"
#include "app_util.h"

uint32_t ble_event_dec(uint8_t const * const p_buf,
                       uint32_t              packet_len,
                       ble_evt_t * const     p_event,
                       uint32_t * const      p_event_len)
{
    uint32_t err_code;
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);
    SER_ASSERT_LENGTH_LEQ(SER_EVT_HEADER_SIZE, packet_len);
    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_LENGTH_LEQ(sizeof (ble_evt_hdr_t), *p_event_len);
    *p_event_len -= sizeof (ble_evt_hdr_t);

    const uint16_t  event_id       = uint16_decode(&p_buf[SER_EVT_ID_POS]);
    const uint8_t * p_sub_buffer   = &p_buf[SER_EVT_HEADER_SIZE];
    const uint32_t  sub_packet_len = packet_len - SER_EVT_HEADER_SIZE;

    uint32_t (*fp_event_decoder)(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 ble_evt_t * const     p_event,
                                 uint32_t * const      p_event_len) = NULL;

    switch (event_id)
    {
        case BLE_EVT_TX_COMPLETE:
            fp_event_decoder = ble_evt_tx_complete_dec;
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            fp_event_decoder = ble_evt_user_mem_request_dec;
            break;

        case BLE_EVT_USER_MEM_RELEASE:
            fp_event_decoder = ble_evt_user_mem_release_dec;
            break;

        case BLE_EVT_DATA_LENGTH_CHANGED:
            fp_event_decoder = ble_evt_data_length_changed_dec;
            break;

        case BLE_GAP_EVT_PASSKEY_DISPLAY:
            fp_event_decoder = ble_gap_evt_passkey_display_dec;
            break;

        case BLE_GAP_EVT_AUTH_KEY_REQUEST:
            fp_event_decoder = ble_gap_evt_auth_key_request_dec;
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
            fp_event_decoder = ble_gap_evt_conn_param_update_dec;
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            fp_event_decoder = ble_gap_evt_conn_param_update_request_dec;
            break;

        case BLE_GAP_EVT_CONN_SEC_UPDATE:
            fp_event_decoder = ble_gap_evt_conn_sec_update_dec;
            break;

        case BLE_GAP_EVT_CONNECTED:
            fp_event_decoder = ble_gap_evt_connected_dec;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            fp_event_decoder = ble_gap_evt_disconnected_dec;
            break;

        case BLE_GAP_EVT_TIMEOUT:
            fp_event_decoder = ble_gap_evt_timeout_dec;
            break;

        case BLE_GAP_EVT_RSSI_CHANGED:
            fp_event_decoder = ble_gap_evt_rssi_changed_dec;
            break;

        case BLE_GAP_EVT_SEC_INFO_REQUEST:
            fp_event_decoder = ble_gap_evt_sec_info_request_dec;
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            fp_event_decoder = ble_gap_evt_sec_params_request_dec;
            break;

        case BLE_GAP_EVT_AUTH_STATUS:
            fp_event_decoder = ble_gap_evt_auth_status_dec;
            break;

        case BLE_GAP_EVT_SEC_REQUEST:
            fp_event_decoder = ble_gap_evt_sec_request_dec;
            break;

        case BLE_GAP_EVT_KEY_PRESSED:
            fp_event_decoder = ble_gap_evt_key_pressed_dec;
            break;

        case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
            fp_event_decoder = ble_gap_evt_lesc_dhkey_request_dec;
            break;

        case BLE_GATTC_EVT_CHAR_DISC_RSP:
            fp_event_decoder = ble_gattc_evt_char_disc_rsp_dec;
            break;

        case BLE_GATTC_EVT_CHAR_VAL_BY_UUID_READ_RSP:
            fp_event_decoder = ble_gattc_evt_char_val_by_uuid_read_rsp_dec;
            break;

        case BLE_GATTC_EVT_DESC_DISC_RSP:
            fp_event_decoder = ble_gattc_evt_desc_disc_rsp_dec;
            break;

        case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
            fp_event_decoder = ble_gattc_evt_prim_srvc_disc_rsp_dec;
            break;

        case BLE_GATTC_EVT_READ_RSP:
            fp_event_decoder = ble_gattc_evt_read_rsp_dec;
            break;

        case BLE_GATTC_EVT_HVX:
            fp_event_decoder = ble_gattc_evt_hvx_dec;
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            fp_event_decoder = ble_gattc_evt_timeout_dec;
            break;

        case BLE_GATTC_EVT_WRITE_RSP:
            fp_event_decoder = ble_gattc_evt_write_rsp_dec;
            break;

        case BLE_GATTC_EVT_CHAR_VALS_READ_RSP:
            fp_event_decoder = ble_gattc_evt_char_vals_read_rsp_dec;
            break;

        case BLE_GATTC_EVT_REL_DISC_RSP:
            fp_event_decoder = ble_gattc_evt_rel_disc_rsp_dec;
            break;

        case BLE_GATTC_EVT_ATTR_INFO_DISC_RSP:
            fp_event_decoder = ble_gattc_evt_attr_info_disc_rsp_dec;
            break;

        case BLE_GATTC_EVT_EXCHANGE_MTU_RSP:
            fp_event_decoder = ble_gattc_evt_exchange_mtu_rsp_dec;
            break;

        case BLE_GATTS_EVT_WRITE:
            fp_event_decoder = ble_gatts_evt_write_dec;
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            fp_event_decoder = ble_gatts_evt_timeout_dec;
            break;

        case BLE_GATTS_EVT_SC_CONFIRM:
            fp_event_decoder = ble_gatts_evt_sc_confirm_dec;
            break;

        case BLE_GATTS_EVT_HVC:
            fp_event_decoder = ble_gatts_evt_hvc_dec;
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            fp_event_decoder = ble_gatts_evt_sys_attr_missing_dec;
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            fp_event_decoder = ble_gatts_evt_rw_authorize_request_dec;
            break;

        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            fp_event_decoder = ble_gatts_evt_exchange_mtu_request_dec;
            break;

        case BLE_L2CAP_EVT_RX:
            fp_event_decoder = ble_l2cap_evt_rx_dec;
            break;

        case BLE_GAP_EVT_ADV_REPORT:
            fp_event_decoder = ble_gap_evt_adv_report_dec;
            break;

        case BLE_GAP_EVT_SCAN_REQ_REPORT:
            fp_event_decoder = ble_gap_evt_scan_req_report_dec;
            break;
        default:
            break;
    }
    
    if (fp_event_decoder)
    {
        err_code = fp_event_decoder(p_sub_buffer, sub_packet_len, p_event, p_event_len);
    }
    else
    {
        err_code = NRF_ERROR_NOT_FOUND;
    }

    *p_event_len += offsetof(ble_evt_t, evt);
    p_event->header.evt_id  = (err_code == NRF_SUCCESS) ? event_id : 0;
    p_event->header.evt_len = (err_code == NRF_SUCCESS) ? (uint16_t)*p_event_len : 0;

    return err_code;
}
