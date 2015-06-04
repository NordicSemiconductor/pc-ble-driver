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
#include "nrf_soc.h"
#include "nrf_error.h"

#include <boost/thread/recursive_mutex.hpp>

static boost::recursive_mutex m_critical_region_mutex;

uint32_t sd_nvic_EnableIRQ(IRQn_Type IRQn)
{
    /*if interrupt is a softdevice event handle it as user implemented it.*/
    return NRF_SUCCESS;
}

uint32_t sd_nvic_critical_region_enter(uint8_t * p_is_nested_critical_region)
{
    m_critical_region_mutex.lock();
    return NRF_SUCCESS;
}

uint32_t sd_nvic_critical_region_exit(uint8_t is_nested_critical_region)
{
    m_critical_region_mutex.unlock();
    return NRF_SUCCESS;
}
