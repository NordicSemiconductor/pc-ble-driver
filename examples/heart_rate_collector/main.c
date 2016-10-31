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
#include "ble_gattc.h"
#include "ble_types.h"
#include "nrf_error.h"

#include "sd_rpc.h"
#include "sd_rpc_types.h"

#include <windows.h>
#include <time.h>
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

enum
{
    UNIT_0_625_MS = 625,                                /**< Number of microseconds in 0.625 milliseconds. */
    UNIT_1_25_MS  = 1250,                               /**< Number of microseconds in 1.25 milliseconds. */
    UNIT_10_MS    = 10000                               /**< Number of microseconds in 10 milliseconds. */
};

#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME) * 1000) / (RESOLUTION))

#define SCAN_INTERVAL                    0x00A0                                         /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                      0x0050                                         /**< Determines scan window in units of 0.625 millisecond. */

#define MIN_CONNECTION_INTERVAL          MSEC_TO_UNITS(7.5, UNIT_1_25_MS)                /**< Determines maximum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL          MSEC_TO_UNITS(7.5, UNIT_1_25_MS)                /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY                    0                                              /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT              MSEC_TO_UNITS(4000, UNIT_10_MS)                /**< Determines supervision time-out in units of 10 millisecond. */

#define TARGET_DEV_NAME                  "Nordic_HRM"                                      /**< Target device name that application is looking for. */
#define MAX_PEER_COUNT                   1                                              /**< Maximum number of peer's application intends to manage. */

#define HRM_SERVICE_UUID                 0x180D
#define HRM_MEAS_CHAR_UUID               0x2A37
#define CCCD_UUID                        0x2902
#define CCCD_NOTIFY                      0x01

#define STRING_BUFFER_SIZE               50

typedef struct
{
    uint8_t     * p_data;                                                      /**< Pointer to data. */
    uint16_t      data_len;                                                    /**< Length of data. */
} data_t;

static uint8_t  m_connected_devices = 0;
static uint16_t m_connection_handle = 0;
static uint16_t m_service_start_handle = 0;
static uint16_t m_service_end_handle = 0;
static uint16_t m_hrm_char_handle = 0;
static uint16_t m_hrm_cccd_handle = 0;
static bool     m_connection_is_in_progress = false;
static adapter_t * m_adapter = NULL;
static uint32_t m_hvx_count = 0;
static clock_t m_hvx_start;

static const ble_gap_scan_params_t m_scan_param =
{
     1,                       // Active scanning set.
     0,                       // Selective scanning not set.
#if NRF_SD_BLE_API >= 2
	 0,
#else
     NULL,                    // White-list not set.
#endif
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


/* Local function declarations */
static void log_handler(adapter_t * adapter, sd_rpc_log_severity_t severity, const char * message);
static void status_handler(adapter_t * adapter, sd_rpc_app_status_t code, const char * message);
static uint32_t ble_stack_init();
static void on_connected(const ble_gap_evt_t * const p_ble_gap_evt);
static void on_adv_report(const ble_gap_evt_t * const p_ble_gap_evt);
static void on_timeout(const ble_gap_evt_t * const p_ble_gap_evt);
static void on_service_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt);
static void on_characteristic_discover_response(const ble_gattc_evt_t * const p_ble_gattc_evt);
static void on_descriptor_discover_response(const ble_gattc_evt_t * const p_ble_gattc_evt);
static void on_write_response(const ble_gattc_evt_t * const p_ble_gattc_evt);
static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata);
static uint32_t scan_start();
static uint32_t service_discovery_start();
static uint32_t char_discovery_start();
static uint32_t descr_discovery_start();
static uint32_t hrm_cccd_set(uint8_t value);
static void ble_address_to_string_convert(ble_gap_addr_t address, uint8_t * string_buffer);

static void log_handler(adapter_t * adapter, sd_rpc_log_severity_t severity, const char * message)
{
    switch (severity)
    {
    case SD_RPC_LOG_ERROR:
        printf("Error: %s\n", message); fflush(stdout);
        break;

    case SD_RPC_LOG_WARNING:
        printf("Warning: %s\n", message); fflush(stdout);
        break;

    case SD_RPC_LOG_INFO:
        printf("Warning: %s\n", message); fflush(stdout);
        break;

    default:
        //printf("Log: %s\n", message); fflush(stdout);
        break;
    }
}

static void status_handler(adapter_t *adapter, sd_rpc_app_status_t code, const char * message)
{
    printf("Status: %d, message: %s", (uint32_t)code, message);
}

