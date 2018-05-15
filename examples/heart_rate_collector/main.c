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
/**@example examples/heart_rate_collector
 *
 * @brief Heart Rate Collector Sample Application main file.
 *
 * This file contains the source code for a sample application that acts as a BLE Central device.
 * This application scans for a Heart Rate Sensor device and reads it's heart rate data.
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.heart_rate.xml
 */

#include "ble.h"
#include "sd_rpc.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define UART_PORT_NAME "COM1"
#define BAUD_RATE 1000000 /**< The baud rate to be used for serial communication with nRF5 device. */
#endif
#ifdef __APPLE__
#define UART_PORT_NAME "/dev/tty.usbmodem00000"
#define BAUD_RATE 115200 /**< Baud rate 1M is not supported on MacOS. */
#endif
#ifdef __linux__
#define UART_PORT_NAME "/dev/ttyACM0"
#define BAUD_RATE 1000000
#endif

enum
{
    UNIT_0_625_MS = 625,  /**< Number of microseconds in 0.625 milliseconds. */
    UNIT_1_25_MS  = 1250, /**< Number of microseconds in 1.25 milliseconds. */
    UNIT_10_MS    = 10000 /**< Number of microseconds in 10 milliseconds. */
};

#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME) * 1000) / (RESOLUTION))

#define SCAN_INTERVAL 0x00A0 /**< Determines scan interval in units of 0.625 milliseconds. */
#define SCAN_WINDOW   0x0050 /**< Determines scan window in units of 0.625 milliseconds. */
#define SCAN_TIMEOUT  0x0    /**< Scan timeout between 0x01 and 0xFFFF in seconds, 0x0 disables timeout. */

#define MIN_CONNECTION_INTERVAL         MSEC_TO_UNITS(7.5, UNIT_1_25_MS) /**< Determines minimum connection interval in milliseconds. */
#define MAX_CONNECTION_INTERVAL         MSEC_TO_UNITS(7.5, UNIT_1_25_MS) /**< Determines maximum connection interval in milliseconds. */
#define SLAVE_LATENCY                   0                                /**< Slave Latency in number of connection events. */
#define CONNECTION_SUPERVISION_TIMEOUT  MSEC_TO_UNITS(4000, UNIT_10_MS)  /**< Determines supervision time-out in units of 10 milliseconds. */

#define TARGET_DEV_NAME "Nordic_HRM" /**< Connect to a peripheral using a given advertising name here. */
#define MAX_PEER_COUNT 1            /**< Maximum number of peer's application intends to manage. */

#define BLE_UUID_HEART_RATE_SERVICE          0x180D /**< Heart Rate service UUID. */
#define BLE_UUID_HEART_RATE_MEASUREMENT_CHAR 0x2A37 /**< Heart Rate Measurement characteristic UUID. */
#define BLE_UUID_CCCD                        0x2902
#define BLE_CCCD_NOTIFY                      0x01

#define STRING_BUFFER_SIZE 50

typedef struct
{
    uint8_t *     p_data;   /**< Pointer to data. */
    uint16_t      data_len; /**< Length of data. */
} data_t;

static uint8_t     m_connected_devices = 0;
static uint16_t    m_connection_handle = 0;
static uint16_t    m_service_start_handle = 0;
static uint16_t    m_service_end_handle = 0;
static uint16_t    m_hrm_char_handle = 0;
static uint16_t    m_hrm_cccd_handle = 0;
static bool        m_connection_is_in_progress = false;
static adapter_t * m_adapter = NULL;

static const ble_gap_scan_params_t m_scan_param =
{
     1,                       // Active scanning set.
     0,                       // Selective scanning not set.
#if NRF_SD_BLE_API == 2
     NULL,                    // White-list not set.
#endif
#if NRF_SD_BLE_API >= 3
     0,                       // adv_dir_report not set.
#endif

     (uint16_t)SCAN_INTERVAL,
     (uint16_t)SCAN_WINDOW,
     (uint16_t)SCAN_TIMEOUT
};

