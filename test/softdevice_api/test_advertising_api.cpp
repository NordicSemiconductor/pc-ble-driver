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

#if NRF_SD_BLE_API >= 6

// Logging support
#include <internal/log.h>

// Test support
#include <test_setup.h>
#include <test_util.h>

#include <ble.h>
#include <sd_rpc.h>

#include <memory>
#include <sstream>
#include <thread>

// Indicates if an error has occurred in a callback.
// The test framework is not thread safe so this variable is used to communicate that an issues has
// occurred in a callback.
bool error = false;

enum TestState {
    P_ADVERTISING_FAST_EXTENDED,
    P_ADVERTISING_FAST_EXTENDED_NOTIFICATION, // BLE_ADV_EVT_FAST
    C_ADVERTISING_EXTENDED_RECEIVED,
    P_RECEIVED_SCAN_REQ_NOTIFICATION, // BLE_GAP_EVT_SCAN_REQ_REPORT
};

TestState testState;

TEST_CASE("test_advertising_api")
{
    auto env = ::test::getEnvironment();
    REQUIRE(env.serialPorts.size() >= 2);
    const auto central    = env.serialPorts.at(0);
    const auto peripheral = env.serialPorts.at(1);

#if NRF_SD_BLE_API == 6
    SECTION("test_extended")
    {
        const auto baudRate = central.baudRate;

        INFO("Central serial port used: " << central.port);
        INFO("Peripheral serial port used: " << peripheral.port);
        INFO("Baud rate used: " << baudRate);

        std::vector<uint8_t> peripheralAdvNameBase;
        testutil::appendRandomAlphaNumeric(peripheralAdvNameBase, testutil::ADV_DATA_BUFFER_SIZE -
                                                                      2 /* length and AD type */);

        const std::string peripheralAdvName(peripheralAdvNameBase.begin(),
                                            peripheralAdvNameBase.end());

        // Instantiate an adapter to use as BLE Central in the test
        auto c = std::make_shared<testutil::AdapterWrapper>(testutil::Central, central.port,
                                                            baudRate, 150);

        // Instantiated an adapter to use as BLE Peripheral in the test
        auto p = std::make_shared<testutil::AdapterWrapper>(testutil::Peripheral, peripheral.port,
                                                            baudRate, 150);

        REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);
        REQUIRE(sd_rpc_log_handler_severity_filter_set(p->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);

        c->setGapEventCallback([&c, peripheralAdvName](const uint16_t eventId,
                                                       const ble_gap_evt_t *gapEvent) -> bool {
            switch (eventId)
            {
                case BLE_GAP_EVT_ADV_REPORT:
                    if (testutil::findAdvName(&(gapEvent->params.adv_report), peripheralAdvName))
                    {
                        // TODO: do additional checks on the received advertisement report
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
                            NRF_LOG(c->role()
                                    << " Scan start error, err_code " << std::hex << err_code);
                            error = true;
                        }
                    }
                    return true;
                default:
                    return false;
            }
        });

        c->setEventCallback([&c](const ble_evt_t *p_ble_evt) -> bool {
            const auto eventId = p_ble_evt->header.evt_id;
            NRF_LOG(c->role() << " Received an un-handled event with ID: " << std::hex << eventId);
            return true;
        });

        p->setGapEventCallback([](const uint16_t eventId, const ble_gap_evt_t *gapEvent) {
            switch (eventId)
            {
                case BLE_GAP_EVT_TIMEOUT:
                    return true;
                case BLE_GAP_EVT_SCAN_REQ_REPORT:
                case BLE_GAP_EVT_ADV_SET_TERMINATED:
                    return true;
                default:
                    return false;
            }
        });

        p->setEventCallback([&p](const ble_evt_t *p_ble_evt) -> bool {
            const auto eventId = p_ble_evt->header.evt_id;
            NRF_LOG(p->role() << " Received an un-handled event with ID: " << std::hex << eventId);
            return true;
        });

        // Open the adapters
        REQUIRE(c->open() == NRF_SUCCESS);
        REQUIRE(p->open() == NRF_SUCCESS);

        REQUIRE(c->configure() == NRF_SUCCESS);
        REQUIRE(p->configure() == NRF_SUCCESS);

        // Create advertising data and scan response data
        std::vector<uint8_t> advertising;
#if 1
        testutil::appendAdvertisingName(advertising, peripheralAdvName);

        std::vector<uint8_t> scanResponse;
        std::vector<uint8_t> scanResponseData;
        testutil::appendRandomData(scanResponseData,
                                   testutil::ADV_DATA_BUFFER_SIZE - 2 /* length and AD type */);

        scanResponse.reserve(scanResponseData.size() + 2);
        scanResponse.push_back(
            static_cast<uint8_t>(scanResponseData.size() + 1)); // Data + advertisement type
        scanResponse.push_back(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA);
        std::copy(scanResponseData.begin(), scanResponseData.end(),
                  std::back_inserter(scanResponse));
#else
        testutil::appendAdvertisingName(advertising, "12345");
        const std::vector<uint8_t> scanResponse = {};
#endif

        REQUIRE(p->setupAdvertising(advertising,              // advertising data
                                    scanResponse,             // scan response data
                                    40, //BLE_GAP_ADV_INTERVAL_MIN, // interval
                                    0,                        // duration
                                    false,                    // connectable
                                    true,                     // extended
                                    false, // true,                     // scan_req_notification
                                    0,     // set_id
                                    BLE_GAP_PHY_1MBPS, // primary phy
                                    BLE_GAP_PHY_2MBPS, // secondary phy
                                    0,                 // filter policy
                                    0                  // max_adv_events
                                    ) == NRF_SUCCESS);
        REQUIRE(p->startAdvertising() == NRF_SUCCESS);

        REQUIRE(c->startScan() == NRF_SUCCESS);

        // Wait for the test to complete
        std::this_thread::sleep_for(std::chrono::seconds(5));

        REQUIRE(error == false);

        REQUIRE(c->close() == NRF_SUCCESS);
        sd_rpc_adapter_delete(c->unwrap());

        REQUIRE(p->close() == NRF_SUCCESS);
        sd_rpc_adapter_delete(p->unwrap());
    }
#endif // NRF_SD_BLE_API == 6
}

#else
TEST_CASE("test_advertising_api")
{
    INFO("Not relevant for SoftDevice API version < 3")
}

#endif // NRF_SD_BLE_API >= 6
