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

#ifndef UART_TEST_H
#define UART_TEST_H

#include "uart.h"
#include "uart_defines.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>

#include <stdint.h>

/**
 * @brief The UartTest class is a special test class that is used to retrieve and send data to the
 * UartBoost class object on the other side of a virtual serial port.
 */
class UartTest : public Uart
{
public:
    /**@brief Sets up necessary boost variables.
     */
    UartTest(const data_received_t dataReceived, const data_sent_t dataSent);

    ~UartTest();

    /**@brief Setup of serial port service with parameter data.
     */
    uint32_t open(const UartCommunicationParameters &communicationParameters);

    /**@brief returns true if open.
     */
    bool isOpen() const;

    /**@brief Closes the serial port service.
     */
    uint32_t close();

    /**@brief sends data to serial port to write.
     */
    uint32_t write(const uint8_t *data, uint16_t length);

    /**@brief sends data to serial port to write.
     */
    uint32_t write(const uint8_t data);
    
    /**@brief reads data to buffer.
     */
    uint32_t read(uint8_t *data, size_t numberOfBytes);

    /**@brief clears buffers.
     */
    uint32_t flush();

private:

    boost::asio::io_service ioService;
    boost::asio::serial_port serialPort;

    boost::array<uint8_t, BUFFER_SIZE> readBuffer;
    boost::array<uint8_t, BUFFER_SIZE> writeBuffer;
};

#endif // TEST_UART_H
