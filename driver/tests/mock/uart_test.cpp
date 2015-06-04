/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#include "uart_test.h"
#include "uart_settings_boost.h"
#include "nrf_error.h"

using namespace boost;

UartTest::UartTest(const data_received_t dataReceived, const data_sent_t dataSent)
    : Uart(dataReceived, dataSent),
      ioService(),
      serialPort(ioService),
      readBuffer(),
      writeBuffer()
{
    //Empty
}

UartTest::~UartTest()
{
    close();
    ioService.stop();
}

uint32_t UartTest::open(const UartCommunicationParameters &communicationParameters)
{
    UartSettingsBoost uartSettings(communicationParameters);

    try
    {
        serialPort.open(uartSettings.getPortName());
    }
    catch(std::exception& e)
    {
        //exception message convention?
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    const asio::serial_port_base::baud_rate baudRate = uartSettings.getBoostBaudRate();
    const asio::serial_port_base::flow_control flowControl = uartSettings.getBoostFlowControl();
    const asio::serial_port_base::stop_bits stopBits = uartSettings.getBoostStopBits();
    const asio::serial_port_base::parity parity = uartSettings.getBoostParity();
    const asio::serial_port::character_size characterSize(8);

    serialPort.set_option(baudRate);
    serialPort.set_option(flowControl);
    serialPort.set_option(stopBits);
    serialPort.set_option(parity);
    serialPort.set_option(characterSize);

    return NRF_SUCCESS;
}

bool UartTest::isOpen() const
{
    return serialPort.is_open();
}

uint32_t UartTest::close()
{
    serialPort.close();

    return 0;
}

uint32_t UartTest::write(const uint8_t *data, uint16_t length)
{
    asio::const_buffers_1 constWriteBuffer = asio::buffer(data, length);
    uint32_t bytesWritten = asio::write(serialPort, constWriteBuffer);

    return bytesWritten;
}

uint32_t UartTest::write(const uint8_t data)
{
    asio::const_buffers_1 constWriteBuffer = asio::buffer(&data, 1);
    uint32_t bytesWritten = asio::write(serialPort, constWriteBuffer);

    return bytesWritten;
}

uint32_t UartTest::read(uint8_t *data, size_t numberOfBytes)
{
    asio::mutable_buffers_1 mutableReadBuffer = asio::buffer(data, numberOfBytes);
    uint32_t bytesRead = asio::read(serialPort, mutableReadBuffer);

    return bytesRead;
}

uint32_t UartTest::flush()
{
    //Not implemented;
    return 0;
}
