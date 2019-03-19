/*
* Copyright (c) 2018 Nordic Semiconductor ASA
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
*   1. Redistributions of source code must retain the above copyright notice, this
*   list of conditions and the following disclaimer.
*
*   2. Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
*   3. Neither the name of Nordic Semiconductor ASA nor the names of other
*   contributors to this software may be used to endorse or promote products
*   derived from this software without specific prior written permission.
*
*   4. This software must only be used in or with a processor manufactured by Nordic
*   Semiconductor ASA, or in or with a processor manufactured by a third party that
*   is used in combination with a processor manufactured by Nordic Semiconductor.
*
*   5. Any software provided in binary or object form under this license must not be
*   reverse engineered, decompiled, modified and/or disassembled.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NRF_LOG_H__
#define NRF_LOG_H__

#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>

// You should not use stdout/stderr on Windows since this will have a huge impact on
// the application since this logger is not offloading the displaying of data
// to a separate thread.
//
// This stack overflow thread contains more info in regards to cmd.exe output:
//   https://stackoverflow.com/questions/7404551/why-is-console-output-so-slow

#ifdef NRF_LOG_SETUP
#ifndef NRF_LOG_FILENAME
    std::ostream &nrfLogStream(std::cout); \
    std::mutex nrfLogMutex;
#else
    std::fstream nrfLogStream(\
        NRF_LOG_FILENAME, \
        std::fstream::out | std::fstream::trunc); \
    std::mutex nrfLogMutex;
#endif
#else
extern std::ostream &nrfLogStream;
extern std::mutex nrfLogMutex;
#endif

#ifndef NRF_LOG
#define NRF_LOG(message) do { \
    auto timestamp = std::chrono::system_clock::now(); \
    auto threadID = std::this_thread::get_id(); \
    std::unique_lock<std::mutex> lock(nrfLogMutex); \
    std::stringstream stream; \
    stream << message; \
    nrfLogStream << "@" << std::right << std::setfill('0') << std::setw(10) << std::dec << std::chrono::duration_cast<std::chrono::microseconds>(timestamp.time_since_epoch()).count() \
    << " [" << std::setw(5) << threadID << "] " \
    << "| " << message << std::endl << std::flush; \
} while(0);
#endif // NRF_LOG

#endif // NRF_LOG_H__
