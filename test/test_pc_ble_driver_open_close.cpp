/*
* Copyright (c) 2018 Nordic Semiconductor ASA
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
*   1. Redistributions of source code must retain the above copyright notice, this
*   list of conditions and the following disclaimer.
*
*   2. Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
*   3. Neither the name of Nordic Semiconductor ASA nor the names of other
*   contributors to this software may be used to endorse or promote products
*   derived from this software without specific prior written permission.
*
*   4. This software must only be used in or with a processor manufactured by Nordic
*   Semiconductor ASA, or in or with a processor manufactured by a third party that
*   is used in combination with a processor manufactured by Nordic Semiconductor.
*
*   5. Any software provided in binary or object form under this license must not be
*   reverse engineered, decompiled, modified and/or disassembled.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

 // Test framework
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

 // Logging support
#include "internal/log.h"

#include "test_setup.h"

#include "ble.h"
#include "sd_rpc.h"

#include <string>
#include <thread>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#define BAUD_RATE 1000000 /**< The baud rate to be used for serial communication with nRF5 device. */
#endif
#ifdef __APPLE__
#define BAUD_RATE 115200 /**< Baud rate 1M is not supported on MacOS. */
#endif
#ifdef __linux__
#define BAUD_RATE 1000000
#endif

#define SCAN_INTERVAL 0x00A0 /**< Determines scan interval in units of 0.625 milliseconds. */
#define SCAN_WINDOW   0x0050 /**< Determines scan window in units of 0.625 milliseconds. */
#define SCAN_TIMEOUT  0x0    /**< Scan timeout between 0x01 and 0xFFFF in seconds, 0x0 disables timeout. */

typedef struct
{
    uint8_t *     p_data;   /**< Pointer to data. */
    uint16_t      data_len; /**< Length of data. */
} data_t;

static bool        m_connection_is_in_progress = false;
static adapter_t * m_adapter = NULL;

#if NRF_SD_BLE_API >= 5
static uint32_t    m_config_id = 1;
#endif

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

/* Local function forward declarations */
static uint32_t ble_stack_init();
static uint32_t scan_start();
static std::string ble_address_to_string_convert(ble_gap_addr_t address);

static void status_handler(adapter_t * adapter, sd_rpc_app_status_t code, const char * message)
{
    NRF_LOG("Status: " << code << ", message: " << message);
}

static void log_handler(adapter_t * adapter, sd_rpc_log_severity_t severity, const char * message)
{
    switch (severity)
    {
        case SD_RPC_LOG_ERROR:
            NRF_LOG("Error: " << message);
            break;

        case SD_RPC_LOG_WARNING:
            NRF_LOG("Warning: " << message);
            break;

        case SD_RPC_LOG_INFO:
            NRF_LOG("Info: " << message);
            break;

        default:
            NRF_LOG("Log: " << message);
            break;
    }
}

static void on_adv_report(const ble_gap_evt_t * const p_ble_gap_evt)
{
    // Log the Bluetooth device address of advertisement packet received.
    auto address = ble_address_to_string_convert(p_ble_gap_evt->params.adv_report.peer_addr);
    NRF_LOG("Received advertisement report with device address: " << address);
}

