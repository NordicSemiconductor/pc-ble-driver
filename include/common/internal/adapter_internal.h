#pragma once

#include "log_message.h"
#include "sd_rpc_types.h"
#include "serialization_transport.h"

#include "ble.h"
#include "nrf_error.h"

#include <string>

class AdapterInternal
{
  public:
    explicit AdapterInternal(SerializationTransport *transport);
    ~AdapterInternal();
    uint32_t open(const sd_rpc_status_handler_t status_callback,
                  const sd_rpc_evt_handler_t event_callback,
                  const sd_rpc_log_handler_t log_callback, const void *user_data_status,
                  const void *user_data_event, const void *user_data_log);
    uint32_t close();
    uint32_t logSeverityFilterSet(const sd_rpc_log_severity_t severity_filter);
    static bool isInternalError(const uint32_t error_code);

    void statusHandler(const sd_rpc_app_status_t code, const std::string &error);
    void eventHandler(ble_evt_t *event);
    void logHandler(const LogMessage &logMessage);

    SerializationTransport *transport;

  private:
    sd_rpc_evt_handler_t eventCallback;
    sd_rpc_status_handler_t statusCallback;
    sd_rpc_log_handler_t logCallback;
    sd_rpc_log_severity_t logSeverityFilter;

    const void *userDataStatus;
    const void *userDataEvent;
    const void *userDataLog;

    bool isOpen;
    std::mutex publicMethodMutex;
};
