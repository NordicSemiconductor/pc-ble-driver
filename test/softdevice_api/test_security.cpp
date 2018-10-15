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
#include <internal/log.h>

// Test support
#include <test_setup.h>
#include <test_util.h>

#include <ble.h>
#include <sd_rpc.h>

#include <sstream>
#include <string>
#include <thread>

constexpr uint16_t BLE_UUID_HEART_RATE_SERVICE          = 0x180D;
constexpr uint16_t BLE_UUID_HEART_RATE_MEASUREMENT_CHAR = 0x2A37;
constexpr uint16_t BLE_UUID_CCCD                        = 0x2902;

// Indicates if an error has occurred in a callback.
// The test framework is not thread safe so this variable is used to communicate that an issues has
// occurred in a callback.
bool error = false;

// Set to true when the test is complete
bool testComplete = false;

enum AuthenticationType {
    LEGACY_PASSKEY,
    LEGACY_OOB,
};

AuthenticationType authType;

uint32_t setupPeripheral(const std::shared_ptr<testutil::AdapterWrapper> &p,
                         const std::string &advertisingName,
                         const std::vector<uint8_t> &initialCharacteristicValue,
                         const uint16_t characteristicValueMaxLength)
{
    // Setup the advertisement data
    std::vector<uint8_t> advertisingData;
    testutil::appendAdvertisingName(advertisingData, advertisingName);
    advertisingData.push_back(3); // Length of upcoming advertisement type
    advertisingData.push_back(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE);

    // Store BLE_UUID_HEART_RATE_SERVICE in little-endian format.
    advertisingData.push_back(BLE_UUID_HEART_RATE_SERVICE & 0xFF);
    advertisingData.push_back((BLE_UUID_HEART_RATE_SERVICE & 0xFF00) >> 8);

    auto err_code = p->setupAdvertising(advertisingData);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG(p->role() << " Error setting advertising data, "
                          << ", " << testutil::errorToString(err_code));
        return err_code;
    }

    // Setup service, use service UUID specified in scratchpad.target_service
    err_code =
        sd_ble_gatts_service_add(p->unwrap(), BLE_GATTS_SRVC_TYPE_PRIMARY,
                                 &(p->scratchpad.target_service), &(p->scratchpad.service_handle));
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG(p->role() << " Error adding GATTS service, "
                          << ", " << testutil::errorToString(err_code));
        return err_code;
    }

    // Setup characteristic, use characteristic UUID specified in scratchpad.target_characteristic
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t attr_char_value;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = 1;
    char_md.char_props.notify        = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc         = nullptr;
    char_md.p_char_pf                = nullptr;
    char_md.p_user_desc_md           = nullptr;
    char_md.p_cccd_md                = &cccd_md;
    char_md.p_sccd_md                = nullptr;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &(p->scratchpad.target_characteristic);
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = static_cast<uint16_t>(initialCharacteristicValue.size());
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = characteristicValueMaxLength;
    attr_char_value.p_value   = const_cast<uint8_t *>(initialCharacteristicValue.data());

    err_code = sd_ble_gatts_characteristic_add(p->unwrap(), p->scratchpad.service_handle, &char_md,
                                               &attr_char_value,
                                               &(p->scratchpad.gatts_characteristic_handle));

    NRF_LOG(p->role() << " Error adding GATTS characteristic"
                      << ", " << testutil::errorToString(err_code));

    return err_code;
}

