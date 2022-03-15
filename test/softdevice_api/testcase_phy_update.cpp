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
#include "catch2/catch.hpp"

#if NRF_SD_BLE_API >= 6

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

using namespace testutil;

TEST_CASE(CREATE_TEST_NAME_AND_TAGS(phy_update, [known_issue][PCA10056][PCA10059][nRF52840]))
{
    // Indicates if an error has occurred in a callback.
    // The test framework is not thread safe so this variable is used to communicate that an issues
    // has occurred in a callback.
    auto error = false;

    // Set to true when the test is complete
    auto testComplete = false;

    auto env = ::test::getEnvironment();
    INFO(::test::getEnvironmentAsText(env));
    REQUIRE(env.serialPorts.size() >= 2);
    const auto central    = env.serialPorts.at(0);
    const auto peripheral = env.serialPorts.at(1);

    SECTION("update_from_1mps_to_2mpb")
    {
        const auto advertisementNameLength = 20;
        std::vector<uint8_t> peripheralAdvNameBuffer(advertisementNameLength);
        testutil::appendRandomAlphaNumeric(peripheralAdvNameBuffer, advertisementNameLength);
        const std::string peripheralAdvName(peripheralAdvNameBuffer.begin(),
                                            peripheralAdvNameBuffer.end());
        std::vector<uint8_t> advResponse;
        testutil::appendAdvertisingName(advResponse, peripheralAdvName);

        ble_gap_phys_t requestedPhys{BLE_GAP_PHY_2MBPS, BLE_GAP_PHY_2MBPS};

        // Instantiate an adapter to use as BLE Central in the test
        auto c = std::make_shared<testutil::AdapterWrapper>(
            testutil::Role::Central, central.port, env.baudRate, env.mtu,
            env.retransmissionInterval, env.responseTimeout);

        // Instantiated an adapter to use as BLE Peripheral in the test
        auto p = std::make_shared<testutil::AdapterWrapper>(
            testutil::Role::Peripheral, peripheral.port, env.baudRate, env.mtu,
            env.retransmissionInterval, env.responseTimeout);

        REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);
        REQUIRE(sd_rpc_log_handler_severity_filter_set(p->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);

        c->setGapEventCallback([&](const uint16_t eventId, const ble_gap_evt_t *gapEvent) -> bool {
            switch (eventId)
            {
                case BLE_GAP_EVT_CONNECTED: {
                    const auto err_code =
                        sd_ble_gap_phy_update(c->unwrap(), gapEvent->conn_handle, &requestedPhys);

                    if (err_code != NRF_SUCCESS)
                    {
                        get_logger()->error(
                            "{} BLE_GAP_EVT_CONNECTED: error calling sd_ble_gap_phy_update, {}",
                            c->role(), testutil::errorToString(err_code));
                        error = true;
                    }
                }
                    return true;
                case BLE_GAP_EVT_DISCONNECTED:
                    return true;
                case BLE_GAP_EVT_ADV_REPORT:
                    if (testutil::findAdvName(gapEvent->params.adv_report, peripheralAdvName))
                    {
                        if (!c->scratchpad.connection_in_progress)
                        {
                            const auto err_code =
                                c->connect(&(gapEvent->params.adv_report.peer_addr));

                            if (err_code != NRF_SUCCESS)
                            {
                                get_logger()->error(
                                    "{}  Error connecting to {}, {}", c->role(),
                                    testutil::asText(gapEvent->params.adv_report.peer_addr),
                                    testutil::errorToString(err_code));
                                error = true;
                            }
                        }
                    }
                    else
                    {
                        c->startScan(true);
                    }
                    return true;
                case BLE_GAP_EVT_TIMEOUT:
                    if (gapEvent->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
                    {
                        const auto err_code = c->startScan();

                        if (err_code != NRF_SUCCESS)
                        {
                            get_logger()->error("{} Scan start error, err_code {:x}", c->role(),
                                                err_code);
                            error = true;
                        }
                    }
                    return true;
                case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST: {
                    const auto err_code = sd_ble_gap_conn_param_update(
                        c->unwrap(), c->scratchpad.connection_handle,
                        &(gapEvent->params.conn_param_update_request.conn_params));

                    if (err_code != NRF_SUCCESS)
                    {
                        get_logger()->error("{} Conn params update failed, err_code {:x}",
                                            c->role(), err_code);
                        error = true;
                    }
                }
                    return true;
                case BLE_GAP_EVT_PHY_UPDATE: {
                    if (gapEvent->params.phy_update.rx_phy != requestedPhys.rx_phys ||
                        gapEvent->params.phy_update.tx_phy != requestedPhys.tx_phys)
                    {
                        get_logger()->error(
                            "{} BLE_GAP_EVT_PHY_UPDATE: phy is not updated according to request",
                            c->role());
                        error = true;
                    }
                    else
                    {
                        const auto status = gapEvent->params.phy_update.status;
                        if (status != BLE_HCI_STATUS_CODE_SUCCESS)
                        {
                            get_logger()->error("{} BLE_GAP_EVT_PHY_UPDATE: status is {:x}, should "
                                                "be BLE_HCI_STATUS_CODE_SUCCESS({})",
                                                c->role(), status, BLE_HCI_STATUS_CODE_SUCCESS);
                            error = true;
                        }
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
                case BLE_GAP_EVT_CONNECTED:
                    return true;
                case BLE_GAP_EVT_DISCONNECTED: {
                    // Use scratchpad defaults when advertising
                    get_logger()->debug("{} Starting advertising.", p->role());
                    const auto err_code = p->startAdvertising();
                    if (err_code != NRF_SUCCESS)
                    {
                        get_logger()->error("{} start advertising failed, error is {:x}", c->role(),
                                            err_code);

                        error = true;
                    }
                }
                    return true;
                case BLE_GAP_EVT_TIMEOUT:
                    return true;
                case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
                    if (gapEvent->params.phy_update_request.peer_preferred_phys != requestedPhys)
                    {
                        get_logger()->error("{} BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST: update "
                                            "request is not equal to the one sent from central",
                                            p->role());
                        error = true;
                    }
                    else
                    {
                        const auto err_code = sd_ble_gap_phy_update(
                            p->unwrap(), gapEvent->conn_handle,
                            &(gapEvent->params.phy_update_request.peer_preferred_phys));

                        if (err_code != NRF_SUCCESS)
                        {
                            get_logger()->error("{} BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST: error "
                                                "calling sd_ble_gap_phy_update, {}",
                                                p->role(), testutil::errorToString(err_code));

                            error = true;
                        }
                    }
                }
                    return true;
                case BLE_GAP_EVT_PHY_UPDATE: {
                    if (gapEvent->params.phy_update.rx_phy != requestedPhys.rx_phys ||
                        gapEvent->params.phy_update.tx_phy != requestedPhys.tx_phys)
                    {
                        get_logger()->error(
                            "{} BLE_GAP_EVT_PHY_UPDATE: phy is not updated according to request",
                            p->role());
                        error = true;
                    }
                    else
                    {
                        const auto status = gapEvent->params.phy_update.status;

                        if (status != BLE_HCI_STATUS_CODE_SUCCESS)
                        {
                            get_logger()->error("{} BLE_GAP_EVT_PHY_UPDATE: status is {:x}, should "
                                                "be BLE_HCI_STATUS_CODE_SUCCESS({})",
                                                p->role(), status, BLE_HCI_STATUS_CODE_SUCCESS);
                            error = true;
                        }
                        else
                        {
                            testComplete = true;
                        }
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

                error = true;
            }
        });

        p->setStatusCallback([&](const sd_rpc_app_status_t code, const std::string &message) {
            if (code == PKT_DECODE_ERROR || code == PKT_SEND_MAX_RETRIES_REACHED ||
                code == PKT_UNEXPECTED)
            {
                get_logger()->error("{} error in status callback {:x}:{}", p->role(),
                                    static_cast<uint32_t>(code), message);

                error = true;
            }
        });

        // Open the adapters
        REQUIRE(c->open() == NRF_SUCCESS);
        REQUIRE(p->open() == NRF_SUCCESS);

        REQUIRE(c->configure() == NRF_SUCCESS);
        REQUIRE(p->configure() == NRF_SUCCESS);

        REQUIRE(p->setupAdvertising(advResponse, // advertising data
                                    {},          // scan response data
                                    40,          // interval
                                    0,           // duration
                                    true         // connectable
                                    ) == NRF_SUCCESS);
        REQUIRE(p->startAdvertising() == NRF_SUCCESS);

        REQUIRE(c->startScan() == NRF_SUCCESS);

        // Wait for the test to complete
        std::this_thread::sleep_for(std::chrono::seconds(5));

        CHECK(error == false);
        CHECK(testComplete == true);

        CHECK(c->close() == NRF_SUCCESS);
        sd_rpc_adapter_delete(c->unwrap());

        CHECK(p->close() == NRF_SUCCESS);
        sd_rpc_adapter_delete(p->unwrap());
    }
}
#endif // NRF_SD_BLE_API >= 6