static void on_connected(const ble_gap_evt_t * const p_ble_gap_evt)
{
    printf("Connection established\n"); fflush(stdout);
    m_connected_devices++;
    m_connection_handle = p_ble_gap_evt->conn_handle;
    m_connection_is_in_progress = false;
    service_discovery_start();
}

static void on_adv_report(const ble_gap_evt_t * const p_ble_gap_evt)
{
    uint32_t    err_code;
    data_t      adv_data;
    data_t      type_data;
    uint8_t     str[STRING_BUFFER_SIZE] = {0};

    ble_address_to_string_convert(p_ble_gap_evt->params.adv_report.peer_addr, str);
    printf("Received advertisment report, address: 0x%s\n", str); fflush(stdout);

    adv_data.p_data     = (uint8_t *)(p_ble_gap_evt->params.adv_report.data);
    adv_data.data_len   = p_ble_gap_evt->params.adv_report.dlen;

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

    adv_data.p_data[adv_data.data_len] = 0;
    printf("Name: %s\n", adv_data.p_data);

    if (0 == memcmp(TARGET_DEV_NAME, type_data.p_data, type_data.data_len))
    {
        if (m_connected_devices >= MAX_PEER_COUNT)
        {
            return;
        }

        if (m_connection_is_in_progress)
        {
            return;
        }

        err_code = sd_ble_gap_connect(m_adapter,
                                      &(p_ble_gap_evt->params.adv_report.peer_addr),
                                      &m_scan_param,
                                      &m_connection_param);

        if (err_code != NRF_SUCCESS)
        {
            printf("Connection Request Failed, reason %d\n", err_code); fflush(stdout);
            return;
        }

        m_connection_is_in_progress = true;
    }
}

static void on_timeout(const ble_gap_evt_t * const p_ble_gap_evt)
{
    if (p_ble_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
    {
        m_connection_is_in_progress = false;
    }
    else if (p_ble_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
    {
        scan_start();
    }
}

static void on_service_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    int count;
    int service_index;
    const ble_gattc_service_t * service;

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Error. Service discovery failed. Error code 0x%X\n",
                p_ble_gattc_evt->gatt_status);
        fflush(stdout);
        return;
    }

    count = p_ble_gattc_evt->params.prim_srvc_disc_rsp.count;

    if (count == 0)
    {
        printf("Error. Service not found\n"); fflush(stdout);
        return;
    }

    printf("Received service discovery response\n"); fflush(stdout);

    service_index = 0; /* We specifically requested to discover Heart Rate service, so we can
                        * safely select the first returned service. */

    service = &(p_ble_gattc_evt->params.prim_srvc_disc_rsp.services[service_index]);

    m_service_start_handle  = service->handle_range.start_handle;
    m_service_end_handle    = service->handle_range.end_handle;

    printf("UUID: 0x%04X, start handle: 0x%04X, end handle: 0x%04X\n",
        service->uuid.uuid, m_service_start_handle, m_service_end_handle);
    fflush(stdout);

    char_discovery_start();
}

static void on_characteristic_discover_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    int i;
    int count;
    ble_gattc_char_t * p_characteristic;

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Error. Characteristic discovery failed. Error code 0x%X\n",
               p_ble_gattc_evt->gatt_status);
        fflush(stdout);
        return;
    }

    count = p_ble_gattc_evt->params.char_disc_rsp.count;

    printf("Received characteristic discovery response, characteristics count: %d\n", count);
    fflush(stdout);

    for (i = 0; i < count; i++)
    {
        p_characteristic = (ble_gattc_char_t *)&(p_ble_gattc_evt->params.char_disc_rsp.chars[i]);
        printf("Handle: 0x%04X, UUID: 0x%04X\n",
            p_characteristic->handle_decl,
            p_characteristic->uuid.uuid);
        fflush(stdout);

        if (p_characteristic->uuid.uuid == HRM_MEAS_CHAR_UUID)
        {
            m_hrm_char_handle = p_characteristic->handle_decl;
        }
    }

    descr_discovery_start();
}

