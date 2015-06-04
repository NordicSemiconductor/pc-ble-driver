/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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
#include "app_timer.h"
#include "app_timer_extension.h"
#include "timer.h"
#include "timer_boost.h"
#include "app_error.h"
#include "nordic_common.h"

static const app_timer_id_t m_timer_id = 0;
static const uint32_t m_max_timers     = 1;

static uint32_t m_prescaler_value = 0;
static Timer * mp_timer_object    = NULL;
static uint32_t timeout_in_ms = 0;

uint32_t timer_stop()
{
    if (mp_timer_object == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    uint32_t stop_result = mp_timer_object->stopTimer();

    return stop_result;
}

void app_timer_close()
{
    timer_stop();

    if(mp_timer_object != NULL)
    {
        delete mp_timer_object;
        mp_timer_object = NULL;
    }
}

void app_timer_timeout_ms_set(uint32_t timeout_ms)
{
    timeout_in_ms = timeout_ms;
    app_log_handler(APP_LOG_TRACE, "Retransmission timeout set to: %d", timeout_in_ms);
}

/**@note This implementation of app_timer supports only one timer (max_timers = 1).
 */
uint32_t app_timer_init(uint32_t                      prescaler,
                        uint8_t                       max_timers,
                        uint8_t                       op_queues_size,
                        void *                        p_buffer,
                        app_timer_evt_schedule_func_t evt_schedule_func)
{
    UNUSED_PARAMETER(op_queues_size);
    UNUSED_PARAMETER(p_buffer);
    UNUSED_PARAMETER(evt_schedule_func);

    if (max_timers == 0)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    else if (max_timers > 1)
    {
        return NRF_ERROR_NOT_SUPPORTED;
    }

    m_prescaler_value = prescaler;

    return NRF_SUCCESS;
}

uint32_t app_timer_create(app_timer_id_t *            p_timer_id,
                          app_timer_mode_t            mode,
                          app_timer_timeout_handler_t timeout_handler)
{
    if (mp_timer_object != NULL)
    {
        /* Maximum number of timers (1) has already been reached */
        return NRF_ERROR_NO_MEM;
    }

    if (timeout_handler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    mp_timer_object = new TimerBoost(0, (TimerMode)mode, (timer_handler_t)timeout_handler);

    *p_timer_id = m_timer_id;

    return NRF_SUCCESS;
}

/* @note p_context is ignored and not passed to the timeout handler in this implementation.
 */
uint32_t app_timer_start(app_timer_id_t timer_id, uint32_t timeout_ticks, void * p_context)
{
    UNUSED_PARAMETER(p_context);

    if (mp_timer_object == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (timer_id != m_timer_id)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    uint32_t thousand_times_ticks = 1000 * timeout_ticks;
    uint32_t prescaler_value = (m_prescaler_value + 1);
    uint32_t ticks_prescaler_value = (thousand_times_ticks * prescaler_value);
    uint32_t timeout_ms = ticks_prescaler_value / APP_TIMER_CLOCK_FREQ;

    if (timeout_in_ms > 0)
    {
        timeout_ms = timeout_in_ms;
    }

    return mp_timer_object->startTimer(timeout_ms);
}

uint32_t app_timer_stop(app_timer_id_t timer_id)
{
    if (mp_timer_object == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (timer_id != m_timer_id)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    return timer_stop();
}

uint32_t app_timer_stop_all(void)
{
    return app_timer_stop(m_timer_id);
}

uint32_t app_timer_cnt_get(uint32_t * p_ticks)
{
    UNUSED_PARAMETER(p_ticks);

    return NRF_ERROR_NOT_SUPPORTED;
}

uint32_t app_timer_cnt_diff_compute(uint32_t   ticks_to,
                                    uint32_t   ticks_from,
                                    uint32_t * p_ticks_diff)
{
    UNUSED_PARAMETER(ticks_to);
    UNUSED_PARAMETER(ticks_from);
    UNUSED_PARAMETER(p_ticks_diff);

    return NRF_ERROR_NOT_SUPPORTED;
}
