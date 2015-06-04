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

#include "ble.h"
#include "ble_gap.h"
#include "ble_types.h"
#include "nrf_error.h"

#include "sd_rpc.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#define ADVERTISING_INTERVAL_40_MS  64 // * 0.625 ms = 40 ms
#define ADVERTISING_TIMEOUT_3_MIN   180 // * 1 sec = 3 min

#ifdef _WIN32
#define UART_PORT_NAME "COM1"
#endif
#ifdef __APPLE__
#define UART_PORT_NAME "/dev/tty.usbmodem00000"
#endif
#ifdef __linux__
#define UART_PORT_NAME "/dev/ttyACM0"
#endif

enum
{
    UNIT_0_625_MS = 625,                                /**< Number of microseconds in 0.625 milliseconds. */
    UNIT_1_25_MS  = 1250,                               /**< Number of microseconds in 1.25 milliseconds. */
    UNIT_10_MS    = 10000                               /**< Number of microseconds in 10 milliseconds. */
};

#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME) * 1000) / (RESOLUTION))

#define SCAN_INTERVAL                    0x00A0                                         /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                      0x0050                                         /**< Determines scan window in units of 0.625 millisecond. */

#define MIN_CONNECTION_INTERVAL          MSEC_TO_UNITS(30, UNIT_1_25_MS)                /**< Determines maximum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL          MSEC_TO_UNITS(60, UNIT_1_25_MS)                /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY                    0                                              /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT              MSEC_TO_UNITS(4000, UNIT_10_MS)                /**< Determines supervision time-out in units of 10 millisecond. */

#define MAX_PEER_COUNT                   8                                              /**< Maximum number of peer's application intends to manage. */

typedef struct
{
    uint8_t     * p_data;                                                      /**< Pointer to data. */
    uint16_t      data_len;                                                    /**< Length of data. */
}data_t;

#define TARGET_LIST_LENGTH 1
#define TARGET_LIST_NAME_MAX_LENGTH 20 /**< Max length of the target names. */

/** List of all the target device names that the application is looking for. */
static char target_list[TARGET_LIST_LENGTH][TARGET_LIST_NAME_MAX_LENGTH] = {
    "Example"
    };
static uint8_t number_of_connected_devices = 0;

static const ble_gap_scan_params_t m_scan_param =
{
     0,                       // Active scanning set.
     0,                       // Selective scanning not set.
     NULL,                    // White-list not set.
     (uint16_t)SCAN_INTERVAL, // Scan interval.
     (uint16_t)SCAN_WINDOW,   // Scan window.
     0                        // Never stop scanning unless explicit asked to.
};

static const ble_gap_conn_params_t m_connection_param =
{
    (uint16_t)MIN_CONNECTION_INTERVAL,   // Minimum connection
    (uint16_t)MAX_CONNECTION_INTERVAL,   // Maximum connection
    0,                                   // Slave latency
    (uint16_t)SUPERVISION_TIMEOUT        // Supervision time-out
};

static void scan_start();
static void on_evt_connected(ble_evt_t* p_ble_evt);
static void on_evt_disconnected(ble_evt_t* p_ble_evt);
static void on_evt_adv_report(ble_evt_t* p_ble_evt);

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
    case BLE_GAP_EVT_CONNECTED:
        on_evt_connected(p_ble_evt);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        on_evt_disconnected(p_ble_evt);
        break;

    case BLE_GAP_EVT_ADV_REPORT:
        on_evt_adv_report(p_ble_evt);
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

static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint32_t index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->data_len)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index+1];

        if (field_type == type)
        {
            p_typedata->p_data   = &p_data[index+2];
            p_typedata->data_len = field_length-1;
            return NRF_SUCCESS;
        }
        index += field_length+1;
    }
    return NRF_ERROR_NOT_FOUND;
}

