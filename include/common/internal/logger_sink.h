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
 * @brief Implementation of a custom sink for the spdlog framework.
 *
 */

#pragma once

#include "log_helper.h"
#include "log_message.h"
#include "sd_rpc_types.h"
#include <algorithm>
#include <fmt/format.h>
#include <functional>
#include <memory>
#include <mutex>
#include <spdlog/common.h>

#if SPDLOG_VERSION < 10601L
#include <spdlog/details/pattern_formatter.h>
#else
#include <spdlog/pattern_formatter.h>
#endif

#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

/**@brief Nordic Semiconductor logger sink for spdlog.
 *
 * This is a custom logger sink for the spdlog framework. The sink makes it possible to bind log
 * messages to the library callback.
 */
template <typename Mutex> class LoggerSink : public spdlog::sinks::base_sink<Mutex>
{
  private:
    /* Pointer is weak since we do not want the sink to keep the event
     * handler alive and since an event handler can own a sink itself. */
    std::vector<std::weak_ptr<EventHandler>> event_handlers;
    std::mutex event_handler_mutex;

  protected:
    void sink_it_(const spdlog::details::log_msg & msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        auto level_found           = LogHelper::levelToAPI(msg.level);
        sd_rpc_log_severity_t api_level = SD_RPC_LOG_FATAL;

        if (level_found.first)
        {
            api_level = level_found.second;
        }

        std::string msg_str = fmt::to_string(formatted);
        std::scoped_lock{event_handler_mutex};
        for (auto event_handler : event_handlers)
        {
            if (auto sptr = event_handler.lock())
            {
                // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
                sptr->publishLogEvent(std::make_unique<LogMessage>(api_level, msg_str));
            }
        }
    }

    /* Need to override this function to remove end-of-line. */
    void set_pattern_(const std::string &pattern) override
    {
        auto formatter = std::make_unique<spdlog::pattern_formatter>(
            pattern, spdlog::pattern_time_type::local, "");
        this->set_formatter_(std::move(formatter));
    }

  public:
    explicit LoggerSink()
    {
        /* Set empty EOL string by default. */
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
        this->set_formatter(
            std::make_unique<spdlog::pattern_formatter>(spdlog::pattern_time_type::local, ""));
    }

    void flush_() override
    {}
};
