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

#include "transport.h"
#include "uart_settings_boost.h"
#include "uart_defines.h"

#include <boost/array.hpp>
#include <boost/thread.hpp>

#include <deque>
#include <mutex>

#include <stdint.h>

/**
 * @brief The UartBoost class opens, reads and writes a serial port using the boost asio library
 */
class UartBoost : public Transport
{
public:

    /**@brief Is called by app_uart_init() stores function pointers and sets up necessary boost variables.
     */
    UartBoost(const UartCommunicationParameters &communicationParameters);

    ~UartBoost();

    /**@brief Setup of serial port service with parameter data.
     */
    uint32_t open(status_cb_t status_callback, data_cb_t data_callback, log_cb_t log_callback);

    /**@brief Closes the serial port service.
     */
    uint32_t close();

    /**@brief sends data to serial port to write.
     */
    uint32_t send(std::vector<uint8_t> &data);

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
    std::mutex queueMutex;

    boost::function<void(const boost::system::error_code, const size_t)> callbackReadHandle;
    boost::function<void(const boost::system::error_code, const size_t)> callbackWriteHandle;

    bool asyncWriteInProgress;
    UartSettingsBoost uartSettingsBoost;
};

#endif //UART_BOOST_H
