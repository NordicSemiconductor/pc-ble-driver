#include "adapter_internal.h"

#include "adapter.h"
#include "nrf_error.h"
#include "serialization_transport.h"

#include <string>

AdapterInternal::AdapterInternal(SerializationTransport *_transport)
    : transport(_transport)
    , eventCallback(nullptr)
    , statusCallback(nullptr)
    , logCallback(nullptr)
    , logSeverityFilter(SD_RPC_LOG_TRACE)
    , isOpen(false)
{}

AdapterInternal::~AdapterInternal()
{
    delete transport;
}

uint32_t AdapterInternal::open(const sd_rpc_status_handler_t status_callback,
                               const sd_rpc_evt_handler_t event_callback,
                               const sd_rpc_log_handler_t log_callback,
                               const void *user_data_status, const void *user_data_event,
                               const void *user_data_log)
{
    std::lock_guard<std::mutex> lck(publicMethodMutex);

    if (isOpen)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    isOpen = true;

    statusCallback = status_callback;
    eventCallback  = event_callback;
    logCallback    = log_callback;
    userDataStatus = user_data_status;
    userDataEvent  = user_data_event;
    userDataLog    = user_data_log;

    const auto boundStatusHandler = std::bind(&AdapterInternal::statusHandler, this,
                                              std::placeholders::_1, std::placeholders::_2);
    const auto boundEventHandler =
        std::bind(&AdapterInternal::eventHandler, this, std::placeholders::_1);
    const auto boundLogHandler =
        std::bind(&AdapterInternal::logHandler, this, std::placeholders::_1);
    return transport->open(boundStatusHandler, boundEventHandler, boundLogHandler);
}

uint32_t AdapterInternal::close()
{
    std::lock_guard<std::mutex> lck(publicMethodMutex);

    if (!isOpen)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    isOpen = false;

    return transport->close();
}

void AdapterInternal::statusHandler(const sd_rpc_app_status_t code, const std::string &message)
{
    adapter_t adapter = {};
    adapter.internal  = static_cast<void *>(this);

    if (statusCallback != nullptr)
    {
        statusCallback(&adapter, code, message.c_str(), userDataStatus);
    }
}

void AdapterInternal::eventHandler(ble_evt_t *event)
{
    // Event Thread
    adapter_t adapter = {};
    adapter.internal  = static_cast<void *>(this);

    if (eventCallback != nullptr)
    {
        eventCallback(&adapter, event, userDataEvent);
    }
}

void AdapterInternal::logHandler(const LogMessage &logMessage)
{
    adapter_t adapter = {};
    adapter.internal  = static_cast<void *>(this);

    if (logCallback != nullptr)
    {
        logCallback(&adapter, logMessage.data(), userDataLog);
    }
}

bool AdapterInternal::isInternalError(const uint32_t error_code)
{
    return error_code != NRF_SUCCESS;
}

uint32_t AdapterInternal::logSeverityFilterSet(const sd_rpc_log_severity_t severity_filter)
{
    std::lock_guard<std::mutex> lck(publicMethodMutex);
    logSeverityFilter = severity_filter;
    return NRF_SUCCESS;
}
