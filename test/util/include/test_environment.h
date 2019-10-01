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

#pragma once

#include "test_util.h"

#include <condition_variable>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#define STR(s) #s
#define EXPAND(s) STR(s)
#define CREATE_TEST_NAME_AND_TAGS(name, tags)                                                      \
#name "_sdv" EXPAND(NRF_SD_BLE_API), #tags "[sdv" EXPAND(NRF_SD_BLE_API) "]"

constexpr uint32_t defaultRetransmissionInterval()
{
#if NRF_SD_BLE_API == 2
    return 300; // nRF51 devices use SDv2. They are a bit slower than the nRF52 series devices.
#else
    return 250;
#endif
}

constexpr uint32_t defaultBaudRate =
    1000000; /**< The baud rate to be used for serial communication with nRF5 device. */

#define STR(s) #s
#define EXPAND(s) STR(s)
#define CREATE_TEST_NAME_AND_TAGS(name, tags)                                                      \
#name "_sdv" EXPAND(NRF_SD_BLE_API), #tags "[sdv" EXPAND(NRF_SD_BLE_API) "]"

namespace test {
struct SerialPort
{
    SerialPort(std::string port, uint32_t baudRate);
    std::string port;
    uint32_t baudRate;
};

struct Environment
{
    std::vector<SerialPort> serialPorts{};
    uint32_t numberOfIterations{10};
    sd_rpc_log_severity_t driverLogLevel{SD_RPC_LOG_INFO};
    bool driverLogLevelSet{false};
    uint32_t baudRate{defaultBaudRate};
    uint32_t retransmissionInterval{defaultRetransmissionInterval()};
    uint32_t responseTimeout{1500};
    uint16_t mtu{150};
    std::string hardwareInfo{};
};

extern Environment ConfiguredEnvironment;

Environment getEnvironment();
std::string getEnvironmentAsText(const Environment &env);
} // namespace test
