/**
 * Copyright (c) 2013 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_6lowpan_eval_main 6LoWPAN Adaptation Layer.
 * @{
 * @ingroup ble_sdk_6lowpan_eval
 * @brief 6LoWPAN Adaptation Layer Example.
 */

#include <stdbool.h>
#include <stdint.h>
#if NRF_SD_BLE_API >= 6
//TODO:
#include "ble_srv_common.h"
#include "ble_ipsp.h"

#include "sd_rpc.h"

/** Definitions */
#define DEFAULT_BAUD_RATE 1000000 /**< The baud rate to be used for serial communication with nRF5 device. */

#ifdef _WIN32
#define DEFAULT_UART_PORT_NAME "COM1"
#endif
#ifdef __APPLE__
#define DEFAULT_UART_PORT_NAME "/dev/tty.usbmodem00000"
#endif
#ifdef __linux__
#define DEFAULT_UART_PORT_NAME "/dev/ttyACM0"
#endif

/** Global variables */
static adapter_t *m_adapter = NULL;

//TODO: app_error.h
#define APP_ERROR_CHECK(err_code) {if (err_code !=  NRF_SUCCESS) printf("APP_ERROR_CHECK:%d (%d)\n", __LINE__, err_code); }
//TODO: nrf_log.h
#define NRF_LOG_INFO printf


#define APP_IPSP_TAG                        35                                                      /**< Identifier for L2CAP configuration with the softdevice. */
#define APP_IPSP_INITIATOR_PRIO             1                                                       /**< Priority with the SDH on receiving events from the softdevice. */
//#define SCANNING_LED                        BSP_LED_0_MASK                                          /**< Is on when device is scanning. */
//#define CONNECTED_LED                       BSP_LED_1_MASK                                          /**< Is on when device is connected. */
#define BUTTON_DETECTION_DELAY              APP_TIMER_TICKS(50)
#define SCAN_INTERVAL                       0x00A0                                                  /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                         0x0050                                                  /**< Determines scan window in units of 0.625 millisecond. */
#define MIN_CONNECTION_INTERVAL             MSEC_TO_UNITS(30, UNIT_1_25_MS)                         /**< Determines maximum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL             MSEC_TO_UNITS(60, UNIT_1_25_MS)                         /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY                       0                                                       /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT                 MSEC_TO_UNITS(4000, UNIT_10_MS)                         /**< Determines supervision time-out in units of 10 millisecond. */
#define DEAD_BEEF                           0xDEADBEEF                                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_ENABLE_LOGS                 1                                                           /**< Enable logs in the application. */

#if (APP_ENABLE_LOGS == 1)

#define APPL_LOG  NRF_LOG_INFO
#define APPL_DUMP NRF_LOG_RAW_HEXDUMP_INFO
#define APPL_ADDR IPV6_ADDRESS_LOG

#else // APP_ENABLE_LOGS

#define APPL_LOG(...)
#define APPL_DUMP(...)
#define APPL_ADDR(...)

#endif // APP_ENABLE_LOGS

static ble_ipsp_handle_t    m_handle;
static uint16_t             m_conn_handle = BLE_CONN_HANDLE_INVALID;
static const ble_gap_addr_t m_peer_addr =
{
    .addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
    .addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00}
};


/**
 * @brief Scan parameters requested for scanning and connection.
 */
static const ble_gap_scan_params_t m_scan_param =
{
     .active         = 0,                          // Passive scanning.
     .filter_policy  = BLE_GAP_SCAN_FP_ACCEPT_ALL, // Do not use whitelist.
     .interval       = (uint16_t)SCAN_INTERVAL,    // Scan interval.
     .window         = (uint16_t)SCAN_WINDOW,      // Scan window.
     .timeout        = 0,                          // Never stop scanning unless explicit asked to.
     .scan_phys      = BLE_GAP_PHY_AUTO            // Automatic PHY selection.
};


/**
 * @brief Connection parameters requested for connection.
 */
