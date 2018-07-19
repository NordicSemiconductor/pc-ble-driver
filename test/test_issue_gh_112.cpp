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

// This issue is only relevant for SoftDevice API >= 3
#if NRF_SD_BLE_API >= 3

// Logging support
#include "internal/log.h"

// Test support
#include "test_setup.h"
#include "test_util.h"

#include "ble.h"
#include "sd_rpc.h"

#include <string>
#include <thread>
#include <sstream>

constexpr uint16_t BLE_UUID_HEART_RATE_SERVICE = 0x180D;
constexpr uint16_t BLE_UUID_HEART_RATE_MEASUREMENT_CHAR = 0x2A37;
constexpr uint16_t BLE_UUID_CCCD = 0x2902;

// Indicates if an error has occurred in a callback.
// The test framework is not thread safe so this variable is used to communicate that an issues has occurred in a callback.
bool error = false;

// Set to true when the test is complete
bool testComplete = false;

uint32_t setupPeripheral(const std::shared_ptr<testutil::AdapterWrapper> &p, const std::string &advertisingNme, const std::vector<uint8_t> &initialCharaciteristicValue, const uint16_t characteristicValueMaxLength)
{
    // Setup the advertisement data
    std::vector<uint8_t> advertisingData;
    testutil::appendAdvertisingName(advertisingData, advertisingNme);
    advertisingData.push_back(3); // Length of upcoming advertisement type
    advertisingData.push_back(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE);

    // Store BLE_UUID_HEART_RATE_SERVICE in little - endian format.
    advertisingData.push_back(BLE_UUID_HEART_RATE_SERVICE & 0xFF);
    advertisingData.push_back((BLE_UUID_HEART_RATE_SERVICE & 0xFF00) >> 8);

    const uint8_t * sr_data = nullptr;
    const uint8_t   sr_data_length = 0;

    auto err_code = sd_ble_gap_adv_data_set(
        p->unwrap(),
        advertisingData.data(),
        static_cast<uint8_t>(advertisingData.size()),
        sr_data,
        sr_data_length);
    if (err_code != NRF_SUCCESS) return err_code;

    // Setup service, use service UUID specified in scratchpad.target_service
    err_code = sd_ble_gatts_service_add(
        p->unwrap(),
        BLE_GATTS_SRVC_TYPE_PRIMARY,
        &(p->scratchpad.target_service),
        &(p->scratchpad.service_handle));
    if (err_code != NRF_SUCCESS) return err_code;

    // Setup characteristic, use characteristic UUID specified in scratchpad.target_characteristic
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read = 1;
    char_md.char_props.notify = 1;
    char_md.char_props.write = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc = nullptr;
    char_md.p_char_pf = nullptr;
    char_md.p_user_desc_md = nullptr;
    char_md.p_cccd_md = &cccd_md;
    char_md.p_sccd_md = nullptr;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid = &(p->scratchpad.target_characteristic);
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = static_cast<uint16_t>(initialCharaciteristicValue.size());
    attr_char_value.init_offs = 0;
    attr_char_value.max_len = characteristicValueMaxLength;
    attr_char_value.p_value = const_cast<uint8_t*>(initialCharaciteristicValue.data());

    return sd_ble_gatts_characteristic_add(
        p->unwrap(),
        p->scratchpad.service_handle,
        &char_md,
        &attr_char_value,
        &(p->scratchpad.gatts_characteristic_handle));
}

