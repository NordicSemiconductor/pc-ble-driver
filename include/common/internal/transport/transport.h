

#pragma once

#include "log_message.h"
#include "sd_rpc_types.h"


#include <cstdint>
#include <exception>
#include <functional>
#include <string>
#include <vector>

typedef std::function<void(const sd_rpc_app_status_t code, const std::string &message)> status_cb_t;
typedef std::function<void(const uint8_t *data, const size_t length)> data_cb_t;
typedef std::function<void(const LogMessage &logMessage)> log_cb_t;

class Transport
{
  public:
    virtual ~Transport() noexcept;
    virtual uint32_t open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                          const log_cb_t &log_callback) noexcept;
    virtual uint32_t close() noexcept = 0;

    virtual uint32_t send(const std::vector<uint8_t> &data) noexcept = 0;

    void status(const sd_rpc_app_status_t code, const std::string &message) const noexcept;
    void status(const sd_rpc_app_status_t code, const std::string &message,
                const std::exception &ex) const noexcept;

  protected:
    Transport();

    status_cb_t upperStatusCallback;
    data_cb_t upperDataCallback;
    log_cb_t upperLogCallback;
};
