/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

#include "transport.h"

#include "nrf_error.h"

#include <stdint.h>

using namespace std;

Transport::Transport()
{
    /* Intentional empty */
}

Transport::~Transport()
{
    /* Intentional empty */
}

uint32_t Transport::open(status_cb_t status_callback, data_cb_t data_callback, log_cb_t log_callback)
{
    statusCallback = status_callback;
    dataCallback = data_callback;
    logCallback = log_callback;

    return NRF_SUCCESS;
}

uint32_t Transport::close()
{
    return NRF_SUCCESS;
}
