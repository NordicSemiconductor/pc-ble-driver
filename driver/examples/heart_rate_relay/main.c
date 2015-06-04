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

#include "ble.h"
#include "ble_hci.h"
#include "ble_gap.h"
#include "ble_gattc.h"
#include "ble_types.h"
#include "nrf_error.h"

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

enum
{
    UNIT_0_625_MS = 625,                                /**< Number of microseconds in 0.625 milliseconds. */
    UNIT_1_25_MS  = 1250,                               /**< Number of microseconds in 1.25 milliseconds. */
    UNIT_10_MS    = 10000                               /**< Number of microseconds in 10 milliseconds. */
};

#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME) * 1000) / (RESOLUTION))

#define SCAN_INTERVAL                    MSEC_TO_UNITS(100, UNIT_0_625_MS)      /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                      MSEC_TO_UNITS(50, UNIT_0_625_MS)       /**< Determines scan window in units of 0.625 millisecond. */

#define ADVERTISING_INTERVAL_40_MS  MSEC_TO_UNITS(40, UNIT_0_625_MS)
#define ADVERTISING_TIMEOUT_3_MIN   180 // * 1 sec = 3 min

#define MIN_CONNECTION_INTERVAL          MSEC_TO_UNITS(30, UNIT_1_25_MS)        /**< Determines maximum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL          MSEC_TO_UNITS(60, UNIT_1_25_MS)        /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY                    0                                      /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT              MSEC_TO_UNITS(4000, UNIT_10_MS)        /**< Determines supervision time-out in units of 10 millisecond. */

#define TARGET_DEV_NAME                  "HRM Example"                          /**< Target device name that application is looking for. */
#define MAX_PEER_COUNT                   1                                      /**< Maximum number of peer's application intends to manage. */

#define BLE_UUID_HEART_RATE_SERVICE                              0x180D         /**< Heart Rate service UUID. */
#define BLE_UUID_HEART_RATE_MEASUREMENT_CHAR                     0x2A37         /**< Heart Rate Measurement characteristic UUID. */

#define OPCODE_LENGTH  1                                                        /**< Length of opcode inside Heart Rate Measurement packet. */
#define HANDLE_LENGTH  2                                                        /**< Length of handle inside Heart Rate Measurement packet. */
#define MAX_HRM_LEN    (BLE_L2CAP_MTU_DEF - OPCODE_LENGTH - HANDLE_LENGTH)      /**< Maximum size of a transmitted Heart Rate Measurement. */

#define CCCD_NOTIFY                      0x01

#define STRING_BUFFER_SIZE               50

typedef struct
{
    uint8_t     * p_data;       /**< Pointer to data. */
    uint16_t      data_len;     /**< Length of data. */
} data_t;

static uint16_t m_peripheral_conn_handle = BLE_CONN_HANDLE_INVALID;
static uint16_t m_central_conn_handle = BLE_CONN_HANDLE_INVALID;

static uint16_t m_service_start_handle = 0;
static uint16_t m_service_end_handle = 0;
static uint16_t m_hrm_char_handle = 0;
static uint16_t m_hrm_cccd_handle = 0;
static bool     m_connection_is_in_progress = false;

static uint16_t                 m_heart_rate_service_handle     = 0;
static ble_gatts_char_handles_t m_heart_rate_measurement_handle;
static bool                     m_send_notifications            = false;

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

/* Local function declarations */
static void log_handler(sd_rpc_log_severity_t severity, const char * message);
static uint32_t ble_stack_init();
static uint32_t advertisement_data_set();
static uint32_t advertising_start();
static uint32_t heart_rate_relay_send(uint16_t length, uint8_t *data);
static void on_connected(const ble_gap_evt_t * const p_ble_gap_evt);
static void on_disconnected(const ble_gap_evt_t * const p_ble_gap_evt);
static void on_adv_report(const ble_gap_evt_t * const p_ble_gap_evt);
static void on_timeout(const ble_gap_evt_t * const p_ble_gap_evt);
static void on_service_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt);
static void on_characteristic_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt);
static void on_descriptor_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt);
static void on_hvx(const ble_gattc_evt_t * const p_ble_gattc_evt);
static void on_write_response(const ble_gattc_evt_t * const p_ble_gattc_evt);
static void on_write(const ble_gatts_evt_t * const p_ble_gatts_evt);
static void ble_address_to_string_convert(ble_gap_addr_t address, uint8_t * string_buffer);
static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata);
static uint32_t scan_start();
static uint32_t service_discovery_start();
static uint32_t characteristic_discovery_start();
static uint32_t descriptor_discovery_start();
static uint32_t characteristic_init();
static uint32_t services_init();
static uint32_t hrm_cccd_set(uint8_t value);
static void ble_evt_dispatch(ble_evt_t * p_ble_evt);

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
    uint8_t  data_buffer[STRING_BUFFER_SIZE]; //Sufficiently large buffer for the advertising data

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


