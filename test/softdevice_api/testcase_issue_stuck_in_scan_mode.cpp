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

#include <test_setup.h>
#include <test_util.h>

#include <ble.h>
#include <sd_rpc.h>

#include <sstream>
#include <thread>

TEST_CASE(CREATE_TEST_NAME_AND_TAGS(issue_stuck_in_scan_mode,
                                    [issue][PCA10028][PCA10031][PCA10040][PCA10056][PCA10059]))
{
    // Indicates if an error has occurred in a callback.
    // The test framework is not thread safe so this variable is used to communicate that an issues
    // has occurred in a callback.
    auto error = false;

    std::chrono::steady_clock::time_point adv_report_received;

    auto env = ::test::getEnvironment();
    INFO(::test::getEnvironmentAsText(env));
    REQUIRE(!env.serialPorts.empty());
    const auto serialPort = env.serialPorts.at(0);
    INFO("Serial port used: " << serialPort.port);

    const auto scanIterations = env.numberOfIterations;
    INFO("Purpose of this test:");
    INFO("1) Assert that closing the adapter when scan is running does not prevent opening the "
         "adapter again.");
    INFO("2) Assert that advertisement reports are received all the time (timeout is set to "
         "0x00)");
    INFO("Running " << scanIterations << " adapter open -> scan -> adapter close iterations");

    for (auto i = static_cast<uint32_t>(0); i < scanIterations; i++)
    {
        auto adv_report_count = 0;

        auto c = std::make_shared<testutil::AdapterWrapper>(testutil::Central, serialPort.port,
                                                            env.baudRate, env.mtu, env.retransmissionInterval,
                                                            env.responseTimeout);

        NRF_LOG(c->role() << " Starting scan iteration #" << std::dec << static_cast<uint32_t>(i)
                          << " of " << static_cast<uint32_t>(scanIterations));

        REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);

        // Scan forever
        c->scratchpad.scan_param.timeout = 0;

        c->setGapEventCallback([&](const uint16_t eventId, const ble_gap_evt_t *gapEvent) -> bool {
            switch (eventId)
            {
                case BLE_GAP_EVT_ADV_REPORT:
                    adv_report_count++;
                    NRF_LOG("adv_report_count: " << adv_report_count);
#if NRF_SD_BLE_API == 6
                    {
                        const auto err_code = c->startScan(true);
                        if (err_code != NRF_SUCCESS)
                        {
                            NRF_LOG(c->role()
                                    << " startScan in BLE_GAP_EVT_ADV_REPORT failed, err_code "
                                    << err_code);
                            error = true;
                        }
                    }
#endif
                    adv_report_received = std::chrono::steady_clock::now();

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
                default:
                    return false;
            }
        });

        REQUIRE(c->open() == NRF_SUCCESS);
        REQUIRE(c->configure() == NRF_SUCCESS);
        REQUIRE(c->startScan() == NRF_SUCCESS);

        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Check that we have recently received an advertisement report
        auto now = std::chrono::steady_clock::now();
        auto silence_duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - adv_report_received)
                .count();
        CHECK(silence_duration < 1000);

        CHECK(error == false);

        // Cleanup current adapter connection
        CHECK(c->close() == NRF_SUCCESS);
        sd_rpc_adapter_delete(c->unwrap());

        NRF_LOG(c->role() << " Scan iteration #" << std::dec << static_cast<uint32_t>(i) << " of "
                          << static_cast<uint32_t>(scanIterations) << " completed");
    }
}
