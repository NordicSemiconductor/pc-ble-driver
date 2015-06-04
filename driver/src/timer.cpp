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

#include "timer.h"

Timer::Timer()
    : timeoutMs(0),
      timerMode(TimerModeSingleShot),
      timerHandler(0)
{
    // Empty
}

Timer::Timer(const uint32_t timeout, const TimerMode mode, timer_handler_t handler)
    : timeoutMs(timeout),
      timerMode(mode),
      timerHandler(handler)
{
    // Empty
}

Timer::~Timer()
{
}
