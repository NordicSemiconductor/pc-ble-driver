/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

#include "nrf_error.h"
#include "ble.h"
#include "ble_types.h"
#include "ble_l2cap.h"
#include "ble_gatts.h"
#include "sd_rpc.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef _WIN32
#define UART_PORT_NAME "COM1"
#endif
#ifdef __APPLE__
#define UART_PORT_NAME "/dev/tty.usbmodem00000"
#endif
#ifdef __linux__
#define UART_PORT_NAME "/dev/ttyACM0"
#endif

#define ADVERTISING_INTERVAL_40_MS  64 // * 0.625 ms = 40 ms
#define ADVERTISING_TIMEOUT_3_MIN   180 // * 1 sec = 3 min

#define OPCODE_LENGTH  1                                                    /**< Length of opcode inside Heart Rate Measurement packet. */
#define HANDLE_LENGTH  2                                                    /**< Length of handle inside Heart Rate Measurement packet. */
#define MAX_HRM_LEN    (BLE_L2CAP_MTU_DEF - OPCODE_LENGTH - HANDLE_LENGTH)  /**< Maximum size of a transmitted Heart Rate Measurement. */

#define BLE_UUID_HEART_RATE_SERVICE                              0x180D     /**< Heart Rate service UUID. */
#define BLE_UUID_HEART_RATE_MEASUREMENT_CHAR                     0x2A37     /**< Heart Rate Measurement characteristic UUID. */

#define HEART_RATE_BASE     65
#define HEART_RATE_INCREASE 3
#define HEART_RATE_LIMIT    190

#define BUFFER_SIZE    30

static uint16_t                 m_connection_handle             = BLE_CONN_HANDLE_INVALID;
static uint16_t                 m_heart_rate_service_handle     = 0;
static ble_gatts_char_handles_t m_heart_rate_measurement_handle;
static uint8_t                  m_heart_rate                    = HEART_RATE_BASE;
static bool                     m_send_notifications            = false;
static bool                     m_advertisement_timed_out       = false;

static uint32_t advertising_start();

static void log_handler(sd_rpc_log_severity_t severity, const char * message)
{
    switch (severity)
    {
    case LOG_ERROR:
        printf("Error: %s\n", message); fflush(stdout);
        break;

    case LOG_WARNING:
        printf("Warning: %s\n", message); fflush(stdout);
        break;

    default:
        printf("Log: %s\n", message); fflush(stdout);
        break;
    }
}

static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    if (p_ble_evt == NULL)
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        m_connection_handle = p_ble_evt->evt.gap_evt.conn_handle;
        printf("Connected, connection handle 0x%04X\n", m_connection_handle); fflush(stdout);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        printf("Disconnected\n"); fflush(stdout);
        m_connection_handle = BLE_CONN_HANDLE_INVALID;
        m_send_notifications = false;
        advertising_start();
        break;

    case BLE_GAP_EVT_TIMEOUT:
        printf("Advertisement timed out\n"); fflush(stdout);
        m_advertisement_timed_out = true;
        break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        sd_ble_gap_sec_params_reply(m_connection_handle,
                                    BLE_GAP_SEC_STATUS_SUCCESS, NULL, NULL);
        break;

    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        sd_ble_gatts_sys_attr_set(m_connection_handle, NULL, 0, 0);
        break;

    case BLE_GATTS_EVT_WRITE:
        if (p_ble_evt->evt.gatts_evt.params.write.context.char_uuid.uuid ==
            BLE_UUID_HEART_RATE_MEASUREMENT_CHAR)
        {
            uint8_t write_data = p_ble_evt->evt.gatts_evt.params.write.data[0];
            m_send_notifications = write_data == BLE_GATT_HVX_NOTIFICATION;
        }
        break;

    default:
        printf("Received event with ID: %d\n", p_ble_evt->header.evt_id); fflush(stdout);
        break;
    }
}

static uint32_t ble_stack_init()
{
    uint32_t err_code;
    ble_enable_params_t ble_enable_params;

    memset(&ble_enable_params, 0, sizeof(ble_enable_params));

    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
    ble_enable_params.gatts_enable_params.service_changed = false;

    err_code = sd_ble_enable(&ble_enable_params);

    if (err_code == NRF_SUCCESS)
    {
        return err_code;
    }

    if (err_code == NRF_ERROR_INVALID_STATE)
    {
        printf("BLE stack already enabled\n"); fflush(stdout);
        return NRF_SUCCESS;
    }

    printf("Failed to enable BLE stack.\n"); fflush(stdout);
    return err_code;
}

static uint32_t advertisement_data_set()
{
    uint32_t error_code;
    uint32_t index;
    uint8_t  data_buffer[BUFFER_SIZE]; //Sufficiently large buffer for the advertising data

    const char  * device_name = "HRM Example";
    const uint8_t name_length = strlen(device_name);
    const uint8_t data_length = name_length + 1; //Device name + data type
    const uint8_t data_type   = BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME;

    const uint8_t * sr_data = NULL;
    const uint8_t   sr_data_length = 0;

    index = 0;
    data_buffer[index] = data_length;
    index++;
    data_buffer[index] = data_type;
    index++;
    memcpy((char *)&data_buffer[index], device_name, name_length);

    error_code = sd_ble_gap_adv_data_set(data_buffer, data_length + 1, sr_data, sr_data_length);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to set advertisement data. Error code: 0x%02X\n", error_code); fflush(stdout);
        return error_code;
    }

    printf("Advertising data set\n"); fflush(stdout);

    return NRF_SUCCESS;
}