static void on_descriptor_discover_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    int i;
    int count;
    ble_gattc_desc_t * p_descriptor;

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Error. Descriptor discovery failed. Error code 0x%X\n",
                p_ble_gattc_evt->gatt_status);
        fflush(stdout);
        return;
    }

    count = p_ble_gattc_evt->params.desc_disc_rsp.count;

    printf("Received descriptor discovery response, descriptor count: %d\n", count);
    fflush(stdout);

    for (i = 0; i < count; i++)
    {
        p_descriptor = (ble_gattc_desc_t *)&(p_ble_gattc_evt->params.desc_disc_rsp.descs[i]);
        printf("Handle: 0x%04X, UUID: 0x%04X\n", p_descriptor->handle, p_descriptor->uuid.uuid);
        fflush(stdout);

        if (p_descriptor->uuid.uuid == CCCD_UUID)
        {
            m_hrm_cccd_handle = p_descriptor->handle;
        }
    }

    printf("Press enter to toggle notifications\n"); fflush(stdout);
}

static void on_write_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    printf("Received write response.\n"); fflush(stdout);

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Error. Write operation failed. Error code 0x%X\n",
                p_ble_gattc_evt->gatt_status);
        fflush(stdout);
        return;
    }
}

static void on_hvx(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    clock_t now = clock();

    if (!m_hvx_start) {
        m_hvx_start = now;
    }

    m_hvx_count++;

    float seconds = (float)(now - m_hvx_start) / CLOCKS_PER_SEC;

    int i = 0;
    int length = p_ble_gattc_evt->params.hvx.len;

    float pkt_per_sec = m_hvx_count / seconds;

    if (m_hvx_count % 10 == 0) {
        printf("pkts: %06d, pkt/s: %f\n", m_hvx_count, pkt_per_sec);
    }

    if (length < 4) {
        return;
    }

    fflush(stdout);
}

static void ble_address_to_string_convert(ble_gap_addr_t address, uint8_t * string_buffer)
{
    const int address_length = 6;
    char temp_str[3];
    int i = 0;

    for (i = address_length - 1; i >= 0; --i)
    {
        sprintf(temp_str, "%02X", address.addr[i]);
        strcat((char *)string_buffer, temp_str);
    }
}

static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint8_t * p_data;
    uint32_t  index;

    p_data = p_advdata->p_data;
    index = 0;

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

