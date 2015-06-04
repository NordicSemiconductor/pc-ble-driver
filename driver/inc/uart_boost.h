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

#ifndef UART_BOOST_H
#define UART_BOOST_H

#include "uart.h"
#include "uart_defines.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>

#include <deque>
#include <queue>

#include <stdint.h>

/**
 * @brief The UartBoost class opens, reads and writes a serial port using the boost asio library
 */
class UartBoost : public Uart
{
public:

    /**@brief Is called by app_uart_init() stores function pointers and sets up necessary boost variables.
     */
    UartBoost(const data_received_t dataReceived, const data_sent_t dataSent);

    ~UartBoost();

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

    /**@brief clears buffers.
     */
    uint32_t flush();

private:

    /**@brief Called when background thread receives bytes from uart.
     */
    void readHandler(const boost::system::error_code &errorCode, const size_t bytesTransferred);

    /**@brief Called when write is finished doing asynchronous write.
     */
    void writeHandler(const boost::system::error_code &errorCode, const size_t);

    /**@brief Starts asynchronous read on a background thread.
     */
    void startRead();

    /**@brief Starts an asynchronous read.
     */
    void asyncRead();

    /**@brief Starts an asynchronous write.
     */
    void asyncWrite();

    boost::asio::io_service ioService;
    boost::asio::serial_port serialPort;
    boost::asio::io_service::work workNotifier;
    boost::thread ioWorkThread;

    boost::array<uint8_t, BUFFER_SIZE> readBuffer;
    std::vector<uint8_t> writeBufferVector;
    std::deque<uint8_t> writeQueue;
    boost::mutex queueMutex;

    boost::function<void(const boost::system::error_code, const size_t)> callbackReadHandle;
    boost::function<void(const boost::system::error_code, const size_t)> callbackWriteHandle;

    bool asyncWriteInProgress;
};

#endif //UART_BOOST_H
