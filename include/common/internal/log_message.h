/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
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
/** @file
 *
 * @brief Implementation of a log message in C++. Wraps around a C representation of a log message.
 *
 */

#pragma once

#include "sd_rpc_types.h"
#include <string>
#include <string_helper.h>

/**@brief Log message.
 *
 * This class keeps all the information of a log message. The underlying
 * implementation has a raw C structure of the message. This C++ class
 * manages all the memory related to the C structure and makes sure it is
 * consistent when manipulating the C++ object.
 */
class LogMessage
{
  private:
    sd_rpc_log_t log_raw{};
    std::string message;

  public:
    LogMessage(const sd_rpc_log_severity_t severity, std::string msg)
        : message(std::move(msg))
    {
        log_raw.severity = severity;
        log_raw.message  = message.c_str();
    }

    explicit LogMessage(const sd_rpc_log_t *other_raw)
        : LogMessage(SD_RPC_LOG_FATAL, "")
    {
        if (other_raw == nullptr)
        {
            return;
        }
        log_raw         = *other_raw;
        message         = StringHelper::toOptional(other_raw->message).value_or("");
        log_raw.message = message.c_str();
    }

    LogMessage(const LogMessage &other)
    {
        *this = other;
    }

    LogMessage(LogMessage &&other) noexcept
    {
        *this = other;
    }

    auto operator=(const LogMessage &other) -> LogMessage &
    {
        if (this != &other)
        {
            log_raw         = other.log_raw;
            message         = other.message;
            log_raw.message = message.c_str();
        }
        return *this;
    }

    auto operator=(LogMessage &&other) noexcept -> LogMessage &
    {
        *this = other;
        return *this;
    }

    ~LogMessage() = default;

    /**@brief Get the raw data.
     *
     * This will return a pointer to the underlying raw data represented as a C-struct.
     * The raw data is managed and kept consistent by this class.
     *
     * @returns  The raw data as a pointer to a C-struct.
     */
    auto data() const noexcept -> const sd_rpc_log_t *
    {
        return &log_raw;
    }
};
