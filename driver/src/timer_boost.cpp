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

#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>

using namespace boost;

TimerBoost::TimerBoost()
    : Timer(),
      service(),
      timer(service),
      workNotifier(service),
      timerThread(bind(&asio::io_service::run, &service)),
      timerState(TimerIdle)
{
    // Empty
}

TimerBoost::TimerBoost(const uint32_t timeout, const TimerMode mode,
                       timer_handler_t handler)
    : Timer(timeout, mode, handler),
      service(),
      timer(service, posix_time::milliseconds(timeoutMs)),
      workNotifier(service),
      timerThread(bind(&asio::io_service::run, &service)),
      timerState(TimerIdle)
{
    // Empty
}

TimerBoost::~TimerBoost()
{
    try
    {
        service.stop();
        timerThread.join();
    }
    catch(std::exception)
    {
        // No logging of exception here since we don't
        // know what's been taken down around us.
    }
}

uint32_t TimerBoost::startTimer ()
{
    if (timeoutMs == 0 || timerHandler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    if (timerState == TimerRunning)
    {
        return NRF_ERROR_INTERNAL;
    }

    if (internalStartTimer())
    {
        return NRF_ERROR_INTERNAL;
    }

    return NRF_SUCCESS;
}

uint32_t TimerBoost::startTimer (const uint32_t timeout)
{
    if (timerState == TimerRunning)
    {
        return NRF_ERROR_INTERNAL;
    }

    this->timeoutMs = timeout;

    if (!internalStartTimer())
    {
        return NRF_ERROR_INTERNAL;
    }

    return NRF_SUCCESS;
}

uint32_t TimerBoost::stopTimer()
{
    bool stopped = internalStopTimer();

    if (!stopped)
    {
        return NRF_ERROR_INTERNAL;
    }

    return NRF_SUCCESS;
}

bool TimerBoost::internalStartTimer()
{
    try
    {
        timer.expires_from_now(posix_time::milliseconds(timeoutMs));
        timerState = TimerRunning;

        service.reset();
        timer.async_wait(boost::bind(&TimerBoost::timerExpirationHandler,
                                       this,
                                       asio::placeholders::error));
    }
    catch(std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";

        return false;
    }

    return true;
}

bool TimerBoost::internalStopTimer()
{
    try
    {
        size_t canceledTimers = timer.cancel_one();

        if(canceledTimers == 1)
        {
            // We cancelled our one running timer
        }
    }
    catch(std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
        return false;
    }

    return true;
}

void TimerBoost::timerExpirationHandler(const boost::system::error_code& error)
{
    if (error == asio::error::operation_aborted)
    {
        timerState = TimerStopped;
        return;
    }

    timerState = TimerIdle;

    if (timerHandler != NULL)
    {
        timerHandler(NULL);
    }

    bool timerIsRepeater = timerMode == TimerModeRepeated;

    if (timerIsRepeater)
    {
        startTimer();
    }
}
