/** @file
 *
 * @brief Represents an event handler for all events to the client of the library.
 *
 */

#pragma once

#include "log_message.h"
#include "sd_rpc_types.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <spdlog/spdlog.h>
#include <thread>

class Event
{
    std::unique_ptr<LogMessage> log_message_;
    uint32_t error_         = NRF_SUCCESS;
    bool exit_event_handler = false;

  public:
    void setLogMessage(std::unique_ptr<LogMessage> message) noexcept
    {
        log_message_ = std::move(message);
    }

    [[nodiscard]] auto hasLogMessage() const noexcept -> bool
    {
        return log_message_ != nullptr;
    }

    auto getLogMessage() noexcept -> std::unique_ptr<LogMessage>
    {
        return std::move(log_message_);
    }
};

/**@brief Library event handler.
 *
 * This takes care of registering event callbacks and publishing events from/to the user of the
 * library. The event handler has it's own thread and is meant to run as long as the library is
 * open.
 */
class EventHandler
{
  private:
    std::thread event_thread;
    std::atomic_bool stop_thread = true;
    sd_rpc_log_handler_t log_cb  = nullptr;
    std::queue<Event> event_queue;
    std::mutex event_queue_mutex;
    std::mutex callbacks_mutex;
    std::condition_variable event_queue_condition;
    std::shared_ptr<spdlog::logger> logger;
    adapter_t *adapter        = nullptr;
    void *user_data_log_event = nullptr;

    auto threadFunction() -> void;
    auto joinThread() noexcept -> bool;
    auto waitForNonEmptyQueue(int wait_ms) -> bool;
    void processLogMessage(adapter_t *adapter, Event &event, sd_rpc_log_handler_t cb_fn);

  public:
    EventHandler();
    ~EventHandler() noexcept;
    /* Delete copy constructor because of threading. */
    EventHandler(const EventHandler &) = delete;
    /* Delete assignment operator because of threading. */
    auto operator=(const EventHandler &) -> EventHandler & = delete;
    /* Delete move operator because of threading. May be implemented later if needed. */
    auto operator=(EventHandler &&obj) -> EventHandler & = delete;
    /* Delete move constructor because of threading. May be implemented later if needed. */
    EventHandler(EventHandler &&obj) = delete;
    auto start() -> void;
    auto stop() noexcept -> void;
    auto assignAdapter(adapter_t *adapter) -> void;
    auto registerLogEvent(sd_rpc_log_handler_t callback, void *user_data) -> void;
    auto isLogEventRegistered() -> bool;
    auto unregisterLogEvent() -> void;

    auto publishLogEvent(std::unique_ptr<LogMessage> message) -> void;
};
