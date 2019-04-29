/*
 * Copyright (c) 2016-2018 Nordic Semiconductor ASA
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

#ifndef UART_BOOST_H
#define UART_BOOST_H

#include "transport.h"
#include "uart_defines.h"
#include "uart_settings_boost.h"

#include <asio.hpp>

#include <array>
#include <deque>
#include <mutex>
#include <thread>

#include <stdint.h>

/**
 * @brief The UartBoost class opens, reads and writes a serial port using the boost asio library
 */
class UartBoost : public Transport
{
  public:
    /**
     *@brief Is called by app_uart_init() stores function pointers and sets up necessary boost
     * variables.
     */
    UartBoost(const UartCommunicationParameters &communicationParameters);

    ~UartBoost() noexcept override;

    /**
     *@brief Setup of serial port service with parameter data.
     */
    uint32_t open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                  const log_cb_t &log_callback) override;

    /**
     *@brief Closes the serial port service.
     */
    uint32_t close() override;

    /**
     *@brief sends data to serial port to write.
     */
    uint32_t send(const std::vector<uint8_t> &data) override;

  private:
    /**
     *@brief Called when background thread receives bytes from uart.
     */
    void readHandler(const asio::error_code &errorCode, const size_t bytesTransferred);

    /**
     *@brief Called when write is finished doing asynchronous write.
     */
    void writeHandler(const asio::error_code &errorCode, const size_t);

    /**
     *@brief Starts asynchronous read on a background thread.
     */
    void startRead();

    /**
     *@brief Starts an asynchronous read.
     */
    void asyncRead();

    /**
     *@brief Starts an asynchronous write.
     */
    void asyncWrite();

    std::array<uint8_t, BUFFER_SIZE> readBuffer;
    std::vector<uint8_t> writeBufferVector;
    std::deque<uint8_t> writeQueue;
    std::mutex queueMutex;
    std::mutex publicMethodMutex;
    bool isOpen;

    std::function<void(const asio::error_code, const size_t)> callbackReadHandle;
    std::function<void(const asio::error_code, const size_t)> callbackWriteHandle;

    UartSettingsBoost uartSettingsBoost;
    bool asyncWriteInProgress;
    std::unique_ptr<std::thread> ioServiceThread;

    std::unique_ptr<asio::io_service> ioService;
    std::unique_ptr<asio::serial_port> serialPort;
    std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> workNotifier;

    /**
     * @brief      Purge RX and TX data in serial buffers. On WIN32, in addition, abort any overlapped operations
     */
    void purge();
};

#endif // UART_BOOST_H
