/*
 * Copyright (c) 2016-2019 Nordic Semiconductor ASA
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

#include "uart_transport.h"

#include "log_helper.h"
#include "nrf_error.h"
#include "uart_settings.h"
#include "uart_settings_boost.h"

#include <deque>
#include <functional>
#include <mutex>
#include <sstream>
#include <system_error>

#if defined(__APPLE__)
#include <IOKit/serial/ioss.h>
#include <cerrno>
#include <system_error>
#endif

#include <asio.hpp>
#include <fmt/format.h>

constexpr auto DELAY_BEFORE_READ_WRITE = std::chrono::milliseconds(200);

// On Windows, some users experience a permission denied error
// if opening a UART port right after close.
constexpr auto DELAY_BEFORE_OPEN = std::chrono::milliseconds(200);

#if defined(_WIN32)
// Wait for 20ms for data before returning from a read
constexpr auto readTotalTimeoutConstant = 20;
#endif

struct UartTransport::impl : Transport
{
    std::array<uint8_t, UartTransportBufferSize> readBuffer;
    std::vector<uint8_t> writeBufferVector;
    std::deque<uint8_t> writeQueue;
    std::mutex queueMutex;

    bool isOpen;
    std::recursive_mutex isOpenMutex;

    std::function<void(const asio::error_code, const size_t)> callbackReadHandle;
    std::function<void(const asio::error_code, const size_t)> callbackWriteHandle;

    UartSettingsBoost uartSettingsBoost;
    bool asyncWriteInProgress;
    std::unique_ptr<std::thread> ioServiceThread;

    std::unique_ptr<asio::io_service> ioService;
    std::unique_ptr<asio::serial_port> serialPort;
    std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> workNotifier;

    impl(const UartCommunicationParameters &communicationParameters)
        : readBuffer()
        , isOpen(false)
        , uartSettingsBoost(communicationParameters)
        , asyncWriteInProgress(false)
        , ioServiceThread(nullptr)
    {}

    /**
     *@brief Called when background thread receives bytes from uart.
     */
    void readHandler(const asio::error_code &errorCode, const size_t bytesTransferred)
    {
        if (!errorCode)
        {
            const auto readBufferData = readBuffer.data();

            if (upperDataCallback)
            {
                upperDataCallback(readBufferData, bytesTransferred);
            }

            asyncRead(); // Initiate a new read
        }
        else if (errorCode == asio::error::operation_aborted)
        {
            getLogger()->debug("serial port read on port {} aborted.",
                               uartSettingsBoost.getPortName());
        }
        else
        {
            std::stringstream message;
            message << "serial port read failed on port " << uartSettingsBoost.getPortName()
                    << ". ";
            message << "Error: " << errorCode.message() << " [" << errorCode.value() << "]";

            status(IO_RESOURCES_UNAVAILABLE, message.str());
        }
    }

    void writeHandler(const asio::error_code &errorCode, const size_t bytesTransferred)
    {
        if (!errorCode)
        {
            asyncWrite();
        }
        else if (errorCode == asio::error::operation_aborted)
        {
            getLogger()->debug("serial port write operation on port {} aborted.",
                               uartSettingsBoost.getPortName());

            // In case of an aborted write operation, suppress notifications and return (i.e. no
            // asyncWrite)
            queueMutex.lock();
            writeQueue.clear();
            asyncWriteInProgress = false;
            queueMutex.unlock();
        }
        else
        {
            getLogger()->error("serial port write operation on port {} failed. Error {} [{}]",
                               uartSettingsBoost.getPortName(), errorCode.message(),
                               errorCode.value());
        }
    }

    void startRead()
    {
        asyncRead();
    }

    void asyncRead()
    {
        const auto mutableReadBuffer = asio::buffer(readBuffer, UartTransportBufferSize);
        serialPort->async_read_some(mutableReadBuffer, callbackReadHandle);
    }

    void asyncWrite()
    {
        { // lock_guard scope
            std::lock_guard<std::mutex> guard(queueMutex);

            const auto numBytesPending = writeQueue.size();

            if (numBytesPending == 0)
            {
                asyncWriteInProgress = false;
                return;
            }

            asyncWriteInProgress = true;
            writeBufferVector.clear();

            /* Write all available bytes in one operation */
            writeBufferVector.insert(writeBufferVector.begin(), writeQueue.begin(),
                                     writeQueue.end());
            writeQueue.clear();
        }

        const auto buffer = asio::buffer(writeBufferVector, writeBufferVector.size());
        asio::async_write(*serialPort, buffer, callbackWriteHandle);
    }

    /**
     * @brief      Purge RX and TX data in serial buffers. On WIN32, in addition, abort any
     * overlapped operations
     */
    void purge() const
    {
#if _WIN32
        const auto result =
            PurgeComm(serialPort->native_handle(),
                      PURGE_RXCLEAR | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_TXABORT);
        if (result == 0)
        {
            const auto lastError = GetLastError();

            if (lastError != ERROR_SUCCESS)
            {
                getLogger()->error("Error purging UART {}", lastError);
            }
        }
#endif

#ifdef __unix__
        const auto result = tcflush(serialPort->native_handle(), TCIOFLUSH);

        if (result == -1)
        {
            getLogger()->error("Error purging UART {}", result);
        }
#endif
    }

    uint32_t open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                  const log_cb_t &log_callback) noexcept
    {
        std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

        if (isOpen)
        {
            return NRF_ERROR_SD_RPC_SERIAL_PORT_ALREADY_OPEN;
        }

        isOpen = true;

        Transport::open(status_callback, data_callback, log_callback);

        const auto portName = uartSettingsBoost.getPortName();

        try
        {
            ioService  = std::make_unique<asio::io_service>();
            serialPort = std::make_unique<asio::serial_port>(*ioService);
            workNotifier =
                std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
                    asio::make_work_guard(*ioService));

            std::this_thread::sleep_for(DELAY_BEFORE_OPEN);
            serialPort->open(portName);
            purge();
            const auto flowControl   = uartSettingsBoost.getBoostFlowControl();
            const auto stopBits      = uartSettingsBoost.getBoostStopBits();
            const auto parity        = uartSettingsBoost.getBoostParity();
            const auto characterSize = uartSettingsBoost.getBoostCharacterSize();

            serialPort->set_option(flowControl);
            serialPort->set_option(stopBits);
            serialPort->set_option(parity);
            serialPort->set_option(characterSize);

#if !defined(__APPLE__)
            asio::serial_port::baud_rate baudRate;

            // Set requested baud rate
            baudRate = uartSettingsBoost.getBoostBaudRate();
            serialPort->set_option(baudRate);
#else // !defined(__APPLE__)
      // Set requested baud rate
            const auto speed = (speed_t)uartSettingsBoost.getBaudRate();

            if (ioctl(serialPort->native_handle(), IOSSIOSPEED, &speed) < 0)
            {
                const auto error = std::error_code(errno, std::system_category());
                throw std::system_error(error,
                                        "Failed to set baud rate to " + std::to_string(speed));
            }
#endif

#if defined(_WIN32)
            ::COMMTIMEOUTS timeouts;

            // See
            // https://docs.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-commtimeouts
            // for documentation of these parameters
            timeouts.ReadIntervalTimeout        = MAXDWORD;
            timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
            timeouts.ReadTotalTimeoutConstant   = readTotalTimeoutConstant;

            timeouts.WriteTotalTimeoutMultiplier = 0;
            timeouts.WriteTotalTimeoutConstant   = 0;

            if (!::SetCommTimeouts(serialPort->native_handle(), &timeouts))
            {
                const auto error = std::error_code(errno, std::system_category());
                throw std::system_error(error, "Failed to set communication timeout parameters");
            }
#endif

            // Wait a bit before making the device available since there are problems
            // if data is sent right after open.
            //
            // Not sure if this is an OS issue or if it is an issue with the device USB stack.
            // The 200ms wait time is based on testing with PCA10028, PCA10031 and PCA10040.
            // All of these devices use the SEGGER OB which at the time of testing has firmware
            // version "J-Link OB-SAM3U128-V2-NordicSemi compiled Jan 12 2018 16:05:20"
            std::this_thread::sleep_for(DELAY_BEFORE_READ_WRITE);
        }
        catch (const std::exception &ex)
        {
            try
            {
                std::stringstream message;
                message << "Error setting up serial port " << uartSettingsBoost.getPortName()
                        << ". " << ex.what();
                status(IO_RESOURCES_UNAVAILABLE, message.str());
            }
            catch (const std::exception &)
            {
                getLogger()->error("Error creating status message entry");
            }

            return NRF_ERROR_SD_RPC_SERIAL_PORT;
        }

        try
        {
            callbackReadHandle = std::bind(&UartTransport::impl::readHandler, this,
                                           std::placeholders::_1, std::placeholders::_2);

            callbackWriteHandle = std::bind(&UartTransport::impl::writeHandler, this,
                                            std::placeholders::_1, std::placeholders::_2);

            const auto asioWorker = [&]() {
                try
                {
                    // If ioService has ran before it needs to be restarted
                    if (ioService->stopped())
                    {
                        ioService->restart();
                    }

                    const auto count = ioService->run();
                    getLogger()->trace("serial io_context executed {} handlers.", count);
                }
                catch (std::exception &e)
                {
                    LogHelper::tryToLogException(spdlog::level::err, e, "serial io_context error");
                }
            };

            ioServiceThread = std::make_unique<std::thread>(asioWorker);
        }
        catch (std::exception &ex)
        {
            try
            {
                std::stringstream message;
                message << "Error starting serial port work thread. " << ex.what()
                        << " on serial port " << uartSettingsBoost.getPortName() << ".";
                status(IO_RESOURCES_UNAVAILABLE, message.str());
            }
            catch (const std::exception &)
            {
                getLogger()->error(
                    "Error creating status message entry or invoking status callback.");
            }

            return NRF_ERROR_SD_RPC_SERIAL_PORT;
        }

        startRead();

        try
        {
            /*            getLogger()->info("Successfully opened {}. Baud rate: {}. Flow control:
               {}. Parity: {}", uartSettingsBoost.getPortName(), uartSettingsBoost.getBaudRate(),
                                          uartSettingsBoost.getFlowControl(),
               uartSettingsBoost.getParity());*/

            getLogger()->info("Successfully opened {}. Baud rate: {}.",
                              uartSettingsBoost.getPortName(), uartSettingsBoost.getBaudRate());
        }
        catch (...)
        {
            std::terminate();
        }

        return NRF_SUCCESS;
    }

    uint32_t close() noexcept
    {
        std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

        if (!isOpen)
        {
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_ALREADY_CLOSED;
        }

        isOpen = false;

        try
        {
            serialPort->cancel();
            purge();
            serialPort->close();
            ioService->stop();
            workNotifier->reset();

            if (ioServiceThread != nullptr)
            {
                if (ioServiceThread->joinable())
                {
                    ioServiceThread->join();
                }
            }

            serialPort.reset();
            ioService.reset();

            getLogger()->info("serial port {} closed.", uartSettingsBoost.getPortName());

            asyncWriteInProgress = false;
        }
        catch (std::exception &ex)
        {
            LogHelper::tryToLogException(spdlog::level::err, ex, "Error closing serial port");
            return NRF_ERROR_SD_RPC_SERIAL_PORT_INTERNAL_ERROR;
        }

        return NRF_SUCCESS;
    }

    uint32_t send(const std::vector<uint8_t> &data) noexcept
    {
        {
            std::lock_guard<std::recursive_mutex> openLck(isOpenMutex);

            if (!isOpen)
            {
                getLogger()->error("Trying to send packets to device when serial device is closed "
                                   "is not supported");

                return NRF_ERROR_SD_RPC_SERIAL_PORT_STATE;
            }
        }

        {
            std::lock_guard<std::mutex> guard(queueMutex);

            try
            {
                writeQueue.insert(writeQueue.end(), data.begin(), data.end());
            }
            catch (const std::exception &e)
            {
                LogHelper::tryToLogException(spdlog::level::err, e, "Error adding TX data");
                return NRF_ERROR_SD_RPC_SERIAL_PORT_INTERNAL_ERROR;
            }
        }

        if (!asyncWriteInProgress)
        {
            try
            {
                asyncWrite();
            }
            catch (const std::exception &e)
            {
                LogHelper::tryToLogException(spdlog::level::err, e,
                                             "Error writing data async to device");
                return NRF_ERROR_SD_RPC_SERIAL_PORT_INTERNAL_ERROR;
            }
        }

        return NRF_SUCCESS;
    }
};

UartTransport::UartTransport(const UartCommunicationParameters &communicationParameters)
    : pimpl{new impl(communicationParameters)}
{}

UartTransport::~UartTransport() noexcept
{
    UartTransport::close();
}

uint32_t UartTransport::open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                             const log_cb_t &log_callback) noexcept
{
    return pimpl->open(status_callback, data_callback, log_callback);
}

uint32_t UartTransport::close() noexcept
{
    return pimpl->close();
}

uint32_t UartTransport::send(const std::vector<uint8_t> &data) noexcept
{
    return pimpl->send(data);
}