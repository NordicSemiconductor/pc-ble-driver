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

TEST_CASE(CREATE_TEST_NAME_AND_TAGS(
    advertising, [gap][known_error][PCA10028][PCA10031][PCA10040][PCA10056][PCA10059]))
{
    using namespace testutil;

    auto env = ::test::getEnvironment();
    INFO(::test::getEnvironmentAsText(env));

    REQUIRE(env.serialPorts.size() >= 2);
    const auto central    = env.serialPorts.at(0);
    const auto peripheral = env.serialPorts.at(1);

    // Indicates if an error has occurred in a callback.
    // The test framework is not thread safe so this variable is used to communicate that an issues
    // has occurred in a callback.
    auto error = false;

#if NRF_SD_BLE_API == 6
    SECTION("extended")
    {
        const auto baudRate = central.baudRate;

        const auto maxLengthOfAdvData        = testutil::ADV_DATA_BUFFER_SIZE;
        const auto maxNumberOfAdvertisements = 20;
        const auto advertisementNameLength   = 40;
        const auto advertisementSetId        = 5;

        auto scanRequestCountOtherCentral = 0;
        auto scanRequestCountThisCentral  = 0;

        std::vector<uint8_t> peripheralAdvNameBuffer(advertisementNameLength);
        testutil::appendRandomAlphaNumeric(peripheralAdvNameBuffer, advertisementNameLength);
        const std::string peripheralAdvName(peripheralAdvNameBuffer.begin(),
                                            peripheralAdvNameBuffer.end());

        // Create advertising data and scan response data
        std::vector<uint8_t> advertising;
        std::vector<uint8_t> scanResponse;

        testutil::appendAdvertisingName(scanResponse, peripheralAdvName);

        // Append max number of bytes in advertisement packet with manufacturer specific random data
        // after the peripheral name
        const auto remainingSpace =
            maxLengthOfAdvData - scanResponse.size() - 2 /* length and AD type */;
        std::vector<uint8_t> randomData;

        testutil::appendRandomData(randomData, remainingSpace);

        scanResponse.reserve(maxLengthOfAdvData);
        testutil::appendManufacturerSpecificData(scanResponse, randomData, true);

        // Instantiate an adapter to use as BLE Central in the test
        auto c = std::make_shared<testutil::AdapterWrapper>(testutil::Central, central.port,
                                                            baudRate, env.retransmissionInterval,
                                                            env.responseTimeout);

        // Instantiated an adapter to use as BLE Peripheral in the test
        auto p = std::make_shared<testutil::AdapterWrapper>(testutil::Peripheral, peripheral.port,
                                                            baudRate, env.retransmissionInterval,
                                                            env.responseTimeout);

        REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);
        REQUIRE(sd_rpc_log_handler_severity_filter_set(p->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);

        c->setGapEventCallback([&](const uint16_t eventId, const ble_gap_evt_t *gapEvent) -> bool {
            switch (eventId)
            {
                case BLE_GAP_EVT_ADV_REPORT:
                {
                    const auto advReport = gapEvent->params.adv_report;

                    if (testutil::findAdvName(advReport, peripheralAdvName))
                    {
                        std::vector<uint8_t> manufacturerSpecificData;

                        if (testutil::findManufacturerSpecificData(advReport,
                                                                   manufacturerSpecificData))
                        {
                            // Check that the received data is according to setupAdvertisement
                            if (manufacturerSpecificData != randomData)
                            {
                                NRF_LOG(c->role() << " Data configured in peripheral does not "
                                                     "match data received on central");
                                error = true;
                            }
                            else
                            {
                                // TODO: check previous state
                                if (!(advReport.primary_phy == BLE_GAP_PHY_1MBPS &&
                                      advReport.secondary_phy == BLE_GAP_PHY_2MBPS &&
                                      advReport.type.extended_pdu == 1 &&
                                      advReport.type.scan_response == 1 &&
                                      advReport.type.connectable == 0 &&
                                      advReport.set_id == advertisementSetId))
                                {
                                    NRF_LOG(c->role()
                                            << " Configured advertisement on peripheral does not "
                                               "match event received on central");
                                    error = true;
                                }
                            }
                        }
                    }

                    if (!error)
                    {
                        c->startScan(true);
                    }
                }
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

        p->setGapEventCallback([&](const uint16_t eventId, const ble_gap_evt_t *gapEvent) {
            switch (eventId)
            {
                case BLE_GAP_EVT_TIMEOUT:
                    return false;
                case BLE_GAP_EVT_SCAN_REQ_REPORT:
                {
                    const auto scanRequestReport = gapEvent->params.scan_req_report;
                    if (scanRequestReport.adv_handle != p->scratchpad.adv_handle)
                    {
                        NRF_LOG(p->role() << " BLE_GAP_EVT_SCAN_REQ_REPORT:  Received "
                                             "advertisement handle does not match the "
                                             "one setup with sd_ble_gap_adv_set_configure.");
                        error = true;
                    }
                    else
                    {
                        if (scanRequestReport.peer_addr == c->address)
                        {
                            scanRequestCountThisCentral += 1;
                        }
                        else
                        {
                            scanRequestCountOtherCentral += 1;
                        }

                        NRF_LOG(p->role()
                                << " SCAN_REQ_REPORT count, this: " << scanRequestCountThisCentral
                                << ", other: " << scanRequestCountOtherCentral);
                    }
                }
                    return true;
                case BLE_GAP_EVT_ADV_SET_TERMINATED:
                {
                    const auto setTerminated = gapEvent->params.adv_set_terminated;

                    if (setTerminated.adv_handle != p->scratchpad.adv_handle)
                    {
                        NRF_LOG(p->role() << " BLE_GAP_EVT_ADV_SET_TERMINATED: Received "
                                             "advertisement handle does not match the "
                                             "one setup with sd_ble_gap_adv_set_configure.");
                        error = true;
                    }

                    if (setTerminated.reason != BLE_GAP_EVT_ADV_SET_TERMINATED_REASON_LIMIT_REACHED)
                    {
                        NRF_LOG(p->role()
                                << " BLE_GAP_EVT_ADV_SET_TERMINATED: Limit reason was not "
                                   "LIMIT_REACHED which it should be.");
                        error = true;
                    }

                    if (setTerminated.num_completed_adv_events != maxNumberOfAdvertisements)
                    {
                        NRF_LOG(p->role()
                                << " BLE_GAP_EVT_ADV_SET_TERMINATED: Number of completed "
                                   "advertisement events does not match max_adv_evts set in "
                                   "sd_ble_gap_adv_set_configure.");
                        error = true;
                    }

                    std::vector<uint8_t> manufacturerSpecificData;
                    const auto advReport = setTerminated.adv_data;

                    std::vector<uint8_t> advData;
                    const auto data       = advReport.adv_data.p_data;
                    const auto dataLength = advReport.adv_data.len;
                    advData.assign(data, data + dataLength);

                    if (scanResponse != advData)
                    {
                        NRF_LOG(p->role() << " BLE_GAP_EVT_ADV_SET_TERMINATED: Advertisement "
                                             "buffers set in sd_ble_gap_adv_set_configure does "
                                             "not match with advertisement received.");
                        error = true;
                    }

                    return true;
                }
                default:
                    return false;
            }
        });

        c->setStatusCallback([&](const sd_rpc_app_status_t code, const std::string &message) {
            if (code == PKT_DECODE_ERROR || code == PKT_SEND_MAX_RETRIES_REACHED ||
                code == PKT_UNEXPECTED)
            {
                error = true;
            }
        });

        p->setStatusCallback([&](const sd_rpc_app_status_t code, const std::string &message) {
            if (code == PKT_DECODE_ERROR || code == PKT_SEND_MAX_RETRIES_REACHED ||
                code == PKT_UNEXPECTED)
            {
                error = true;
            }
        });

        // Open the adapters
        REQUIRE(c->open() == NRF_SUCCESS);
        REQUIRE(p->open() == NRF_SUCCESS);

        REQUIRE(c->configure() == NRF_SUCCESS);
        REQUIRE(p->configure() == NRF_SUCCESS);

        REQUIRE(p->setupAdvertising({},                       // advertising data
                                    scanResponse,             // scan response data
                                    40,                       // interval
                                    0,                        // duration
                                    false,                    // connectable
                                    true,                     // extended
                                    true,                     // scan_req_notification
                                    advertisementSetId,       // set_id
                                    BLE_GAP_PHY_1MBPS,        // primary phy
                                    BLE_GAP_PHY_2MBPS,        // secondary phy
                                    0,                        // filter policy
                                    maxNumberOfAdvertisements // max_adv_events
                                    ) == NRF_SUCCESS);
        REQUIRE(p->startAdvertising() == NRF_SUCCESS);

        REQUIRE(c->startScan(false, true, false) == NRF_SUCCESS);

        // Wait for the test to complete
        std::this_thread::sleep_for(std::chrono::seconds(3));

        REQUIRE(error == false);

        REQUIRE(c->close() == NRF_SUCCESS);
        sd_rpc_adapter_delete(c->unwrap());

        REQUIRE(p->close() == NRF_SUCCESS);
        sd_rpc_adapter_delete(p->unwrap());
    }
#endif // NRF_SD_BLE_API == 6
}