static const ble_gap_conn_params_t m_connection_param =
{
    (uint16_t)MIN_CONNECTION_INTERVAL,
    (uint16_t)MAX_CONNECTION_INTERVAL,
    (uint16_t)SLAVE_LATENCY,
    (uint16_t)CONNECTION_SUPERVISION_TIMEOUT
};

/* Local function forward declarations */
static uint32_t ble_stack_init();
static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata);
static bool find_adv_name(const ble_gap_evt_adv_report_t *p_adv_report, const char * name_to_find);
static uint32_t scan_start();
static uint32_t service_discovery_start();
static uint32_t char_discovery_start();
static uint32_t descr_discovery_start();
static uint32_t hrm_cccd_set(uint8_t value);
static void ble_address_to_string_convert(ble_gap_addr_t address, uint8_t * string_buffer);

/**@brief Function for handling error message events from sd_rpc.
 *
 * @param[in] adapter The transport adapter.
 * @param[in] code Error code that the error message is associated with.
 * @param[in] message The error message that the callback is associated with.
 */
static void status_handler(adapter_t * adapter, sd_rpc_app_status_t code, const char * message)
{
    printf("Status: %d, message: %s\n", (uint32_t)code, message);
    fflush(stdout);
}

/**@brief Function for handling the log message events from sd_rpc.
 *
 * @param[in] adapter The transport adapter.
 * @param[in] severity Level of severity that the log message is associated with.
 * @param[in] message The log message that the callback is associated with.
 */
static void log_handler(adapter_t * adapter, sd_rpc_log_severity_t severity, const char * message)
{
    switch (severity)
    {
        case SD_RPC_LOG_ERROR:
            printf("Error: %s\n", message);
            fflush(stdout);
            break;

        case SD_RPC_LOG_WARNING:
            printf("Warning: %s\n", message);
            fflush(stdout);
            break;

        case SD_RPC_LOG_INFO:
            printf("Info: %s\n", message);
            fflush(stdout);
            break;

        default:
            printf("Log: %s\n", message);
            fflush(stdout);
            break;
    }
}

/**@brief Function called on BLE_GAP_EVT_CONNECTED event.
 *
 * @details Update connection state and proceed to discovering the peer's GATT services.
 *
 * @param[in] p_ble_gap_evt GAP event.
 */
static void on_connected(const ble_gap_evt_t * const p_ble_gap_evt)
{
    printf("Connection established\n");
    fflush(stdout);

    m_connected_devices++;
    m_connection_handle = p_ble_gap_evt->conn_handle;
    m_connection_is_in_progress = false;

    service_discovery_start();
}

/**@brief Function called on BLE_GAP_EVT_ADV_REPORT event.
 *
 * @details Create a connection if received advertising packet corresponds to desired BLE device.
 *
 * @param[in] p_ble_gap_evt Advertising Report Event.
 */
static void on_adv_report(const ble_gap_evt_t * const p_ble_gap_evt)
{
    uint32_t err_code;
    uint8_t  str[STRING_BUFFER_SIZE] = {0};

    // Log the Bluetooth device address of advertisement packet received.
    ble_address_to_string_convert(p_ble_gap_evt->params.adv_report.peer_addr, str);
    printf("Received advertisement report with device address: 0x%s\n", str);
    fflush(stdout);

    if (find_adv_name(&p_ble_gap_evt->params.adv_report, TARGET_DEV_NAME))
    {
        if (m_connected_devices >= MAX_PEER_COUNT || m_connection_is_in_progress)
        {
            return;
        }

        err_code = sd_ble_gap_connect(m_adapter,
                                      &(p_ble_gap_evt->params.adv_report.peer_addr),
                                      &m_scan_param,
                                      &m_connection_param);

        if (err_code != NRF_SUCCESS)
        {
            printf("Connection Request Failed, reason %d\n", err_code);
            fflush(stdout);
            return;
        }

        m_connection_is_in_progress = true;
    }
}

/**@brief Function called on BLE_GAP_EVT_TIMEOUT event.
 *
 * @param[in] ble_gap_evt_t Timeout Event.
 */
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

