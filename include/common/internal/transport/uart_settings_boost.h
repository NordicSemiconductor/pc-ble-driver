
#pragma once

#include "uart_settings.h"
#include <asio/serial_port.hpp>

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
    asio::serial_port::baud_rate getBoostBaudRate() const;

    /*@brief Returns flow control in Boost format. */
    asio::serial_port::flow_control getBoostFlowControl() const;

    /*@brief Returns parity in Boost format. */
    asio::serial_port::parity getBoostParity() const;

    /*@brief Returns stop bits in Boost format. */
    asio::serial_port::stop_bits getBoostStopBits() const;

    /*@brief Returns character size in Boost format. */
    asio::serial_port::character_size getBoostCharacterSize() const;
};
