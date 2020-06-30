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

// This issue is only relevant for SoftDevice API >= 3
#if NRF_SD_BLE_API >= 5

// Test framework
#include "catch2/catch.hpp"

// Logging support
#include "logging.h"

// Test support
#include <test_environment.h>
#include <test_util.h>

#include <ble.h>
#include <sd_rpc.h>

#include <sstream>
#include <string>
#include <thread>

TEST_CASE(CREATE_TEST_NAME_AND_TAGS(issue_128bit_uuid, [issue][PCA10040][PCA10056][PCA10059]))
{
    auto env = ::test::getEnvironment();
    INFO(::test::getEnvironmentAsText(env));
    REQUIRE(env.serialPorts.size() >= 2);
    const auto central    = env.serialPorts.at(0);
    const auto peripheral = env.serialPorts.at(1);

    // Service #1
    constexpr ble_uuid128_t CUSTOM_SERVICE_UUID_BASE_1 = {{0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                                           0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00,
                                                           0x00, 0xf0}};

    constexpr uint16_t CUSTOM_SERVICE_UUID_1 = 0xf000;

    // Service #2
    constexpr ble_uuid128_t CUSTOM_SERVICE_UUID_BASE_2 = {{0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                                           0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00,
                                                           0x01, 0xf0}};

    constexpr uint16_t CUSTOM_SERVICE_UUID_2 = 0xf000;

    constexpr ble_uuid128_t CUSTOM_SERVICE_2_CHARACTERISTIC_UUID_BASE_1 = {
        {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x01,
         0xf0}};
    constexpr uint16_t CUSTOM_SERVICE_2_VALUE_CHAR_UUID_1 = 0xf001;

    constexpr ble_uuid128_t CUSTOM_SERVICE_2_CHARACTERISTIC_UUID_BASE_2 = {
        {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x01,
         0xf0}};
    constexpr uint16_t CUSTOM_SERVICE_2_VALUE_CHAR_UUID_2 = 0xf002;

    // Service #3
    constexpr ble_uuid128_t CUSTOM_SERVICE_UUID_BASE_3 = {{0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                                           0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00,
                                                           0x02, 0xf0}};

    constexpr uint16_t CUSTOM_SERVICE_UUID_3 = 0xf000;

    constexpr ble_uuid128_t CUSTOM_SERVICE_3_CHARACTERISTIC_UUID_BASE_1 = {
        {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x02,
         0xf0}};

    constexpr uint16_t CUSTOM_SERVICE_3_VALUE_CHAR_UUID_1 = 0xf001;

    constexpr ble_uuid128_t CUSTOM_SERVICE_3_CHARACTERISTIC_UUID_BASE_2 = {
        {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x02,
         0xf0}};
    constexpr uint16_t CUSTOM_SERVICE_3_VALUE_CHAR_UUID_2 = 0xf001;

    // CCCD
    constexpr uint16_t BLE_UUID_CCCD = 0x2902;

    // Indicates if an error has occurred in a callback.
    // The test framework is not thread safe so this variable is used to communicate that an
    // issue has occurred in a callback.
    auto centralError    = false;
    auto peripheralError = false;

    // Set to true when the test is complete
    auto testComplete = false;

    // 128-bit UUID needed by lambdas
    uint8_t p_service_uuid_type;

    uint8_t p_service_2_uuid_type;
    uint8_t p_service_2_characteristic_uuid_type_1;
    uint8_t p_service_2_characteristic_uuid_type_2;

    uint8_t p_service_3_uuid_type;
    uint8_t p_service_3_characteristic_uuid_type_1;
    uint8_t p_service_3_characteristic_uuid_type_2;

    uint8_t c_service_uuid_type;

    auto setupPeripheral = [&](const std::shared_ptr<testutil::AdapterWrapper> &p,
                               const std::string &advertisingName,
                               const std::vector<uint8_t> &initialCharacteristicValue,
                               const uint16_t characteristicValueMaxLength) -> uint32_t {
        // Setup the advertisement data
        std::vector<uint8_t> advertisingData;
        testutil::appendAdvertisingName(advertisingData, advertisingName);
        get_logger()->info("Advertising as '{}'", advertisingName);
        auto err_code = p->setupAdvertising(advertisingData);
        if (err_code != NRF_SUCCESS)
            return err_code;

        // Setup service #1
        err_code = sd_ble_gatts_service_add(p->unwrap(), BLE_GATTS_SRVC_TYPE_PRIMARY,
                                            &(p->scratchpad.target_service),
                                            &(p->scratchpad.service_handle));
        if (err_code != NRF_SUCCESS)
            return err_code;

        // Setup service #2 - getting invalid param
        err_code = sd_ble_gatts_service_add(p->unwrap(), BLE_GATTS_SRVC_TYPE_PRIMARY,
                                            &(p->scratchpad.target_service_2),
                                            &(p->scratchpad.service_handle_2));
        if (err_code != NRF_SUCCESS)
            return err_code;

        ble_gatts_char_md_t service_2_char_md_1{0};
        ble_gatts_attr_md_t service_2_cccd_md_1{0};
        ble_gatts_attr_t service_2_attr_char_value_1{0};
        ble_gatts_attr_md_t service_2_attr_md_1{0};

        ble_gatts_char_md_t service_2_char_md_2{0};
        ble_gatts_attr_md_t service_2_cccd_md_2{0};
        ble_gatts_attr_t service_2_attr_char_value_2{0};
        ble_gatts_attr_md_t service_2_attr_md_2{0};

        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_2_cccd_md_1.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_2_cccd_md_1.write_perm);
        service_2_cccd_md_1.vloc = BLE_GATTS_VLOC_STACK;

        service_2_char_md_1.char_props.read          = 1;
        service_2_char_md_1.char_props.notify        = 1;
        service_2_char_md_1.char_props.write         = 1;
        service_2_char_md_1.char_props.write_wo_resp = 1;
        service_2_char_md_1.p_char_user_desc         = nullptr;
        service_2_char_md_1.p_char_pf                = nullptr;
        service_2_char_md_1.p_user_desc_md           = nullptr;
        service_2_char_md_1.p_cccd_md                = &service_2_cccd_md_1;
        service_2_char_md_1.p_sccd_md                = nullptr;

        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_2_attr_md_1.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_2_attr_md_1.write_perm);
        service_2_attr_md_1.vloc    = BLE_GATTS_VLOC_STACK;
        service_2_attr_md_1.rd_auth = 0;
        service_2_attr_md_1.wr_auth = 0;
        service_2_attr_md_1.vlen    = 1;

        service_2_attr_char_value_1.p_uuid    = &(p->scratchpad.target_characteristic);
        service_2_attr_char_value_1.p_attr_md = &service_2_attr_md_1;
        service_2_attr_char_value_1.init_len =
            static_cast<uint16_t>(initialCharacteristicValue.size());
        service_2_attr_char_value_1.init_offs = 0;
        service_2_attr_char_value_1.max_len   = characteristicValueMaxLength;
        service_2_attr_char_value_1.p_value =
            const_cast<uint8_t *>(initialCharacteristicValue.data());

        auto ret = sd_ble_gatts_characteristic_add(
            p->unwrap(), p->scratchpad.service_handle_2, &service_2_char_md_1,
            &service_2_attr_char_value_1, &(p->scratchpad.gatts_characteristic_handle));

        if (ret != NRF_SUCCESS)
        {
            return ret;
        }

        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_2_cccd_md_2.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_2_cccd_md_2.write_perm);
        service_2_cccd_md_2.vloc = BLE_GATTS_VLOC_STACK;

        service_2_char_md_2.char_props.read          = 1;
        service_2_char_md_2.char_props.notify        = 1;
        service_2_char_md_2.char_props.write         = 1;
        service_2_char_md_2.char_props.write_wo_resp = 1;
        service_2_char_md_2.p_char_user_desc         = nullptr;
        service_2_char_md_2.p_char_pf                = nullptr;
        service_2_char_md_2.p_user_desc_md           = nullptr;
        service_2_char_md_2.p_cccd_md                = &service_2_cccd_md_1;
        service_2_char_md_2.p_sccd_md                = nullptr;

        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_2_attr_md_2.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_2_attr_md_2.write_perm);
        service_2_attr_md_2.vloc    = BLE_GATTS_VLOC_STACK;
        service_2_attr_md_2.rd_auth = 0;
        service_2_attr_md_2.wr_auth = 0;
        service_2_attr_md_2.vlen    = 1;

        service_2_attr_char_value_2.p_uuid    = &(p->scratchpad.target_characteristic_2);
        service_2_attr_char_value_2.p_attr_md = &service_2_attr_md_2;
        service_2_attr_char_value_2.init_len =
            static_cast<uint16_t>(initialCharacteristicValue.size());
        service_2_attr_char_value_2.init_offs = 0;
        service_2_attr_char_value_2.max_len   = characteristicValueMaxLength;
        service_2_attr_char_value_2.p_value =
            const_cast<uint8_t *>(initialCharacteristicValue.data());

        ret = sd_ble_gatts_characteristic_add(p->unwrap(), p->scratchpad.service_handle_2,
                                              &service_2_char_md_2, &service_2_attr_char_value_2,
                                              &(p->scratchpad.gatts_characteristic_handle_2));

        if (ret != NRF_SUCCESS)
        {
            return ret;
        }

        // Setup service #3
        err_code = sd_ble_gatts_service_add(p->unwrap(), BLE_GATTS_SRVC_TYPE_PRIMARY,
                                            &(p->scratchpad.target_service_3),
                                            &(p->scratchpad.service_handle_3));

        ble_gatts_char_md_t service_3_char_md_1{0};
        ble_gatts_attr_md_t service_3_cccd_md_1{0};
        ble_gatts_attr_t service_3_attr_char_value_1{0};
        ble_gatts_attr_md_t service_3_attr_md_1{0};

        ble_gatts_char_md_t service_3_char_md_2{0};
        ble_gatts_attr_md_t service_3_cccd_md_2{0};
        ble_gatts_attr_t service_3_attr_char_value_2{0};
        ble_gatts_attr_md_t service_3_attr_md_2{0};

        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_3_cccd_md_1.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_3_cccd_md_1.write_perm);
        service_3_cccd_md_1.vloc = BLE_GATTS_VLOC_STACK;

        service_3_char_md_1.char_props.read          = 1;
        service_3_char_md_1.char_props.notify        = 1;
        service_3_char_md_1.char_props.write         = 1;
        service_3_char_md_1.char_props.write_wo_resp = 1;
        service_3_char_md_1.p_char_user_desc         = nullptr;
        service_3_char_md_1.p_char_pf                = nullptr;
        service_3_char_md_1.p_user_desc_md           = nullptr;
        service_3_char_md_1.p_cccd_md                = &service_3_cccd_md_1;
        service_3_char_md_1.p_sccd_md                = nullptr;

        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_3_attr_md_1.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_3_attr_md_1.write_perm);
        service_3_attr_md_1.vloc    = BLE_GATTS_VLOC_STACK;
        service_3_attr_md_1.rd_auth = 0;
        service_3_attr_md_1.wr_auth = 0;
        service_3_attr_md_1.vlen    = 1;

        service_3_attr_char_value_1.p_uuid    = &(p->scratchpad.target_characteristic_3);
        service_3_attr_char_value_1.p_attr_md = &service_3_attr_md_1;
        service_3_attr_char_value_1.init_len =
            static_cast<uint16_t>(initialCharacteristicValue.size());
        service_3_attr_char_value_1.init_offs = 0;
        service_3_attr_char_value_1.max_len   = characteristicValueMaxLength;
        service_3_attr_char_value_1.p_value =
            const_cast<uint8_t *>(initialCharacteristicValue.data());

        ret = sd_ble_gatts_characteristic_add(p->unwrap(), p->scratchpad.service_handle_3,
                                              &service_3_char_md_1, &service_3_attr_char_value_1,
                                              &(p->scratchpad.gatts_characteristic_handle_3));

        return ret;
        //        if (ret != NRF_SUCCESS)
        //            return ret;