/**@brief Function called on BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP event.
 *
 * @details Update service state and proceed to discovering the service's GATT characteristics.
 *
 * @param[in] p_ble_gattc_evt Primary Service Discovery Response Event.
 */
static void on_service_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    int count;
    int service_index;
    const ble_gattc_service_t * service;

    printf("Received service discovery response\n");
    fflush(stdout);

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Service discovery failed. Error code 0x%X\n", p_ble_gattc_evt->gatt_status);
        fflush(stdout);
        return;
    }

    count = p_ble_gattc_evt->params.prim_srvc_disc_rsp.count;

    if (count == 0)
    {
        printf("Service not found\n");
        fflush(stdout);
        return;
    }

    if (count > 1)
    {
        printf("Warning, discovered multiple primary services. Ignoring all but the first\n");
    }

    service_index = 0; /* We expect to discover only the Heart Rate service as requested. */
    service = &(p_ble_gattc_evt->params.prim_srvc_disc_rsp.services[service_index]);

    if (service->uuid.uuid != BLE_UUID_HEART_RATE_SERVICE)
    {
        printf("Unknown service discovered with UUID: 0x%04X\n", service->uuid.uuid);
        fflush(stdout);
        return;
    }

    m_service_start_handle  = service->handle_range.start_handle;
    m_service_end_handle    = service->handle_range.end_handle;

    printf("Discovered heart rate service. UUID: 0x%04X, "
                   "start handle: 0x%04X, end handle: 0x%04X\n",
        service->uuid.uuid, m_service_start_handle, m_service_end_handle);
    fflush(stdout);

    char_discovery_start();
}

/**@brief Function called on BLE_GATTC_EVT_CHAR_DISC_RSP event.
 *
 * @details Update characteristic state and proceed to discovering the characteristicss descriptors.
 *
 * @param[in] p_ble_gattc_evt Characteristic Discovery Response Event.
 */
static void on_characteristic_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    int count = p_ble_gattc_evt->params.char_disc_rsp.count;

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Characteristic discovery failed. Error code 0x%X\n", p_ble_gattc_evt->gatt_status);
        fflush(stdout);
        return;
    }

    printf("Received characteristic discovery response, characteristics count: %d\n", count);
    fflush(stdout);

    for (int i = 0; i < count; i++)
    {
        printf("Characteristic handle: 0x%04X, UUID: 0x%04X\n",
               p_ble_gattc_evt->params.char_disc_rsp.chars[i].handle_decl,
               p_ble_gattc_evt->params.char_disc_rsp.chars[i].uuid.uuid);
        fflush(stdout);

        if (p_ble_gattc_evt->params.char_disc_rsp.chars[i].uuid.uuid ==
            BLE_UUID_HEART_RATE_MEASUREMENT_CHAR)
        {
            m_hrm_char_handle = p_ble_gattc_evt->params.char_disc_rsp.chars[i].handle_decl;
        }
    }

    descr_discovery_start();
}

/**@brief Function called on BLE_GATTC_EVT_DESC_DISC_RSP event.
 *
 * @details Update CCCD descriptor state and proceed to prompting user to toggle notifications.
 *
 * @param[in] p_ble_gattc_evt Descriptor Discovery Response Event.
 */
static void on_descriptor_discovery_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    int count = p_ble_gattc_evt->params.desc_disc_rsp.count;

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Descriptor discovery failed. Error code 0x%X\n", p_ble_gattc_evt->gatt_status);
        fflush(stdout);
        return;
    }

    printf("Received descriptor discovery response, descriptor count: %d\n", count);
    fflush(stdout);

    for (int i = 0; i < count; i++)
    {
        printf("Descriptor handle: 0x%04X, UUID: 0x%04X\n",
               p_ble_gattc_evt->params.desc_disc_rsp.descs[i].handle,
               p_ble_gattc_evt->params.desc_disc_rsp.descs[i].uuid.uuid);
        fflush(stdout);

        if (p_ble_gattc_evt->params.desc_disc_rsp.descs[i].uuid.uuid == BLE_UUID_CCCD)
        {
            m_hrm_cccd_handle = p_ble_gattc_evt->params.desc_disc_rsp.descs[i].handle;
            printf("Press enter to toggle notifications on the HRM characteristic\n");
            fflush(stdout);
        }
    }
}

