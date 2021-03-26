/** @file
 *
 * @brief Implementation of a custom sink for the spdlog framework.
 *
 */

#pragma once

#include "event_handler.h"
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
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        auto level_found                = LogHelper::levelToAPI(msg.level);
        sd_rpc_log_severity_t api_level = SD_RPC_LOG_INFO;

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

    auto findEventHandler(std::weak_ptr<EventHandler> ehandler)
        -> std::vector<std::weak_ptr<EventHandler>>::const_iterator
    {
        return std::find_if(event_handlers.begin(), event_handlers.end(),
                            [&](auto ptr) { return ptr.lock() == ehandler.lock(); });
    }

    void addEventHandler(std::weak_ptr<EventHandler> ehandler)
    {
        std::scoped_lock{event_handler_mutex};
        const auto contains = findEventHandler(ehandler);
        assert(contains == event_handlers.end());
        event_handlers.emplace_back(std::move(ehandler));
    }

    void removeEventHandler(std::weak_ptr<EventHandler> ehandler)
    {
        std::scoped_lock{event_handler_mutex};
        const auto contains = findEventHandler(ehandler);
        assert(contains != event_handlers.end());
        event_handlers.erase(contains);
    }

    void flush_() override
    {}
};
