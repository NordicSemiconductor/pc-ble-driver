/*
 * Copyright (c) 2016 Nordic Semiconductor ASA
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

#include "transport.h"

#include "nrf_error.h"

#include <cstdint>
#include <iostream>
#include <sstream>

using namespace std;

Transport::Transport()           = default;
Transport::~Transport() noexcept = default;

uint32_t Transport::open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                         const log_cb_t &log_callback) noexcept
{
    if (!status_callback || !data_callback || !log_callback)
    {
        return NRF_ERROR_SD_RPC_INVALID_ARGUMENT;
    }

    upperStatusCallback = status_callback;
    upperDataCallback   = data_callback;
    upperLogCallback    = log_callback;

    return NRF_SUCCESS;
}

void Transport::log(const sd_rpc_log_severity_t severity, const std::string &message) const noexcept
{
    if (upperLogCallback)
    {
        try
        {
            upperLogCallback(severity, message);
        }
        catch (const std::exception &ex)
        {
            try
            {
                std::cerr << "Exception thrown in log callback, " << ex.what() << '\n';
            }
            catch (const std::exception &)
            {
                std::cerr << "Fatal error creating log callback string\n";
            }
        }
    }
    else
    {
        std::cerr << "LOG(" << static_cast<uint32_t>(severity) << "): " << message << std::endl;
    }
}

void Transport::log(const sd_rpc_log_severity_t severity, const std::string &message,
                    const std::exception &ex) const noexcept
{
    try
    {
        std::stringstream message_with_exception;
        message_with_exception << message << ", " << ex.what();
        log(severity, message_with_exception.str());
    }
    catch (const std::exception &)
    {
        std::cerr << "Fatal error creating log callback string" << std::endl;
    }
}

void Transport::status(const sd_rpc_app_status_t code, const std::string &message) const noexcept
{
    if (upperLogCallback)
    {
        try
        {
            upperStatusCallback(code, message);
        }
        catch (const std::exception &ex)
        {
            try
            {
                std::cerr << "Exception thrown in status callback, " << ex.what() << '\n';
            }
            catch (const std::exception &)
            {
                std::cerr << "Fatal error creating status callback string" << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "status(" << static_cast<uint32_t>(code) << ") " << message << std::endl;
    }
}

void Transport::status(const sd_rpc_app_status_t code, const std::string &message,
                       const std::exception &ex) const noexcept
{
    try
    {
        std::stringstream status_with_exception;
        status_with_exception << message << ", " << ex.what();
        status(code, status_with_exception.str());
    }
    catch (const std::exception &)
    {
        std::cerr << "Fatal error creating status callback string" << std::endl;
    }
}