/**@brief Function called on BLE_GATTC_EVT_WRITE_RSP event.
 *
 * @param[in] p_ble_gattc_evt Write Response Event.
 */
static void on_write_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    printf("Received write response.\n");
    fflush(stdout);

    if (p_ble_gattc_evt->gatt_status != NRF_SUCCESS)
    {
        printf("Error. Write operation failed. Error code 0x%X\n", p_ble_gattc_evt->gatt_status);
        fflush(stdout);
    }
}

/**@brief Function called on BLE_GATTC_EVT_HVX event.
 *
 * @details Logs the received heart rate measurement.
 *
 * @param[in] p_ble_gattc_evt Handle Value Notification/Indication Event.
 */
static void on_hvx(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    if (p_ble_gattc_evt->params.hvx.handle >= m_hrm_char_handle ||
            p_ble_gattc_evt->params.hvx.handle <= m_hrm_cccd_handle) // Heart rate measurement.
    {
        // We know the heart rate reading is encoded as 2 bytes [flag, value].
        printf("Received heart rate measurement: %d\n", p_ble_gattc_evt->params.hvx.data[1]);
    }
    else // Unknown data.
    {
        printf("Un-parsed data received on handle: %04X\n", p_ble_gattc_evt->params.hvx.handle);
    }

    fflush(stdout);
}

/**@brief Function called on BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST event.
 *
 * @details Update GAP connection parameters.
 *
 * @param[in] p_ble_gap_evt Connection Parameter Update Event.
 */
static void on_conn_params_update_request(const ble_gap_evt_t * const p_ble_gap_evt)
{
    uint32_t err_code = sd_ble_gap_conn_param_update(m_adapter, m_connection_handle,
                                            &(p_ble_gap_evt->
                                                    params.conn_param_update_request.conn_params));
    if (err_code != NRF_SUCCESS)
    {
        printf("Conn params update failed, err_code %d\n", err_code);
        fflush(stdout);
    }
}

#if NRF_SD_BLE_API >= 3
/**@brief Function called on BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event.
 *
 * @details Replies to an ATT_MTU exchange request by sending an Exchange MTU Response to the client.
 *
 * @param[in] p_ble_gatts_evt Exchange MTU Request Event.
 */
static void on_exchange_mtu_request(const ble_gatts_evt_t * const p_ble_gatts_evt)
{
    uint32_t err_code = sd_ble_gatts_exchange_mtu_reply(m_adapter, m_connection_handle,
                                                        GATT_MTU_SIZE_DEFAULT);

    if (err_code != NRF_SUCCESS)
    {
        printf("MTU exchange request reply failed, err_code %d\n", err_code);
        fflush(stdout);
    }
}

/**@brief Function called on BLE_GATTC_EVT_EXCHANGE_MTU_RSP event.
 *
 * @details Logs the new BLE server RX MTU size.
 *
 * @param[in] p_ble_gattc_evt Exchange MTU Response Event.
 */
static void on_exchange_mtu_response(const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    uint16_t server_rx_mtu = p_ble_gattc_evt->params.exchange_mtu_rsp.server_rx_mtu;

    printf("MTU response received. New ATT_MTU is %d\n", server_rx_mtu);
    fflush(stdout);
}
#endif

/**@brief Function for initializing serial communication with the target nRF5 Bluetooth slave.
 *
 * @param[in] serial_port The serial port the target nRF5 device is connected to.
 *
 * @return The new transport adapter.
 */