#if 0
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_3_cccd_md_2.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_3_cccd_md_2.write_perm);
        service_3_cccd_md_2.vloc = BLE_GATTS_VLOC_STACK;

        service_3_char_md_2.char_props.read          = 1;
        service_3_char_md_2.char_props.notify        = 1;
        service_3_char_md_2.char_props.write         = 1;
        service_3_char_md_2.char_props.write_wo_resp = 1;
        service_3_char_md_2.p_char_user_desc         = nullptr;
        service_3_char_md_2.p_char_pf                = nullptr;
        service_3_char_md_2.p_user_desc_md           = nullptr;
        service_3_char_md_2.p_cccd_md                = &service_3_cccd_md_1;
        service_3_char_md_2.p_sccd_md                = nullptr;

        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_3_attr_md_2.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&service_3_attr_md_2.write_perm);
        service_3_attr_md_2.vloc    = BLE_GATTS_VLOC_STACK;
        service_3_attr_md_2.rd_auth = 0;
        service_3_attr_md_2.wr_auth = 0;
        service_3_attr_md_2.vlen    = 1;

        service_3_attr_char_value_2.p_uuid    = &(p->scratchpad.target_characteristic_4);
        service_3_attr_char_value_2.p_attr_md = &service_3_attr_md_2;
        service_3_attr_char_value_2.init_len =
            static_cast<uint16_t>(initialCharacteristicValue.size());
        service_3_attr_char_value_2.init_offs = 0;
        service_3_attr_char_value_2.max_len   = characteristicValueMaxLength;
        service_3_attr_char_value_2.p_value =
            const_cast<uint8_t *>(initialCharacteristicValue.data());

        return sd_ble_gatts_characteristic_add(p->unwrap(), p->scratchpad.service_handle_3,
                                               &service_3_char_md_2, &service_3_attr_char_value_2,
                                               &(p->scratchpad.gatts_characteristic_handle_4));
