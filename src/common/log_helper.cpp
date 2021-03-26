#include "log_helper.h"
#include "sd_rpc_types.h"

#include <spdlog/common.h>
#include <spdlog/sinks/null_sink.h>

std::unordered_map<sd_rpc_log_severity_t, spdlog::level::level_enum>
    LogHelper::apilevel_map; // NOLINT
std::unordered_map<spdlog::level::level_enum, sd_rpc_log_severity_t>
    LogHelper::spdloglevel_map; // NOLINT

auto getLogger() noexcept -> std::shared_ptr<spdlog::logger>
{
    try
    {
        const std::string loggerName(static_cast<const char *>(&NRF_BLE_DRIVER_LOGGER_NAME[0]));

        if (nrfdl_logger == nullptr)
        {
            // Setup the logger, the sink responsible for API callbacks is registered later
            nrfdl_logger = spdlog::null_logger_mt(loggerName);
        }

        return spdlog::get(loggerName);
    }
    catch (...)
    {
        std::terminate();
    }
}

void LogHelper::initLogLevelMaps()
{
    if (!apilevel_map.empty())
    {
        return;
    }

    apilevel_map = {{SD_RPC_LOG_TRACE, spdlog::level::level_enum::trace},
                    {SD_RPC_LOG_DEBUG, spdlog::level::level_enum::debug},
                    {SD_RPC_LOG_INFO, spdlog::level::level_enum::info},
                    {SD_RPC_LOG_WARNING, spdlog::level::level_enum::warn},
                    {SD_RPC_LOG_ERROR, spdlog::level::level_enum::err},
                    {SD_RPC_LOG_FATAL, spdlog::level::level_enum::critical}};

    spdloglevel_map = {{spdlog::level::level_enum::trace, SD_RPC_LOG_TRACE},
                       {spdlog::level::level_enum::debug, SD_RPC_LOG_DEBUG},
                       {spdlog::level::level_enum::info, SD_RPC_LOG_INFO},
                       {spdlog::level::level_enum::warn, SD_RPC_LOG_WARNING},
                       {spdlog::level::level_enum::err, SD_RPC_LOG_ERROR},
                       {spdlog::level::level_enum::critical, SD_RPC_LOG_FATAL}};
}

auto LogHelper::levelToSpdlog(const sd_rpc_log_severity_t level)
    -> std::pair<bool, spdlog::level::level_enum>
{
    initLogLevelMaps();
    auto level_found = apilevel_map.find(level);

    if (level_found == apilevel_map.end())
    {
        return std::make_pair(false, spdlog::level::off);
    }

    return std::make_pair(true, level_found->second);
}

auto LogHelper::levelToAPI(spdlog::level::level_enum level)
    -> std::pair<bool, sd_rpc_log_severity_t>
{
    initLogLevelMaps();
    auto level_found = spdloglevel_map.find(level);

    if (level_found == spdloglevel_map.end())
    {
        return std::make_pair(false, SD_RPC_LOG_FATAL);
    }

    return std::make_pair(true, level_found->second);
}

void LogHelper::tryToLogException(const std::shared_ptr<spdlog::logger> &logger,
                                  const spdlog::level::level_enum &level,
                                  const std::exception &exc) noexcept
{
    try
    {
        if (logger)
        {
            logger->log(level, "Exception: {}", exc.what());
        }
    }
    catch (...)
    {
        /* Ignore errors. */
    }
}

void LogHelper::tryToLogException(const spdlog::level::level_enum &level, const std::exception &exc,
                                  const char *msg) noexcept
{
    try
    {
        auto logger = getLogger();
        logger->log(level, "{} Exception: {}", msg, exc.what());
    }
    catch (...)
    {
        /* Ignore errors. */
    }
}

void LogHelper::tryToLog(const std::shared_ptr<spdlog::logger> &logger,
                         const spdlog::level::level_enum &level, const std::string &msg) noexcept
{
    try
    {
        logger->log(level, msg);
    }
    catch (...)
    {
        /* Ignore errors. */
    }
}

void LogHelper::tryToLog(const spdlog::level::level_enum &level, const std::string &msg) noexcept
{
    try
    {
        auto logger = getLogger();
        logger->log(level, msg);
    }
    catch (...)
    {
        /* Ignore errors. */
    }
}
