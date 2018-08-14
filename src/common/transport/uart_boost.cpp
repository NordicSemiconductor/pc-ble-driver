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
#include "uart_settings_boost.h"
#include "nrf_error.h"

#include <functional>
#include <iostream>
#include <sstream>
#include <mutex>
#include <system_error>

#include "asio.hpp"

UartBoost::UartBoost(const UartCommunicationParameters &communicationParameters)
    : Transport(),
      readBuffer(),
      writeBufferVector(),
      writeQueue(),
      queueMutex(),
      callbackReadHandle(),
      callbackWriteHandle(),
      uartSettingsBoost(communicationParameters),
      asyncWriteInProgress(false),
      ioServiceThread(nullptr)
{
    ioService = new asio_io_context();
    serialPort = new asio::serial_port(*ioService);
    workNotifier = new asio_io_context::work(*ioService);
}

// The order of destructor invocation is important here. See:
// https://svn.boost.org/trac10/ticket/10447
UartBoost::~UartBoost()
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
    catch (std::exception& e)
    {
        std::cerr << "Error in ~UartBoost::UartBoost" << e.what() << std::endl;
        std::terminate();
    }
}

uint32_t UartBoost::open(status_cb_t status_callback, data_cb_t data_callback, log_cb_t log_callback)
{
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
        // All of these devices use the Segger OB which at the time of testing has firmware version
        // "J-Link OB-SAM3U128-V2-NordicSemi compiled Jan 12 2018 16:05:20"
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));

        const auto baudRate = uartSettingsBoost.getBoostBaudRate();
        const auto flowControl = uartSettingsBoost.getBoostFlowControl();
        const auto stopBits = uartSettingsBoost.getBoostStopBits();
        const auto parity = uartSettingsBoost.getBoostParity();
        const auto characterSize = uartSettingsBoost.getBoostCharacterSize();

        serialPort->set_option(baudRate);
        serialPort->set_option(flowControl);
        serialPort->set_option(stopBits);
        serialPort->set_option(parity);
        serialPort->set_option(characterSize);
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Error setting up serial port " << uartSettingsBoost.getPortName() << ". " << ex.what();
        upperStatusCallback(IO_RESOURCES_UNAVAILABLE, message.str().c_str());
        return NRF_ERROR_INTERNAL;
    }

    try
    {
        // boost::bind not compatible with std::bind
        callbackReadHandle = std::bind(&UartBoost::readHandler, this,
                                  std::placeholders::_1,
                                  std::placeholders::_2);

        callbackWriteHandle = std::bind(&UartBoost::writeHandler, this,
                                  std::placeholders::_1,
                                  std::placeholders::_2);

        // run execution of io_service handlers in a separate thread
        if (ioServiceThread != nullptr)
        {
            std::cerr << "ioServiceThread already exists.... aborting." << std::endl;
            std::terminate();
        }

        ioServiceThread = new std::thread([&]() {
            try {
                const auto count = ioService->run();
                std::stringstream message;
                message << "serial io_context executed " << count << " handlers.";
                upperLogCallback(SD_RPC_LOG_TRACE, message.str());
            }
            catch (std::exception &e)
            {
                std::stringstream message;
                message << "serial io_context error: " << e.what() << ".";
                upperLogCallback(SD_RPC_LOG_ERROR, message.str());
            }
        });
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Error starting serial port work thread. " << ex.what() << " on serial port " << uartSettingsBoost.getPortName() << ".";
        upperStatusCallback(IO_RESOURCES_UNAVAILABLE, message.str().c_str());
        return NRF_ERROR_INTERNAL;
    }

    startRead();

    std::stringstream flow_control_string;
    std::stringstream parity_string;

    switch(uartSettingsBoost.getParity())
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

    switch(uartSettingsBoost.getFlowControl())
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
    message << "Successfully opened "
        << uartSettingsBoost.getPortName() << ". "
        << "Baud rate: " << uartSettingsBoost.getBaudRate() << ". "
        << "Flow control: " << flow_control_string.str() << ". "
        << "Parity: " << parity_string.str() << "." << std::endl;

    upperLogCallback(SD_RPC_LOG_INFO, message.str());

    return NRF_SUCCESS;
}

uint32_t UartBoost::close()
{
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
        upperLogCallback(SD_RPC_LOG_INFO, message.str());
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Error closing serial port " << uartSettingsBoost.getPortName() << ", " << ex.what() << ".";
        upperLogCallback(SD_RPC_LOG_ERROR, message.str());
    }

    asyncWriteInProgress = false;

    Transport::close();
    return NRF_SUCCESS;
}

uint32_t UartBoost::send(const std::vector<uint8_t> &data)
{
    queueMutex.lock();
    writeQueue.insert(writeQueue.end(), data.begin(), data.end());
    queueMutex.unlock();

    if (!asyncWriteInProgress)
    {
        asyncWrite();
    }

    return NRF_SUCCESS;
}

void UartBoost::readHandler(const asio::error_code& errorCode, const size_t bytesTransferred)
{
    if (!errorCode)
    {
        auto readBufferData = readBuffer.data();
        upperDataCallback(readBufferData, bytesTransferred);
        asyncRead(); // Initiate a new read
    }
    else if (errorCode == asio::error::operation_aborted)
    {
        std::stringstream message;
        message << "serial port read on port " << uartSettingsBoost.getPortName() << " aborted.";
        upperLogCallback(SD_RPC_LOG_DEBUG, message.str());
        return;
    }
    else
    {
        std::stringstream message;
        message << "serial port read failed on port " << uartSettingsBoost.getPortName() << ". ";
        message << "Error: " << errorCode.message() << " [" << errorCode.value() << "]";
        upperStatusCallback(IO_RESOURCES_UNAVAILABLE, message.str().c_str());
        // TODO: handle this case in upper layers
    }
}

void UartBoost::writeHandler(const asio::error_code& errorCode, const size_t bytesTransferred)
{
    if (!errorCode)
    {
        asyncWrite();
    }
    else if (errorCode == asio::error::operation_aborted)
    {
        std::stringstream message;
        message << "serial port write operation on port " << uartSettingsBoost.getPortName() << " aborted.";
        upperLogCallback(SD_RPC_LOG_DEBUG, message.str());

        // In case of an aborted write operation, suppress notifications and return (i.e. no asyncWrite)
        queueMutex.lock();
        writeQueue.clear();
        asyncWriteInProgress = false;
        queueMutex.unlock();
        return;
    }
    else
    {
        std::stringstream message;
        message << "serial port write operation on port " << uartSettingsBoost.getPortName()
            << " failed. Error: " << errorCode.message() << "[" << errorCode.value() << "]";
        upperLogCallback(SD_RPC_LOG_ERROR, message.str());
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
    { //lock_guard scope
        std::lock_guard<std::mutex> guard(queueMutex);

        auto numBytesPending = writeQueue.size();

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

    auto buffer = asio::buffer(writeBufferVector, writeBufferVector.size());
    asio::async_write(*serialPort, buffer, callbackWriteHandle);
}
