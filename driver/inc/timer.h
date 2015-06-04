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

#include <stdint.h>

#ifndef TIMER_H
#define TIMER_H

/**@brief Application timeout handler type. */
typedef void (*timer_handler_t)(void *p_context);

/**@brief Timer modes. */
typedef enum
{
    TimerModeSingleShot,
    TimerModeRepeated
} TimerMode;

/**@brief Base class for timer implementations*/
class Timer
{
public:

    /**@brief Default constructor */
    Timer();

    /**@brief Constructor that fully initializes the timer*/
    Timer(const uint32_t timeoutMs, const TimerMode timerMode,
          timer_handler_t timerHandler);

    /**@brief Destructor */
    virtual ~Timer();

    /**@brief Starts the timer with preconfigured timeout and context.
     **@note Multiple calls are ignored while the timer is running */
    virtual uint32_t startTimer() = 0;

    /**@brief Starts the timer with a timeout given in ms.
     **@note Multiple calls are ignored while the timer is running */
    virtual uint32_t startTimer(const uint32_t timeoutTimeMs) = 0;

    /**@brief Stops the timer immediately */
    virtual uint32_t stopTimer() = 0;

protected:
    uint32_t timeoutMs;
    TimerMode timerMode;
    timer_handler_t timerHandler;

private:

};

#endif // TIMER_H
