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

#ifndef UART_DEFINES_H
#define UART_DEFINES_H

/**@brief Set TEST_VERBOSE_LOGGING to 1 to enable logging of buffers.
 */
#define TEST_VERBOSE_LOGGING 0

/**@brief Controls the buffer sizes for all read and write buffers as well as the number of
 * bytes we actually read and write
 */
#define BUFFER_SIZE 256
#define BUFFER_SIZE_LARGE 8192

#endif // UARTDEFINES_H
