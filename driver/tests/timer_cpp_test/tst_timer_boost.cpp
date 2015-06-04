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

#include "timer_boost.h"

#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

// Test related includes
#define BOOST_TEST_MODULE tst_timer_boost
#include <boost/test/unit_test.hpp>

using namespace std;

using namespace boost::posix_time;
using namespace boost;


// Forward declarations -- BEGIN --
void timeoutEventHandler(void *pContext);
// Forward declarations -- END --

// Variables/structs used during testing -- BEGIN --
vector<ptime> timeout_stamps;
uint32_t timeout_precision = 100; // Timeout precision in milliseconds

struct timeout_data {
    uint32_t timeout_ms;
    TimerMode timer_mode;
};

// Test vectors
vector<timeout_data *> start_and_timeout_test_vector;
vector<timeout_data *> start_and_stop_test_vector;
vector<timeout_data *> repeat_timeout_test_vector;

// Variables/structs used during testing -- END --

BOOST_AUTO_TEST_CASE( startAndTimeout )
{
    // Setup test data
    timeout_data *t;

    // Test #1
    t = new timeout_data();
    t->timeout_ms = 500;
    t->timer_mode = TimerModeSingleShot;

    start_and_timeout_test_vector.push_back(t);

    // Test #2
    t = new timeout_data();
    t->timeout_ms = 250;
    t->timer_mode = TimerModeSingleShot;

    start_and_timeout_test_vector.push_back(t);

    // Test #3
    t = new timeout_data();
    t->timeout_ms = 100;
    t->timer_mode = TimerModeSingleShot;

    start_and_timeout_test_vector.push_back(t);

    // Iterate over test vector
    for (vector<timeout_data *>::iterator it = start_and_timeout_test_vector.begin();
         it != start_and_timeout_test_vector.end();
         ++it)
    {
        timeout_stamps.clear();

        uint32_t timeout_ms = (*it)->timeout_ms;
        TimerBoost timerBoost(timeout_ms, (*it)->timer_mode, timeoutEventHandler);

        ptime startTime = microsec_clock::local_time();

        timerBoost.startTimer();
        boost::this_thread::sleep_for(boost::chrono::milliseconds(timeout_ms + timeout_precision));
        timerBoost.stopTimer();

        BOOST_CHECK_MESSAGE(!timeout_stamps.empty(),
                            boost::format("Timeout have not happened even %1% ms after expected time.")
                            % timeout_precision);

        ptime earliestExpectedTimeout = startTime + millisec(timeout_ms - timeout_precision);
        ptime timeoutTime = timeout_stamps.at(0);

        BOOST_CHECK_MESSAGE(timeoutTime >= earliestExpectedTimeout,
                            boost::format("Timeout happened atleast %1% ms before expected time.")
                            % timeout_precision);

        delete *it;
    }

    start_and_timeout_test_vector.clear();
}

BOOST_AUTO_TEST_CASE ( startAndStop )
{
    // Setup test data
    timeout_data *t;

    // Test #1
    t = new timeout_data();
    t->timeout_ms = 500;
    t->timer_mode = TimerModeSingleShot;

    start_and_stop_test_vector.push_back(t);

    // Test #2
    t = new timeout_data();
    t->timeout_ms = 250;
    t->timer_mode = TimerModeSingleShot;

    start_and_stop_test_vector.push_back(t);

    // Test #3
    t = new timeout_data();
    t->timeout_ms = 100;
    t->timer_mode = TimerModeSingleShot;

    start_and_stop_test_vector.push_back(t);

    // Iterate over test vector
    for (vector<timeout_data *>::iterator it = start_and_stop_test_vector.begin();
         it != start_and_stop_test_vector.end();
         ++it)
    {
        timeout_stamps.clear();
        uint32_t timeout_ms = (*it)->timeout_ms;

        TimerBoost timerBoost(timeout_ms, (*it)->timer_mode, timeoutEventHandler);
        timerBoost.startTimer();

        if (timeout_precision <= timeout_ms)
        {
            uint64_t sleep_time = timeout_ms - timeout_precision;

            if (sleep_time <= 0)
            {
                sleep_time = 1;
            }

            boost::this_thread::sleep_for(boost::chrono::milliseconds(sleep_time));
        }

        timerBoost.stopTimer();

        BOOST_CHECK_MESSAGE(timeout_stamps.empty(),
                            boost::format("Timeout happened atleast %1% ms before expected time.") % timeout_precision);

        boost::this_thread::sleep_for(boost::chrono::milliseconds(2 * timeout_precision));

        BOOST_CHECK_MESSAGE(timeout_stamps.empty(),
                            "Timeout have happened even after stop command.");

        delete *it;
    }

    start_and_stop_test_vector.clear();
}


BOOST_AUTO_TEST_CASE ( repeatTimeout )
{
    // Setup test data
    timeout_data *t;

    // Test #1
    t = new timeout_data();
    t->timeout_ms = 500;
    t->timer_mode = TimerModeRepeated;

    repeat_timeout_test_vector.push_back(t);

    // Test #2
    t = new timeout_data();
    t->timeout_ms = 250;
    t->timer_mode = TimerModeRepeated;

    repeat_timeout_test_vector.push_back(t);

    // Iterate over test vector
    for (vector<timeout_data *>::iterator it = repeat_timeout_test_vector.begin();
         it != repeat_timeout_test_vector.end();
         ++it)
    {
        timeout_stamps.clear();

        uint32_t timeout_ms = (*it)->timeout_ms;
        TimerBoost timerBoost(timeout_ms, (*it)->timer_mode, timeoutEventHandler);

        int32_t expected_timeouts = 5;

        ptime startTime = microsec_clock::local_time();
        timerBoost.startTimer();
        boost::this_thread::sleep_for(boost::chrono::milliseconds(expected_timeouts * (timeout_ms + timeout_precision)));
        timerBoost.stopTimer();

        int32_t numberOfTimeouts = timeout_stamps.size();

        BOOST_CHECK_MESSAGE(numberOfTimeouts >= expected_timeouts,
                            boost::format("%1% timeouts happened. %2% was expected.")
                            % numberOfTimeouts
                            % expected_timeouts);

        ptime earliestExpectedTimeout = startTime + millisec(timeout_ms - timeout_precision);

        for (int i = 0; i < expected_timeouts; i++) {
            ptime timeoutTime = timeout_stamps.at(i);

            BOOST_CHECK_MESSAGE(timeoutTime > earliestExpectedTimeout,
                                boost::format("Timeout #%1% happened atleast %2% ms before expected time")
                                % i
                                % timeout_precision
            );

            ptime latestExpectedTimeout = earliestExpectedTimeout + millisec(2 * timeout_precision);

            BOOST_CHECK_MESSAGE(latestExpectedTimeout > timeoutTime,
                                boost::format("Timeout #%1% happened atleast %2% ms after expected time. Latest expected: %3%, was %4%.")
                                % i
                                % timeout_precision
                                % to_simple_string(latestExpectedTimeout)
                                % to_simple_string(timeoutTime)
            );

            earliestExpectedTimeout = timeoutTime + millisec(timeout_ms - timeout_precision);
        }

        delete *it;
    }

    repeat_timeout_test_vector.clear();
}

void timeoutEventHandler(void *)
{
    timeout_stamps.push_back(microsec_clock::local_time());
}