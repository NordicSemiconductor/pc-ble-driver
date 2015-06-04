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

#include "uart_settings_boost.h"

#include <vector>

#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>


// Variables/structs used during testing
UartSettingsBoost settings;

struct stop_bits_data {
    UartStopBits stop_bits;
    boost::asio::serial_port_base::stop_bits boost_stop_bits;
};

std::vector<stop_bits_data*> stop_bits_test_vector;

struct flow_control_data {
    UartFlowControl flow_control;
    boost::asio::serial_port_base::flow_control boost_flow_control;
};

std::vector<flow_control_data*> flow_control_test_vector;

struct parity_data {
    UartParity parity;
    boost::asio::serial_port_base::parity boost_parity;
};

std::vector<parity_data*> parity_test_vector;

struct data_bits_data {
    UartDataBits data_bits;
    boost::asio::serial_port_base::character_size boost_data_bits;
};

std::vector<data_bits_data*> data_bits_test_vector;

// BOOST_FIXTURE_TEST_SUITE( tst_uart_settings, SuiteFixture )

BOOST_AUTO_TEST_CASE( stopBits )
{
    // Setup test data
    stop_bits_data *t;

    // Test #1
    t = new stop_bits_data();
    t->stop_bits = UartStopBitsOne;
    t->boost_stop_bits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one);

    // Test #2
    t = new stop_bits_data();
    t->stop_bits = UartStopBitsOnePointFive;
    t->boost_stop_bits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::onepointfive);

    // Test #3
    t = new stop_bits_data();
    t->stop_bits = UartStopBitsTwo;
    t->boost_stop_bits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::two);

    // Iterate over test vector
    for (std::vector<stop_bits_data*>::iterator it = stop_bits_test_vector.begin();
         it != stop_bits_test_vector.end();
         ++it) {
        BOOST_CHECK_EQUAL((int) (*it)->stop_bits, (int) (*it)->boost_stop_bits.value());

        settings.setStopBits((*it)->stop_bits);
        UartStopBits resultStopBits = settings.getStopBits();
        boost::asio::serial_port_base::stop_bits resultBoostStopBits(settings.getBoostStopBits());

        BOOST_CHECK_EQUAL((*it)->stop_bits, resultStopBits);
        BOOST_CHECK_EQUAL((*it)->boost_stop_bits.value(), resultBoostStopBits.value());
    }
}

BOOST_AUTO_TEST_CASE( flowControl )
{
    // Setup test data
    flow_control_data *t;

    // Test #1
    t = new flow_control_data();
    t->flow_control = UartFlowControlNone;
    t->boost_flow_control = boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none);

    flow_control_test_vector.push_back(t);

    // Test #2
    t = new flow_control_data();
    t->flow_control = UartFlowControlSoftware;
    t->boost_flow_control = boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::software);

    flow_control_test_vector.push_back(t);

    // Test #3
    t = new flow_control_data();
    t->flow_control = UartFlowControlHardware;
    t->boost_flow_control = boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::hardware);

    flow_control_test_vector.push_back(t);

    // Iterate over test vector
    for (std::vector<flow_control_data*>::iterator it = flow_control_test_vector.begin();
         it != flow_control_test_vector.end();
         ++it) {
        UartFlowControl flowControlValue((*it)->flow_control);
        settings.setFlowControl(flowControlValue);
        UartFlowControl resultFlowControl = settings.getFlowControl();
        BOOST_CHECK_EQUAL(flowControlValue, resultFlowControl);
        BOOST_CHECK_EQUAL((*it)->boost_flow_control.value(), settings.getBoostFlowControl().value());

    }
}

BOOST_AUTO_TEST_CASE( parity )
{
    // Setup test data
    parity_data *t;

    // Test #1
    t = new parity_data();
    t->parity = UartParityNone;
    t->boost_parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none);

    parity_test_vector.push_back(t);

    // Test #2
    t = new parity_data();
    t->parity = UartParityEven;
    t->boost_parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::even);

    // Test #2
    t = new parity_data();
    t->parity = UartParityOdd;
    t->boost_parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::odd);

    parity_test_vector.push_back(t);

    // Iterate over test vector
    for (std::vector<parity_data*>::iterator it = parity_test_vector.begin();
         it != parity_test_vector.end();
         ++it) {
        UartParity parityValue((*it)->parity);
        boost::asio::serial_port_base::parity boostParity((*it)->boost_parity);

        settings.setParity(parityValue);
        UartParity resultParity = settings.getParity();
        boost::asio::serial_port_base::parity resultBoostParity(settings.getBoostParity());

        BOOST_CHECK_EQUAL(parityValue, resultParity);
        BOOST_CHECK_EQUAL(boostParity.value(), resultBoostParity.value());
    }
}

BOOST_AUTO_TEST_CASE( dataBits )
{
    // Setup test data
    data_bits_data *t;

    // Test #1
    t = new data_bits_data();
    t->data_bits = UartDataBitsFive;
    t->boost_data_bits = boost::asio::serial_port_base::character_size(5);

    data_bits_test_vector.push_back(t);

    // Test #2
    t = new data_bits_data();
    t->data_bits = UartDataBitsSix;
    t->boost_data_bits = boost::asio::serial_port_base::character_size(6);

    data_bits_test_vector.push_back(t);

    // Test #3
    t = new data_bits_data();
    t->data_bits = UartDataBitsSeven;
    t->boost_data_bits = boost::asio::serial_port_base::character_size(7);

    data_bits_test_vector.push_back(t);

    // Test #4
    t = new data_bits_data();
    t->data_bits = UartDataBitsEight;
    t->boost_data_bits = boost::asio::serial_port_base::character_size(8);

    data_bits_test_vector.push_back(t);

    // Iterate over test vector
    for (std::vector<data_bits_data*>::iterator it = data_bits_test_vector.begin();
         it != data_bits_test_vector.end();
         ++it)
    {

        UartDataBits dataBitsValue((*it)->data_bits);
        boost::asio::serial_port_base::character_size boostCharacterSize((*it)->boost_data_bits);

        settings.setDataBits(dataBitsValue);

        UartDataBits resultDataBits = settings.getDataBits();
        boost::asio::serial_port_base::character_size resultBoostCharacterSize(settings.getBoostCharacterSize());

        BOOST_CHECK_EQUAL(dataBitsValue, resultDataBits);
        BOOST_CHECK_EQUAL(boostCharacterSize.value(), resultBoostCharacterSize.value());
    }
}

// BOOST_AUTO_TEST_SUITE_END()