#endif
    };

    const auto peripheralAdvName = testutil::createRandomAdvertisingName();

    // Instantiate an adapter to use as BLE Central in the test
    auto c = std::make_shared<testutil::AdapterWrapper>(
        testutil::Role::Central, central.port, env.baudRate, env.mtu, env.retransmissionInterval,
        env.responseTimeout);

    // Instantiated an adapter to use as BLE Peripheral in the test
    auto p = std::make_shared<testutil::AdapterWrapper>(
        testutil::Role::Peripheral, peripheral.port, env.baudRate, env.mtu,
        env.retransmissionInterval, env.responseTimeout);

    REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) == NRF_SUCCESS);
    REQUIRE(sd_rpc_log_handler_severity_filter_set(p->unwrap(), env.driverLogLevel) == NRF_SUCCESS);

    c->setGapEventCallback([&](const uint16_t eventId, const ble_gap_evt_t *gapEvent) -> bool {
        switch (eventId)
        {
            case BLE_GAP_EVT_CONNECTED:
                c->startServiceDiscovery(c_service_uuid_type, CUSTOM_SERVICE_UUID_1);
                return true;
            case BLE_GAP_EVT_DISCONNECTED:
                return true;
            case BLE_GAP_EVT_ADV_REPORT:
                if (testutil::findAdvName(gapEvent->params.adv_report, peripheralAdvName))
                {
                    if (!c->scratchpad.connection_in_progress)
                    {
                        const auto err_code = c->connect(&(gapEvent->params.adv_report.peer_addr));

                        if (err_code != NRF_SUCCESS)
                        {
                            get_logger()->error(
                                "{} Error connecting to {}, {}", c->role(),
                                testutil::asText(gapEvent->params.adv_report.peer_addr),
                                testutil::errorToString(err_code));
                            centralError = true;
                        }
                    }
                }
#if NRF_SD_BLE_API == 6
                else
                {
                    c->startScan(true);
                }
#endif
                return true;
            case BLE_GAP_EVT_TIMEOUT:
                if (gapEvent->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
                {
                    const auto err_code = c->startScan();

                    if (err_code != NRF_SUCCESS)
                    {
                        get_logger()->error("{} Scan start error, err_code: {:x}", c->role(),
                                            err_code);
                        centralError = true;
                    }
                }
                return true;
            case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST: {
                const auto err_code = sd_ble_gap_conn_param_update(
                    c->unwrap(), c->scratchpad.connection_handle,
                    &(gapEvent->params.conn_param_update_request.conn_params));
                if (err_code != NRF_SUCCESS)
                {
                    get_logger()->error("{} Conn params update failed, err_code {:x}", c->role(),
                                        err_code);
                    centralError = true;
                }
            }
                return true;
            default:
                return false;
        }
    });

    c->setGattcEventCallback([&](const uint16_t eventId,
                                 const ble_gattc_evt_t *gattcEvent) -> bool {
        switch (eventId)
        {
            case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP: {
                get_logger()->debug("{} Received service discovery response.", c->role());

                if (gattcEvent->gatt_status != NRF_SUCCESS)
                {
                    get_logger()->error("{} Service discovery failed.", c->role(),
                                        testutil::gattStatusToString(gattcEvent->gatt_status));
                    centralError = true;
                    return true;
                }

                const auto count = gattcEvent->params.prim_srvc_disc_rsp.count;

                if (count == 0)
                {
                    get_logger()->error("{} No services not found.", c->role());
                    centralError = true;
                    return true;
                }

                auto targetServiceFound = false;

                ble_gattc_service_t service{};

                for (auto service_index = 0; service_index < count; service_index++)
                {
                    service = gattcEvent->params.prim_srvc_disc_rsp.services[service_index];

                    if (service.uuid.uuid == c->scratchpad.target_service.uuid &&
                        service.uuid.type == c->scratchpad.target_service.type)
                    {
                        get_logger()->debug("{} Found target service {}", c->role(),
                                            testutil::asText(c->scratchpad.target_service));
                        targetServiceFound = true;
                        break;
                    }
                }

                if (!targetServiceFound)
                {
                    centralError = true;
                    get_logger()->error("{} Did not find target service {}", c->role(),
                                        testutil::asText(c->scratchpad.target_service));
                    return true;
                }

                c->scratchpad.service_start_handle = service.handle_range.start_handle;
                c->scratchpad.service_end_handle   = service.handle_range.end_handle;

                get_logger()->debug(
                    "{} Discovered target service. {} start_handle: {} end_handle: {}", c->role(),
                    testutil::asText(service.uuid),
                    testutil::asText(c->scratchpad.service_start_handle),
                    testutil::asText(c->scratchpad.service_end_handle));

                auto res = c->startCharacteristicDiscovery();

                if (res != NRF_SUCCESS)
                {
                    get_logger()->error("{} startCharacteristicDiscovery failed with error {:x}",
                                        c->role(), res);
                    centralError = true;
                }
            }
                return true;
            case BLE_GATTC_EVT_CHAR_DISC_RSP: {
                const auto count = gattcEvent->params.char_disc_rsp.count;

                if (gattcEvent->gatt_status != NRF_SUCCESS)
                {
                    get_logger()->error("{} Characteristic discovery failed. {}", c->role(),
                                        testutil::gattStatusToString(gattcEvent->gatt_status));
                    centralError = true;
                    return true;
                }

                get_logger()->debug(
                    "{} Received characteristic discovery response, characteristics count: {}",
                    c->role(), count);

                auto foundTargetCharacteristic = false;

                for (auto i = 0; i < count; i++)
                {
                    get_logger()->debug(
                        "{} [characteristic #{}] {}", c->role(), i,
                        testutil::asText(gattcEvent->params.char_disc_rsp.chars[i]));

                    if (gattcEvent->params.char_disc_rsp.chars[i].uuid.uuid ==
                            c->scratchpad.target_characteristic.uuid &&
                        gattcEvent->params.char_disc_rsp.chars[i].uuid.type ==
                            c->scratchpad.target_characteristic.type)
                    {
                        get_logger()->debug("{} Found target characteristic, {}", c->role(),
                                            testutil::asText(c->scratchpad.target_characteristic));
                        c->scratchpad.characteristic_decl_handle =
                            gattcEvent->params.char_disc_rsp.chars[i].handle_decl;
                        c->scratchpad.characteristic_value_handle =
                            gattcEvent->params.char_disc_rsp.chars[i].handle_value;
                        foundTargetCharacteristic = true;
                    }
                }

                if (!foundTargetCharacteristic)
                {
                    get_logger()->error("{} Did not find target characteristic {}", c->role(),
                                        testutil::asText(c->scratchpad.target_characteristic));
                    centralError = true;
                }
                else
                {
                    auto res = c->startDescriptorDiscovery();
                    if (res != NRF_SUCCESS)
                    {
                        get_logger()->error(
                            "{} Was not able to start descriptor discovery, got error {:x}",
                            c->role(), res);

                        centralError = true;
                    }
                }

                return true;
            }
            case BLE_GATTC_EVT_DESC_DISC_RSP: {
                const auto count = gattcEvent->params.desc_disc_rsp.count;

                if (gattcEvent->gatt_status != NRF_SUCCESS)
                {
                    get_logger()->error("{} Descriptor discovery failed. {}", c->role(),
                                        testutil::gattStatusToString(gattcEvent->gatt_status));
                    centralError = true;
                    return true;
                }

                get_logger()->debug(
                    "{} Received descriptor discovery response, descriptor count: {}", c->role(),
                    count);

                // Change the MTU
                const auto err_code = sd_ble_gattc_exchange_mtu_request(
                    c->unwrap(), c->scratchpad.connection_handle, c->scratchpad.mtu);

                if (err_code != NRF_SUCCESS)
                {
                    get_logger()->error("{} MTU exchange request failed, {} ", c->role(),
                                        testutil::errorToString(err_code));
                    centralError = true;
                }
            }
                return true;
            case BLE_GATTC_EVT_WRITE_RSP:
                get_logger()->debug("{} Received write response.", c->role());

                if (gattcEvent->gatt_status != NRF_SUCCESS)
                {
                    get_logger()->error("{}  Error. Write operation failed. {}", c->role(),
                                        testutil::gattStatusToString(gattcEvent->gatt_status));
                    centralError = true;
                }
                else
                {
                    testComplete = true;
                }

                return true;
            case BLE_GATTC_EVT_EXCHANGE_MTU_RSP: {
                auto const server_rx_mtu = gattcEvent->params.exchange_mtu_rsp.server_rx_mtu;
                get_logger()->debug("{} MTU response received. New ATT_MTU is  {}", c->role(),
                                    server_rx_mtu);

                // Write some data to characteristic to trigger reported error
                std::vector<uint8_t> data;

                // Send negotiated MTU - 3 bytes  used by GATT header
                testutil::appendRandomData(data, c->scratchpad.mtu - 3);

                const auto err_code =
                    c->writeCharacteristicValue(c->scratchpad.characteristic_value_handle, data);

                if (err_code != NRF_SUCCESS)
                {
                    get_logger()->error("{} Error writing data to characteristic. err_code: {:x}",
                                        c->role(), err_code);
                    centralError = true;
                }
            }
                return true;
            default:
                return false;
        }
    });

    c->setGattsEventCallback(
        [&](const uint16_t eventId, const ble_gatts_evt_t *gattsEvent) -> bool {
            switch (eventId)
            {
                case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST: {
                    const auto err_code = sd_ble_gatts_exchange_mtu_reply(
                        c->unwrap(), c->scratchpad.connection_handle, c->scratchpad.mtu);

                    if (err_code != NRF_SUCCESS)
                    {
                        get_logger()->error("{} MTU exchange request reply failed, {}", c->role(),
                                            testutil::errorToString(err_code));
                        centralError = true;
                    }
                }
                    return true;
                default:
                    return false;
            }
        });

    p->setGapEventCallback([&](const uint16_t eventId, const ble_gap_evt_t *gapEvent) {
        switch (eventId)
        {
            case BLE_GAP_EVT_DISCONNECTED: {
                // Use scratchpad defaults when advertising
                get_logger()->debug("{} Starting advertising.", p->role());

                const auto err_code = p->startAdvertising();
                if (err_code != NRF_SUCCESS)
                {
                    get_logger()->error("{} Error starting advertising after disconnect, {}",
                                        p->role(), testutil::errorToString(err_code));
                    peripheralError = true;
                }
            }
                return true;
            case BLE_GAP_EVT_SEC_PARAMS_REQUEST: {
                const auto err_code =
                    sd_ble_gap_sec_params_reply(p->unwrap(), p->scratchpad.connection_handle,
                                                BLE_GAP_SEC_STATUS_SUCCESS, nullptr, nullptr);

                if (err_code != NRF_SUCCESS)
                {
                    peripheralError = true;
                    get_logger()->error("{} Failed reply with GAP security parameters. {}",
                                        p->role(), testutil::errorToString(err_code));
                }
            }
                return true;
            case BLE_GATTS_EVT_SYS_ATTR_MISSING: {
                const auto err_code = sd_ble_gatts_sys_attr_set(
                    p->unwrap(), p->scratchpad.connection_handle, nullptr, 0, 0);

                if (err_code != NRF_SUCCESS)
                {
                    peripheralError = true;
                    get_logger()->error("{} Failed updating persistent sys attr info. {}",
                                        p->role(), testutil::errorToString(err_code));
                }
            }
                return true;
            default:
                return false;
        }
    });

    p->setGattsEventCallback(
        [&](const uint16_t eventId, const ble_gatts_evt_t *gattsEvent) -> bool {
            switch (eventId)
            {
                case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST: {
                    const auto err_code = sd_ble_gatts_exchange_mtu_reply(
                        p->unwrap(), p->scratchpad.connection_handle, p->scratchpad.mtu);

                    if (err_code != NRF_SUCCESS)
                    {
                        peripheralError = true;
                        get_logger()->error("{} MTU exchange request reply failed. {}", p->role(),
                                            testutil::errorToString(err_code));
                    }
                }
                    return true;
                default:
                    return false;
            }
        });

    c->setStatusCallback([&](const sd_rpc_app_status_t code, const std::string &message) {
        if (code == PKT_DECODE_ERROR || code == PKT_SEND_MAX_RETRIES_REACHED ||
            code == PKT_UNEXPECTED)
        {
            get_logger()->error("{} error in status callback {:x}:{}", c->role(),
                                static_cast<uint32_t>(code), message);
            centralError = true;
        }
    });

    p->setStatusCallback([&](const sd_rpc_app_status_t code, const std::string &message) {
        if (code == PKT_DECODE_ERROR || code == PKT_SEND_MAX_RETRIES_REACHED ||
            code == PKT_UNEXPECTED)
        {
            peripheralError = true;
            get_logger()->error("{} error in status callback {:x}:{}", p->role(),
                                static_cast<uint32_t>(code), message);
        }
    });

    // Open the adapters
    REQUIRE(c->open() == NRF_SUCCESS);
    REQUIRE(p->open() == NRF_SUCCESS);

    REQUIRE(c->configure() == NRF_SUCCESS);
    REQUIRE(p->configure() == NRF_SUCCESS);

    // Service #1
    ble_uuid128_t p_service_base_uuid_1 = {CUSTOM_SERVICE_UUID_BASE_1};
    REQUIRE(sd_ble_uuid_vs_add(p->unwrap(), &p_service_base_uuid_1, &p_service_uuid_type) ==
            NRF_SUCCESS);

    p->scratchpad.target_service.uuid = CUSTOM_SERVICE_UUID_1;
    p->scratchpad.target_service.type = p_service_uuid_type;

    // Service #2
    ble_uuid128_t p_service_base_uuid_2 = {CUSTOM_SERVICE_UUID_BASE_2};
    REQUIRE(sd_ble_uuid_vs_add(p->unwrap(), &p_service_base_uuid_2, &p_service_2_uuid_type) ==
            NRF_SUCCESS);

    p->scratchpad.target_service_2.uuid = CUSTOM_SERVICE_UUID_2;
    p->scratchpad.target_service_2.type = p_service_2_uuid_type;

    // Characteristic #2.1
    ble_uuid128_t p_service_2_characteristic_base_uuid_1 = {
        CUSTOM_SERVICE_2_CHARACTERISTIC_UUID_BASE_1};
    REQUIRE(sd_ble_uuid_vs_add(p->unwrap(), &p_service_2_characteristic_base_uuid_1,
                               &p_service_2_characteristic_uuid_type_1) == NRF_SUCCESS);
    p->scratchpad.target_characteristic.uuid = CUSTOM_SERVICE_2_VALUE_CHAR_UUID_1;
    p->scratchpad.target_characteristic.type = p_service_2_characteristic_uuid_type_1;
    p->scratchpad.target_descriptor.uuid     = BLE_UUID_CCCD;
    p->scratchpad.target_descriptor.type     = BLE_UUID_TYPE_BLE;

    // Characteristic #2.2
    ble_uuid128_t p_service_2_characteristic_base_uuid_2 = {
        CUSTOM_SERVICE_2_CHARACTERISTIC_UUID_BASE_2};
    REQUIRE(sd_ble_uuid_vs_add(p->unwrap(), &p_service_2_characteristic_base_uuid_2,
                               &p_service_2_characteristic_uuid_type_2) == NRF_SUCCESS);
    p->scratchpad.target_characteristic_2.uuid = CUSTOM_SERVICE_2_VALUE_CHAR_UUID_2;
    p->scratchpad.target_characteristic_2.type = p_service_2_characteristic_uuid_type_2;

    p->scratchpad.target_descriptor_2.uuid = BLE_UUID_CCCD;
    p->scratchpad.target_descriptor_2.type = BLE_UUID_TYPE_BLE;

    // Service #3
    ble_uuid128_t p_service_base_uuid_3 = {CUSTOM_SERVICE_UUID_BASE_3};
    REQUIRE(sd_ble_uuid_vs_add(p->unwrap(), &p_service_base_uuid_3, &p_service_3_uuid_type) ==
            NRF_SUCCESS);
    p->scratchpad.target_service_3.uuid = CUSTOM_SERVICE_UUID_3;
    p->scratchpad.target_service_3.type = p_service_2_uuid_type;

    // Characteristic #3.1
    ble_uuid128_t p_service_3_characteristic_base_uuid_1 = {
        CUSTOM_SERVICE_3_CHARACTERISTIC_UUID_BASE_1};
    REQUIRE(sd_ble_uuid_vs_add(p->unwrap(), &p_service_3_characteristic_base_uuid_1,
                               &p_service_3_characteristic_uuid_type_1) == NRF_SUCCESS);
    p->scratchpad.target_characteristic_3.uuid = CUSTOM_SERVICE_3_VALUE_CHAR_UUID_1;
    p->scratchpad.target_characteristic_3.type = p_service_3_characteristic_uuid_type_1;
    p->scratchpad.target_descriptor_3.uuid     = BLE_UUID_CCCD;
    p->scratchpad.target_descriptor_3.type     = BLE_UUID_TYPE_BLE;

    // Characteristic #3.2
    ble_uuid128_t p_service_3_characteristic_base_uuid_2 = {
        CUSTOM_SERVICE_3_CHARACTERISTIC_UUID_BASE_2};
    REQUIRE(sd_ble_uuid_vs_add(p->unwrap(), &p_service_3_characteristic_base_uuid_2,
                               &p_service_3_characteristic_uuid_type_2) == NRF_SUCCESS);
    p->scratchpad.target_characteristic_4.uuid = CUSTOM_SERVICE_3_VALUE_CHAR_UUID_2;
    p->scratchpad.target_characteristic_4.type = p_service_3_characteristic_uuid_type_2;

    p->scratchpad.target_descriptor_4.uuid = BLE_UUID_CCCD;
    p->scratchpad.target_descriptor_4.type = BLE_UUID_TYPE_BLE;

    // Generic setup
    p->scratchpad.mtu = 150;

#if 0
    // BLE Central scratchpad
    ble_uuid128_t c_service_base_uuid = {CUSTOM_SERVICE_UUID_BASE};
    REQUIRE(sd_ble_uuid_vs_add(c->unwrap(), &c_service_base_uuid, &c_service_uuid_type) ==
            NRF_SUCCESS);

    c->scratchpad.target_service.uuid = CUSTOM_SERVICE_UUID;
    c->scratchpad.target_service.type = c_service_uuid_type;

    uint8_t c_characteristic_uuid_type;
    ble_uuid128_t c_characteristic_base_uuid = {CUSTOM_CHARACTERISTIC_UUID_BASE_1};
    REQUIRE(sd_ble_uuid_vs_add(c->unwrap(), &c_characteristic_base_uuid,
                                &c_characteristic_uuid_type) == NRF_SUCCESS);

    c->scratchpad.target_characteristic.uuid = CUSTOM_VALUE_CHAR_UUID;
    c->scratchpad.target_characteristic.type = c_characteristic_uuid_type;

    c->scratchpad.target_descriptor.uuid = BLE_UUID_CCCD;
    c->scratchpad.target_descriptor.type = BLE_UUID_TYPE_BLE;
    c->scratchpad.mtu                    = 150;
#endif

    REQUIRE(setupPeripheral(p, peripheralAdvName, {0x00}, p->scratchpad.mtu) == NRF_SUCCESS);
    REQUIRE(p->startAdvertising() == NRF_SUCCESS);

    // Starting the scan starts the sequence of operations to get a connection established

    // REQUIRE(c->startScan() == NRF_SUCCESS);

    // Wait for the test to complete
    std::this_thread::sleep_for(std::chrono::seconds(300));

    CHECK(centralError == false);
    CHECK(peripheralError == false);
    CHECK(testComplete == true);

    CHECK(c->close() == NRF_SUCCESS);
    sd_rpc_adapter_delete(c->unwrap());

    CHECK(p->close() == NRF_SUCCESS);
    sd_rpc_adapter_delete(p->unwrap());
}

#endif // This issue is only relevant for SoftDevice API >= 5
