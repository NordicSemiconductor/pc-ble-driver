#pragma once

#include <cstdint>
#include <string>

/*@brief Flow control modes. */
typedef enum {
    UartFlowControlNone,
    UartFlowControlSoftware,
    UartFlowControlHardware
} UartFlowControl;

/*@brief Parity modes. */
typedef enum { UartParityNone, UartParityOdd, UartParityEven } UartParity;

/*@brief Stop bit modes. */
typedef enum { UartStopBitsOne, UartStopBitsOnePointFive, UartStopBitsTwo } UartStopBits;

/*@brief Data bit modes. */
typedef enum {
    UartDataBitsFive  = 5,
    UartDataBitsSix   = 6,
    UartDataBitsSeven = 7,
    UartDataBitsEight = 8
} UartDataBits;

/*@brief UART communication parameters. */
typedef struct
{
    const char *portName;
    uint32_t baudRate;
    UartFlowControl flowControl;
    UartParity parity;
    UartStopBits stopBits;
    UartDataBits dataBits;
} UartCommunicationParameters;

/**
 * @brief The UartSettings class parses a uart_comm_params_t struct and gives out corresponding
 * boost enum values for use in the UartTransport class.
 */
class UartSettings
{
  public:
    /*@brief Constructor. */
    UartSettings();

    /*@brief Constructor that initializes with communication paramters. */
    UartSettings(const UartCommunicationParameters &communicationParameters);

    /*@brief Destructor. */
    virtual ~UartSettings();

    /*@brief Sets the name of the serial port. */
    void setPortName(const std::string value);

    /*@brief Sets the baud rate. */
    void setBaudRate(const uint32_t value);

    /*@brief Sets the flow control parameter. */
    void setFlowControl(const UartFlowControl value);

    /*@brief Sets the Parity parameter. */
    void setParity(const UartParity value);

    /*@brief Sets the Stop Bits parameter. */
    void setStopBits(const UartStopBits value);

    /*@brief Sets the Data Bits parameter. */
    void setDataBits(const UartDataBits value);

    /*@brief Returns the currently configured serial port name. */
    std::string getPortName();

    /*@brief Returns the currently configured Baud Rate. */
    uint32_t getBaudRate() const;

    /*@brief Returns the currently configured Flow Control setting. */
    UartFlowControl getFlowControl() const;

    /*@brief Returns the currently configured Parity setting. */
    UartParity getParity() const;

    /*@brief Returns the currently configured Stop Bits setting. */
    UartStopBits getStopBits() const;

    /*@brief Returns the currently configured Data Bits setting. */
    UartDataBits getDataBits() const;

  protected:
    std::string portName;
    uint32_t baudRate;
    UartFlowControl flowControl;
    UartParity parity;
    UartStopBits stopBits;
    UartDataBits dataBits;
};