static const ble_gap_conn_params_t m_connection_param =
{
    .min_conn_interval = (uint16_t)MIN_CONNECTION_INTERVAL,   // Minimum connection
    .max_conn_interval = (uint16_t)MAX_CONNECTION_INTERVAL,   // Maximum connection
    .slave_latency     = 0,                                   // Slave latency
    .conn_sup_timeout  = (uint16_t)SUPERVISION_TIMEOUT        // Supervision time-out
};


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t const * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            APPL_LOG("Connected, handle 0x%04X.\n", p_ble_evt->evt.gap_evt.conn_handle);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

            if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
            {
                ble_ipsp_handle_t ipsp_handle;
                ipsp_handle.conn_handle = m_conn_handle;
                uint32_t err_code = ble_ipsp_connect(m_adapter, &ipsp_handle);
                if (err_code != NRF_SUCCESS
                    && err_code != NRF_ERROR_BLE_IPSP_CHANNEL_ALREADY_EXISTS) {
                    APPL_LOG("ble_ipsp_connect fail %x", err_code);
                }
            }
            else
            {
                APPL_LOG("No physical link exists with peer");
            }

            break;
        case BLE_GAP_EVT_DISCONNECTED:
            APPL_LOG("Disconnected.");

            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;
        default:
            printf("on_ble_evt - default\n");
            break;
    }
}

/**@brief IPSP event handler.
 * @param[in] p_handle Identifies the connection and channel on which the event occurred.
 * @param[in] p_evt    Event and related parameters (if any).
 *
 * @returns    NRF_SUCCESS.
 */
static uint32_t app_ipsp_event_handler(ble_ipsp_handle_t const * p_handle,
                                       ble_ipsp_evt_t const    * p_evt)
{
    switch (p_evt->evt_id)
    {
        case BLE_IPSP_EVT_CHANNEL_CONNECTED:
        {
            APPL_LOG("[0x%04X]:[0x%04X] BLE_IPSP_EVT_CHANNEL_CONNECTED Event. Result "
                     "0x%08lX", p_handle->conn_handle, p_handle->cid, p_evt->evt_result);
            if (p_evt->evt_result == NRF_SUCCESS)
            {
                m_handle = (*p_handle);
            }
            break;
        }
        case BLE_IPSP_EVT_CHANNEL_DISCONNECTED:
        {
            APPL_LOG("[0x%04X]:[0x%04X] BLE_IPSP_EVT_CHANNEL_DISCONNECTED Event. Result "
                     "0x%08lX", p_handle->conn_handle, p_handle->cid, p_evt->evt_result);
            break;
        }

        default:
        {
            APPL_LOG("[0x%04X]:[0x%04X] Unknown Event 0x%04X. Result 0x%08lX",
                     p_evt->evt_id, p_handle->conn_handle, p_handle->cid, p_evt->evt_result);
            break;
        }
    }
    return NRF_SUCCESS;
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(adapter_t * adapter, ble_evt_t * p_ble_evt)
{
    ble_ipsp_evt_handler(p_ble_evt);
    on_ble_evt(p_ble_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t     err_code = NRF_SUCCESS;
    uint32_t     ram_start = 0;
    ble_cfg_t    ble_cfg;

    if (err_code == NRF_SUCCESS)
    {
        // Configure the maximum number of connections.
        memset(&ble_cfg, 0, sizeof(ble_cfg));
        ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = 0;
        ble_cfg.gap_cfg.role_count_cfg.central_role_count = BLE_IPSP_MAX_CHANNELS;
        ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = 0;
        err_code = sd_ble_cfg_set(m_adapter, BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);
    }

    if (err_code == NRF_SUCCESS)
    {
        memset(&ble_cfg, 0, sizeof(ble_cfg));

        // Configure total number of connections.
        ble_cfg.conn_cfg.conn_cfg_tag                     = APP_IPSP_TAG;
        ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count   = BLE_IPSP_MAX_CHANNELS;
        ble_cfg.conn_cfg.params.gap_conn_cfg.event_length = BLE_GAP_EVENT_LENGTH_DEFAULT;
        err_code = sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_GAP, &ble_cfg, ram_start);

    }

    if (err_code ==  NRF_SUCCESS)
    {
        memset(&ble_cfg, 0, sizeof(ble_cfg));

         // Configure the number of custom UUIDS.
        ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 0;
        err_code = sd_ble_cfg_set(m_adapter, BLE_COMMON_CFG_VS_UUID, &ble_cfg, ram_start);
    }

    if (err_code == NRF_SUCCESS)
    {
        memset(&ble_cfg, 0, sizeof(ble_cfg));

        // Set L2CAP channel configuration

        // @note The TX MPS and RX MPS of initiator and acceptor are not symmetrically set.
        // This will result in effective MPS of 50 in reach direction when using the initiator and
        // acceptor example against each other. In the IPSP, the TX MPS is set to a higher value
        // as Linux which is the border router for 6LoWPAN examples uses default RX MPS of 230
        // bytes. Setting TX MPS of 212 in place of 50 results in better credit and hence bandwidth
        // utilization.
        ble_cfg.conn_cfg.conn_cfg_tag                        = APP_IPSP_TAG;
        ble_cfg.conn_cfg.params.l2cap_conn_cfg.rx_mps        = BLE_IPSP_RX_MPS;
        ble_cfg.conn_cfg.params.l2cap_conn_cfg.rx_queue_size = BLE_IPSP_RX_BUFFER_COUNT;
        ble_cfg.conn_cfg.params.l2cap_conn_cfg.tx_mps        = BLE_IPSP_TX_MPS;
        ble_cfg.conn_cfg.params.l2cap_conn_cfg.tx_queue_size = 1;
        ble_cfg.conn_cfg.params.l2cap_conn_cfg.ch_count      = 1; // One L2CAP channel per link.
        err_code = sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_L2CAP, &ble_cfg, ram_start);
    }

    if (err_code == NRF_SUCCESS)
    {
        memset(&ble_cfg, 0, sizeof(ble_cfg));

        // Set the ATT table size.
        ble_cfg.gatts_cfg.attr_tab_size.attr_tab_size = 256;
        err_code = sd_ble_cfg_set(m_adapter, BLE_GATTS_CFG_ATTR_TAB_SIZE, &ble_cfg, ram_start);
    }

    if (err_code ==  NRF_SUCCESS)
    {
        err_code = sd_ble_enable(m_adapter, &ram_start);
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
    }

    APP_ERROR_CHECK(err_code);
}


