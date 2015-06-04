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

#include "app_log.h"

#include <stdio.h>
#include <stdarg.h>

void app_log_byte_array_handler(const app_log_severity_t severity, const char *log_message,
                           const uint8_t *byte_array, const uint16_t array_length)
{
    int i;
    char array_buffer[50 + array_length*5];
    char *buffer = array_buffer;

    if (severity < APP_LOG_DEBUG)
    {
        return;
    }


    for (i = 0; i < array_length; i++)
    {
        if (i > 0)
        {
            buffer += sprintf(buffer, " ");
        }
        buffer += sprintf(buffer, "0x%02X", byte_array[i]);
    }

    app_log_handler(severity, log_message, array_buffer);
}

void app_log_handler(app_log_severity_t severity, const char * log_message, ...)
{
    if (severity < APP_LOG_DEBUG)
    {
        return;
    }

    char buffer[256];
    va_list args;
    va_start(args, log_message);
    sprintf(buffer, "<!-- %s -->\n", log_message);
    vprintf(buffer, args);
    va_end(args);
}
