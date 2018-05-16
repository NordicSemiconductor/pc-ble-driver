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
/**@example examples/heart_rate_monitor
 *
 * @brief Heart Rate Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Heart Rate service.
 * This service exposes heart rate data from a Heart Rate Sensor intended for fitness applications.
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.heart_rate.xml
 */

#include "ble.h"
#include "sd_rpc.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

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
#define ADVERTISING_INTERVAL_40_MS 64  /**< 0.625 ms = 40 ms */
#define ADVERTISING_TIMEOUT_3_MIN  180 /**< 1 sec = 3 min */

#define OPCODE_LENGTH 1                                                 /**< Length of opcode inside Heart Rate Measurement packet. */
#define HANDLE_LENGTH 2                                                 /**< Length of handle inside Heart Rate Measurement packet. */
#define MAX_HRM_LEN (BLE_L2CAP_MTU_DEF - OPCODE_LENGTH - HANDLE_LENGTH) /**< Maximum size of a transmitted Heart Rate Measurement. */

#define BLE_UUID_HEART_RATE_SERVICE          0x180D /**< Heart Rate service UUID. */
#define BLE_UUID_HEART_RATE_MEASUREMENT_CHAR 0x2A37 /**< Heart Rate Measurement characteristic UUID. */

#define HEART_RATE_BASE     65
#define HEART_RATE_INCREASE 3
#define HEART_RATE_LIMIT    190

#define BUFFER_SIZE 30           /**< Sufficiently large buffer for the advertising data.  */
#define DEVICE_NAME "Nordic_HRM" /**< Name device advertises as over Bluetooth. */

static uint16_t                 m_connection_handle             = BLE_CONN_HANDLE_INVALID;
static uint16_t                 m_heart_rate_service_handle     = 0;
static ble_gatts_char_handles_t m_heart_rate_measurement_handle;
static uint8_t                  m_heart_rate                    = HEART_RATE_BASE;
static bool                     m_send_notifications            = false;
static bool                     m_advertisement_timed_out       = false;
static adapter_t *              m_adapter                       = NULL;

static uint32_t advertising_start();

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

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] adapter The transport adapter.
 * @param[in] p_ble_evt Bluetooth stack event.
 */
static void ble_evt_dispatch(adapter_t * adapter, ble_evt_t * p_ble_evt)
{
    uint32_t err_code;

    if (p_ble_evt == NULL)
    {
        printf("Received an empty BLE event\n");
        fflush(stdout);
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_connection_handle = p_ble_evt->evt.gap_evt.conn_handle;
            printf("Connected, connection handle 0x%04X\n", m_connection_handle);
            fflush(stdout);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            printf("Disconnected\n");
            fflush(stdout);
            m_connection_handle = BLE_CONN_HANDLE_INVALID;
            m_send_notifications = false;
            advertising_start();
            break;

        case BLE_GAP_EVT_TIMEOUT:
            printf("Advertisement timed out\n");
            fflush(stdout);
            m_advertisement_timed_out = true;
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            err_code = sd_ble_gap_sec_params_reply(adapter, m_connection_handle,
                                                   BLE_GAP_SEC_STATUS_SUCCESS, NULL, NULL);

            if (err_code != NRF_SUCCESS)
            {
                printf("Failed reply with GAP security parameters. Error code: 0x%02X\n", err_code);
                fflush(stdout);
            }
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            err_code = sd_ble_gatts_sys_attr_set(adapter, m_connection_handle, NULL, 0, 0);

            if (err_code != NRF_SUCCESS)
            {
                printf("Failed updating persistent sys attr info. Error code: 0x%02X\n", err_code);
                fflush(stdout);
            }
            break;

        case BLE_GATTS_EVT_WRITE:
            if (p_ble_evt->evt.gatts_evt.params.write.handle ==
                    m_heart_rate_measurement_handle.cccd_handle)
            {
                uint8_t write_data = p_ble_evt->evt.gatts_evt.params.write.data[0];
                m_send_notifications = write_data == BLE_GATT_HVX_NOTIFICATION;
            }
            break;

#if NRF_SD_BLE_API >= 3
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            err_code = sd_ble_gatts_exchange_mtu_reply(adapter, m_connection_handle,
                                                       GATT_MTU_SIZE_DEFAULT);

            if (err_code != NRF_SUCCESS)
            {
                printf("MTU exchange request reply failed. Error code: 0x%02X\n", err_code);
                fflush(stdout);
            }
            break;
#endif

        case BLE_EVT_TX_COMPLETE:
#ifdef DEBUG
            printf("Successfully transmitted a heart rate reading.");
            fflush(stdout);
#endif
            break;

        default:
            printf("Received an un-handled event with ID: %d\n", p_ble_evt->header.evt_id);
            fflush(stdout);
            break;
        }
}

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

    ble_enable_params.gatts_enable_params.attr_tab_size     = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
    ble_enable_params.gatts_enable_params.service_changed   = false;
    ble_enable_params.gap_enable_params.periph_conn_count   = 1;
    ble_enable_params.gap_enable_params.central_conn_count  = 0;
    ble_enable_params.gap_enable_params.central_sec_count   = 0;
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