static uint32_t advertising_start()
{
    uint32_t             error_code;
    ble_gap_adv_params_t adv_params;

    adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    adv_params.p_peer_addr = NULL;                           // Undirected advertisement.
    adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    adv_params.p_whitelist = NULL;
    adv_params.interval    = ADVERTISING_INTERVAL_40_MS;
    adv_params.timeout     = ADVERTISING_TIMEOUT_3_MIN;

    error_code = sd_ble_gap_adv_start(&adv_params);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to start advertising. Error code: 0x%02X\n", error_code); fflush(stdout);
        return error_code;
    }

    printf("Started advertising\n"); fflush(stdout);
    return NRF_SUCCESS;
}

static uint8_t heart_rate_measurement_encode(uint8_t * encoded_hrm, uint8_t heart_rate)
{
    uint8_t flags = 0;
    uint8_t length = 1;

    // Very simple encoding
    encoded_hrm[length++] = heart_rate;

    // Add flags
    encoded_hrm[0] = flags;

    return length;
}

static uint32_t characteristic_init()
{
    uint32_t            error_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint8_t             encoded_initial_hrm[MAX_HRM_LEN];
    uint16_t            attr_char_value_init_len;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_HEART_RATE_MEASUREMENT_CHAR);

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value_init_len = heart_rate_measurement_encode(encoded_initial_hrm, 0);

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = attr_char_value_init_len;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = MAX_HRM_LEN;
    attr_char_value.p_value      = encoded_initial_hrm;

    error_code = sd_ble_gatts_characteristic_add(m_heart_rate_service_handle,
                                                 &char_md,
                                                 &attr_char_value,
                                                 &m_heart_rate_measurement_handle);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to initialize characteristics. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    printf("Characteristics initiated\n"); fflush(stdout);

    return NRF_SUCCESS;
}

static uint32_t services_init()
{
    uint32_t    error_code;
    ble_uuid_t  ble_uuid;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_HEART_RATE_SERVICE);

    error_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                          &ble_uuid,
                                          &m_heart_rate_service_handle);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to initialize heart rate service. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    printf("Services initiated\n"); fflush(stdout);

    error_code = characteristic_init();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    return NRF_SUCCESS;
}

static void heart_rate_generate()
{
    m_heart_rate += HEART_RATE_INCREASE;

    if (m_heart_rate > HEART_RATE_LIMIT)
    {
        m_heart_rate = HEART_RATE_BASE;
    }
}

static uint32_t heart_rate_measurement_send()
{
    uint32_t               error_code;
    uint8_t                encoded_hrm[MAX_HRM_LEN];
    uint16_t               hvx_length;
    ble_gatts_hvx_params_t hvx_params;
    uint8_t                length;

    heart_rate_generate();

    length = heart_rate_measurement_encode(encoded_hrm, m_heart_rate);

    hvx_length = length;

    hvx_params.handle   = m_heart_rate_measurement_handle.value_handle;
    hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset   = 0;
    hvx_params.p_len    = &hvx_length;
    hvx_params.p_data   = encoded_hrm;

    error_code = sd_ble_gatts_hvx(m_connection_handle, &hvx_params);

    if (error_code == NRF_SUCCESS && (hvx_length != length))
    {
        error_code = NRF_ERROR_DATA_SIZE;
        printf("Failed to send heart rate measurement. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }


    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to send heart rate measurement. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    return NRF_SUCCESS;
}

int main(int argc, char *argv[])
{
    uint32_t error_code;

    char* serial_port;

    if (argc > 1)
    {
        serial_port = argv[1];
    }
    else
    {
        serial_port = UART_PORT_NAME;
    }

    printf("Serial port used: %s\n", serial_port); fflush(stdout);

    sd_rpc_serial_port_name_set(serial_port);

    sd_rpc_serial_baud_rate_set(115200);

    sd_rpc_log_handler_set(log_handler);

    sd_rpc_evt_handler_set(ble_evt_dispatch);

    error_code = sd_rpc_open();

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to open the nRF51 ble driver"); fflush(stdout);
        return error_code;
    }

    error_code = ble_stack_init();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    error_code = services_init();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    error_code = advertisement_data_set();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    error_code = advertising_start();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    while (!m_advertisement_timed_out)
    {
        int i;

        if (m_connection_handle != BLE_CONN_HANDLE_INVALID && m_send_notifications)
        {
            heart_rate_measurement_send();
        }

        /* Looping to to simulate a pause for aproximatly 1 second between notifications */
        for (i = 0; i < 500000000; i++)
        {
            /* Empty */
        }
    }

    error_code = sd_rpc_close();

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to close the nRF51 ble driver"); fflush(stdout);
        return error_code;
    }

    printf("Closed"); fflush(stdout);

    return 0;
}
