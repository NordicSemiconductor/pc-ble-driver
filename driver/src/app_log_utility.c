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

#include "app_log_utility.h"
#include "app_log.h"
#include "ble.h"
#include "ble_gap.h"
#include "ble_gattc.h"
#include "ble_gatts.h"

static void fill_in_command_name(app_log_severity_t severity, const char * message,
                                 const uint16_t command_number)
{
    const char * command_name;

    switch (command_number)
    {
    /* GAP */
    case SD_BLE_GAP_ADDRESS_GET:
        command_name = "sd_ble_gap_address_get";
        break;

    case SD_BLE_GAP_ADDRESS_SET:
        command_name = "sd_ble_gap_address_set";
        break;

    case SD_BLE_GAP_ADV_DATA_SET:
        command_name = "sd_ble_gap_adv_data_set";
        break;

    case SD_BLE_GAP_ADV_START:
        command_name = "sd_ble_gap_adv_start";
        break;

    case SD_BLE_GAP_ADV_STOP:
        command_name = "sd_ble_gap_adv_stop";
        break;

    case SD_BLE_GAP_APPEARANCE_GET:
        command_name = "sd_ble_gap_appearance_get";
        break;

    case SD_BLE_GAP_APPEARANCE_SET:
        command_name = "sd_ble_gap_appearance_set";
        break;

    case SD_BLE_GAP_AUTH_KEY_REPLY:
        command_name = "sd_ble_gap_auth_key_reply";
        break;

    case SD_BLE_GAP_AUTHENTICATE:
        command_name = "sd_ble_gap_authenticate";
        break;

    case SD_BLE_GAP_CONN_PARAM_UPDATE:
        command_name = "sd_ble_gap_conn_param_update";
        break;

    case SD_BLE_GAP_CONN_SEC_GET:
        command_name = "sd_ble_gap_conn_sec_get";
        break;

    case SD_BLE_GAP_DEVICE_NAME_GET:
        command_name = "sd_ble_gap_device_name_get";
        break;

    case SD_BLE_GAP_DEVICE_NAME_SET:
        command_name = "sd_ble_gap_device_name_set";
        break;

    case SD_BLE_GAP_DISCONNECT:
        command_name = "sd_ble_gap_disconnect";
        break;

    case SD_BLE_GAP_PPCP_GET:
        command_name = "sd_ble_gap_ppcp_get";
        break;

    case SD_BLE_GAP_PPCP_SET:
        command_name = "sd_ble_gap_ppcp_set";
        break;

    case SD_BLE_GAP_RSSI_START:
        command_name = "sd_ble_gap_rssi_start";
        break;

    case SD_BLE_GAP_RSSI_STOP:
        command_name = "sd_ble_gap_rssi_stop";
        break;

    case SD_BLE_GAP_SEC_INFO_REPLY:
        command_name = "sd_ble_gap_sec_info_reply";
        break;

    case SD_BLE_GAP_SEC_PARAMS_REPLY:
        command_name = "sd_ble_gap_sec_params_reply";
        break;

    case SD_BLE_GAP_TX_POWER_SET:
        command_name = "sd_ble_gap_tx_power_set";
        break;

    /* GATTC */
    case SD_BLE_GATTC_CHAR_VALUE_BY_UUID_READ:
        command_name = "sd_ble_gattc_char_value_by_uuid_read";
        break;

    case SD_BLE_GATTC_CHAR_VALUES_READ:
        command_name = "sd_ble_gattc_char_values_read";
        break;

    case SD_BLE_GATTC_CHARACTERISTICS_DISCOVER:
        command_name = "sd_ble_gattc_characteristics_discover";
        break;

    case SD_BLE_GATTC_DESCRIPTORS_DISCOVER:
        command_name = "sd_ble_gattc_descriptors_discover";
        break;

    case SD_BLE_GATTC_HV_CONFIRM:
        command_name = "sd_ble_gattc_hv_confirm";
        break;

    case SD_BLE_GATTC_PRIMARY_SERVICES_DISCOVER:
        command_name = "sd_ble_gattc_primary_services_discover";
        break;

    case SD_BLE_GATTC_READ:
        command_name = "sd_ble_gattc_read";
        break;

    case SD_BLE_GATTC_RELATIONSHIPS_DISCOVER:
        command_name = "sd_ble_gattc_relationships_discover";
        break;

    case SD_BLE_GATTC_WRITE:
        command_name = "sd_ble_gattc_write";
        break;

    /* GATTS */
    case SD_BLE_GATTS_CHARACTERISTIC_ADD:
        command_name = "sd_ble_gatts_characteristic_add";
        break;

    case SD_BLE_GATTS_DESCRIPTOR_ADD:
        command_name = "sd_ble_gatts_descriptor_add";
        break;

    case SD_BLE_GATTS_HVX:
        command_name = "sd_ble_gatts_hvx";
        break;

    case SD_BLE_GATTS_INCLUDE_ADD:
        command_name = "sd_ble_gatts_include_add";
        break;

    case SD_BLE_GATTS_RW_AUTHORIZE_REPLY:
        command_name = "sd_ble_gatts_rw_authorize_reply";
        break;

    case SD_BLE_GATTS_SERVICE_ADD:
        command_name = "sd_ble_gatts_service_add";
        break;

    case SD_BLE_GATTS_SERVICE_CHANGED:
        command_name = "sd_ble_gatts_service_changed";
        break;

    case SD_BLE_GATTS_SYS_ATTR_GET:
        command_name = "sd_ble_gatts_sys_attr_get";
        break;

    case SD_BLE_GATTS_SYS_ATTR_SET:
        command_name = "sd_ble_gatts_sys_attr_set";
        break;

    case SD_BLE_GATTS_VALUE_GET:
        command_name = "sd_ble_gatts_value_get";
        break;

    case SD_BLE_GATTS_VALUE_SET:
        command_name = "sd_ble_gatts_value_set";
        break;

    /* BLE */
    case SD_BLE_TX_BUFFER_COUNT_GET:
        command_name = "sd_ble_tx_buffer_count_get";
        break;

    case SD_BLE_UUID_DECODE:
        command_name = "sd_ble_uuid_decode";
        break;

    case SD_BLE_UUID_ENCODE:
        command_name = "sd_ble_uuid_encode";
        break;

    case SD_BLE_UUID_VS_ADD:
        command_name = "sd_ble_uuid_vs_add";
        break;

    case SD_BLE_VERSION_GET:
        command_name = "sd_ble_version_get";
        break;

    default:
        //TODO: Consider adding command_number
        command_name = "UNKNOWN_COMMAND";
        severity = APP_LOG_FATAL;
        break;
    }

    app_log_handler(severity, message, command_name);
}

