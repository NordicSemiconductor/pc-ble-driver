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
#define NRF_LOG_SETUP
#include <internal/log.h>

#include <nrf_error.h>
#include <sd_rpc.h>

#include <test_environment.h>

#if defined(_MSC_VER)
// Disable warning "This function or variable may be unsafe. Consider using _dupenv_s instead."
#pragma warning(disable : 4996)
#endif

TEST_CASE("SerialPortEnumeration")
{
    SECTION("Array not large enough for devices found")
    {
        std::vector<sd_rpc_serial_port_desc_t> devices(0);
        auto size = static_cast<uint32_t>(devices.capacity());
        REQUIRE(sd_rpc_serial_port_enum(devices.data(), &size) == NRF_ERROR_DATA_SIZE);
    }

    SECTION("Array large enough for devices found")
    {
        std::vector<sd_rpc_serial_port_desc_t> devices(100);
        auto size = static_cast<uint32_t>(devices.capacity());
        REQUIRE(sd_rpc_serial_port_enum(devices.data(), &size) == NRF_SUCCESS);
        REQUIRE(size > 0);
        NRF_LOG("Found " << size << " devices.");
    }
}