static uint32_t ble_stack_init()
{
    uint32_t err_code;
    ble_enable_params_t ble_enable_params;
    uint32_t * app_ram_base = NULL;

    memset(&ble_enable_params, 0, sizeof(ble_enable_params));

    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
    ble_enable_params.gatts_enable_params.service_changed = false;
    ble_enable_params.gap_enable_params.periph_conn_count = 1;
    ble_enable_params.gap_enable_params.central_conn_count = 5;
    ble_enable_params.gap_enable_params.central_sec_count = 1;
    ble_enable_params.common_enable_params.p_conn_bw_counts = NULL;
    ble_enable_params.common_enable_params.vs_uuid_count = 5;

    err_code = sd_ble_enable(m_adapter, &ble_enable_params, app_ram_base);

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

static uint32_t scan_start()
{
    uint32_t error_code = sd_ble_gap_scan_start(m_adapter, &m_scan_param);

    if (error_code != NRF_SUCCESS)
    {
        printf("Scan start failed\n"); fflush(stdout);
        return error_code;
    }

    printf("Scan started\n"); fflush(stdout);

    return NRF_SUCCESS;
}

static uint32_t service_discovery_start()
{
    uint16_t start_handle;
    uint32_t err_code;
    ble_uuid_t srvc_uuid;

    printf("Discovering primary services\n"); fflush(stdout);
    start_handle = 0x0001;

    srvc_uuid.type = BLE_UUID_TYPE_BLE;
    srvc_uuid.uuid = HRM_SERVICE_UUID;

    err_code = sd_ble_gattc_primary_services_discover(m_adapter,
                                                      m_connection_handle, start_handle,
                                                      &srvc_uuid);
    if (err_code != NRF_SUCCESS)
    {
        printf("Failed to discover primary services\n"); fflush(stdout);
        return err_code;
    }

    return NRF_SUCCESS;
}

static uint32_t char_discovery_start()
{
    uint32_t err_code;
    ble_gattc_handle_range_t handle_range;

    printf("Discovering characteristics\n"); fflush(stdout);

    handle_range.start_handle = m_service_start_handle;
    handle_range.end_handle = m_service_end_handle;

    err_code = sd_ble_gattc_characteristics_discover(m_adapter, m_connection_handle, &handle_range);

    return err_code;
}

static uint32_t descr_discovery_start()
{
    uint32_t err_code;
    ble_gattc_handle_range_t handle_range;

    printf("Discovering descriptors\n"); fflush(stdout);

    if (m_hrm_char_handle == 0)
    {
        printf("Error. No HRM characteristic handle has been found\n"); fflush(stdout);
        return NRF_ERROR_INVALID_STATE;
    }

    handle_range.start_handle = m_hrm_char_handle;
    handle_range.end_handle = m_service_end_handle;

    err_code = sd_ble_gattc_descriptors_discover(m_adapter, m_connection_handle, &handle_range);
    return err_code;
}

static uint32_t hrm_cccd_set(uint8_t value)
{
    uint32_t err_code;
    ble_gattc_write_params_t write_params;
    uint8_t cccd_value[2] = {value, 0};

    printf("Setting HRM CCCD\n"); fflush(stdout);

    if (m_hrm_cccd_handle == 0)
    {
        printf("Error. No CCCD handle has been found\n"); fflush(stdout);
        return NRF_ERROR_INVALID_STATE;
    }

    write_params.handle = m_hrm_cccd_handle;
    write_params.len = 2;
    write_params.p_value = cccd_value;
    write_params.write_op = BLE_GATT_OP_WRITE_REQ;
    write_params.offset = 0;
    err_code = sd_ble_gattc_write(m_adapter, m_connection_handle, &write_params);

    return err_code;
}

static void ble_evt_dispatch(adapter_t * adapter, ble_evt_t * p_ble_evt)
{
    if (p_ble_evt == NULL)
    {
        printf("Received empty ble_event\n"); fflush(stdout);
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        on_connected(&(p_ble_evt->evt.gap_evt));
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        printf("Disconnected, reason: 0x%02X\n", p_ble_evt->evt.gap_evt.params.disconnected.reason);
        fflush(stdout);
        m_connected_devices--;
        m_connection_handle = 0;
        break;

    case BLE_GAP_EVT_ADV_REPORT:
        on_adv_report(&(p_ble_evt->evt.gap_evt));
        break;

    case BLE_GAP_EVT_TIMEOUT:
        on_timeout(&(p_ble_evt->evt.gap_evt));
        break;

    case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
        on_service_discovery_response(&(p_ble_evt->evt.gattc_evt));
        break;

    case BLE_GATTC_EVT_CHAR_DISC_RSP:
        on_characteristic_discover_response(&(p_ble_evt->evt.gattc_evt));
        break;

    case BLE_GATTC_EVT_DESC_DISC_RSP:
        on_descriptor_discover_response(&(p_ble_evt->evt.gattc_evt));
        break;

    case BLE_GATTC_EVT_WRITE_RSP:
        on_write_response(&(p_ble_evt->evt.gattc_evt));
        break;

    case BLE_GATTC_EVT_HVX:
        on_hvx(&(p_ble_evt->evt.gattc_evt));
        break;

    default:
        printf("Unhandled event with ID: %d\n", p_ble_evt->header.evt_id); fflush(stdout);
        break;
    }
}

int main(int argc, char *argv[])
{
    uint32_t error_code;
    uint8_t cccd_value;
    char* serial_port;
    physical_layer_t * phy;
    data_link_layer_t * data_link_layer;
    transport_layer_t * transport_layer;

     cccd_value = 0;

    if (argc > 1)
    {
        serial_port = argv[1];
    }
    else
    {
        serial_port = UART_PORT_NAME;
    }

    printf("Serial port used: %s\n", serial_port); fflush(stdout);

    phy = sd_rpc_physical_layer_create_uart(serial_port, 115200, SD_RPC_FLOW_CONTROL_NONE, SD_RPC_PARITY_NONE);

    data_link_layer = sd_rpc_data_link_layer_create_bt_three_wire(phy, 100);

    transport_layer = sd_rpc_transport_layer_create(data_link_layer, 100);

    m_adapter = sd_rpc_adapter_create(transport_layer);

    //sd_rpc_log_handler_severity_filter_set(adapter, SD_RPC_LOG_INFO);

    error_code = sd_rpc_open(m_adapter, status_handler, ble_evt_dispatch, log_handler);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to open the nRF5 BLE Driver. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    error_code = ble_stack_init();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    scan_start();

    while (true)
    {
        getchar();
        cccd_value ^= CCCD_NOTIFY;
        m_hvx_start = 0;
        m_hvx_count = 0;
        hrm_cccd_set(cccd_value);
    }

    error_code = sd_rpc_close(m_adapter);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to close the nRF5 BLE Driver. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    printf("Closed\n"); fflush(stdout);

    return 0;
}