TEST_CASE("test_security")
{
    auto env = ::test::getEnvironment();
    REQUIRE(env.serialPorts.size() >= 2);
    const auto central    = env.serialPorts.at(0);
    const auto peripheral = env.serialPorts.at(1);

    SECTION("legacy_passkey")
    {
        const auto baudRate = central.baudRate;

        INFO("Central serial port used: " << central.port);
        INFO("Peripheral serial port used: " << peripheral.port);
        INFO("Baud rate used: " << baudRate);

        const auto peripheralAdvName = "peripheral";

        // Instantiate an adapter to use as BLE Central in the test
        auto c =
            std::make_shared<testutil::AdapterWrapper>(testutil::Central, central.port, baudRate);

        // Instantiated an adapter to use as BLE Peripheral in the test
        auto p = std::make_shared<testutil::AdapterWrapper>(testutil::Peripheral, peripheral.port,
                                                            baudRate);

        // Use Heart rate service and characteristics as target for testing but
        // the values sent are not according to the Heart Rate service

        // BLE Central scratchpad
        c->scratchpad.target_service.uuid = BLE_UUID_HEART_RATE_SERVICE;
        c->scratchpad.target_service.type = BLE_UUID_TYPE_BLE;

        c->scratchpad.target_characteristic.uuid = BLE_UUID_HEART_RATE_MEASUREMENT_CHAR;
        c->scratchpad.target_characteristic.type = BLE_UUID_TYPE_BLE;

        c->scratchpad.target_descriptor.uuid = BLE_UUID_CCCD;
        c->scratchpad.target_descriptor.type = BLE_UUID_TYPE_BLE;

        // BLE Peripheral scratchpad
        p->scratchpad.target_service.uuid = BLE_UUID_HEART_RATE_SERVICE;
        p->scratchpad.target_service.type = BLE_UUID_TYPE_BLE;

        p->scratchpad.target_characteristic.uuid = BLE_UUID_HEART_RATE_MEASUREMENT_CHAR;
        p->scratchpad.target_characteristic.type = BLE_UUID_TYPE_BLE;

        p->scratchpad.target_descriptor.uuid = BLE_UUID_CCCD;
        p->scratchpad.target_descriptor.type = BLE_UUID_TYPE_BLE;

        REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);
        REQUIRE(sd_rpc_log_handler_severity_filter_set(p->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);

        c->setGapEventCallback([&c, &p, peripheralAdvName](const uint16_t eventId,
                                                           const ble_gap_evt_t *gapEvent) -> bool {
            switch (eventId)
            {
                case BLE_GAP_EVT_CONNECTED:
                {
                    authType            = LEGACY_PASSKEY;
                    const auto err_code = c->startAuthentication(true, true, false, true);

                    if (err_code != NRF_SUCCESS)
                    {
                        error = true;
                    }
                }
                    return true;
                case BLE_GAP_EVT_DISCONNECTED:
                    return true;
                case BLE_GAP_EVT_ADV_REPORT:
                    if (testutil::findAdvName(&(gapEvent->params.adv_report), peripheralAdvName))
                    {
                        if (!c->scratchpad.connection_in_progress)
                        {
                            const auto err_code =
                                c->connect(&(gapEvent->params.adv_report.peer_addr));

                            if (err_code != NRF_SUCCESS)
                            {
                                NRF_LOG(c->role()
                                        << " Error connecting to "
                                        << testutil::asText(gapEvent->params.adv_report.peer_addr)
                                        << ", " << testutil::errorToString(err_code));
                                error = true;
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
                            NRF_LOG(c->role() << " Scan start error, err_code " << err_code);
                            error = true;
                        }
                    }
                    return true;
                case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
                {
                    const auto err_code = sd_ble_gap_conn_param_update(
                        c->unwrap(), c->scratchpad.connection_handle,
                        &(gapEvent->params.conn_param_update_request.conn_params));

                    if (err_code != NRF_SUCCESS)
                    {
                        NRF_LOG(c->role() << " Conn params update failed, err_code " << err_code);
                        error = true;
                    }
                }
                    return true;
                case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
                {
                    ble_gap_sec_keyset_t keyset;
                    memset(&keyset, 0, sizeof(ble_gap_sec_keyset_t));

                    ble_gap_sec_keys_t keys_own;
                    ble_gap_sec_keys_t keys_peer;
                    memset(&keys_own, 0, sizeof(keys_own));
                    memset(&keys_peer, 0, sizeof(keys_peer));

                    keyset.keys_own  = keys_own;
                    keyset.keys_peer = keys_peer;

                    const auto err_code = c->securityParamsReply(keyset);

                    if (err_code != NRF_SUCCESS)
                    {
                        error = true;
                    }
                }
                    return true;
                case BLE_GAP_EVT_PASSKEY_DISPLAY:
                {
                    size_t size = 0;

                    switch (p->scratchpad.key_type)
                    {
                        case BLE_GAP_AUTH_KEY_TYPE_PASSKEY:
                            size = 6;
                            break;
                        case BLE_GAP_AUTH_KEY_TYPE_OOB:
                            size = 16;
                            break;
                        default:
                            break;
                    }

                    if (size != 0)
                    {
                        std::memcpy(c->scratchpad.key, gapEvent->params.passkey_display.passkey,
                                    size);
                    }

                    const auto err_code =
                        p->authKeyReply(p->scratchpad.key_type, c->scratchpad.key);

                    if (err_code != NRF_SUCCESS)
                    {
                        error = true;
                    }
                }
                    return true;
                case BLE_GAP_EVT_SEC_REQUEST:
                    return true;
                case BLE_GAP_EVT_AUTH_KEY_REQUEST:
                    return true;
                case BLE_GAP_EVT_CONN_SEC_UPDATE:
                    return true;
                case BLE_GAP_EVT_AUTH_STATUS:
                    if (gapEvent->params.auth_status.auth_status == BLE_GAP_SEC_STATUS_SUCCESS)
                    {
                        testComplete = true;
                    }
                    else
                    {
                        error = true;
                    }

                    return true;
                default:
                    return false;
            }
        });

        c->setGattcEventCallback([&c](const uint16_t eventId,
                                      const ble_gattc_evt_t *gattcEvent) -> bool {
            switch (eventId)
            {
                case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
                {
                    NRF_LOG(c->role() << " BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP");

                    if (gattcEvent->gatt_status != NRF_SUCCESS)
                    {
                        NRF_LOG(c->role() << " Service discovery failed. "
                                          << testutil::gattStatusToString(gattcEvent->gatt_status));
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

                        if (service.uuid.uuid == c->scratchpad.target_service.uuid &&
                            service.uuid.type == c->scratchpad.target_service.type)
                        {
                            NRF_LOG(c->role() << " Found target service "
                                              << testutil::asText(c->scratchpad.target_service));
                            targetServiceFound = true;
                            break;
                        }
                    }

                    if (!targetServiceFound)
                    {
                        error = true;
                        NRF_LOG(c->role() << " Did not find target service "
                                          << testutil::asText(c->scratchpad.target_service));
                        return true;
                    }

                    c->scratchpad.service_start_handle = service.handle_range.start_handle;
                    c->scratchpad.service_end_handle   = service.handle_range.end_handle;

                    NRF_LOG(c->role() << " Discovered target service. "
                                      << testutil::asText(service.uuid) << " start_handle: "
                                      << testutil::asText(c->scratchpad.service_start_handle)
                                      << " end_handle: "
                                      << testutil::asText(c->scratchpad.service_end_handle));

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
                        NRF_LOG(c->role() << " Characteristic discovery failed. "
                                          << testutil::gattStatusToString(gattcEvent->gatt_status));
                        error = true;
                        return true;
                    }

                    NRF_LOG(
                        c->role()
                        << " Received characteristic discovery response, characteristics count: "
                        << count);

                    auto foundTargetCharacteristic = false;

                    for (auto i = 0; i < count; i++)
                    {
                        NRF_LOG(c->role()
                                << " [characteristic #" << i << "] "
                                << testutil::asText(gattcEvent->params.char_disc_rsp.chars[i]));

                        if (gattcEvent->params.char_disc_rsp.chars[i].uuid.uuid ==
                                c->scratchpad.target_characteristic.uuid &&
                            gattcEvent->params.char_disc_rsp.chars[i].uuid.type ==
                                c->scratchpad.target_characteristic.type)
                        {
                            NRF_LOG(c->role()
                                    << " Found target characteristic, "
                                    << testutil::asText(c->scratchpad.target_characteristic));
                            c->scratchpad.characteristic_decl_handle =
                                gattcEvent->params.char_disc_rsp.chars[i].handle_decl;
                            c->scratchpad.characteristic_value_handle =
                                gattcEvent->params.char_disc_rsp.chars[i].handle_value;
                            foundTargetCharacteristic = true;
                        }
                    }

                    if (!foundTargetCharacteristic)
                    {
                        NRF_LOG(c->role() << " Did not find target characteristic "
                                          << testutil::asText(c->scratchpad.target_characteristic));
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
                        NRF_LOG(c->role() << " Descriptor discovery failed. "
                                          << testutil::gattStatusToString(gattcEvent->gatt_status));
                        error = true;
                        return true;
                    }

                    NRF_LOG(c->role()
                            << " Received descriptor discovery response, descriptor count: "
                            << count);
                }
                    return true;
                case BLE_GATTC_EVT_WRITE_RSP:
                    NRF_LOG(c->role() << " Received write response.");

                    if (gattcEvent->gatt_status != NRF_SUCCESS)
                    {
                        NRF_LOG(c->role() << " Error. Write operation failed.  "
                                          << testutil::gattStatusToString(gattcEvent->gatt_status));
                        error = true;
                    }

                    testComplete = true;

                    return true;
                default:
                    return false;
            }
        });

        c->setEventCallback([&c](const ble_evt_t *p_ble_evt) -> bool {
            const auto eventId = p_ble_evt->header.evt_id;
            NRF_LOG(c->role() << " Received an un-handled event with ID: " << eventId);
            return true;
        });

        p->setGapEventCallback([&p](const uint16_t eventId, const ble_gap_evt_t *gapEvent) {
            switch (eventId)
            {
                case BLE_GAP_EVT_CONNECTED:
                    return true;
                case BLE_GAP_EVT_DISCONNECTED:
                {
                    // Use scratchpad defaults when advertising
                    NRF_LOG(p->role() << " Starting advertising.");
                    const auto err_code = p->startAdvertising();
                    if (err_code != NRF_SUCCESS)
                    {
                        error = true;
                    }
                }
                    return true;
                case BLE_GAP_EVT_TIMEOUT:
                    return true;
                case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
                {
                    ble_gap_sec_keyset_t keyset;
                    std::memset(&keyset, 0, sizeof(keyset));

                    const auto err_code =
                        p->securityParamsReply(BLE_GAP_SEC_STATUS_SUCCESS, keyset);

                    if (err_code != NRF_SUCCESS)
                    {
                        error = true;
                    }
                }
                    return true;
                case BLE_GAP_EVT_PASSKEY_DISPLAY:
                    return true;
                case BLE_GAP_EVT_SEC_REQUEST:
                    return true;
                case BLE_GAP_EVT_AUTH_KEY_REQUEST:
                    return true;
                case BLE_GAP_EVT_CONN_SEC_UPDATE:
                    return true;
                case BLE_GAP_EVT_AUTH_STATUS:
                    if (gapEvent->params.auth_status.auth_status == BLE_GAP_SEC_STATUS_SUCCESS)
                    {
                        // Move on to the next type of bonding/pairing
                        testComplete = true;
                    }
                    else
                    {
                        error = true;
                    }

                    return true;
                case BLE_GATTS_EVT_SYS_ATTR_MISSING:
                {
                    NRF_LOG(p->role() << " BLE_GATTS_EVT_SYS_ATTR_MISSING");

                    const auto err_code = sd_ble_gatts_sys_attr_set(
                        p->unwrap(), p->scratchpad.connection_handle, nullptr, 0, 0);

                    if (err_code != NRF_SUCCESS)
                    {
                        error = true;
                        NRF_LOG(p->role() << "Failed updating persistent sys attr info. "
                                          << testutil::errorToString(err_code));
                    }
                }
                    return true;
                default:
                    return false;
            }
        });

        p->setEventCallback([&p](const ble_evt_t *p_ble_evt) -> bool {
            const auto eventId = p_ble_evt->header.evt_id;
            NRF_LOG(p->role() << " Received an un-handled event with ID: " << eventId);
            return true;
        });

        // Open the adapters
        REQUIRE(c->open() == NRF_SUCCESS);
        REQUIRE(p->open() == NRF_SUCCESS);

        REQUIRE(c->configure() == NRF_SUCCESS);
        REQUIRE(p->configure() == NRF_SUCCESS);

        // Starting the scan starts the sequence of operations to get a connection established
        REQUIRE(c->startScan() == NRF_SUCCESS);

        REQUIRE(setupPeripheral(p, peripheralAdvName, {0x00}, p->scratchpad.mtu) == NRF_SUCCESS);
        REQUIRE(p->startAdvertising() == NRF_SUCCESS);

        // Wait for the test to complete
        std::this_thread::sleep_for(std::chrono::seconds(5));

        REQUIRE(error == false);
        REQUIRE(testComplete == true);

        REQUIRE(c->close() == NRF_SUCCESS);
        sd_rpc_adapter_delete(c->unwrap());

        REQUIRE(p->close() == NRF_SUCCESS);
        sd_rpc_adapter_delete(p->unwrap());
    }
}
