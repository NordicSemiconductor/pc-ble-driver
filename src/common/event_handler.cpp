#include "event_handler.h"
#include "log_helper.h"
#include <iostream>

EventHandler::EventHandler()
{
    logger = getLogger();
}

EventHandler::~EventHandler() noexcept
{
    stop_thread = true;
    joinThread();
}

auto EventHandler::start() -> void
{
    if (!stop_thread)
    {
        return;
    }

    stop_thread = false;
    joinThread();
    event_thread = std::thread{&EventHandler::threadFunction, this};
}

auto EventHandler::stop() noexcept -> void
{}

auto EventHandler::assignAdapter(adapter_t *adapter) -> void
{
    EventHandler::adapter = adapter;
}

auto EventHandler::registerLogEvent(sd_rpc_log_handler_t callback, void *user_data) -> void
{
    const std::scoped_lock<std::mutex> lock(callbacks_mutex);
    log_cb              = callback;
    user_data_log_event = user_data;
}

auto EventHandler::isLogEventRegistered() -> bool
{
    const std::scoped_lock<std::mutex> lock(callbacks_mutex);
    return log_cb != nullptr;
}

auto EventHandler::unregisterLogEvent() -> void
{
    const std::scoped_lock<std::mutex> lock(callbacks_mutex);
    log_cb              = nullptr;
    user_data_log_event = nullptr;
}

auto EventHandler::publishLogEvent(std::unique_ptr<LogMessage> message) -> void
{
    { // Locked scope
        const std::scoped_lock<std::mutex> lock(event_queue_mutex);
        Event event;
        event.setLogMessage(std::move(message));
        event_queue.push(std::move(event));
    }

    event_queue_condition.notify_all();
}

auto EventHandler::waitForNonEmptyQueue(int wait_ms) -> bool
{
    using namespace std::chrono_literals;
    std::unique_lock lock{event_queue_mutex};
    return event_queue_condition.wait_for(lock, wait_ms * 1ms,
                                          [this] { return !event_queue.empty(); });
}

void EventHandler::processLogMessage(adapter_t *ctx, Event &event, sd_rpc_log_handler_t cb_fn)
{
    if (!event.hasLogMessage() || cb_fn == nullptr)
    {
        return;
    }

    auto log = event.getLogMessage();

    try
    {
        cb_fn(ctx, log->data(), user_data_log_event);
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Log callback failed with exception: " << ex.what() << std::endl;
    }
}

/* TODO: Consider one seperate queue for log messages since we (may) have so many of them. */
auto EventHandler::threadFunction() -> void
{
    // Make sure we are not waiting forever in case of exceptions etc.
    static constexpr auto sleep_interval_empty_ms = 10;

    while (!stop_thread)
    {
        std::this_thread::yield();

        bool empty_queue = true;

        { // Locked scope
            std::scoped_lock lock{event_queue_mutex};
            empty_queue = event_queue.empty();
        }

        if (empty_queue)
        {
            waitForNonEmptyQueue(sleep_interval_empty_ms);
        }

        Event event{};

        { // Locked scope
            std::scoped_lock<std::mutex> lock{event_queue_mutex};

            if (event_queue.empty())
            {
                continue;
            }

            event = std::move(event_queue.front());
            event_queue.pop();
        }

        sd_rpc_log_handler_t log_cb_tmp = nullptr;

        { // Locked scope
            std::scoped_lock<std::mutex> lock{callbacks_mutex};
            log_cb_tmp = log_cb;
        }

        processLogMessage(adapter, event, log_cb_tmp);
    }
}

auto EventHandler::joinThread() noexcept -> bool
{
    try
    {
        if (!event_thread.joinable())
        {
            return false;
        }

        event_thread.join();
    }
    catch (const std::exception &)
    {
        return false;
    }

    return true;
}