static void services_init()
{
    ble_ipsp_init_t init_param;
    init_param.evt_handler = app_ipsp_event_handler;

    uint32_t err_code = ble_ipsp_init(m_adapter, &init_param);
    APP_ERROR_CHECK(err_code);

    ble_gap_addr_t m_my_addr;

    m_my_addr.addr[5]   = 0x00;
    m_my_addr.addr[4]   = 0x11;
    m_my_addr.addr[3]   = 0x22;
    m_my_addr.addr[2]   = 0x33;
    m_my_addr.addr[1]   = 0x44;
    m_my_addr.addr[0]   = 0x55;
    m_my_addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;

    err_code = sd_ble_gap_addr_set(m_adapter, &m_my_addr);
    APP_ERROR_CHECK(err_code);
}

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

/**@brief Function for initializing serial communication with the target nRF5 Bluetooth slave.
 *
 * @param[in] serial_port The serial port the target nRF5 device is connected to.
 *
 * @return The new transport adapter.
 */
static adapter_t * adapter_init(char * serial_port, uint32_t baud_rate)
{
    physical_layer_t  * phy;
    data_link_layer_t * data_link_layer;
    transport_layer_t * transport_layer;

    phy = sd_rpc_physical_layer_create_uart(serial_port,
                                            baud_rate,
                                            SD_RPC_FLOW_CONTROL_NONE,
                                            SD_RPC_PARITY_NONE);
    data_link_layer = sd_rpc_data_link_layer_create_bt_three_wire(phy, 250);
    transport_layer = sd_rpc_transport_layer_create(data_link_layer, 1500);
    return sd_rpc_adapter_create(transport_layer);
}

#endif

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    #if NRF_SD_BLE_API >= 6
    uint32_t error_code;
    char *   serial_port = DEFAULT_UART_PORT_NAME;
    uint32_t baud_rate = DEFAULT_BAUD_RATE;

    printf("Serial port used: %s\n", serial_port);
    printf("Baud rate used: %d\n", baud_rate);
    fflush(stdout);

    m_adapter =  adapter_init(serial_port, baud_rate);
    sd_rpc_log_handler_severity_filter_set(m_adapter, SD_RPC_LOG_INFO);
    error_code = sd_rpc_open(m_adapter, status_handler, ble_evt_dispatch, log_handler);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to open nRF BLE Driver. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    ble_stack_init();
    services_init();

    uint32_t err_code;
    if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        err_code = sd_ble_gap_connect(m_adapter, &m_peer_addr,
                                      &m_scan_param, &m_connection_param, APP_IPSP_TAG);
        if (err_code != NRF_SUCCESS)
        {
            APPL_LOG("Connection Request Failed, reason 0x%08lX", err_code);
        }
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        APPL_LOG("Connection exists with peer");
    }

    for (;;)
    {
        char c = (char)getchar();
        if (c == 'q' || c == 'Q')
        {
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
    }
    #endif
}

/**
 * @}
 */
