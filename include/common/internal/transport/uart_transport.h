#pragma once

#include "transport.h"
#include "uart_settings.h"

#include <cstdint>
#include <memory>
#include <vector>

/**
 * @brief Controls the buffer sizes for read buffers
 */
constexpr size_t UartTransportBufferSize = 1024;

/**
 * @brief The UartTransport class opens, reads and writes a serial port using the boost asio library
 */
class UartTransport : public Transport
{
  public:
    /**
     *@brief Is called by app_uart_init() stores function pointers and sets up necessary boost
     * variables.
     */
    UartTransport(const UartCommunicationParameters &communicationParameters);

    ~UartTransport() noexcept override;

    /**
     *@brief Setup of serial port service with parameter data.
     */
    uint32_t open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                  const log_cb_t &log_callback) noexcept override;

    /**
     *@brief Closes the serial port service.
     */
    uint32_t close() noexcept override;

    /**
     *@brief sends data to serial port to write.
     */
    uint32_t send(const std::vector<uint8_t> &data) noexcept override;

  private:
    struct impl;
    std::unique_ptr<impl> pimpl;
};
