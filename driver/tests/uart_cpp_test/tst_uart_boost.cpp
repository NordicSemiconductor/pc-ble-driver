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

#include <stdlib.h>
#include <vector>
#include <algorithm>

#define BOOST_TEST_MODULE tst_uart_boost
#include <boost/test/unit_test.hpp>

#include <boost/format.hpp>
#include <boost/thread.hpp>

#include "uart_test.h"
#include "uart_boost.h"
#include "nrf_error.h"

// Forward declarations -- BEGIN --
void open(Uart &uart, char *portName, UartCommunicationParameters &parameters);
void close(Uart &uart);
void isOpenTest(Uart &uart, bool expectedState, std::string message);

void data_received(uint32_t error_code, uint8_t *data, uint16_t length);
void data_sent(uint32_t error_code, uint16_t length);

// Forward declarations -- END --

// Variables used during testing -- BEGIN --
boost::mutex waitMutex;
boost::condition_variable_any waitCondition;

char *port1;
char *port2;
uint8_t *dataBuffer = 0;
uint32_t bufferIndex = 0;
size_t bufferLength = 0;

// Variables used during testing -- END --

// Suite fixture setup
struct SuiteFixture
{
    SuiteFixture()
    {
        port1 = getenv("TESTSERIALPORT1");
        port2 = getenv("TESTSERIALPORT2");

        if(port1 == NULL) port1 = (char*)"COM1";
        if(port2 == NULL) port2 = (char*)"COM2";
    }

    ~SuiteFixture() {
    }
};

// Test vector structures used during tests
struct connect_serial_port_data {
    char* port1NameString;
    char* port2NameString;
    char* actualPortNameString;
    UartCommunicationParameters* parametersVoid;
    uint32_t openExpectedReturnValue;
    bool isOpenExpectedReturnValue;
};

std::vector<connect_serial_port_data*> connect_serial_port_data_test_vector;

struct open_already_open_port_data {
    char* port1NameString;
    UartCommunicationParameters* parametersVoid;
};

std::vector<open_already_open_port_data*> open_already_open_port_data_test_vector;

struct serial_port_data {
   char* port1NameString;
   char* port2NameString;
   UartCommunicationParameters* parametersVoid;
   void *testDataVoid;
   uint16_t testDataLength;
};

std::vector<serial_port_data *> read_serial_port_data_test_vector;
std::vector<serial_port_data *> write_serial_port_data_test_vector;

BOOST_FIXTURE_TEST_SUITE( tst_uart_boost, SuiteFixture )

BOOST_AUTO_TEST_CASE( connectToSerialPort )
{
    // Setup test data
    connect_serial_port_data *t;
    UartCommunicationParameters *p;

    // Test #1
    t = new connect_serial_port_data();
    p = new UartCommunicationParameters;
    p->baudRate = 9600;
    p->flowControl = UartFlowControlNone;
    p->parity = UartParityNone;
    p->stopBits = UartStopBitsTwo;
    p->dataBits = UartDataBitsEight;
    t->parametersVoid = p;

    t->port1NameString = port1;
    t->port2NameString = port2;
    t->actualPortNameString = port1;
    t->openExpectedReturnValue = (uint32_t)NRF_SUCCESS;
    t->isOpenExpectedReturnValue = true;

    connect_serial_port_data_test_vector.push_back(t);

    // Test #2
    t = new connect_serial_port_data();
    p = new UartCommunicationParameters;
    p->baudRate = 19200;
    p->flowControl = UartFlowControlHardware;
    p->parity = UartParityEven;
    p->stopBits = UartStopBitsTwo;
    p->dataBits = UartDataBitsEight;
    t->parametersVoid = p;

    t->port1NameString = port1;
    t->port2NameString = port2;
    t->actualPortNameString = port2;
    t->openExpectedReturnValue = (uint32_t)NRF_SUCCESS;
    t->isOpenExpectedReturnValue = true;

    connect_serial_port_data_test_vector.push_back(t);

    // Test #3
    t = new connect_serial_port_data();
    p = new UartCommunicationParameters;
    p->baudRate = 19200;
    p->flowControl = UartFlowControlHardware;
    p->parity = UartParityEven;
    p->stopBits = UartStopBitsTwo;
    p->dataBits = UartDataBitsEight;
    t->parametersVoid = p;

    t->port1NameString = port1;
    t->port2NameString = port2;
    t->actualPortNameString = (char*)"COM117";
    t->openExpectedReturnValue = (uint32_t)NRF_ERROR_INTERNAL;
    t->isOpenExpectedReturnValue = false;

    connect_serial_port_data_test_vector.push_back(t);

    // Iterate over test vector
    for (std::vector<connect_serial_port_data*>::iterator it = connect_serial_port_data_test_vector.begin();
         it != connect_serial_port_data_test_vector.end();
         ++it) {
        const char *port1Name = (*it)->port1NameString;
        const char *port2Name = (*it)->port2NameString;

        UartCommunicationParameters *parameters = (*it)->parametersVoid;
        parameters->portName = (*it)->actualPortNameString;

        UartBoost uartBoost(data_received, data_sent);

        isOpenTest(uartBoost, false, "UartBoost is somehow already open.");

        uint32_t uartOpened = uartBoost.open(*parameters);

        boost::format message = boost::format("Open failed for UartBoost. %2% %3% . Return value: %1%.")
                  % uartOpened
                  % port1Name
                  % port2Name;
        BOOST_CHECK_MESSAGE(uartOpened == (*it)->openExpectedReturnValue, message);

        isOpenTest(uartBoost, (*it)->isOpenExpectedReturnValue, "UartBoost not in correct state after open.");

        close(uartBoost);

        // Free memory used by this test run.
        delete (*it)->parametersVoid;
        delete (*it);
    }
}