static adapter_t * adapter_init(char * serial_port)
{
    physical_layer_t  * phy;
    data_link_layer_t * data_link_layer;
    transport_layer_t * transport_layer;

    phy = sd_rpc_physical_layer_create_uart(serial_port, BAUD_RATE, SD_RPC_FLOW_CONTROL_NONE,
                                            SD_RPC_PARITY_NONE);
    data_link_layer = sd_rpc_data_link_layer_create_bt_three_wire(phy, 100);
    transport_layer = sd_rpc_transport_layer_create(data_link_layer, 100);
    return sd_rpc_adapter_create(transport_layer);
}

/**@brief Function for converting a BLE address to a string.
 *
 * @param[in] address       Bluetooth Low Energy address.
 * @param[out] string_buffer The serial port the target nRF5 device is connected to.
 */
static void ble_address_to_string_convert(ble_gap_addr_t address, uint8_t * string_buffer)
{
    const int address_length = 6;
    char      temp_str[3];

    for (int i = address_length - 1; i >= 0; --i)
    {
        sprintf(temp_str, "%02X", address.addr[i]);
        strcat((char *)string_buffer, temp_str);
    }
}

/**
 * @brief Parses advertisement data, providing length and location of the field in case
 *        matching data is found.
 *
 * @param[in]  Type of data to be looked for in advertisement data.
 * @param[in]  Advertisement report length and pointer to report.
 * @param[out] If data type requested is found in the data report, type data length and
 *             pointer to data will be populated here.
 *
 * @retval NRF_SUCCESS if the data type is found in the report.
 * @retval NRF_ERROR_NOT_FOUND if the data type could not be found.
 */
static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint32_t  index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->data_len)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if (field_type == type)
        {
            p_typedata->p_data   = &p_data[index + 2];
            p_typedata->data_len = field_length - 1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
}

/**@brief Function for searching a given name in the advertisement packets.
 *
 * @details Use this function to parse received advertising data and to find a given
 * name in them either as 'complete_local_name' or as 'short_local_name'.
 *
 * @param[in]   p_adv_report   advertising data to parse.
 * @param[in]   name_to_find   name to search.
 * @return   true if the given name was found, false otherwise.
 */
static bool find_adv_name(const ble_gap_evt_adv_report_t *p_adv_report, const char * name_to_find)
{
    uint32_t err_code;
    data_t   adv_data;
    data_t   dev_name;

    // Initialize advertisement report for parsing
    adv_data.p_data     = (uint8_t *)p_adv_report->data;
    adv_data.data_len   = p_adv_report->dlen;


    //search for advertising names
    err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
                                &adv_data,
                                &dev_name);
    if (err_code == NRF_SUCCESS)
    {
        if (memcmp(name_to_find, dev_name.p_data, dev_name.data_len )== 0)
        {
            return true;
        }
    }
    else
    {
        // Look for the short local name if it was not found as complete
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME,
                                    &adv_data,
                                    &dev_name);
        if (err_code != NRF_SUCCESS)
        {
            return false;
        }
        if (memcmp(name_to_find, dev_name.p_data, dev_name.data_len )== 0)
        {
            return true;
        }
    }
    return false;
}

/**@brief Function for initializing the BLE stack.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t ble_stack_init()
{
    uint32_t            err_code;
    ble_enable_params_t ble_enable_params;
    uint32_t *          app_ram_base = NULL;

    memset(&ble_enable_params, 0, sizeof(ble_enable_params));

#if NRF_SD_BLE_API >= 3
    ble_enable_params.gatt_enable_params.att_mtu = GATT_MTU_SIZE_DEFAULT;
#endif
    ble_enable_params.gatts_enable_params.attr_tab_size     = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
    ble_enable_params.gatts_enable_params.service_changed   = false;
    ble_enable_params.gap_enable_params.periph_conn_count   = 1;
    ble_enable_params.gap_enable_params.central_conn_count  = 1;
    ble_enable_params.gap_enable_params.central_sec_count   = 1;
    ble_enable_params.common_enable_params.p_conn_bw_counts = NULL;
    ble_enable_params.common_enable_params.vs_uuid_count    = 1;

    err_code = sd_ble_enable(m_adapter, &ble_enable_params, app_ram_base);

    switch (err_code) {
        case NRF_SUCCESS:
            break;
        case NRF_ERROR_INVALID_STATE:
            printf("BLE stack already enabled\n");
            fflush(stdout);
            break;
        default:
            printf("Failed to enable BLE stack. Error code: %d\n", err_code);
            fflush(stdout);
            break;
    }

    return err_code;
}

/**@brief Set BLE option for the BLE role and connection bandwidth.
 *
 * @return NRF_SUCCESS on option set successfully, otherwise an error code.
 */