void app_log_utility_command_log(uint16_t command_number)
{
    fill_in_command_name(APP_LOG_DEBUG, "Command %s called", command_number);
}

void app_log_utility_command_response_log(uint16_t command_number)
{
    fill_in_command_name(APP_LOG_TRACE, "Response for %s received", command_number);
}

static void fill_in_event_name(app_log_severity_t severity, const char * message,
                               const uint16_t event_number)
{
    const char * event_name;

    switch (event_number)
    {
    /* BLE */
    case BLE_EVT_TX_COMPLETE:
        event_name = "ble_evt_tx_complete";
        break;

    case BLE_EVT_USER_MEM_RELEASE:
        event_name = "ble_evt_user_mem_release";
        break;

    case BLE_EVT_USER_MEM_REQUEST:
        event_name = "ble_evt_user_mem_request";
        break;

    /* GAP */
    case BLE_GAP_EVT_AUTH_KEY_REQUEST:
        event_name = "ble_gap_evt_auth_key_request";
        break;

    case BLE_GAP_EVT_AUTH_STATUS:
        event_name = "ble_gap_evt_auth_status";
        break;

    case BLE_GAP_EVT_CONN_PARAM_UPDATE:
        event_name = "ble_gap_evt_conn_param_update";
        break;

    case BLE_GAP_EVT_CONN_SEC_UPDATE:
        event_name = "ble_gap_evt_conn_sec_update";
        break;

    case BLE_GAP_EVT_CONNECTED:
        event_name = "ble_gap_evt_connected";
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        event_name = "ble_gap_evt_disconnected";
        break;

    case BLE_GAP_EVT_PASSKEY_DISPLAY:
        event_name = "ble_gap_evt_passkey_display";
        break;

    case BLE_GAP_EVT_RSSI_CHANGED:
        event_name = "ble_gap_evt_rssi_changed";
        break;

    case BLE_GAP_EVT_SEC_INFO_REQUEST:
        event_name = "ble_gap_evt_sec_info_request";
        break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        event_name = "ble_gap_evt_sec_params_request";
        break;

    case BLE_GAP_EVT_TIMEOUT:
        event_name = "ble_gap_evt_timeout";
        break;

    /* GATTC */
    case BLE_GATTC_EVT_CHAR_DISC_RSP:
        event_name = "ble_gattc_evt_char_disc_rsp";
        break;

    case BLE_GATTC_EVT_CHAR_VAL_BY_UUID_READ_RSP:
        event_name = "ble_gattc_evt_char_val_by_uuid_read_rsp";
        break;

    case BLE_GATTC_EVT_CHAR_VALS_READ_RSP:
        event_name = "ble_gattc_evt_char_vals_read_rsp";
        break;

    case BLE_GATTC_EVT_DESC_DISC_RSP:
        event_name = "ble_gattc_evt_desc_disc_rsp";
        break;

    case BLE_GATTC_EVT_HVX:
        event_name = "ble_gattc_evt_hvx";
        break;

    case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
        event_name = "ble_gattc_evt_prim_srvc_disc_rsp";
        break;

    case BLE_GATTC_EVT_READ_RSP:
        event_name = "ble_gattc_evt_read_rsp";
        break;

    case BLE_GATTC_EVT_REL_DISC_RSP:
        event_name = "ble_gattc_evt_rel_disc_rsp";
        break;

    case BLE_GATTC_EVT_TIMEOUT:
        event_name = "ble_gattc_evt_timeout";
        break;

    case BLE_GATTC_EVT_WRITE_RSP:
        event_name = "ble_gattc_evt_write_rsp";
        break;

    /* GATTS */
    case BLE_GATTS_EVT_HVC:
        event_name = "ble_gatts_evt_hvc";
        break;

    case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        event_name = "ble_gatts_evt_rw_authorize_request";
        break;

    case BLE_GATTS_EVT_SC_CONFIRM:
        event_name = "ble_gatts_evt_sc_confirm";
        break;

    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        event_name = "ble_gatts_evt_sys_attr_missing";
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        event_name = "ble_gatts_evt_timeout";
        break;

    case BLE_GATTS_EVT_WRITE:
        event_name = "ble_gatts_evt_write";
        break;

    default:
        //TODO: Consider adding command_number
        event_name = "UNKNOWN_EVENT";
        severity = APP_LOG_FATAL;    
        break;
    }

    app_log_handler(severity, message, event_name);
}

void app_log_utility_event_log(uint16_t event_number)
{
    fill_in_event_name(APP_LOG_DEBUG, "Event %s received", event_number);
}