BOOST_AUTO_TEST_CASE( openAlreadyOpenPort )
{
    // Setup test data
    open_already_open_port_data *t;
    UartCommunicationParameters *p;

    // Test #1
    t = new open_already_open_port_data;
    p = new UartCommunicationParameters;
    p->portName = port1;
    p->baudRate = 9600;
    p->flowControl = UartFlowControlNone;
    p->parity = UartParityNone;
    p->stopBits = UartStopBitsTwo;
    p->dataBits = UartDataBitsEight;
    t->parametersVoid = p;
    t->port1NameString = port1;

    open_already_open_port_data_test_vector.push_back(t);

    // Test #2
    p = new UartCommunicationParameters;
    p->portName = port1;
    p->baudRate = 19200;
    p->flowControl = UartFlowControlHardware;
    p->parity = UartParityEven;
    p->stopBits = UartStopBitsTwo;
    p->dataBits = UartDataBitsEight;
    t->parametersVoid = p;
    t->port1NameString = port1;


    // Iterate over test vector
    for (std::vector<open_already_open_port_data*>::iterator it = open_already_open_port_data_test_vector.begin();
         it != open_already_open_port_data_test_vector.end();
         ++it)
    {
        char *port1Name = (*it)->port1NameString;
        UartCommunicationParameters *parameters = (*it)->parametersVoid;

        UartBoost uartBoost(data_received, data_sent);
        open(uartBoost, port1Name, *parameters);

        uint32_t uartOpened = uartBoost.open(*parameters);

        close(uartBoost);

        delete parameters;

        boost::format message;
        message = boost::format("Open had unexpected success for already open port: %2%. Return value: %1%.")
                  % uartOpened
                  % port1;

        BOOST_CHECK_MESSAGE(uartOpened == NRF_ERROR_INVALID_STATE, message);
    }
}