static uint32_t ble_options_set()
{
    ble_opt_t        opt;
    ble_common_opt_t common_opt;

    common_opt.conn_bw.role = BLE_GAP_ROLE_CENTRAL;
    common_opt.conn_bw.conn_bw.conn_bw_rx = BLE_CONN_BW_HIGH;
    common_opt.conn_bw.conn_bw.conn_bw_tx = BLE_CONN_BW_HIGH;
    opt.common_opt = common_opt;

    return sd_ble_opt_set(m_adapter, BLE_COMMON_OPT_CONN_BW, &opt);
}

/**@brief Start scanning (GAP Discovery procedure, Observer Procedure).
 * *
 * @return NRF_SUCCESS on successfully initiating scanning procedure, otherwise an error code.
 */
static uint32_t scan_start()
{
    uint32_t error_code = sd_ble_gap_scan_start(m_adapter, &m_scan_param);

    if (error_code != NRF_SUCCESS)
    {
        printf("Scan start failed\n");
        fflush(stdout);
    } else
    {
        printf("Scan started\n");
        fflush(stdout);
    }

    return error_code;
}

/**@brief Function called upon connecting to BLE peripheral.
 *
 * @details Initiates primary service discovery.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t service_discovery_start()
{
    uint32_t   err_code;
    uint16_t   start_handle = 0x01;
    ble_uuid_t srvc_uuid;

    printf("Discovering primary services\n");
    fflush(stdout);

    srvc_uuid.type = BLE_UUID_TYPE_BLE;
    srvc_uuid.uuid = BLE_UUID_HEART_RATE_SERVICE;

    // Initiate procedure to find the primary BLE_UUID_HEART_RATE_SERVICE.
    err_code = sd_ble_gattc_primary_services_discover(m_adapter,
                                                      m_connection_handle, start_handle,
                                                      &srvc_uuid);
    if (err_code != NRF_SUCCESS)
    {
        printf("Failed to initiate or continue a GATT Primary Service Discovery procedure\n");
        fflush(stdout);
    }

    return err_code;
}

/**@brief Function called upon discovering a BLE peripheral's primary service(s).
 *
 * @details Initiates service's (m_service) characteristic discovery.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t char_discovery_start()
{
    ble_gattc_handle_range_t handle_range;

    printf("Discovering characteristics\n");
    fflush(stdout);

    handle_range.start_handle = m_service_start_handle;
    handle_range.end_handle = m_service_end_handle;

    return sd_ble_gattc_characteristics_discover(m_adapter, m_connection_handle, &handle_range);
}

/**@brief Function called upon discovering service's characteristics.
 *
 * @details Initiates heart rate monitor (m_hrm_char_handle) characteristic's descriptor discovery.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t descr_discovery_start()
{
    ble_gattc_handle_range_t handle_range;

    printf("Discovering characteristic's descriptors\n");
    fflush(stdout);

    if (m_hrm_char_handle == 0)
    {
        printf("No heart rate measurement characteristic handle found\n");
        fflush(stdout);
        return NRF_ERROR_INVALID_STATE;
    }

    handle_range.start_handle = m_hrm_char_handle;
    handle_range.end_handle = m_service_end_handle;

    return sd_ble_gattc_descriptors_discover(m_adapter, m_connection_handle, &handle_range);
}

/**@brief Function that write's the HRM characteristic's CCCD.
 * *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t hrm_cccd_set(uint8_t value)
{
    ble_gattc_write_params_t write_params;
    uint8_t                  cccd_value[2] = {value, 0};

    printf("Setting HRM CCCD\n");
    fflush(stdout);

    if (m_hrm_cccd_handle == 0)
    {
        printf("Error. No CCCD handle has been found\n");
        fflush(stdout);
        return NRF_ERROR_INVALID_STATE;
    }

    write_params.handle = m_hrm_cccd_handle;
    write_params.len = 2;
    write_params.p_value = cccd_value;
    write_params.write_op = BLE_GATT_OP_WRITE_REQ;
    write_params.offset = 0;

    return sd_ble_gattc_write(m_adapter, m_connection_handle, &write_params);
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] adapter The transport adapter.
 * @param[in] p_ble_evt Bluetooth stack event.
 */
