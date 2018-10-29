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

TEST_CASE("test_pc_ble_driver_open_close")
{
    // Indicates if an error has occurred in a callback.
    // The test framework is not thread safe so this variable is used to communicate that an issues
    // has occurred in a callback.
    auto error = false;

    auto env = ::test::getEnvironment();
    REQUIRE(!env.serialPorts.empty());
    const auto serialPort         = env.serialPorts.at(0);
    const auto numberOfIterations = env.numberOfIterations;

    SECTION("open_already_opened_adapter")
    {
        const auto baudRate = serialPort.baudRate;

        INFO("Serial port used: " << serialPort.port);
        INFO("Baud rate used: " << baudRate);

        auto c = std::make_unique<testutil::AdapterWrapper>(testutil::Central, serialPort.port, baudRate);

        REQUIRE(c->open() == NRF_SUCCESS);
        REQUIRE(c->open() == NRF_ERROR_INVALID_STATE);
        REQUIRE(c->close() == NRF_SUCCESS);
    }

    SECTION("close_already_closed_adapter")
    {
        const auto baudRate = serialPort.baudRate;

        INFO("Serial port used: " << serialPort.port);
        INFO("Baud rate used: " << baudRate);

        auto c = std::make_unique<testutil::AdapterWrapper>(testutil::Central, serialPort.port, baudRate);

        REQUIRE(c->close() == NRF_ERROR_INVALID_STATE);
        REQUIRE(c->open() == NRF_SUCCESS);
        REQUIRE(c->close()  == NRF_SUCCESS);
        REQUIRE(c->close() == NRF_ERROR_INVALID_STATE);
    }

    SECTION("open_close_open_iterations")
    {
        const auto baudRate = serialPort.baudRate;

        INFO("Serial port used: " << serialPort.port);
        INFO("Baud rate used: " << baudRate);

        for (uint32_t i = 0; i < numberOfIterations; i++)
        {
            auto c = std::make_shared<testutil::AdapterWrapper>(testutil::Central, serialPort.port,
                                                                baudRate);

            REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) ==
                    NRF_SUCCESS);

            c->setGapEventCallback([&](const uint16_t eventId,
                                        const ble_gap_evt_t *gapEvent) -> bool {
                switch (eventId)
                {
                    case BLE_GAP_EVT_ADV_REPORT:
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

            std::this_thread::sleep_for(std::chrono::seconds(2));

            REQUIRE(error == false);

            REQUIRE(c->close() == NRF_SUCCESS);
            sd_rpc_adapter_delete(c->unwrap());

            NRF_LOG("Iteration #" << (i + 1) << " of " << numberOfIterations << " complete.");
        }
    }
}