BOOST_AUTO_TEST_CASE( readSerialPort )
{
    // Setup test data
    serial_port_data *t;
    UartCommunicationParameters *p;
    uint16_t dataLength;
    uint8_t *data;

    // Test #1
    t = new serial_port_data();

    t->port1NameString = port1;
    t->port2NameString = port2;

    p = new UartCommunicationParameters;
    p->baudRate = 9600;
    p->flowControl = UartFlowControlNone;
    p->parity = UartParityNone;
    p->stopBits = UartStopBitsTwo;
    p->dataBits = UartDataBitsEight;
    t->parametersVoid = p;

    dataLength = BUFFER_SIZE;
    data = new uint8_t[dataLength];
    for (uint32_t i = 0; i < dataLength; i++)
    {
        data[i] = (uint8_t)(i % 256);
    }

    t->testDataVoid = data;
    t->testDataLength = dataLength;

    read_serial_port_data_test_vector.push_back(t);

    // Test #2
    t = new serial_port_data();

    t->port1NameString = port1;
    t->port2NameString = port2;

    p = new UartCommunicationParameters;
    p->baudRate = 19200;
    p->flowControl = UartFlowControlHardware;
    p->parity = UartParityEven;
    p->stopBits = UartStopBitsTwo;
    p->dataBits = UartDataBitsEight;
    t->parametersVoid = p;

    dataLength = BUFFER_SIZE;
    data = new uint8_t[dataLength];
    for (uint32_t i = 0; i < dataLength; i++)
    {
        data[i] = (uint8_t)(i % 256);
    }

    t->testDataVoid = data;
    t->testDataLength = dataLength;

    read_serial_port_data_test_vector.push_back(t);

    // Test #3
    t = new serial_port_data();

    t->port1NameString = port1;
    t->port2NameString = port2;

    p = new UartCommunicationParameters;
    p->baudRate = 19200;
    p->flowControl = UartFlowControlHardware;
    p->parity = UartParityEven;
    p->stopBits = UartStopBitsTwo;
    p->dataBits = UartDataBitsEight;
    t->parametersVoid = p;

    dataLength = BUFFER_SIZE;
    data = new uint8_t[dataLength];
    for (uint32_t i = 0; i < dataLength; i++)
    {
        data[i] = (uint8_t)(i % 256);
    }

    t->testDataVoid = data;
    t->testDataLength = dataLength;

    read_serial_port_data_test_vector.push_back(t);

    // Iterate over test vector
    for (std::vector<serial_port_data *>::iterator it = read_serial_port_data_test_vector.begin();
         it != read_serial_port_data_test_vector.end();
         ++it)
    {
        char *port1Name = (*it)->port1NameString;
        char *port2Name = (*it)->port2NameString;

        UartCommunicationParameters *parameters = (*it)->parametersVoid;
        uint8_t *testData = (uint8_t *) (*it)->testDataVoid;

        UartBoost uartBoost(data_received, data_sent);
        UartTest uartTest(data_received, data_sent);

        dataBuffer = new uint8_t[(*it)->testDataLength];
        memset(dataBuffer, 0, (*it)->testDataLength);
        bufferLength = (*it)->testDataLength;

        open(uartBoost, port1Name, *parameters);
        open(uartTest, port2Name, *parameters);

        waitMutex.lock();

        uartTest.write(testData, (*it)->testDataLength);

        bool waitTimeout = waitCondition.timed_wait(waitMutex, boost::posix_time::milliseconds(10000));
        waitMutex.unlock();

        bool testSuccess = std::equal(testData, testData + (*it)->testDataLength, dataBuffer);

        delete parameters;
        delete testData;
        delete dataBuffer;

        close(uartBoost);
        close(uartTest);

        BOOST_CHECK_MESSAGE(waitTimeout, "Timeout on read.");
        BOOST_CHECK_MESSAGE(testSuccess, "Failed to read correct data from uart.");
    }
}

