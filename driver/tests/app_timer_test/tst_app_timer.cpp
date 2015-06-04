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

#define BOOST_TEST_MODULE app_timer_test
#include <boost/test/unit_test.hpp>

#include "nrf_error.h"
#include "app_timer.h"
#include "app_timer_extension.h"
#include "timer_boost.h"

static bool timeout_handler_called = false;
static app_timer_id_t timerId;
static const uint32_t prescaler = 0;

static void timeout_handler(void * pContext)
{
    timeout_handler_called = true;
}

void setup()
{
    uint8_t maxTimers = 1;
    uint8_t opQueueSize = 10;

    uint32_t error_code = app_timer_init(prescaler, maxTimers, opQueueSize, NULL, NULL);
    BOOST_CHECK(NRF_SUCCESS == error_code);

    app_timer_mode_t mode = APP_TIMER_MODE_SINGLE_SHOT;

    error_code = app_timer_create(&timerId, mode, (app_timer_timeout_handler_t)timeout_handler);
    BOOST_CHECK_EQUAL(NRF_SUCCESS, error_code);

    TimerBoost *instance = TimerBoost::getInstance();

    BOOST_CHECK(mode == (app_timer_mode_t)instance->getTimerMode());
}

void cleanup()
{
    app_timer_stop(timerId);
    app_timer_close();
}

static int convert_ticks_to_ms(int timeout_ticks)
{
    /* Algorithm from app_timer.h/app_timer_pc.c */
    uint32_t thousand_times_ticks = 1000 * timeout_ticks;
    uint32_t prescaler_value = (prescaler + 1);
    uint32_t ticks_prescale_value = (thousand_times_ticks * prescaler_value);
    uint32_t expected_timeout_ms = ticks_prescale_value / APP_TIMER_CLOCK_FREQ;
    return expected_timeout_ms;
}

void init_max_timers_function(uint8_t max_timers, uint32_t expected_result)
{
    uint8_t op_queue_size = 10;
    void *p_buffer = NULL;
    app_timer_evt_schedule_func_t func = NULL;
    uint32_t result = app_timer_init(prescaler, max_timers, op_queue_size, p_buffer, func);

    BOOST_CHECK_EQUAL(expected_result, result);
}

BOOST_AUTO_TEST_CASE(init_max_timers_test)
{
    uint32_t success = NRF_SUCCESS;
    uint32_t invalid_param = NRF_ERROR_INVALID_PARAM;
    uint32_t not_supported = NRF_ERROR_NOT_SUPPORTED;

    setup();
    init_max_timers_function(0, invalid_param);
    cleanup();

    setup();
    init_max_timers_function(1, success);
    cleanup();

    setup();
    init_max_timers_function(3, not_supported);
    cleanup();
}

BOOST_AUTO_TEST_CASE(create_timer_already_created_test)
{
    setup();
    app_timer_mode_t mode = APP_TIMER_MODE_SINGLE_SHOT;
    app_timer_timeout_handler_t handler = NULL;
    uint32_t error_code = app_timer_create(&timerId, mode, handler);
    BOOST_CHECK_EQUAL(NRF_ERROR_NO_MEM, error_code);
    cleanup();
}

BOOST_AUTO_TEST_CASE(create_timer_no_handler_test)
{
    setup();
    cleanup();

    app_timer_mode_t mode = APP_TIMER_MODE_SINGLE_SHOT;
    app_timer_timeout_handler_t handler = NULL;
    uint32_t error_code = app_timer_create(&timerId, mode, handler);
    BOOST_CHECK_EQUAL(NRF_ERROR_INVALID_PARAM, error_code);
}

BOOST_AUTO_TEST_CASE(timer_start_test)
{
    void * pContext = NULL;
    uint32_t timeout_ticks = 100;

    setup();

    TimerBoost::getInstance()->setResponseValue(NRF_SUCCESS);
    uint32_t error_code = app_timer_start(timerId, timeout_ticks, pContext);
    BOOST_CHECK_EQUAL(NRF_SUCCESS, error_code);

    uint32_t expected_timeout_ms = convert_ticks_to_ms(timeout_ticks);
    BOOST_CHECK_EQUAL(expected_timeout_ms, TimerBoost::getInstance()->getTimeoutValue());

    BOOST_CHECK(TimerBoost::getInstance()->wasStartTimerCalled());

    cleanup();
}

BOOST_AUTO_TEST_CASE(timer_start_invalid_test)
{
    setup();
    cleanup();

    void * pContext = NULL;
    int timeout_ticks = 1000;

    uint32_t error_code = app_timer_start(timerId, timeout_ticks, pContext);
    BOOST_CHECK_EQUAL(NRF_ERROR_INVALID_STATE, error_code);
}

BOOST_AUTO_TEST_CASE(timer_stop_test)
{
    setup();

    TimerBoost::getInstance()->setResponseValue(NRF_SUCCESS);
    BOOST_CHECK(!TimerBoost::getInstance()->wasStopTimerCalled());

    uint32_t error_code = app_timer_stop(timerId);
    BOOST_CHECK(TimerBoost::getInstance()->wasStopTimerCalled());
    BOOST_CHECK_EQUAL(NRF_SUCCESS, error_code);

    cleanup();
}

BOOST_AUTO_TEST_CASE(timer_stop_invalid_test)
{
    setup();
    cleanup();

    uint32_t error_code = app_timer_stop(timerId);
    BOOST_CHECK_EQUAL(NRF_ERROR_INVALID_STATE, error_code);
}

BOOST_AUTO_TEST_CASE(timer_stop_all_test)
{
    setup();

    TimerBoost::getInstance()->setResponseValue(NRF_SUCCESS);
    BOOST_CHECK(!TimerBoost::getInstance()->wasStopTimerCalled());

    uint32_t error_code = app_timer_stop_all();
    BOOST_CHECK(TimerBoost::getInstance()->wasStopTimerCalled());
    BOOST_CHECK_EQUAL(NRF_SUCCESS, error_code);

    cleanup();
}

BOOST_AUTO_TEST_CASE(timeout_callback_test)
{
    setup();

    BOOST_CHECK(!timeout_handler_called);
    TimerBoost::getInstance()->triggerTimeoutCallback();
    BOOST_CHECK(timeout_handler_called);

    cleanup();
}

BOOST_AUTO_TEST_CASE(timer_cnt_get_test)
{
    setup();
    uint32_t * pTicks = NULL;
    uint32_t error_code = app_timer_cnt_get(pTicks);

    BOOST_CHECK_EQUAL(NRF_ERROR_NOT_SUPPORTED, error_code);

    cleanup();
}

BOOST_AUTO_TEST_CASE(timer_cnt_diff_compute_test)
{
    uint32_t ticksTo = 0;
    uint32_t ticksFrom = 0;
    uint32_t * pTicksDiff = NULL;
    uint32_t error_code = app_timer_cnt_diff_compute(ticksTo, ticksFrom, pTicksDiff);

    setup();

    BOOST_CHECK_EQUAL(NRF_ERROR_NOT_SUPPORTED, error_code);

    cleanup();
}
