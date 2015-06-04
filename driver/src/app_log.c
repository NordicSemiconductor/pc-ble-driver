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

#include "app_log.h"
#include "nrf_error.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost;

static app_log_handler_t  m_log_handler_function = NULL;
static char               m_log_file_path[512] = {0};
static app_log_severity_t m_log_handler_severity_filter = APP_LOG_INFO;
static app_log_severity_t m_log_file_severity_filter = APP_LOG_TRACE;
static mutex              m_log_handler_mutex;
static mutex              m_log_file_mutex;

void app_log_byte_array_handler(const app_log_severity_t severity, const char * log_message,
                                const uint8_t * byte_array, const uint16_t array_length)
{
    bool handler_filter_accepted = severity >= m_log_handler_severity_filter;
    bool file_filter_accepted = severity >= m_log_file_severity_filter;

    if (!handler_filter_accepted && ! file_filter_accepted)
    {
        return;
    }

    uint32_t buffer_size = 50 + array_length * 5;
    char array_buffer[buffer_size];
    char *buffer = array_buffer;
    uint32_t remaining_buffer = buffer_size;

    for (int i = 0; i < array_length; i++)
    {
        if (i > 0)
        {
            // Insert space between values
            buffer += snprintf(buffer, remaining_buffer, " ");
            remaining_buffer = buffer_size - (buffer - array_buffer);
        }
        buffer += snprintf(buffer, remaining_buffer, "0x%02X", byte_array[i]);
        remaining_buffer = buffer_size - (buffer - array_buffer);
    }

    app_log_handler(severity, log_message, array_buffer);
}


void app_log_handler(app_log_severity_t severity, const char * log_message, ...)
{
    bool has_log_handler = m_log_handler_function != NULL;
    bool has_log_file_name = m_log_file_path[0] != '\0';

    if (!has_log_handler && !has_log_file_name)
    {
        return;
    }

    bool handler_filter_accepted = severity >= m_log_handler_severity_filter;
    bool file_filter_accepted = severity >= m_log_file_severity_filter;

    if (!handler_filter_accepted && ! file_filter_accepted)
    {
        return;
    }

    char buffer[256];
    va_list args;
    va_start(args, log_message);
    vsnprintf(buffer, sizeof(buffer), log_message, args);
    va_end(args);

    if (has_log_handler && handler_filter_accepted)
    {
        lock_guard<mutex> guard(m_log_handler_mutex);
        m_log_handler_function(severity, buffer);
    }

    if (has_log_file_name && file_filter_accepted)
    {
        lock_guard<mutex> guard(m_log_file_mutex);
        FILE * p_file;
        p_file = fopen(m_log_file_path, "a+");

        if (p_file != NULL)
        {
            /* add timestamp */
            posix_time::ptime local_time = posix_time::microsec_clock::local_time();
            posix_time::time_duration clock_time = local_time.time_of_day();
            std::string clock_time_string = to_simple_string(clock_time);
            const char *c_string = clock_time_string.c_str();

            fprintf(p_file, "[%s]: %s\n", c_string, buffer);
            fclose(p_file);
        }
    }
}

uint32_t app_log_handler_set(app_log_handler_t log_handler)
{
    m_log_handler_function = log_handler;

    return NRF_SUCCESS;
}

uint32_t app_log_file_path_set(const char *file_path)
{
    if (file_path == NULL)
    {
        m_log_file_path[0] = '\0';
        return NRF_ERROR_INVALID_PARAM;
    }

    FILE * p_file;
    p_file = fopen(file_path, "a+");

    if (p_file == NULL)
    {
        m_log_file_path[0] = '\0';
        return NRF_ERROR_INVALID_PARAM;
    }

    strcpy(m_log_file_path, file_path);
    fclose(p_file);

    return NRF_SUCCESS;
}

uint32_t app_log_handler_severity_filter_set(app_log_severity_t severity_filter)
{
    if (severity_filter < APP_LOG_TRACE || APP_LOG_FATAL < severity_filter)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    m_log_handler_severity_filter = severity_filter;
    return NRF_SUCCESS;
}

uint32_t app_log_file_severity_filter_set(app_log_severity_t severity_filter)
{
    if (severity_filter < APP_LOG_TRACE || APP_LOG_FATAL < severity_filter)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    m_log_file_severity_filter = severity_filter;

    return NRF_SUCCESS;
}