BOOST_AUTO_TEST_CASE( writeSerialPort )
{
    // Setup test data
    serial_port_data *t;
    UartCommunicationParameters *p;
    uint16_t dataLength;
    uint8_t *data;

    // Test #1
    t = new serial_port_data;

    t->port1NameString = port1;
    t->port2NameString = port2;

    p = new UartCommunicationParameters;
    p->baudRate = 9600;
    p->flowControl = UartFlowControlNone;
    p->parity = UartParityEven;
    p->stopBits = UartStopBitsTwo;
    p->dataBits = UartDataBitsEight;
    t->parametersVoid = p;

    dataLength = BUFFER_SIZE;
    data = new uint8_t[dataLength];
    for (uint32_t i = 0; i < dataLength; i++)
    {
        data[i] = (uint8_t)(i % 256);
    }

    t->testDataVoid = data;
    t->testDataLength = dataLength;

    write_serial_port_data_test_vector.push_back(t);

    // Test #2
    t = new serial_port_data();

    t->port1NameString = port1;
    t->port2NameString = port2;

    p = new UartCommunicationParameters;
    p->baudRate = 19200;
    p->flowControl = UartFlowControlHardware;
    p->parity = UartParityEven;
    p->stopBits = UartStopBitsTwo;
    p->dataBits = UartDataBitsEight;
    t->parametersVoid = p;

    dataLength = BUFFER_SIZE;
    data = new uint8_t[dataLength];
    for (uint32_t i = 0; i < dataLength; i++)
    {
        data[i] = (uint8_t)(i % 256);
    }

    t->testDataVoid = data;
    t->testDataLength = dataLength;

    // Test #3
    t = new serial_port_data;

    t->port1NameString = port1;
    t->port2NameString = port2;

    p = new UartCommunicationParameters;
    p->baudRate = 19200;
    p->flowControl = UartFlowControlHardware;
    p->parity = UartParityEven;
    p->stopBits = UartStopBitsTwo;
    p->dataBits = UartDataBitsEight;
    t->parametersVoid = p;

    dataLength = BUFFER_SIZE;
    data = new uint8_t[dataLength];
    for (uint32_t i = 0; i < dataLength; i++)
    {
        data[i] = (uint8_t)(i % 256);
    }

    t->testDataVoid = data;
    t->testDataLength = dataLength;

    write_serial_port_data_test_vector.push_back(t);

    // Iterate over test vector
    for (std::vector<serial_port_data *>::iterator it = write_serial_port_data_test_vector.begin();
         it != write_serial_port_data_test_vector.end();
         ++it)
    {
        char *port1Name = (*it)->port1NameString;
        char *port2Name = (*it)->port2NameString;

        UartCommunicationParameters *parameters = (*it)->parametersVoid;
        uint8_t *testData = (uint8_t *) (*it)->testDataVoid;

        UartBoost uartBoost(data_received, data_sent);
        UartTest uartTest(data_received, data_sent);

        dataBuffer = new uint8_t[(*it)->testDataLength];
        memset(dataBuffer, 0, (*it)->testDataLength);
        bufferLength = (*it)->testDataLength;

        open(uartBoost, port1Name, *parameters);
        open(uartTest, port2Name, *parameters);

        uartBoost.write(testData, (*it)->testDataLength);
        uartTest.read(dataBuffer, (*it)->testDataLength);

        bool testSuccess = std::equal(testData, testData + (*it)->testDataLength, dataBuffer);

        delete parameters;
        delete testData;
        delete dataBuffer;

        close(uartBoost);
        close(uartTest);

        BOOST_CHECK_MESSAGE(testSuccess, "Failed to read correct data from uart.");
    }
}

BOOST_AUTO_TEST_SUITE_END()

void data_received(uint32_t error_code, uint8_t *data, uint16_t length)
{
    if (error_code != NRF_SUCCESS)
    {
        return;
    }

    memcpy(&dataBuffer[bufferIndex], data, length);
    bufferIndex += length;

    if (bufferIndex == bufferLength)
    {
        bufferIndex = 0;
        waitMutex.lock();
        waitCondition.notify_all();
        waitMutex.unlock();
    }
}

void data_sent(uint32_t error_code, uint16_t length)
{
    if (error_code != 0)
    {
        std::cout << "data_sent received error code: " << error_code;
        return;
    }
}

void open(Uart &uart, char *portName, UartCommunicationParameters &parameters)
{
    boost::format message;
    uint32_t uartOpened;
    parameters.portName = portName;

    isOpenTest(uart, false, "UartBoost is somehow already open.");

    uartOpened = uart.open(parameters);

    message = boost::format("Open failed for UartBoost. %2%. Return value: %1%.") % uartOpened % portName;
    BOOST_CHECK_MESSAGE(uartOpened == NRF_SUCCESS, message);

    isOpenTest(uart, true, "UartBoost not in correct state after open.");
}

void close(Uart &uart)
{
    uart.close();
    isOpenTest(uart, false, "Port did not close");

    // Sleep a bit to let the serial port really close
    // We experienced problems on Windows if a serial port is opened too fast after a close
    boost::this_thread::sleep(boost::posix_time::milliseconds(400));
}

void isOpenTest(Uart &uart, bool expectedState, std::string message)
{
    bool uartIsOpen = uart.isOpen();
    BOOST_CHECK_MESSAGE(uartIsOpen == expectedState, message.c_str());
}
