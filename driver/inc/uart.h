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

#ifndef UART_H
#define UART_H

#include "uart_settings.h"

#include <stdint.h>

/* Function pointer types for callbacks */
typedef void (*data_sent_t)(uint32_t error_code, uint16_t length);
typedef void (*data_received_t)(uint32_t error_code, uint8_t *data, uint16_t length);

/**
 * @brief The Uart class is a virtual class used by app_uart to communicate by uart (from pc).
 * Implemented by UartBoost and UartTest.
 */
class Uart
{
    public:

        /**@brief Is called by app_uart_init() stores function pointers and sets up necessary variables.
         */
        Uart(const data_received_t dataReceived, const data_sent_t dataSent);

        /**@brief Calls close().
         */
        virtual ~Uart();

        /**@brief Setup of serial port service with parameter data.
         */
        virtual uint32_t open(const UartCommunicationParameters &communicationParameters) = 0;

        /**@brief returns true if open.
         */
        virtual bool isOpen() const = 0;

        /**@brief Closes the serial port service.
         */
        virtual uint32_t close() = 0;

        /**@brief sends data to serial port to write.
         */
        virtual uint32_t write(const uint8_t *data, uint16_t length) = 0;

        /**@brief sends data to serial port to write.
         */
        virtual uint32_t write(const uint8_t data) = 0;

        /**@brief clears buffers.
         */
        virtual uint32_t flush() = 0;

    protected:

        data_received_t dataReceivedHandle;
        data_sent_t dataSentHandle;

    private:

        /**@brief Registers callback functions.
         */
        void registerCallbacks(const data_received_t dataReceived, const data_sent_t dataSent);

};

#endif //UART_H
