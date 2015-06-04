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

#include "timer_boost.h"
#include "nrf_error.h"
#include <cstddef>

TimerBoost *TimerBoost::instance = 0;

TimerBoost::TimerBoost()
    : Timer(),
      timerState(TimerIdle),
      responseValue(NRF_ERROR_NULL),
      stopTimerCalled(false),
      startTimerCalled(false)
{
    resetMockVariables();
    instance = this;
}

TimerBoost::TimerBoost(const uint32_t timeout, const TimerMode mode,
                       timer_handler_t handler)
    : Timer(timeout, mode, handler),
      timerState(TimerIdle),
      responseValue(NRF_ERROR_NULL),
      stopTimerCalled(false),
      startTimerCalled(false)
{
    resetMockVariables();
    instance = this;
}

TimerBoost::~TimerBoost()
{
}

uint32_t TimerBoost::startTimer()
{
    startTimerCalled = true;
    timerState = TimerRunning;
    return responseValue;
}

uint32_t TimerBoost::startTimer(const uint32_t timeout)
{
    timeoutMs = timeout;
    return startTimer();
}

uint32_t TimerBoost::stopTimer()
{
    stopTimerCalled = true;
    return responseValue;
}

TimerBoost *TimerBoost::getInstance()
{
    return instance;
}

void TimerBoost::triggerTimeoutCallback()
{
    timerHandler(NULL);
}

void TimerBoost::setResponseValue(uint32_t value)
{
    responseValue = value;
}

uint32_t TimerBoost::getTimeoutValue()
{
    return timeoutMs;
}

TimerMode TimerBoost::getTimerMode()
{
    return timerMode;
}

bool TimerBoost::wasStopTimerCalled()
{
    return stopTimerCalled;
}

bool TimerBoost::wasStartTimerCalled()
{
    return startTimerCalled;
}

void TimerBoost::resetMockVariables()
{
    stopTimerCalled = false;
    startTimerCalled = false;
    responseValue = NRF_ERROR_NULL;
    timeoutMs = 0;
}