static bool name_in_connect_list(data_t * type_data)
{
    int i = 0;

    for (i = 0; i < TARGET_LIST_LENGTH; ++i)
    {
        if (0 == memcmp(target_list[i], type_data->p_data, type_data->data_len))
        {
            return true;
        }
    }

    return false;
}

static void on_evt_connected(ble_evt_t* p_ble_evt)
{
    ble_gap_evt_connected_t connected_evt = p_ble_evt->evt.gap_evt.params.connected;

    printf("Connected to device 0x%X%X%X%X%X%X\n",
           connected_evt.peer_addr.addr[5],
           connected_evt.peer_addr.addr[4],
           connected_evt.peer_addr.addr[3],
           connected_evt.peer_addr.addr[2],
           connected_evt.peer_addr.addr[1],
           connected_evt.peer_addr.addr[0]); fflush(stdout);

    number_of_connected_devices++;
    printf("%d devices connected.\n", number_of_connected_devices); fflush(stdout);
    scan_start();
}

static void on_evt_disconnected(ble_evt_t* p_ble_evt)
{
    ble_gap_evt_disconnected_t disconnected_evt = p_ble_evt->evt.gap_evt.params.disconnected;
    printf("Disconnected, reason: %d\n", disconnected_evt.reason); fflush(stdout);
    number_of_connected_devices--;
    printf("%d devices connected.\n", number_of_connected_devices); fflush(stdout);
}

static void on_evt_adv_report(ble_evt_t* p_ble_evt)
{
    ble_gap_evt_adv_report_t adv_report;
    data_t adv_data;
    data_t type_data;
    uint32_t err_code;

    printf("Received advertisment report\n"); fflush(stdout);

    adv_report = p_ble_evt->evt.gap_evt.params.adv_report;

    adv_data.p_data = adv_report.data;
    adv_data.data_len = adv_report.dlen;

    err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
                                &adv_data,
                                &type_data);

    if (err_code != NRF_SUCCESS)
    {
        // Compare short local name in case complete name does not match.
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME,
                                    &adv_data,
                                    &type_data);

        if (err_code != NRF_SUCCESS)
        {
            return;
        }
    }

    if (name_in_connect_list(&type_data))
    {
        if (number_of_connected_devices >= MAX_PEER_COUNT)
        {
            return;
        }

        sd_ble_gap_scan_stop();

        err_code = sd_ble_gap_connect(&adv_report.peer_addr,
                                      &m_scan_param,
                                      &m_connection_param);

        if (err_code != NRF_SUCCESS)
        {
            printf("Failed to connect. Error code: 0x%02X\n", err_code); fflush(stdout);
        }
    }
}

static void scan_start()
{
    uint32_t error_code;
    error_code = sd_ble_gap_scan_start(&m_scan_param);

    printf("Started scan with return code: 0x%02X\n", error_code); fflush(stdout);
}

static void intro_message_print()
{
    uint32_t i;

    printf("Example for demonstration of connection to multiple devices.\n");
    printf("The example will connect to maximum %d simultaneous devices.\n", MAX_PEER_COUNT);
    printf("The device used is the device on %s.\n", UART_PORT_NAME);
    printf("This example will connect to devices with the following names:\n");
    fflush(stdout);

    for (i = 0; i < TARGET_LIST_LENGTH; ++i)
    {
        printf("  * %s\n", target_list[i]); fflush(stdout);

    }
}

int main(int argc, char *argv[])
{
    uint32_t error_code;
    char* serial_port;

    intro_message_print();

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
        printf("Failed to open the nRF51 ble driver.\n"); fflush(stdout);
        return error_code;
    }

    error_code = ble_stack_init();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    scan_start();

    //Waits for any key to end the example.
    getchar();

    error_code = sd_ble_gap_scan_stop();

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to stop scanning. Reason: %d\n", error_code); fflush(stdout);
    }

    error_code = sd_rpc_close();

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to close the nRF51 ble driver.\n"); fflush(stdout);
        return error_code;
    }

    return 0;
}
