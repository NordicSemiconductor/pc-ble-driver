/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
#include <string.h>

#define BOOST_TEST_MODULE app_log_test
#include <boost/test/unit_test.hpp>

#include "app_log.h"

static char * last_received_message;
static app_log_severity_t last_received_severity = APP_LOG_TRACE;

void log_handler(app_log_severity_t severity, const char * log_message)
{
    last_received_severity = severity;
    int length = strlen(log_message) + 1;
    memcpy(last_received_message, log_message, length);
}

void log_received_message_clear()
{
    int length = sizeof(last_received_message) / sizeof(last_received_message[0]);
    memset(last_received_message, 0, length);
}

// Global fixture setup
struct MyConfig {
    MyConfig() {
        last_received_message = (char *)malloc(100);
        log_received_message_clear();
        app_log_handler_set(log_handler);
    }

    ~MyConfig() {
        free(last_received_message);
    }
};

BOOST_GLOBAL_FIXTURE(MyConfig);

BOOST_AUTO_TEST_CASE( log_message_test )
{
    const char* test_string = "Severity %d";
    const char* expected_string = "Severity 2";
    app_log_severity_t severity = APP_LOG_INFO;

    app_log_handler(severity, test_string, severity);

    int compare = strcmp(expected_string, last_received_message);
    BOOST_CHECK_EQUAL(0, compare);
}

BOOST_AUTO_TEST_CASE( log_test )
{
    app_log_handler((app_log_severity_t)APP_LOG_INFO, "A log message");
    BOOST_CHECK_EQUAL(0, strcmp("A log message", last_received_message));

    app_log_handler((app_log_severity_t)100, "");
    BOOST_CHECK_EQUAL(0, strcmp("", last_received_message));
}