static void on_timeout(const ble_gap_evt_t * const p_ble_gap_evt)
{
    if (p_ble_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
    {
        REQUIRE(sd_ble_gap_scan_start(m_adapter, &m_scan_param) == NRF_SUCCESS);
    }
}

static adapter_t * adapter_init(const char * serial_port, uint32_t baud_rate)
{
    physical_layer_t  * phy;
    data_link_layer_t * data_link_layer;
    transport_layer_t * transport_layer;

    phy = sd_rpc_physical_layer_create_uart(serial_port, baud_rate, SD_RPC_FLOW_CONTROL_NONE,
                                            SD_RPC_PARITY_NONE);
    data_link_layer = sd_rpc_data_link_layer_create_bt_three_wire(phy, 100);
    transport_layer = sd_rpc_transport_layer_create(data_link_layer, 100);
    return sd_rpc_adapter_create(transport_layer);
}

static std::string ble_address_to_string_convert(ble_gap_addr_t address)
{
    const int address_length = 6;
    std::stringstream retval;

    for (int i = sizeof(address.addr) - 1; i >= 0; --i)
    {
        retval << std::hex << static_cast<unsigned int>(address.addr[i]);
    }

    return retval.str();
}

static uint32_t ble_stack_init()
{
    uint32_t            err_code;
    uint32_t *          app_ram_base = NULL;

#if NRF_SD_BLE_API <= 3
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
#endif

#if NRF_SD_BLE_API == 3
    ble_enable_params.gatt_enable_params.att_mtu = GATT_MTU_SIZE_DEFAULT;
#elif NRF_SD_BLE_API < 3
    ble_enable_params.gatts_enable_params.attr_tab_size     = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
    ble_enable_params.gatts_enable_params.service_changed   = false;
    ble_enable_params.gap_enable_params.periph_conn_count   = 1;
    ble_enable_params.gap_enable_params.central_conn_count  = 1;
    ble_enable_params.gap_enable_params.central_sec_count   = 1;
    ble_enable_params.common_enable_params.p_conn_bw_counts = NULL;
    ble_enable_params.common_enable_params.vs_uuid_count    = 1;
#endif

#if NRF_SD_BLE_API <= 3
    err_code = sd_ble_enable(m_adapter, &ble_enable_params, app_ram_base);
#else
    err_code = sd_ble_enable(m_adapter, app_ram_base);
#endif

    switch (err_code) {
        case NRF_SUCCESS:
            break;
        case NRF_ERROR_INVALID_STATE:
            NRF_LOG("BLE stack already enabled");
            break;
        default:
            NRF_LOG("Failed to enable BLE stack. Error code: " << err_code);
            break;
    }

    return err_code;
}

static uint32_t ble_options_set()
{
#if NRF_SD_BLE_API <= 3
    ble_opt_t        opt;
    ble_common_opt_t common_opt;

    common_opt.conn_bw.role = BLE_GAP_ROLE_CENTRAL;
    common_opt.conn_bw.conn_bw.conn_bw_rx = BLE_CONN_BW_HIGH;
    common_opt.conn_bw.conn_bw.conn_bw_tx = BLE_CONN_BW_HIGH;
    opt.common_opt = common_opt;

    return sd_ble_opt_set(m_adapter, BLE_COMMON_OPT_CONN_BW, &opt);
#else
    return NRF_ERROR_NOT_SUPPORTED;
#endif
}

#if NRF_SD_BLE_API >= 5
void ble_cfg_set(uint8_t conn_cfg_tag)
{
    const uint32_t ram_start = 0; // Value is not used by ble-driver
    ble_cfg_t ble_cfg;

    // Configure the connection roles.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = 1;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = 1;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = 1;

    REQUIRE(sd_ble_cfg_set(m_adapter, BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start) == NRF_SUCCESS);

    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                 = conn_cfg_tag;
    ble_cfg.conn_cfg.params.gatt_conn_cfg.att_mtu = 150;

    REQUIRE(sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_GATT, &ble_cfg, ram_start) == NRF_SUCCESS);
}
#endif


static void ble_evt_dispatch(adapter_t * adapter, ble_evt_t * p_ble_evt)
{
    if (p_ble_evt == NULL)
    {
        NRF_LOG("Received an empty BLE event");
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
       case BLE_GAP_EVT_ADV_REPORT:
            on_adv_report(&(p_ble_evt->evt.gap_evt));
            break;
        case BLE_GAP_EVT_TIMEOUT:
            on_timeout(&(p_ble_evt->evt.gap_evt));
            break;
       default:
            NRF_LOG("Received an un-handled event with ID: " << p_ble_evt->header.evt_id);
            break;
    }
}

TEST_CASE("test_pc_ble_driver_open_close")
{
    uint32_t baudRate = BAUD_RATE;

    auto env = ::test::getEnvironment();
    REQUIRE(env.serialPorts.size() > 0);
    auto serialPort = env.serialPorts.at(0);
    auto numberOfIterations = env.numberOfIterations;

    SECTION("open_close_open_iterations")
    {
        baudRate = serialPort.baudRate;

        INFO("Serial port used: " << serialPort.port);
        INFO("Baud rate used: " << baudRate);

        for (uint32_t i = 0; i < numberOfIterations; i++)
        {
            m_adapter = adapter_init(serialPort.port.c_str(), baudRate);
            REQUIRE(sd_rpc_log_handler_severity_filter_set(m_adapter, env.driverLogLevel) == NRF_SUCCESS);
            REQUIRE(sd_rpc_open(m_adapter, status_handler, ble_evt_dispatch, log_handler) == NRF_SUCCESS);
#if NRF_SD_BLE_API >= 5
            ble_cfg_set(m_config_id);
#endif

            REQUIRE(ble_stack_init() == NRF_SUCCESS);

#if NRF_SD_BLE_API < 5
            REQUIRE(ble_options_set() == NRF_SUCCESS);
#endif

            REQUIRE(sd_ble_gap_scan_start(m_adapter, &m_scan_param) == NRF_SUCCESS);

            std::this_thread::sleep_for(std::chrono::seconds(2));

            REQUIRE(sd_rpc_close(m_adapter) == NRF_SUCCESS);
            sd_rpc_adapter_delete(m_adapter);

            NRF_LOG("Iteration #" << (i + 1) << " of " << numberOfIterations << " complete.");
        }
    }
}
