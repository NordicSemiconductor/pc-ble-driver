/*
 * Copyright (c) 2016 Nordic Semiconductor ASA
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

#include "uart_boost.h"
#include "nrf_error.h"
#include "uart_settings_boost.h"

#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <system_error>

#if defined(__APPLE__)
#include <IOKit/serial/ioss.h>
#include <cerrno>
#include <system_error>
#endif

#include <asio.hpp>

constexpr uint32_t DUMMY_BAUD_RATE = 9600;

UartBoost::UartBoost(const UartCommunicationParameters &communicationParameters)
    : Transport()
    , readBuffer()
    , writeBufferVector()
    , writeQueue()
    , queueMutex()
    , isOpen(false)
    , callbackReadHandle()
    , callbackWriteHandle()
    , uartSettingsBoost(communicationParameters)
    , asyncWriteInProgress(false)
    , ioServiceThread(nullptr)
{
    ioService    = new asio::io_service();
    serialPort   = new asio::serial_port(*ioService);
    workNotifier = new asio::io_service::work(*ioService);
}

// The order of destructor invocation is important here. See:
// https://svn.boost.org/trac10/ticket/10447
UartBoost::~UartBoost() noexcept
{
    try
    {
        if (serialPort != nullptr)
        {
            delete serialPort;
        }

        if (workNotifier != nullptr)
        {
            delete workNotifier;
        }

        if (ioServiceThread != nullptr)
        {
            if (ioServiceThread->joinable())
            {
                ioServiceThread->join();
                delete ioServiceThread;
            }
        }

        if (ioService != nullptr)
        {
            delete ioService;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Error in ~UartBoost" << e.what() << std::endl;
        std::terminate();
    }
}

uint32_t UartBoost::open(const status_cb_t &status_callback, const data_cb_t &data_callback,
                         const log_cb_t &log_callback)
{
    std::lock_guard<std::mutex> lck(publicMethodMutex);

    if (isOpen)
    {
        return NRF_ERROR_SD_RPC_SERIAL_PORT_ALREADY_OPEN;
    }

    isOpen = true;

    Transport::open(status_callback, data_callback, log_callback);

    const auto portName = uartSettingsBoost.getPortName();

    try
    {
        serialPort->open(portName);

        // Wait a bit before making the device available since there are problems
        // if data is sent right after open.
        //
        // Not sure if this is an OS issue or if it is an issue with the device USB stack.
        // The 200ms wait time is based on testing with PCA10028, PCA10031 and PCA10040.
        // All of these devices use the SEGGER OB which at the time of testing has firmware version
        // "J-Link OB-SAM3U128-V2-NordicSemi compiled Jan 12 2018 16:05:20"
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        const auto flowControl   = uartSettingsBoost.getBoostFlowControl();
        const auto stopBits      = uartSettingsBoost.getBoostStopBits();
        const auto parity        = uartSettingsBoost.getBoostParity();
        const auto characterSize = uartSettingsBoost.getBoostCharacterSize();

        serialPort->set_option(flowControl);
        serialPort->set_option(stopBits);
        serialPort->set_option(parity);
        serialPort->set_option(characterSize);

        // SEGGER J-LINK-OB VCOM has an issue when auto-detecting UART flow control
        // The VCOM implementation keeps the detected flow-control state if the baud rate
        // is the same as the previous open of UART (MDK-1005).
        if (uartSettingsBoost.getBoostBaudRate().value() == DUMMY_BAUD_RATE)
        {
            std::stringstream message;
            message << "Error setting up serial port " << uartSettingsBoost.getPortName()
                    << ". Baud rate requested (" << uartSettingsBoost.getBoostBaudRate().value()
                    << ") is the dummy baud rate to circumvent a SEGGER J-LINK-OB issue. Please "
                       "use a different baud rate.";

            status(IO_RESOURCES_UNAVAILABLE, message.str());

            return NRF_ERROR_SD_RPC_SERIAL_PORT;
        }

#if !defined(__APPLE__)
        // Set dummy baud rate
        auto baudRate = asio::serial_port::baud_rate(DUMMY_BAUD_RATE);
        serialPort->set_option(baudRate);

        // Set requested baud rate
        baudRate = uartSettingsBoost.getBoostBaudRate();
        serialPort->set_option(baudRate);
#else
        // Workaround for setting non-standard baudrate on macOS
        // get underlying boost serial port handle and apply baud rate directly

        // Set dummy baud rate
        auto speed = (speed_t)DUMMY_BAUD_RATE;
        if (ioctl(serialPort->native_handle(), IOSSIOSPEED, &speed) < 0)
        {
            const auto error = std::error_code(errno, std::system_category());
            throw std::system_error(error, "Failed to set dummy baud rate (" +
                                               std::to_string(speed) + ")");
        }

        // Set requested baud rate
        speed = (speed_t)uartSettingsBoost.getBaudRate();

        if (ioctl(serialPort->native_handle(), IOSSIOSPEED, &speed) < 0)
        {
            const auto error = std::error_code(errno, std::system_category());
            throw std::system_error(error, "Failed to set baud rate to " + std::to_string(speed));
        }
#endif
    }
    catch (std::exception &ex)
    {
        std::stringstream message;
        message << "Error setting up serial port " << uartSettingsBoost.getPortName() << ". "
                << ex.what();

        status(IO_RESOURCES_UNAVAILABLE, message.str());

        return NRF_ERROR_SD_RPC_SERIAL_PORT;
    }

    try
    {
        callbackReadHandle =
            std::bind(&UartBoost::readHandler, this, std::placeholders::_1, std::placeholders::_2);

        callbackWriteHandle =
            std::bind(&UartBoost::writeHandler, this, std::placeholders::_1, std::placeholders::_2);

        // run execution of io_service handlers in a separate thread
        if (ioServiceThread != nullptr)
        {
            std::cerr << "ioServiceThread already exists.... aborting." << std::endl;
            std::terminate();
        }

        const auto asioWorker = [&]() {
            try
            {
                const auto count = ioService->run();
                std::stringstream message;
                message << "serial io_context executed " << count << " handlers.";
                log(SD_RPC_LOG_TRACE, message.str());
            }
            catch (std::exception &e)
            {
                std::stringstream message;
                message << "serial io_context error: " << e.what() << ".";
                log(SD_RPC_LOG_ERROR, message.str());
            }
        };

        ioServiceThread = new std::thread(asioWorker);
    }
    catch (std::exception &ex)
    {
        std::stringstream message;
        message << "Error starting serial port work thread. " << ex.what() << " on serial port "
                << uartSettingsBoost.getPortName() << ".";
        status(IO_RESOURCES_UNAVAILABLE, message.str());
        return NRF_ERROR_SD_RPC_SERIAL_PORT;
    }

    startRead();

    std::stringstream flow_control_string;
    std::stringstream parity_string;

    switch (uartSettingsBoost.getParity())
    {
        case UartParityEven:
            parity_string << "even";
            break;
        case UartParityOdd:
            parity_string << "odd";
            break;
        case UartParityNone:
            parity_string << "none";
            break;
        default:
            parity_string << "unknown";
    }

    switch (uartSettingsBoost.getFlowControl())
    {
        case UartFlowControlNone:
            flow_control_string << "none";
            break;
        case UartFlowControlHardware:
            flow_control_string << "hardware";
            break;
        case UartFlowControlSoftware:
            flow_control_string << "software";
            break;
        default:
            flow_control_string << "unknown";
    }

    std::stringstream message;
    message << "Successfully opened " << uartSettingsBoost.getPortName() << ". "
            << "Baud rate: " << uartSettingsBoost.getBaudRate() << ". "
            << "Flow control: " << flow_control_string.str() << ". "
            << "Parity: " << parity_string.str() << "." << std::endl;

    log(SD_RPC_LOG_INFO, message.str());
    isOpen = true;

    return NRF_SUCCESS;
}

uint32_t UartBoost::close()
{
    std::lock_guard<std::mutex> lck(publicMethodMutex);

    if (!isOpen)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_ALREADY_CLOSED;
    }

    isOpen = false;

    try
    {
        serialPort->close();
        ioService->stop();

        if (ioServiceThread != nullptr)
        {
            if (ioServiceThread->joinable())
            {
                ioServiceThread->join();
            }

            ioServiceThread = nullptr;
        }

        std::stringstream message;
        message << "serial port " << uartSettingsBoost.getPortName() << " closed.";
        log(SD_RPC_LOG_INFO, message.str());
    }
    catch (std::exception &ex)
    {
        std::stringstream message;
        message << "Error closing serial port " << uartSettingsBoost.getPortName() << ", "
                << ex.what() << ".";
        log(SD_RPC_LOG_ERROR, message.str());
    }

    asyncWriteInProgress = false;

    return NRF_SUCCESS;
}

uint32_t UartBoost::send(const std::vector<uint8_t> &data)
{
    std::lock_guard<std::mutex> lck(publicMethodMutex);

    if (!isOpen)
    {
        return NRF_ERROR_SD_RPC_SERIAL_PORT_STATE;
    }

    {
        std::lock_guard<std::mutex> guard(queueMutex);

        if (!isOpen)
        {
            std::stringstream message;
            message << "Trying to send packets to device when serial device is closed is not "
                       "supported.";
            log(SD_RPC_LOG_ERROR, message.str());

            return NRF_ERROR_SD_RPC_SERIAL_PORT_STATE;
        }

        writeQueue.insert(writeQueue.end(), data.begin(), data.end());
    }

    if (!asyncWriteInProgress)
    {
        asyncWrite();
    }

    return NRF_SUCCESS;
}

void UartBoost::readHandler(const asio::error_code &errorCode, const size_t bytesTransferred)
{
    if (!isOpen && !errorCode)
    {
        std::stringstream message;
        message << "serial port closed, but " << bytesTransferred
                << " bytes received. Data will not be sent to transport layer above.";
        log(SD_RPC_LOG_DEBUG, message.str());
    }

    if (!errorCode && isOpen)
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
        std::stringstream message;
        message << "serial port read on port " << uartSettingsBoost.getPortName() << " aborted.";

        log(SD_RPC_LOG_DEBUG, message.str());
    }
    else
    {
        std::stringstream message;
        message << "serial port read failed on port " << uartSettingsBoost.getPortName() << ". ";
        message << "Error: " << errorCode.message() << " [" << errorCode.value() << "]";

        status(IO_RESOURCES_UNAVAILABLE, message.str());
    }
}

void UartBoost::writeHandler(const asio::error_code &errorCode, const size_t bytesTransferred)
{
    if (!errorCode)
    {
        asyncWrite();
    }
    else if (errorCode == asio::error::operation_aborted)
    {
        std::stringstream message;
        message << "serial port write operation on port " << uartSettingsBoost.getPortName()
                << " aborted.";

        log(SD_RPC_LOG_DEBUG, message.str());

        // In case of an aborted write operation, suppress notifications and return (i.e. no
        // asyncWrite)
        queueMutex.lock();
        writeQueue.clear();
        asyncWriteInProgress = false;
        queueMutex.unlock();
    }
    else
    {
        std::stringstream message;
        message << "serial port write operation on port " << uartSettingsBoost.getPortName()
                << " failed. Error: " << errorCode.message() << "[" << errorCode.value() << "]";

        log(SD_RPC_LOG_ERROR, message.str());
    }
}

void UartBoost::startRead()
{
    asyncRead();
}

void UartBoost::asyncRead()
{
    const auto mutableReadBuffer = asio::buffer(readBuffer, BUFFER_SIZE);
    serialPort->async_read_some(mutableReadBuffer, callbackReadHandle);
}

void UartBoost::asyncWrite()
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
        writeBufferVector.insert(writeBufferVector.begin(), writeQueue.begin(), writeQueue.end());
        writeQueue.clear();
    }

    const auto buffer = asio::buffer(writeBufferVector, writeBufferVector.size());
    asio::async_write(*serialPort, buffer, callbackWriteHandle);
}
