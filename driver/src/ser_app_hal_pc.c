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

#include "ser_app_hal.h"
#include "ser_app_hal_pc_extension.h"

#include "app_log.h"
#include "nrf_error.h"
#include "nrf_soc.h"

#include "app_uart_extension.h"
#include "softdevice_handler_extension.h"
#include "ser_softdevice_handler_extension.h"

#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

using namespace boost;

static bool                 m_event_handling_run;
static uint32_t             m_timeout_ms = 1000;
static mutex                m_event_mutex;
static condition_variable   m_event_wait_condition;
static condition_variable   m_uart_wait_condition;
static thread             * mp_event_thread = NULL;

static void run_event_handling()
{
    while (m_event_handling_run)
    {
        SWI2_IRQHandler();

        unique_lock<mutex> event_counter_lock(m_event_mutex);

        uint32_t mailbox_length = 0;
        sd_ble_evt_mailbox_length_get(&mailbox_length);

        if (mailbox_length != 0)
        {
            continue;
        }

        m_event_wait_condition.wait(event_counter_lock);
    }
}

void packet_received_handler()
{
    m_uart_wait_condition.notify_one();
}

void ser_app_hal_pc_event_handling_stop()
{
    m_event_mutex.lock();

    m_event_handling_run = false;

    m_event_wait_condition.notify_one();

    m_event_mutex.unlock();

    if (mp_event_thread != NULL)
    {
        mp_event_thread->join();
        delete mp_event_thread;
        mp_event_thread = NULL;
    }
}

void ser_app_hal_pc_init()
{
    if (mp_event_thread == NULL)
    {
        mp_event_thread = new thread(run_event_handling);
    }
}

uint32_t ser_app_hal_hw_init(void)
{
    m_event_handling_run = true;
    return NRF_SUCCESS;
}

uint32_t sd_app_evt_wait(void)
{
    mutex response_mutex;
    unique_lock<mutex> unique_lock(response_mutex);

    chrono::milliseconds timeout(m_timeout_ms);
    chrono::system_clock::time_point wakeup_time = chrono::system_clock::now() + timeout;

    m_uart_wait_condition.wait_until(unique_lock, wakeup_time);
    return NRF_SUCCESS;
}

void ser_app_hal_delay(uint32_t ms)
{
    //Not needed. This is only used for waiting on connectivity reset time and wakeup.
    return;
}

void ser_app_hal_nrf_reset_pin_clear(void)
{
    //We do not have access to reset pin from PC
    return;
}

void ser_app_hal_nrf_reset_pin_set(void)
{
    //We do not have access to reset pin from PC
    return;
}

void ser_app_hal_nrf_evt_irq_enable(void)
{
    //Do not have interrupt on PC, but this may still be needed for something
    return;
}

void ser_app_hal_nrf_evt_irq_priority_set(void){
    //Do not have interrupt on PC, but this may still be needed for something
    return;
}

void ser_app_hal_nrf_evt_pending(void)
{
    lock_guard<mutex> guard(m_event_mutex);
    m_event_wait_condition.notify_one();
}

void ser_app_hal_nrf_irq_enable(uint32_t irq_id)
{
    //Do not have interrupt on PC, but this may still be needed for something
    return;
}