static uint32_t heart_rate_relay_send(uint16_t length, uint8_t *data)
{
    uint32_t error_code;
    uint16_t hvx_length = length;

    ble_gatts_hvx_params_t hvx_params;
    hvx_params.handle   = m_heart_rate_measurement_handle.value_handle;
    hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset   = 0;
    hvx_params.p_len    = &hvx_length;
    hvx_params.p_data   = data;

    error_code = sd_ble_gatts_hvx(m_peripheral_conn_handle, &hvx_params);

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

static void on_connected(const ble_gap_evt_t * const p_ble_gap_evt)
{
    ble_gap_evt_connected_t connected_evt = p_ble_gap_evt->params.connected;

    if(connected_evt.role == BLE_GAP_ROLE_PERIPH) //If this side is BLE_GAP_ROLE_PERIPH then BLE_GAP_EVT_CONNECTED comes from peer central
    {
        printf("Connection to a central device established\n"); fflush(stdout);
        m_peripheral_conn_handle = p_ble_gap_evt->conn_handle;
        m_connection_is_in_progress = false;
    }
    else if(connected_evt.role == BLE_GAP_ROLE_CENTRAL) //If this side is BLE_GAP_ROLE_CENTRAL then BLE_GAP_EVT_CONNECTED comes from peer peripheral
    {
        printf("Connection to a peripheral device established\n"); fflush(stdout);
        m_central_conn_handle = p_ble_gap_evt->conn_handle;
        service_discovery_start();
        advertising_start();
    }
}

static void on_disconnected(const ble_gap_evt_t * const p_ble_gap_evt)
{
    ble_gap_evt_disconnected_t disconnected_evt = p_ble_gap_evt->params.disconnected;

    if(m_peripheral_conn_handle == p_ble_gap_evt->conn_handle)
    {
        printf("Disconnected from a central, reason: 0x%02X\n", disconnected_evt.reason);
        m_peripheral_conn_handle = BLE_CONN_HANDLE_INVALID;
        fflush(stdout);

        // If we have a connection to a peripheral device we start to advertise again.
        if (m_central_conn_handle != BLE_CONN_HANDLE_INVALID)
        {
            advertising_start();
        }
    }
    else if(m_central_conn_handle == p_ble_gap_evt->conn_handle)
    {
        printf("Disconnected from a peripheral, reason: 0x%02X\n", disconnected_evt.reason);
        m_central_conn_handle = BLE_CONN_HANDLE_INVALID;
        fflush(stdout);

        // If we have a connection to a central device we disconnect from it since we have no
        // peripheral that gives us values to propagate.
        if (m_peripheral_conn_handle != BLE_CONN_HANDLE_INVALID)
        {
            sd_ble_gap_disconnect(m_peripheral_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        }

        scan_start();
    }
}

static void on_adv_report(const ble_gap_evt_t * const p_ble_gap_evt)
{
    uint32_t err_code;
    data_t adv_data;
    data_t type_data;
    uint8_t str[STRING_BUFFER_SIZE] = {0};

    ble_address_to_string_convert(p_ble_gap_evt->params.adv_report.peer_addr, str);
    printf("Received advertisment report, address: 0x%s\n", str); fflush(stdout);

    adv_data.p_data = (uint8_t *)(p_ble_gap_evt->params.adv_report.data);
    adv_data.data_len = p_ble_gap_evt->params.adv_report.dlen;

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

    if (0 != memcmp(TARGET_DEV_NAME, type_data.p_data, type_data.data_len))
    {
        return;
    }

    if (m_central_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        return;
    }

    if (m_connection_is_in_progress)
    {
        return;
    }

    err_code = sd_ble_gap_connect(&(p_ble_gap_evt->params.adv_report.peer_addr),
                                    &m_scan_param,
                                    &m_connection_param);

    if (err_code != NRF_SUCCESS)
    {
        printf("Connection Request Failed, reason %d\n", err_code); fflush(stdout);
        return;
    }

    m_connection_is_in_progress = true;
}

static void on_timeout(const ble_gap_evt_t * const p_ble_gap_evt)
{
    if (p_ble_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
    {
        m_connection_is_in_progress = false;
        scan_start();
    }
    else if (p_ble_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISING)
    {
        advertising_start();
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

    m_service_start_handle = service->handle_range.start_handle;
    m_service_end_handle = service->handle_range.end_handle;

    printf("UUID: 0x%04X, start handle: 0x%04X, end handle: 0x%04X\n",
        service->uuid.uuid, m_service_start_handle, m_service_end_handle);
    fflush(stdout);

    characteristic_discovery_start();
}

static void on_characteristic_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
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

        if (p_characteristic->uuid.uuid == BLE_UUID_HEART_RATE_MEASUREMENT_CHAR)
        {
            m_hrm_char_handle = p_characteristic->handle_decl;
        }
    }

    descriptor_discovery_start();
}

static void on_descriptor_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
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

        if (p_descriptor->uuid.uuid == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
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
    ble_gattc_evt_hvx_t hvx_event = p_ble_gattc_evt->params.hvx;
    int i = 0;
    uint16_t handle = hvx_event.handle;
    uint16_t length = hvx_event.len;
    uint8_t * data = hvx_event.data;

    printf("Received handle value notification, handle: 0x%04X, value: 0x", handle);
    fflush(stdout);

    for (i = 0; i < length; i++)
    {
        printf("%02X", data[i]);
        fflush(stdout);
    }

    printf("\n"); fflush(stdout);

    if (m_peripheral_conn_handle != BLE_CONN_HANDLE_INVALID && m_send_notifications)
    {
        heart_rate_relay_send(length, data);
    }
}

static void on_write(const ble_gatts_evt_t * const p_ble_gatts_evt)
{
    ble_gatts_evt_write_t write_evt = p_ble_gatts_evt->params.write;

    if (write_evt.context.char_uuid.uuid == BLE_UUID_HEART_RATE_MEASUREMENT_CHAR)
    {
        uint8_t cccd_value = write_evt.data[0];
        hrm_cccd_set(cccd_value);
        m_send_notifications = cccd_value == BLE_GATT_HVX_NOTIFICATION;
    }
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
    uint32_t index;

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

static uint32_t scan_start()
{
    uint32_t error_code = sd_ble_gap_scan_start(&m_scan_param);

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
    srvc_uuid.uuid = BLE_UUID_HEART_RATE_SERVICE;

    err_code = sd_ble_gattc_primary_services_discover(m_central_conn_handle, start_handle,
                                                      &srvc_uuid);
    if (err_code != NRF_SUCCESS)
    {
        printf("Failed to discover primary services\n"); fflush(stdout);
        return err_code;
    }

    return NRF_SUCCESS;
}

static uint32_t characteristic_discovery_start()
{
    uint32_t err_code;
    ble_gattc_handle_range_t handle_range;

    printf("Discovering characteristics\n"); fflush(stdout);

    handle_range.start_handle = m_service_start_handle;
    handle_range.end_handle = m_service_end_handle;

    err_code = sd_ble_gattc_characteristics_discover(m_central_conn_handle, &handle_range);

    return err_code;
}

static uint32_t descriptor_discovery_start()
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

    err_code = sd_ble_gattc_descriptors_discover(m_central_conn_handle, &handle_range);
    return err_code;
}

static uint32_t characteristic_init()
{
    uint32_t            error_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint8_t             encoded_initial_hrm[2] = {0,0};

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

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 2;
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
    err_code = sd_ble_gattc_write(m_central_conn_handle, &write_params);

    return err_code;
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
        on_connected(&(p_ble_evt->evt.gap_evt));
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        on_disconnected(&(p_ble_evt->evt.gap_evt));
        break;

    case BLE_GAP_EVT_ADV_REPORT:
        on_adv_report(&(p_ble_evt->evt.gap_evt));
        break;

    case BLE_GAP_EVT_TIMEOUT:
        on_timeout(&(p_ble_evt->evt.gap_evt));
        break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        sd_ble_gap_sec_params_reply(m_peripheral_conn_handle,
                                    BLE_GAP_SEC_STATUS_SUCCESS, NULL, NULL);
        break;

    case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
        on_service_discovery_response(&(p_ble_evt->evt.gattc_evt));
        break;

    case BLE_GATTC_EVT_CHAR_DISC_RSP:
        on_characteristic_discovery_response(&(p_ble_evt->evt.gattc_evt));
        break;

    case BLE_GATTC_EVT_DESC_DISC_RSP:
        on_descriptor_discovery_response(&(p_ble_evt->evt.gattc_evt));
        break;

    case BLE_GATTC_EVT_HVX:
        on_hvx(&(p_ble_evt->evt.gattc_evt));
        break;

    case BLE_GATTC_EVT_WRITE_RSP:
        on_write_response(&(p_ble_evt->evt.gattc_evt));
        break;

    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        sd_ble_gatts_sys_attr_set(m_peripheral_conn_handle, NULL, 0, 0);
        break;

    case BLE_GATTS_EVT_WRITE:
        on_write(&(p_ble_evt->evt.gatts_evt));
        break;

    case BLE_EVT_TX_COMPLETE:
        // do nothing
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

    sd_rpc_serial_port_name_set(serial_port);

    sd_rpc_serial_baud_rate_set(115200);

    sd_rpc_log_handler_set(log_handler);

    sd_rpc_evt_handler_set(ble_evt_dispatch);

    error_code = sd_rpc_open();

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to open the nRF51 BLE Driver. Error code: 0x%02X\n", error_code);
        fflush(stdout);
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

    scan_start();


    while (true)
    {
        char c = getchar();
        if (c == 'q')
        {
            break;
        }

        cccd_value ^= CCCD_NOTIFY;
        hrm_cccd_set(cccd_value);
    }

    if (m_peripheral_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        sd_ble_gap_disconnect(m_peripheral_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    }

    if (m_central_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        sd_ble_gap_disconnect(m_peripheral_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    }

    error_code = sd_rpc_close();

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to close the nRF51 BLE Driver. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    printf("Closed\n"); fflush(stdout);

    return 0;
}
