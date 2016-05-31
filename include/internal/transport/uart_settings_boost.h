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

#ifndef UART_SETTINGS_BOOST_H
#define UART_SETTINGS_BOOST_H

#include "uart_settings.h"

#include <boost/asio/serial_port.hpp>

/**
 * @brief Extension class to UartSettings. Extra functions for returning Boost variables.
 */
class UartSettingsBoost : public UartSettings
{
public:

    /*@brief Default contructor. */
    UartSettingsBoost();

    /*@brief Constructor that initializes with communication parameters. */
    UartSettingsBoost(const UartCommunicationParameters &communicationParameters);

    /*@brief Returns baud rate readable by Boost. */
    boost::asio::serial_port::baud_rate getBoostBaudRate() const;

    /*@brief Returns flow control in Boost format. */
    boost::asio::serial_port::flow_control getBoostFlowControl() const;

    /*@brief Returns parity in Boost format. */
    boost::asio::serial_port::parity getBoostParity() const;

    /*@brief Returns stop bits in Boost format. */
    boost::asio::serial_port::stop_bits getBoostStopBits() const;

    /*@brief Returns character size in Boost format. */
    boost::asio::serial_port::character_size getBoostCharacterSize() const;

};

#endif //UART_SETTINGS_BOOST_H
