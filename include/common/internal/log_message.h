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
