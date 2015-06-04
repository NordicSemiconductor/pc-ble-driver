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

#ifndef SER_CONFIG_APP_HAL_NRF51_H__
#define SER_CONFIG_APP_HAL_NRF51_H__

#include "ser_config.h"

#define CONN_CHIP_RESET_PIN_NO                  30                  /**< Pin used for reseting the nRF51822. */

/* UART configuration */
#define UART_IRQ_PRIORITY                       APP_IRQ_PRIORITY_LOW
#define SER_PHY_UART_RX                         0
#define SER_PHY_UART_TX                         0
#define SER_PHY_UART_CTS                        0
#define SER_PHY_UART_RTS                        0

#endif //SER_CONFIG_APP_HAL_NRF51_H__