/**@brief Function for setting the advertisement data.
 *
 * @details Sets the full device name and its available BLE services in the advertisement data.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t advertisement_data_set()
{
    uint32_t error_code;
    uint8_t  index = 0;
    uint8_t  data_buffer[BUFFER_SIZE];

    const char  * device_name = DEVICE_NAME;
    const uint8_t name_length = strlen(device_name);
    const uint8_t data_type   = BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME;

    // Set the device name.
    data_buffer[index++] = name_length + 1; // Device name + data type
    data_buffer[index++] = data_type;
    memcpy((char *)&data_buffer[index], device_name, name_length);
    index += name_length;

    // Set the device's available services.
    data_buffer[index++] = 3;
    data_buffer[index++] = BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE;
    // Store BLE_UUID_HEART_RATE_SERVICE in little-endian format.
    data_buffer[index++] = BLE_UUID_HEART_RATE_SERVICE & 0xFF;
    data_buffer[index++] = (BLE_UUID_HEART_RATE_SERVICE & 0xFF00) >> 8;

    // No scan response.
    const uint8_t * sr_data        = NULL;
    const uint8_t   sr_data_length = 0;

    error_code = sd_ble_gap_adv_data_set(m_adapter, data_buffer, index, sr_data, sr_data_length);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to set advertisement data. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    printf("Advertising data set\n");
    fflush(stdout);
    return NRF_SUCCESS;
}

/**@brief Function for initializing the Advertising functionality and starting advertising.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t advertising_start()
{
    uint32_t             error_code;
    ble_gap_adv_params_t adv_params;

    adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    adv_params.p_peer_addr = NULL;                        // Undirected advertisement.
    adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    adv_params.interval    = ADVERTISING_INTERVAL_40_MS;
    adv_params.timeout     = ADVERTISING_TIMEOUT_3_MIN;
#if NRF_SD_BLE_API == 2
    adv_params.p_whitelist = NULL;
#endif

    error_code = sd_ble_gap_adv_start(m_adapter, &adv_params);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to start advertising. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    printf("Started advertising\n");
    fflush(stdout);
    return NRF_SUCCESS;
}

/**@brief Function for encoding a Heart Rate Measurement.
 *
 * @param[in]   heart_rate    Measurement to be encoded.
 * @param[out]  encoded_hrm   Buffer where the encoded data will be written.
 *
 * @return      Size of encoded data.
 */
static uint8_t heart_rate_measurement_encode(uint8_t * encoded_hrm, uint8_t heart_rate)
{
    uint8_t flags = 0;

    encoded_hrm[0] = flags;
    encoded_hrm[1] = heart_rate;

    return 2;
}

/**@brief Function for adding the Heart Rate Measurement characteristic.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
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

    error_code = sd_ble_gatts_characteristic_add(m_adapter, m_heart_rate_service_handle,
                                                 &char_md,
                                                 &attr_char_value,
                                                 &m_heart_rate_measurement_handle);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to initialize characteristics. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    printf("Characteristics initiated\n");
    fflush(stdout);
    return NRF_SUCCESS;
}

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate service and it's characteristics and add to GATT.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t services_init()
{
    uint32_t    error_code;
    ble_uuid_t  ble_uuid;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_HEART_RATE_SERVICE);

    error_code = sd_ble_gatts_service_add(m_adapter,
                                          BLE_GATTS_SRVC_TYPE_PRIMARY,
                                          &ble_uuid,
                                          &m_heart_rate_service_handle);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to initialize heart rate service. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    printf("Services initiated\n");
    fflush(stdout);

    error_code = characteristic_init();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for simulating a heart rate sensor reading.
 *
 * @details This modifies a global variable `m_heart_rate`.
 */
static void heart_rate_generate()
{
    m_heart_rate += HEART_RATE_INCREASE;

    if (m_heart_rate > HEART_RATE_LIMIT)
    {
        m_heart_rate = HEART_RATE_BASE;
    }
}

/**@brief Function for sending the heart rate measurement over Bluetooth.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
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

    error_code = sd_ble_gatts_hvx(m_adapter, m_connection_handle, &hvx_params);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to send heart rate measurement. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for application main entry.
 *
 * @param[in]   argc    Number of arguments (program expects 0 or 1 arguments).
 * @param[in]   argv    The serial port of the target nRF5 device (Optional).
 */
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

    error_code = advertisement_data_set();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    error_code = services_init();

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
        if (m_connection_handle != BLE_CONN_HANDLE_INVALID && m_send_notifications)
        {
            heart_rate_measurement_send();
        }

		Sleep(1000);
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
