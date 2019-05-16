/*
 * Copyright (c) 2018-2019 Nordic Semiconductor ASA
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
#include <logging.h>

#include <test_environment.h>
#include <test_util.h>

#include <ble.h>
#include <sd_rpc.h>

#include <sstream>
#include <thread>

TEST_CASE(CREATE_TEST_NAME_AND_TAGS(driver_open_close,
                                    [rpc][PCA10028][PCA10031][PCA10040][PCA10056][PCA10059]))
{
    // Indicates if an error has occurred in a callback.
    // The test framework is not thread safe so this variable is used to communicate that an issues
    // has occurred in a callback.

    auto error = false;

    auto env = ::test::getEnvironment();
    INFO(::test::getEnvironmentAsText(env));
    REQUIRE(!env.serialPorts.empty());
    const auto serialPort         = env.serialPorts.at(0);
    const auto numberOfIterations = env.numberOfIterations;

    SECTION("open_already_opened_adapter")
    {
        auto c = std::make_unique<testutil::AdapterWrapper>(
            testutil::Role::Central, serialPort.port, env.baudRate, env.mtu,
            env.retransmissionInterval, env.responseTimeout);

        REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);

        CHECK(c->open() == NRF_SUCCESS);
        CHECK(c->open() == NRF_ERROR_INVALID_STATE);
        CHECK(c->close() == NRF_SUCCESS);
    }

    SECTION("close_already_closed_adapter")
    {
        auto c = std::make_unique<testutil::AdapterWrapper>(
            testutil::Role::Central, serialPort.port, env.baudRate, env.mtu,
            env.retransmissionInterval, env.responseTimeout);

        c->setStatusCallback([&](const sd_rpc_app_status_t code, const std::string &message) {
            if (code == PKT_DECODE_ERROR || code == PKT_SEND_MAX_RETRIES_REACHED ||
                code == PKT_UNEXPECTED)
            {
                get_logger()->error("{} error in status callback {:x}:{}", c->role(),
                                    static_cast<uint32_t>(code), message);
                error = true;
            }
        });

        REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);

        CHECK(c->close() == NRF_ERROR_INVALID_STATE);
        CHECK(c->open() == NRF_SUCCESS);
        CHECK(c->close() == NRF_SUCCESS);
        CHECK(c->close() == NRF_ERROR_INVALID_STATE);
    }

    SECTION("open_close_open_with_teardown_iterations")
    {
        for (uint32_t i = 0; i < numberOfIterations; i++)
        {
            get_logger()->info("Starting iteration #{} of {}", static_cast<uint32_t>(i + 1),
                               numberOfIterations);

            auto c = std::make_shared<testutil::AdapterWrapper>(
                testutil::Role::Central, serialPort.port, env.baudRate, env.mtu,
                env.retransmissionInterval, env.responseTimeout);

            REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) ==
                    NRF_SUCCESS);

            c->setStatusCallback([&](const sd_rpc_app_status_t code, const std::string &message) {
                if (code == PKT_DECODE_ERROR || code == PKT_SEND_MAX_RETRIES_REACHED ||
                    code == PKT_UNEXPECTED)
                {
                    get_logger()->debug("{} error in status callback: {:x}:{}", c->role(),
                                        static_cast<uint32_t>(code), message);
                }
            });

            c->setGapEventCallback(
                [&](const uint16_t eventId, const ble_gap_evt_t *gapEvent) -> bool {
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
                                    get_logger()->error("{} Scan start error, err_code: {:x}",
                                                        c->role(), err_code);
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

            CHECK(error == false);

            CHECK(c->close() == NRF_SUCCESS);
            sd_rpc_adapter_delete(c->unwrap());

            get_logger()->info("Iteration #{} of {} complete.", static_cast<uint32_t>(i + 1),
                               numberOfIterations);
        }
    }

    SECTION("open_close_open_iterations")
    {
        auto c = std::make_shared<testutil::AdapterWrapper>(
            testutil::Role::Central, serialPort.port, env.baudRate, env.mtu,
            env.retransmissionInterval, env.responseTimeout);

        REQUIRE(sd_rpc_log_handler_severity_filter_set(c->unwrap(), env.driverLogLevel) ==
                NRF_SUCCESS);

        c->setStatusCallback([&](const sd_rpc_app_status_t code, const std::string &message) {
            if (code == PKT_DECODE_ERROR || code == PKT_SEND_MAX_RETRIES_REACHED ||
                code == PKT_UNEXPECTED)
            {
                get_logger()->error("{} error in status callback {:x}:{}", c->role(),
                                    static_cast<uint32_t>(code), message);
            }
        });

        for (uint32_t i = 0; i < numberOfIterations; i++)
        {
            get_logger()->info("Starting iteration #{} of {}", static_cast<uint32_t>(i + 1),
                               numberOfIterations);

            REQUIRE(c->open() == NRF_SUCCESS);
            REQUIRE(c->configure() == NRF_SUCCESS);
            CHECK(c->close() == NRF_SUCCESS);

            get_logger()->info("Iteration #{} of {} complete.", static_cast<uint32_t>(i + 1),
                               numberOfIterations);
        }

        sd_rpc_adapter_delete(c->unwrap());
    }
}
