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

#ifndef SER_CONFIG_H__
#define SER_CONFIG_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * General parameters configuration.
 **************************************************************************************************/

/** Value used as error code on SoftDevice stack dump. Can be used to identify stack location on
 *  stack unwind.*/
#define SER_SD_ERROR_CODE    (uint32_t)(0xDEADBEEF)

/** Value used as error code indicating warning - unusual situation but not critical so system
 *  should NOT be reseted. */
#define SER_WARNING_CODE     (uint32_t)(0xBADDCAFE)

/***********************************************************************************************//**
 * HAL Transport layer configuration.
 **************************************************************************************************/

/** Max packets size in serialization HAL Transport layer (packets before adding PHY header i.e.
 *  packet length). */
#define SER_HAL_TRANSPORT_APP_TO_CONN_MAX_PKT_SIZE    (uint32_t)(384)
#define SER_HAL_TRANSPORT_CONN_TO_APP_MAX_PKT_SIZE    (uint32_t)(384)

#define SER_HAL_TRANSPORT_MAX_PKT_SIZE ((SER_HAL_TRANSPORT_APP_TO_CONN_MAX_PKT_SIZE) >= \
                                        (SER_HAL_TRANSPORT_CONN_TO_APP_MAX_PKT_SIZE)    \
                                        ?                                               \
                                        (SER_HAL_TRANSPORT_APP_TO_CONN_MAX_PKT_SIZE) :  \
                                        (SER_HAL_TRANSPORT_CONN_TO_APP_MAX_PKT_SIZE))
#ifdef SER_CONNECTIVITY
    #define SER_HAL_TRANSPORT_TX_MAX_PKT_SIZE         SER_HAL_TRANSPORT_CONN_TO_APP_MAX_PKT_SIZE
    #define SER_HAL_TRANSPORT_RX_MAX_PKT_SIZE         SER_HAL_TRANSPORT_APP_TO_CONN_MAX_PKT_SIZE

#else /* APPLICATION SIDE */
    #define SER_HAL_TRANSPORT_TX_MAX_PKT_SIZE         SER_HAL_TRANSPORT_APP_TO_CONN_MAX_PKT_SIZE
    #define SER_HAL_TRANSPORT_RX_MAX_PKT_SIZE         SER_HAL_TRANSPORT_CONN_TO_APP_MAX_PKT_SIZE
#endif /* SER_CONNECTIVITY */


/***********************************************************************************************//**
 * SER_PHY layer configuration.
 **************************************************************************************************/

#define SER_PHY_HEADER_SIZE             2

/** Max transfer unit for SPI MASTER and SPI SLAVE. */
#define SER_PHY_SPI_MTU_SIZE            255

/** UART transmission parameters */
#define SER_PHY_UART_FLOW_CTRL          APP_UART_FLOW_CONTROL_ENABLED
#define SER_PHY_UART_PARITY             true
#define SER_PHY_UART_BAUDRATE           UART_BAUDRATE_BAUDRATE_Baud1M

/** Find UART baudrate value based on chosen register setting. */
#if (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud1200)
    #define SER_PHY_UART_BAUDRATE_VAL 1200uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud2400)
    #define SER_PHY_UART_BAUDRATE_VAL 2400uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud4800)
    #define SER_PHY_UART_BAUDRATE_VAL 4800uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud9600)
    #define SER_PHY_UART_BAUDRATE_VAL 9600uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud14400)
    #define SER_PHY_UART_BAUDRATE_VAL 14400uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud19200)
    #define SER_PHY_UART_BAUDRATE_VAL 19200uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud28800)
    #define SER_PHY_UART_BAUDRATE_VAL 28800uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud38400)
    #define SER_PHY_UART_BAUDRATE_VAL 38400uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud57600)
    #define SER_PHY_UART_BAUDRATE_VAL 57600uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud76800)
    #define SER_PHY_UART_BAUDRATE_VAL 76800uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud115200)
    #define SER_PHY_UART_BAUDRATE_VAL 115200uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud230400)
    #define SER_PHY_UART_BAUDRATE_VAL 230400uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud250000)
    #define SER_PHY_UART_BAUDRATE_VAL 250000uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud460800)
    #define SER_PHY_UART_BAUDRATE_VAL 460800uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud921600)
    #define SER_PHY_UART_BAUDRATE_VAL 921600uL
#elif (SER_PHY_UART_BAUDRATE == UART_BAUDRATE_BAUDRATE_Baud1M)
    #define SER_PHY_UART_BAUDRATE_VAL 1000000uL
#endif /* SER_PHY_UART_BAUDRATE */

/** Configuration timeouts of connectivity MCU */
#define CONN_CHIP_RESET_TIME            50      /**< The time to keep the reset line to the nRF51822 low (in milliseconds). */
#define CONN_CHIP_WAKEUP_TIME           500     /**< The time for nRF51822 to reset and become ready to receive serialized commands (in milliseconds). */

#define SER_MAX_CONNECTIONS 1

#ifdef __cplusplus
}
#endif
#endif /* SER_CONFIG_H__ */
