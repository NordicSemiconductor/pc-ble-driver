/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "uart_boost.h"
#include "uart_settings_boost.h"
#include "app_log.h"
#include "nrf_error.h"

#include <boost/bind.hpp>

using namespace boost;

UartBoost::UartBoost(const data_received_t dataReceived, const data_sent_t dataSent)
    : Uart(dataReceived, dataSent),
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
      asyncWriteInProgress(false)
{
    //Empty
}

UartBoost::~UartBoost()
{
    close();
    workNotifier.~work();
    ioWorkThread.join();
    ioService.stop();
}

uint32_t UartBoost::open(const UartCommunicationParameters &communicationParameters)
{
    if (isOpen())
    {
        return NRF_ERROR_INVALID_STATE;
    }

    UartSettingsBoost uartSettingsBoost(communicationParameters);

    const std::string portName = uartSettingsBoost.getPortName();

    try
    {
        serialPort.open(portName);
    }
    catch (std::exception & e)
    {
        app_log_handler(APP_LOG_ERROR, "Exception thrown when opening serial port: %s", e.what());
        return NRF_ERROR_INTERNAL;
    }

    const asio::serial_port::baud_rate baudRate = uartSettingsBoost.getBoostBaudRate();
    const asio::serial_port::flow_control flowControl = uartSettingsBoost.getBoostFlowControl();
    const asio::serial_port::stop_bits stopBits = uartSettingsBoost.getBoostStopBits();
    const asio::serial_port::parity parity = uartSettingsBoost.getBoostParity();
    const asio::serial_port::character_size characterSize = uartSettingsBoost.getBoostCharacterSize();

    serialPort.set_option(baudRate);
    serialPort.set_option(flowControl);
    serialPort.set_option(stopBits);
    serialPort.set_option(parity);
    serialPort.set_option(characterSize);

    try
    {
        callbackReadHandle = bind(&UartBoost::readHandler, this,
                                  asio::placeholders::error,
                                  asio::placeholders::bytes_transferred);

        callbackWriteHandle = bind(&UartBoost::writeHandler, this,
                                   asio::placeholders::error,
                                   asio::placeholders::bytes_transferred);

        // run the IO service as a separate thread, so the main thread can block on standard input
        function<std::size_t()> ioServiceRun = bind(&asio::io_service::run, &ioService);
        ioWorkThread = thread(ioServiceRun);
    }
    catch (std::exception & e)
    {
        app_log_handler(APP_LOG_ERROR, "Exception thrown when starting uart work thread: %s", e.what());
        return NRF_ERROR_INTERNAL;
    }

    if (!isOpen())
    {
        return NRF_ERROR_INTERNAL;
    }

    startRead();

    char message[256];
    char flow_control_string[9];
    char parity_string[5];

    if (communicationParameters.flowControl == UartFlowControlHardware)
    {
        sprintf(flow_control_string, "hardware");
    }
    else
    {
        sprintf(flow_control_string, "none");
    }

    if (communicationParameters.parity == UartParityEven)
    {
        sprintf(parity_string, "even");
    }
    else if (communicationParameters.parity == UartParityOdd)
    {
        sprintf(parity_string, "odd");
    }
    else
    {
        sprintf(parity_string, "none");
    }

    sprintf(message,
            "Successfully opened uart. Port name: %s. Baud rate: %d. Flow control: %s. Parity: %s",
            communicationParameters.portName, communicationParameters.baudRate,
            flow_control_string, parity_string);

    app_log_handler(APP_LOG_DEBUG, message);

    return NRF_SUCCESS;
}

bool UartBoost::isOpen() const
{
    return serialPort.is_open();
}

uint32_t UartBoost::close()
{
    if (!isOpen())
    {
        return NRF_ERROR_INVALID_STATE;
    }

    try
    {
        serialPort.close();
    }
    catch (std::exception &e)
    {
        app_log_handler(APP_LOG_ERROR, "Exception thrown when closing serial port: %s", e.what());
    }

    asyncWriteInProgress = false;

    app_log_handler(APP_LOG_DEBUG, "Serial port closed.");

    return NRF_SUCCESS;
}

uint32_t UartBoost::write(const uint8_t *data, uint16_t length)
{
    std::vector<uint8_t> dataVector(data, data + length);
    queueMutex.lock();
    writeQueue.insert(writeQueue.end(), dataVector.begin(), dataVector.end());
    queueMutex.unlock();

    if (!asyncWriteInProgress)
    {
        asyncWrite();
    }

    return NRF_SUCCESS;
}

uint32_t UartBoost::write(const uint8_t data)
{
    queueMutex.lock();
    writeQueue.push_back(data);
    queueMutex.unlock();

    if (!asyncWriteInProgress)
    {
        asyncWrite();
    }

    return NRF_SUCCESS;
}

uint32_t UartBoost::flush()
{
    //Not implemented. Serial port flush is not supported directly in Boost.
    //Article on how to implement native flush (search for 'native'): http://mnb.ociweb.com/mnb/MiddlewareNewsBrief-201303.html
    return NRF_SUCCESS;
}

void UartBoost::readHandler(const system::error_code& errorCode, const size_t bytesTransferred)
{
    if (errorCode == system::errc::success)
    {
        uint8_t *readBufferData = readBuffer.data();
        app_log_byte_array_handler(APP_LOG_TRACE, "[Read] %s", readBufferData, bytesTransferred);
        dataReceivedHandle(NRF_SUCCESS, readBufferData, bytesTransferred);
        asyncRead();
    }
    else if (errorCode == asio::error::operation_aborted)
    {
        // In case of an aborted connection, suppress notifications and return early
        return;
    }
    else
    {
        dataReceivedHandle(NRF_ERROR_INTERNAL, 0, 0);
    }
}

void UartBoost::writeHandler (const system::error_code& errorCode, const size_t bytesTransferred)
{
    if (errorCode == system::errc::success)
    {
        dataSentHandle(NRF_SUCCESS, bytesTransferred);
    }
    else if (errorCode == asio::error::operation_aborted)
    {
        // In case of an aborted connection, suppress notifications and return (i.e. no asyncWrite)
        queueMutex.lock();
        writeQueue.clear();
        asyncWriteInProgress = false;
        queueMutex.unlock();
        return;
    }
    else
    {
        dataSentHandle(NRF_ERROR_INTERNAL, bytesTransferred);
    }

    asyncWrite();
}

void UartBoost::startRead()
{
    asyncRead();
}

void UartBoost::asyncRead()
{
    asio::mutable_buffers_1 mutableReadBuffer = asio::buffer(readBuffer, BUFFER_SIZE);

    serialPort.async_read_some(mutableReadBuffer, callbackReadHandle);
}

void UartBoost::asyncWrite()
{
    { //lock_guard scope
        lock_guard<mutex> guard(queueMutex);

        size_t numBytesPending = writeQueue.size();

        if (numBytesPending == 0)
        {
            asyncWriteInProgress = false;
            return;
        }

        asyncWriteInProgress = true;
        writeBufferVector.clear();

        /* Write all available bytes at once */
        std::vector<uint8_t>::iterator it = writeBufferVector.begin();
        writeBufferVector.insert(it, writeQueue.begin(), writeQueue.end());
        writeQueue.clear();
    }

    app_log_byte_array_handler(APP_LOG_TRACE, "[Write] %s", writeBufferVector.data(), writeBufferVector.size());

    asio::mutable_buffers_1 mutableWriteBuffer = asio::buffer(writeBufferVector, writeBufferVector.size());
    asio::async_write(serialPort, mutableWriteBuffer, callbackWriteHandle);
}
