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

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <sstream>
#include <mutex>

UartBoost::UartBoost(const UartCommunicationParameters &communicationParameters)
    : Transport(),
      ioService(),
      serialPort(ioService),
      workNotifier(ioService),
      ioWorkThread(),
      readBuffer(),
      writeBufferVector(),
      writeQueue(),
      queueMutex(),
      callbackReadHandle(),
      callbackWriteHandle(),
      asyncWriteInProgress(false),
      uartSettingsBoost(communicationParameters)
{
}

UartBoost::~UartBoost()
{
}

uint32_t UartBoost::open(status_cb_t status_callback, data_cb_t data_callback, log_cb_t log_callback)
{
    Transport::open(status_callback, data_callback, log_callback);

    const auto portName = uartSettingsBoost.getPortName();

    try
    {
        serialPort.open(portName);
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Exception thrown on " << ex.what() << " on UART port " << uartSettingsBoost.getPortName().c_str() << ".";
        statusCallback(IO_RESOURCES_UNAVAILABLE, message.str().c_str());
            return NRF_ERROR_INTERNAL;
    }

    const auto baudRate = uartSettingsBoost.getBoostBaudRate();
    const auto flowControl = uartSettingsBoost.getBoostFlowControl();
    const auto stopBits = uartSettingsBoost.getBoostStopBits();
    const auto parity = uartSettingsBoost.getBoostParity();
    const auto characterSize = uartSettingsBoost.getBoostCharacterSize();

    serialPort.set_option(baudRate);
    serialPort.set_option(flowControl);
    serialPort.set_option(stopBits);
    serialPort.set_option(parity);
    serialPort.set_option(characterSize);

    try
    {
        // boost::bind not compatible with std::bind
        callbackReadHandle = boost::bind(&UartBoost::readHandler, this,
                                  boost::asio::placeholders::error,
                                  boost::asio::placeholders::bytes_transferred);

        callbackWriteHandle = boost::bind(&UartBoost::writeHandler, this,
                                   boost::asio::placeholders::error,
                                   boost::asio::placeholders::bytes_transferred);

        // run the IO service as a separate thread, so the main thread can block on standard input
        boost::function<std::size_t()> ioServiceRun = boost::bind(&boost::asio::io_service::run, &ioService);
        ioWorkThread = boost::thread(ioServiceRun);
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Exception thrown when starting UART work thread. " << ex.what() << " on UART port " << uartSettingsBoost.getPortName().c_str() << ".";
        statusCallback(IO_RESOURCES_UNAVAILABLE, message.str().c_str());
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
        << uartSettingsBoost.getPortName().c_str() << ". "
        << "Baud rate: " << uartSettingsBoost.getBaudRate() << ". "
        << "Flow control: " << flow_control_string.str() << ". "
        << "Parity: " << parity_string.str() << "." << std::endl;

    logCallback(SD_RPC_LOG_INFO, message.str());

    return NRF_SUCCESS;
}

uint32_t UartBoost::close()
{
    try
    {
        serialPort.cancel();
        serialPort.close();
        ioService.stop();
        ioWorkThread.join();

        std::stringstream message;
        message << "UART port " << uartSettingsBoost.getPortName().c_str() << " closed.";
        logCallback(SD_RPC_LOG_INFO, message.str());

    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Exception thrown on " << ex.what() << " on UART port "  << uartSettingsBoost.getPortName().c_str() << ".";
        logCallback(SD_RPC_LOG_ERROR, message.str());
    }

    asyncWriteInProgress = false;

    Transport::close();
    return NRF_SUCCESS;
}

uint32_t UartBoost::send(std::vector<uint8_t> &data)
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

void UartBoost::readHandler(const boost::system::error_code& errorCode, const size_t bytesTransferred)
{
    if (!errorCode)
    {
        auto readBufferData = readBuffer.data();
        dataCallback(readBufferData, bytesTransferred);
        asyncRead(); // Initiate a new read
    }
    else if (errorCode == boost::asio::error::operation_aborted)
    {
        std::stringstream message;
        message << "UART read operation on port " << uartSettingsBoost.getPortName().c_str() << " aborted.";
        logCallback(SD_RPC_LOG_DEBUG, message.str());
        return;
    }
    else
    {
        std::stringstream message;
        message << "UART implementation failed while reading bytes from UART port " << uartSettingsBoost.getPortName().c_str() << ".";
        statusCallback(IO_RESOURCES_UNAVAILABLE, message.str().c_str());
        return;
    }
}

void UartBoost::writeHandler (const boost::system::error_code& errorCode, const size_t bytesTransferred)
{
    if (errorCode == boost::asio::error::operation_aborted)
    {
        std::stringstream message;
        message << "UART write operation on port " << uartSettingsBoost.getPortName().c_str() << " aborted.";
        logCallback(SD_RPC_LOG_DEBUG, message.str());

        // In case of an aborted connection, suppress notifications and return (i.e. no asyncWrite)
        queueMutex.lock();
        writeQueue.clear();
        asyncWriteInProgress = false;
        queueMutex.unlock();
        return;
    }

    asyncWrite();
}

void UartBoost::startRead()
{
    asyncRead();
}

void UartBoost::asyncRead()
{
    auto mutableReadBuffer = boost::asio::buffer(readBuffer, BUFFER_SIZE);
    serialPort.async_read_some(mutableReadBuffer, callbackReadHandle);
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

        /* Write all available bytes at once */
        writeBufferVector.insert(writeBufferVector.begin(), writeQueue.begin(), writeQueue.end());
        writeQueue.clear();
    }

    boost::asio::mutable_buffers_1 mutableWriteBuffer = boost::asio::buffer(writeBufferVector, writeBufferVector.size());
    boost::asio::async_write(serialPort, mutableWriteBuffer, callbackWriteHandle);
}
