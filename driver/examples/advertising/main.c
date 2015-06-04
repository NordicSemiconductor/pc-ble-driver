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
#include "sd_rpc.h"

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
#define BUFFER_SIZE                 20

static uint16_t m_connection_handle    = BLE_CONN_HANDLE_INVALID;
static bool     m_advertisment_timeout = false;

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
        printf("Received empty ble_event\n"); fflush(stdout);
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        uint16_t   length;
        uint8_t  * data;
        int        i;

    case BLE_GAP_EVT_CONNECTED:
        m_connection_handle = p_ble_evt->evt.gap_evt.conn_handle;
        printf("Connected, connection handle 0x%04X\n", m_connection_handle); fflush(stdout);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        printf("Disconnected\n"); fflush(stdout);
        m_connection_handle = BLE_CONN_HANDLE_INVALID;
        advertising_start();
        break;

    case BLE_GAP_EVT_TIMEOUT:
        printf("Advertisement timed out\n"); fflush(stdout);
        m_advertisment_timeout = true;
        break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        printf("Received Security Parameters request\n"); fflush(stdout);
        sd_ble_gap_sec_params_reply(m_connection_handle,
                                    BLE_GAP_SEC_STATUS_SUCCESS,
                                    NULL, NULL);
        break;

    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        printf("Received System Attribute Missing event\n"); fflush(stdout);
        sd_ble_gatts_sys_attr_set(m_connection_handle, NULL, 0, 0);
        break;

    case BLE_GATTS_EVT_WRITE:
        printf("Recieved Write event: "); fflush(stdout);
        length = p_ble_evt->evt.gatts_evt.params.write.len;
        data = p_ble_evt->evt.gatts_evt.params.write.data;

        for (i = 0; i < length; i++)
        {
            printf("%02X ", data[i]);
        }

        printf("\n"); fflush(stdout);
        break;

    default:
        printf("Received event with ID: 0x%02X\n", p_ble_evt->header.evt_id); fflush(stdout);
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

    const char  * device_name = "Example";
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
        printf("Failed to set advertisement data. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    printf("Advertising data set\n"); fflush(stdout);

    return NRF_SUCCESS;
}

static uint32_t advertising_start()
{
    uint32_t             error_code;
    ble_gap_adv_params_t adv_params;

    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.type         = BLE_GAP_ADV_TYPE_ADV_IND;
    adv_params.p_peer_addr  = NULL;                           // Undirected advertisement.
    adv_params.fp           = BLE_GAP_ADV_FP_ANY;
    adv_params.p_whitelist  = NULL;
    adv_params.interval     = ADVERTISING_INTERVAL_40_MS;
    adv_params.timeout      = ADVERTISING_TIMEOUT_3_MIN;

    error_code = sd_ble_gap_adv_start(&adv_params);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to start advertising. Error code: 0x%02X\n", error_code); fflush(stdout);
        return error_code;
    }

    printf("Started advertising\n"); fflush(stdout);
    return NRF_SUCCESS;
}

int main(int argc, char * argv[])
{
    uint32_t error_code;
    char *   serial_port;

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
        printf("Failed to open the nRF51 ble driver. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    error_code = ble_stack_init();

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

    while (!m_advertisment_timeout)
    {
        /* Empty */
    }

    error_code = sd_rpc_close();

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to close the nRF51 BLE Driver"); fflush(stdout);
        return error_code;
    }

    printf("Closing\n"); fflush(stdout);

    return 0;
}
