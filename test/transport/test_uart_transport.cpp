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

// Logging support
#define NRF_LOG_SETUP
#include "internal/log.h"

// Test framework
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <test_setup.h>
#include <uart_transport.h>

#include <chrono>
#include <cstdlib>
#include <iterator>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#if defined(_MSC_VER)
// Disable warning "This function or variable may be unsafe. Consider using _dupenv_s instead."
#pragma warning(disable : 4996)
#endif

using std::chrono::system_clock;

struct PortStats
{
    uint32_t pktCount;
    size_t pktMinSize;
    size_t pktMaxSize;

    PortStats()
        : pktCount(0)
        , pktMinSize(std::numeric_limits<uint32_t>::max())
        , pktMaxSize(0)
    {}
};

std::ostream &operator<<(std::ostream &out, const PortStats &stats)
{
    out << "pkt_count: " << stats.pktCount << " pkt_max_size: " << stats.pktMaxSize
        << " pkt_min_size: ";

    if (stats.pktMinSize == std::numeric_limits<uint32_t>::max())
    {
        out << "-";
    }
    else
    {
        out << stats.pktMinSize;
    }

    return out;
};

TEST_CASE("open_close")
{
    auto env = ::test::getEnvironment();

    INFO("Two serial ports must be specifided. Please specify environment variables "
         "BLE_DRIVER_TEST_SERIAL_PORT_A and BLE_DRIVER_TEST_SERIAL_PORT_B");
    REQUIRE(env.serialPorts.size() == 2);

    auto envPortA = env.serialPorts.at(0);
    auto portA    = envPortA.port.c_str();

    UartCommunicationParameters ap;
    ap.baudRate    = envPortA.baudRate;
    ap.dataBits    = UartDataBitsEight;
    ap.flowControl = UartFlowControlNone;
    ap.parity      = UartParityNone;
    ap.portName    = portA;
    ap.stopBits    = UartStopBitsOne;
    auto a         = new UartTransport(ap);

    auto envPortB = env.serialPorts.at(1);
    auto portB    = envPortB.port.c_str();

    UartCommunicationParameters bp;
    bp.baudRate    = envPortB.baudRate;
    bp.dataBits    = UartDataBitsEight;
    bp.flowControl = UartFlowControlNone;
    bp.parity      = UartParityNone;
    bp.portName    = portB;
    bp.stopBits    = UartStopBitsOne;
    auto b         = new UartTransport(bp);

    bool error = false;

    std::vector<uint8_t> receivedOnA;
    std::vector<uint8_t> receivedOnB;

    // Generate random values to send
    std::vector<uint8_t> sendOnB(10000);
    std::vector<uint8_t> sendOnA(10000);

    std::generate(sendOnB.begin(), sendOnB.end(), std::rand);
    std::generate(sendOnA.begin(), sendOnA.end(), std::rand);

    const auto status_callback = [&error](const sd_rpc_app_status_t code,
                                          const std::string &message) -> void {
        NRF_LOG("code: " << code << " message: " << message);
        if (code != NRF_SUCCESS)
        {
            error = true;
            NRF_LOG("ERROR: code is " << std::to_string(code) << " must be "
                                      << std::to_string(NRF_SUCCESS));
        }
    };

    auto log_callback = [&portA, &portB, &error](sd_rpc_log_severity_t severity,
                                                 std::string message) -> void {
        NRF_LOG("severity: " << severity << " message: " << message);

        if (severity == 1)
        {
            std::regex port_regex(".*read on port (\\S+).*");
            std::smatch matches;
            std::regex_search(message, matches, port_regex);

            for (auto match : matches)
            {
                NRF_LOG(match.str());
            }

            if (matches.size() != 2)
            {
                NRF_LOG("ERROR: match size must be 2");
                error = true;
                return;
            }

            const auto port_ = matches[1].str();
            if (!(port_ == portA || port_ == portB))
            {
                NRF_LOG("ERROR: ports must match either " << portA << " or " << portB);
                error = true;
            }
        }
    };

    PortStats portAStats;
    PortStats portBStats;

    b->open(status_callback,
            [&receivedOnB, &portBStats](const uint8_t *data, const size_t length) -> void {
                receivedOnB.insert(receivedOnB.end(), data, data + length);

                if (portBStats.pktMaxSize < length)
                {
                    portBStats.pktMaxSize = length;
                }

                if (portBStats.pktMinSize > length)
                {
                    portBStats.pktMinSize = length;
                }

                portBStats.pktCount++;
            },
            log_callback);

    a->open(status_callback,
            [&receivedOnA, &portAStats](const uint8_t *data, const size_t length) -> void {
                receivedOnA.insert(receivedOnA.end(), data, data + length);

                if (portAStats.pktMaxSize < length)
                {
                    portAStats.pktMaxSize = length;
                }

                if (portAStats.pktMinSize > length)
                {
                    portAStats.pktMinSize = length;
                }

                portAStats.pktCount++;
            },
            log_callback);

    b->send(sendOnB);
    a->send(sendOnA);

    // Let the data be sent between the serial ports before closing
    std::this_thread::sleep_until(system_clock::now() + std::chrono::seconds(1));

    REQUIRE(error == false);

    b->close();
    a->close();

    delete b;
    delete a;

    REQUIRE(sendOnA == receivedOnB);
    REQUIRE(sendOnB == receivedOnA);

    INFO("stats port A " << portAStats);
    INFO("stats port B " << portBStats);
}