static void ble_evt_dispatch(adapter_t * adapter, ble_evt_t * p_ble_evt)
{
    if (p_ble_evt == NULL)
    {
        printf("Received an empty BLE event\n");
        fflush(stdout);
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connected(&(p_ble_evt->evt.gap_evt));
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            printf("Disconnected, reason: 0x%02X\n",
                   p_ble_evt->evt.gap_evt.params.disconnected.reason);
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
            on_characteristic_discovery_response(&(p_ble_evt->evt.gattc_evt));
            break;

        case BLE_GATTC_EVT_DESC_DISC_RSP:
            on_descriptor_discovery_response(&(p_ble_evt->evt.gattc_evt));
            break;

        case BLE_GATTC_EVT_WRITE_RSP:
            on_write_response(&(p_ble_evt->evt.gattc_evt));
            break;

        case BLE_GATTC_EVT_HVX:
            on_hvx(&(p_ble_evt->evt.gattc_evt));
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            on_conn_params_update_request(&(p_ble_evt->evt.gap_evt));
            break;

    #if NRF_SD_BLE_API >= 3
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            on_exchange_mtu_request(&(p_ble_evt->evt.gatts_evt));
            break;

        case BLE_GATTC_EVT_EXCHANGE_MTU_RSP:
            on_exchange_mtu_response(&(p_ble_evt->evt.gattc_evt));
            break;
    #endif

        default:
            printf("Received an un-handled event with ID: %d\n", p_ble_evt->header.evt_id);
            fflush(stdout);
            break;
    }
}

/**@brief Function for application main entry.
 *
 * @param[in] argc Number of arguments (program expects 0 or 1 arguments).
 * @param[in] argv The serial port of the target nRF5 device (Optional).
 */
int main(int argc, char * argv[])
{
    uint32_t error_code;
    char *   serial_port;
    uint8_t  cccd_value = 0;

    if (argc > 1)
    {
        serial_port = argv[1];
    }
    else
    {
        serial_port = UART_PORT_NAME;
    }

    printf("Serial port used: %s\n", serial_port);
    fflush(stdout);

    m_adapter =  adapter_init(serial_port);
    sd_rpc_log_handler_severity_filter_set(m_adapter, SD_RPC_LOG_INFO);
    error_code = sd_rpc_open(m_adapter, status_handler, ble_evt_dispatch, log_handler);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to open nRF BLE Driver. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    error_code = ble_stack_init();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    error_code = ble_options_set();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }
    
    error_code = scan_start();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    // Endlessly loop.
    for (; ;)
    {
        char c = (char)getchar();
        if (c == 'q' || c == 'Q')
        {
            error_code = sd_ble_gap_scan_stop(m_adapter);

            if (error_code != NRF_SUCCESS)
            {
                printf("Failed to stop scanning. Error code: 0x%02X\n", error_code);
                fflush(stdout);
            }

            error_code = sd_rpc_close(m_adapter);

            if (error_code != NRF_SUCCESS)
            {
                printf("Failed to close nRF BLE Driver. Error code: 0x%02X\n", error_code);
                fflush(stdout);
                return error_code;
            }

            printf("Closed\n");
            fflush(stdout);

            return NRF_SUCCESS;
        }

        // Toggle notifications on the HRM characteristic every time user input is received.
        cccd_value ^= BLE_CCCD_NOTIFY;
        hrm_cccd_set(cccd_value);
    }
}