TEST_CASE("test_issue_gh_112")
{
    auto env = ::test::getEnvironment();
    REQUIRE(env.serialPorts.size() >= 2);
    auto central = env.serialPorts.at(0);
    auto peripheral = env.serialPorts.at(1);

    SECTION("reproduce_error")
    {
        const auto baudRate = central.baudRate;

        INFO("Central serial port used: " << central.port);
        INFO("Peripheral serial port used: " << peripheral.port);
        INFO("Baud rate used: " << baudRate);

        const auto peripheralAdvName = "peripheral";

        // Instantiate an adapter to use as BLE Central in the test
        auto c = std::shared_ptr<testutil::AdapterWrapper>(
            new testutil::AdapterWrapper(testutil::Central, central.port.c_str(), baudRate));

        // Instantiated an adapter to use as BLE Peripheral in the test
        auto p = std::shared_ptr<testutil::AdapterWrapper>(
            new testutil::AdapterWrapper(testutil::Peripheral, peripheral.port.c_str(), baudRate)
        );

        // Use Heart rate service and characteristics as target for testing but
        // the values sent are not according to the Heart Rate service

        // BLE Central scratchpad
        c->scratchpad.target_service.uuid = BLE_UUID_HEART_RATE_SERVICE;
        c->scratchpad.target_service.type = BLE_UUID_TYPE_BLE;

        c->scratchpad.target_characteristic.uuid = BLE_UUID_HEART_RATE_MEASUREMENT_CHAR;
        c->scratchpad.target_characteristic.type = BLE_UUID_TYPE_BLE;

        c->scratchpad.target_descriptor.uuid = BLE_UUID_CCCD;
        c->scratchpad.target_descriptor.type = BLE_UUID_TYPE_BLE;
        c->scratchpad.mtu = 150;

        // BLE Peripheral scratchpad
        p->scratchpad.target_service.uuid = BLE_UUID_HEART_RATE_SERVICE;
        p->scratchpad.target_service.type = BLE_UUID_TYPE_BLE;

        p->scratchpad.target_characteristic.uuid = BLE_UUID_HEART_RATE_MEASUREMENT_CHAR;
        p->scratchpad.target_characteristic.type = BLE_UUID_TYPE_BLE;

        p->scratchpad.target_descriptor.uuid = BLE_UUID_CCCD;
        p->scratchpad.target_descriptor.type = BLE_UUID_TYPE_BLE;
        p->scratchpad.mtu = 150;

        // Register adapters so that C based callbacks can find them in sd_rpc_open callbacks
        testutil::adapters.push_back(c);
        testutil::adapters.push_back(p);

        REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) == NRF_SUCCESS);
        REQUIRE(sd_rpc_log_handler_severity_filter_set(p->unwrap(), env.driverLogLevel) == NRF_SUCCESS);

        c->setGapEventCallback([&c, &peripheralAdvName](const uint16_t eventId, const ble_gap_evt_t * gapEvent) -> bool
        {
            switch (eventId)
            {
            case BLE_GAP_EVT_CONNECTED:
                c->scratchpad.connection_handle = gapEvent->conn_handle;
                c->scratchpad.connection_in_progress = false;
                c->startServiceDiscovery(BLE_UUID_TYPE_BLE, BLE_UUID_HEART_RATE_SERVICE);
                return true;
            case BLE_GAP_EVT_DISCONNECTED:
                NRF_LOG(c->role() << " disconnected, reason: " << std::hex << gapEvent->params.disconnected.reason);
                return true;
            case BLE_GAP_EVT_ADV_REPORT:
                NRF_LOG(c->role() << " Received advertisement report from device: " << testutil::asText(gapEvent->params.adv_report.peer_addr));

                if (testutil::findAdvName(&(gapEvent->params.adv_report), peripheralAdvName))
                {
                    if (!c->scratchpad.connection_in_progress)
                    {
                        const auto err_code = c->connect(&(gapEvent->params.adv_report.peer_addr));

                        if (err_code != NRF_SUCCESS)
                        {
                            NRF_LOG(c->role() << " Error connecting to " << testutil::asText(gapEvent->params.adv_report.peer_addr) << ", " << testutil::errorToString(err_code));
                            error = true;
                        }
                    }
                }
                return true;
            case BLE_GAP_EVT_TIMEOUT:
                if (gapEvent->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
                {
                    c->scratchpad.connection_in_progress = false;
                }
                else if (gapEvent->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
                {
                    const auto err_code = c->startScan();

                    if (err_code != NRF_SUCCESS)
                    {
                        NRF_LOG(c->role() << " Scan start error, err_code " << err_code);
                        error = true;
                    }
                }
                return true;
            case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            {
                const auto  err_code = sd_ble_gap_conn_param_update(c->unwrap(),
                    c->scratchpad.connection_handle,
                    &(gapEvent->params.conn_param_update_request.conn_params)
                );
                if (err_code != NRF_SUCCESS)
                {
                    NRF_LOG(c->role() << " Conn params update failed, err_code " << err_code);
                    error = true;
                }
            }
                return true;
            default:
                return false;
            }
        });

        c->setGattcEventCallback([&c](const uint16_t eventId, const ble_gattc_evt_t *gattcEvent) -> bool
        {
            switch (eventId)
            {
            case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
            {
                NRF_LOG(c->role() << " Received service discovery response.");

                if (gattcEvent->gatt_status != NRF_SUCCESS)
                {
                    NRF_LOG(c->role() << " Service discovery failed. " << testutil::gattStatusToString(gattcEvent->gatt_status));
                    error = true;
                    return true;
                }

                const auto count = gattcEvent->params.prim_srvc_disc_rsp.count;

                if (count == 0)
                {
                    NRF_LOG(c->role() << " No services not found.");
                    error = true;
                    return true;
                }

                auto targetServiceFound = false;

                ble_gattc_service_t service;

                for (auto service_index = 0; service_index < count; service_index++)
                {
                    service = gattcEvent->params.prim_srvc_disc_rsp.services[service_index];

                    if (service.uuid.uuid == c->scratchpad.target_service.uuid
                        && service.uuid.type == c->scratchpad.target_service.type)
                    {
                        NRF_LOG(c->role() << " Found target service " << testutil::asText(c->scratchpad.target_service));
                        targetServiceFound = true;
                        break;
                    }
                }

                if (!targetServiceFound)
                {
                    error = true;
                    NRF_LOG(c->role() << " Did not find target service " << testutil::asText(c->scratchpad.target_service));
                    return true;
                }

                c->scratchpad.service_start_handle = service.handle_range.start_handle;
                c->scratchpad.service_end_handle = service.handle_range.end_handle;

                NRF_LOG(c->role() << " Discovered target service. "
                    << testutil::asText(service.uuid)
                    << " start_handle: " << testutil::asText(c->scratchpad.service_start_handle)
                    << " end_handle: " << testutil::asText(c->scratchpad.service_end_handle)
                );

                if (c->startCharacteristicDiscovery() != NRF_SUCCESS)
                {
                    error = true;
                }
            }
            return true;
            case BLE_GATTC_EVT_CHAR_DISC_RSP:
            {
                const auto count = gattcEvent->params.char_disc_rsp.count;

                if (gattcEvent->gatt_status != NRF_SUCCESS)
                {
                    NRF_LOG(c->role() << " Characteristic discovery failed. " << testutil::gattStatusToString(gattcEvent->gatt_status));
                    error = true;
                    return true;
                }

                NRF_LOG(c->role() << " Received characteristic discovery response, characteristics count: " << count);


                auto foundTargetCharacteristic = false;

                for (auto i = 0; i < count; i++)
                {
                    NRF_LOG(c->role() << " [characteristic #" << i << "] " << testutil::asText(gattcEvent->params.char_disc_rsp.chars[i]));

                    if (gattcEvent->params.char_disc_rsp.chars[i].uuid.uuid == c->scratchpad.target_characteristic.uuid &&
                        gattcEvent->params.char_disc_rsp.chars[i].uuid.type == c->scratchpad.target_characteristic.type)
                    {
                        NRF_LOG(c->role() << " Found target characteristic, " << testutil::asText(c->scratchpad.target_characteristic));
                        c->scratchpad.characteristic_decl_handle = gattcEvent->params.char_disc_rsp.chars[i].handle_decl;
                        c->scratchpad.characteristic_value_handle = gattcEvent->params.char_disc_rsp.chars[i].handle_value;
                        foundTargetCharacteristic = true;
                    }
                }

                if (!foundTargetCharacteristic)
                {
                    NRF_LOG(c->role() << " Did not find target characteristic " << testutil::asText(c->scratchpad.target_characteristic));
                    error = true;
                }
                else
                {
                    if (c->startDescriptorDiscovery() != NRF_SUCCESS)
                    {
                        error = true;
                    }
                }

                return true;
            }
            case BLE_GATTC_EVT_DESC_DISC_RSP:
            {
                const auto count = gattcEvent->params.desc_disc_rsp.count;

                if (gattcEvent->gatt_status != NRF_SUCCESS)
                {
                    NRF_LOG(c->role() << " Descriptor discovery failed. " << testutil::gattStatusToString(gattcEvent->gatt_status));
                    error = true;
                    return true;
                }

                NRF_LOG(c->role() << " Received descriptor discovery response, descriptor count: " << count);

                // Change the MTU
                const auto err_code = sd_ble_gattc_exchange_mtu_request(
                    c->unwrap(),
                    c->scratchpad.connection_handle,
                    c->scratchpad.mtu);

                if (err_code != NRF_SUCCESS)
                {
                    NRF_LOG(c->role() << " MTU exchange request reply failed, err_code: " << err_code);
                    error = true;
                }
            }
            return true;
            case BLE_GATTC_EVT_WRITE_RSP:
                NRF_LOG(c->role() << " Received write response.");

                if (gattcEvent->gatt_status != NRF_SUCCESS)
                {
                    NRF_LOG(c->role() << " Error. Write operation failed.  " << testutil::gattStatusToString(gattcEvent->gatt_status));
                    error = true;
                }

                testComplete = true;

                return true;
            case BLE_GATTC_EVT_EXCHANGE_MTU_RSP:
            {
                auto const server_rx_mtu = gattcEvent->params.exchange_mtu_rsp.server_rx_mtu;
                NRF_LOG(c->role() << " MTU response received. New ATT_MTU is " << server_rx_mtu);

                // Write some data to characteristic to trigger reported error
                std::vector<uint8_t> data;

                // Send negotiated MTU - 3 bytes  used by GATT header
                testutil::appendRandomData(data, c->scratchpad.mtu - 3);

                const auto err_code = c->writeCharacteristicValue(c->scratchpad.characteristic_value_handle, data);

                if (err_code != NRF_SUCCESS)
                {
                    NRF_LOG(c->role() << " Error writing data to characteristic. err_code: " << err_code);
                    error = true;
                }
            }
            return true;
            default:
                return false;
            }
        });

        c->setGattsEventCallback([&c](const uint16_t eventId, const ble_gatts_evt_t *gattsEvent) -> bool {
            switch (eventId)
            {
            case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            {
                const auto err_code = sd_ble_gatts_exchange_mtu_reply(
                    c->unwrap(),
                    c->scratchpad.connection_handle,
                    c->scratchpad.mtu);

                if (err_code != NRF_SUCCESS)
                {
                    NRF_LOG(c->role() << " MTU exchange request reply failed, err_code: " << err_code);
                    error = true;
                }
            }
            return true;
            default:
                return false;
            }
        });

        c->setEventCallback([](const ble_evt_t * p_ble_evt) -> bool
        {
            const auto eventId = p_ble_evt->header.evt_id;
            NRF_LOG("Received an un-handled event with ID: " << eventId);
            return true;
        });

        p->setGapEventCallback([&p](const uint16_t eventId, const ble_gap_evt_t * gapEvent)
        {
            switch (eventId)
            {
            case BLE_GAP_EVT_CONNECTED:
                p->scratchpad.connection_handle = gapEvent->conn_handle;
                NRF_LOG(p->role() << " Connected, connection_handle: " << testutil::asText(gapEvent->conn_handle));
                return true;
            case BLE_GAP_EVT_DISCONNECTED:
            {
                NRF_LOG(p->role() << " Disconnected, connection_handle: " << testutil::asText(gapEvent->conn_handle));
                p->scratchpad.connection_handle = BLE_CONN_HANDLE_INVALID;

                // Use scratchpad defaults when advertising
                NRF_LOG(p->role() << " Starting advertising.");
                const auto err_code = p->startAdvertising();
                if (err_code != NRF_SUCCESS)
                {
                    NRF_LOG(p->role() << " Error starting advertising after disconnect.");
                    error = true;
                }
            }
            return true;
            case BLE_GAP_EVT_TIMEOUT:
                p->scratchpad.advertisement_timed_out = true;
                return true;
            case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            {
                const auto err_code = sd_ble_gap_sec_params_reply(
                    p->unwrap(),
                    p->scratchpad.connection_handle,
                    BLE_GAP_SEC_STATUS_SUCCESS, nullptr, nullptr);

                if (err_code != NRF_SUCCESS)
                {
                    error = true;
                    NRF_LOG(p->role() << "Failed reply with GAP security parameters. " << testutil::errorToString(err_code));
                }
            }
            return true;
            case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            {
                const auto err_code = sd_ble_gatts_sys_attr_set(p->unwrap(), p->scratchpad.connection_handle, nullptr, 0, 0);

                if (err_code != NRF_SUCCESS)
                {
                    error = true;
                    NRF_LOG(p->role() << "Failed updating persistent sys attr info. " << testutil::errorToString(err_code));
                }
            }
            return true;
            default:
                return false;
            }
        });

        p->setGattsEventCallback([&p](const uint16_t eventId, const ble_gatts_evt_t * gattsEvent) -> bool
        {
            switch (eventId)
            {
            case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            {
                const auto err_code = sd_ble_gatts_exchange_mtu_reply(
                    p->unwrap(),
                    p->scratchpad.connection_handle,
                    p->scratchpad.mtu);

                if (err_code != NRF_SUCCESS)
                {
                    error = true;
                    NRF_LOG(p->role() << " MTU exchange request reply failed. " << testutil::errorToString(err_code));
                }
            }
            return true;
            default:
                return false;
            }
        });

        // Open the adapters
        REQUIRE(sd_rpc_open(c->unwrap(), testutil::statusHandler, testutil::eventHandler, testutil::logHandler) == NRF_SUCCESS);
        REQUIRE(sd_rpc_open(p->unwrap(), testutil::statusHandler, testutil::eventHandler, testutil::logHandler) == NRF_SUCCESS);

        REQUIRE(c->configure() == NRF_SUCCESS);
        REQUIRE(p->configure() == NRF_SUCCESS);

        // Starting the scan starts the sequence of operations to get a connection established
        REQUIRE(c->startScan() == NRF_SUCCESS);

        REQUIRE(setupPeripheral(p, peripheralAdvName, { 0x00 }, p->scratchpad.mtu) == NRF_SUCCESS);
        REQUIRE(p->startAdvertising() == NRF_SUCCESS);

        // Wait for the test to complete
        std::this_thread::sleep_for(std::chrono::seconds(2));

        REQUIRE(error == false);
        REQUIRE(testComplete == true);

        REQUIRE(sd_rpc_close(c->unwrap()) == NRF_SUCCESS);
        sd_rpc_adapter_delete(c->unwrap());

        REQUIRE(sd_rpc_close(p->unwrap()) == NRF_SUCCESS);
        sd_rpc_adapter_delete(p->unwrap());
    }
}

#else
TEST_CASE("test_issue_gh_112")
{
    INFO("Not relevant for SoftDevice API version < 3")
}

#endif // This issue is only relevant for SoftDevice API >= 3
