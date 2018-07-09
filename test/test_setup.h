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

#ifndef TEST_SETUP_H__
#define TEST_SETUP_H__

#include "transport.h"
#include "h5_transport.h"

#include "log.h"
#include "test_util.h"

#include "virtual_uart.h"

#include <string>
#include <vector>
#include <thread>
#include <condition_variable>
#include <iostream>

namespace test 
{
    struct SerialPort
    {
        SerialPort(std::string port, uint32_t baudRate) :
            port(port), baudRate(baudRate) {};

        std::string port;
        uint32_t baudRate;
    };

    struct Environment
    {
        std::vector<SerialPort> serialPorts;
        uint32_t numberOfIterations;
        sd_rpc_log_severity_t driverLogLevel;
    };

    Environment getEnvironment()
    {
        Environment env;

        auto baudRate = 1000000;
        auto envBaudRate = std::getenv("BLE_DRIVER_TEST_BAUD_RATE");

        if (envBaudRate != nullptr)
        {
            baudRate = std::stoi(envBaudRate);
        }

        auto envPortA = std::getenv("BLE_DRIVER_TEST_SERIAL_PORT_A");

        if (envPortA != nullptr)
        {
            env.serialPorts.push_back(SerialPort(envPortA, baudRate));
        }

        auto envPortB = std::getenv("BLE_DRIVER_TEST_SERIAL_PORT_B");

        if (envPortB != nullptr)
        {
            env.serialPorts.push_back(SerialPort(envPortB, baudRate));
        }

        auto numberOfIterations = 100;
        auto envNumberOfIterations = std::getenv("BLE_DRIVER_TEST_OPENCLOSE_ITERATIONS");

        if (envNumberOfIterations != nullptr)
        {
            numberOfIterations = std::stoi(envNumberOfIterations);
        }

        env.numberOfIterations = numberOfIterations;

        auto driverLogLevel = SD_RPC_LOG_INFO;
        auto envDriverLogLevel = std::getenv("BLE_DRIVER_TEST_LOGLEVEL");

        if (envDriverLogLevel != nullptr)
        {
            auto envDriverLogLevel_ = std::string(envDriverLogLevel);

            if (envDriverLogLevel_ == "trace")
            {
                driverLogLevel = SD_RPC_LOG_TRACE;
            }
            else if (envDriverLogLevel_ == "debug")
            {
                driverLogLevel = SD_RPC_LOG_DEBUG;
            }
            else if (envDriverLogLevel_ == "info")
            {
                driverLogLevel = SD_RPC_LOG_INFO;
            }
            else if (envDriverLogLevel_ == "warning")
            {
                driverLogLevel = SD_RPC_LOG_WARNING;
            }
            else if (envDriverLogLevel_ == "error")
            {
                driverLogLevel = SD_RPC_LOG_ERROR;
            }
            else if (envDriverLogLevel_ == "fatal")
            {
                driverLogLevel = SD_RPC_LOG_FATAL;
            }
        }

        env.driverLogLevel = driverLogLevel;

        return env;
    };

    class VirtualTransportSendSync : public Transport
    {
    public:
        explicit VirtualTransportSendSync() noexcept
            : Transport(), pushData(false) {}

        uint32_t open(const status_cb_t &status_callback, const data_cb_t &data_callback, const log_cb_t &log_callback)
        {
            Transport::open(status_callback, data_callback, log_callback);
            pushData = true;

            auto inboundDataThread = [=]() -> void
            {
                std::vector<uint8_t> syncPacket{ 0xc0, 0x00, 0x2f, 0x00, 0xd1, 0x01, 0x7e, 0xc0 };

                while (pushData)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
                    upperDataCallback(syncPacket.data(), syncPacket.size());
                }
            };

            dataPusher = std::thread(inboundDataThread);

            return NRF_SUCCESS;
        }

        uint32_t close()
        {
            pushData = false;
            
            if (dataPusher.joinable())
            {
                dataPusher.join();
            }

            Transport::close();
            return NRF_SUCCESS;
        }

        uint32_t send(const std::vector<uint8_t> &data)
        {
            NRF_LOG("->" << testutil::convertToString(data) << " length: " << data.size());
            return NRF_SUCCESS;
        }

    private:
        std::thread dataPusher;
        bool pushData;
    };

    // Since the H5Transport.open is a blocking call
    // we need to run open in separate threads to
    // make the two H5Transports communicate
    class H5TransportWrapper : public H5Transport
    {
    public:
        H5TransportWrapper(Transport *nextTransportLayer, uint32_t retransmission_interval) noexcept
            : H5Transport(nextTransportLayer, retransmission_interval), isOpenDone(false), result(-1)
        {
        }

        ~H5TransportWrapper()
        {
            close();
        }

        void openThread(status_cb_t status_callback, data_cb_t data_callback, log_cb_t log_callback)
        {
            auto lock = std::unique_lock<std::mutex>(openWait);

            result = H5Transport::open(
                status_callback,
                data_callback,
                log_callback
            );

            isOpenDone = true;

            lock.unlock();
            openStateChanged.notify_all();
        }

        // Runs open in a separate thread, call waitForResult to wait for the result of opening.
        //
        // It does not override open becase we can not provide a return value that
        // is of the same type as H5Transport::open
        void wrappedOpen(status_cb_t status_callback, data_cb_t data_callback, log_cb_t log_callback)
        {
            auto lock = std::unique_lock<std::mutex>(openWait);
            h5Thread = std::thread(std::bind(&H5TransportWrapper::openThread, this, status_callback, data_callback, log_callback));
        }

        uint32_t close()
        {
            auto lock = std::unique_lock<std::mutex>(openWait);

            if (isOpenDone)
            {
                isOpenDone = false;
                lock.unlock();
                openStateChanged.notify_all();

                if (h5Thread.joinable())
                {
                    h5Thread.join();
                }
            }

            return H5Transport::close();
        }

        uint32_t getResult()
        {
            auto lock = std::unique_lock<std::mutex>(openWait);
            return result;
        }

        uint32_t waitForResult()
        {
            auto lock = std::unique_lock<std::mutex>(openWait);

            if (isOpenDone)
            {
                return result;
            }

            openStateChanged.wait(lock, [this] { return isOpenDone; });
            return result;
        }

    private:
        std::mutex openWait;
        bool isOpenDone;
        std::condition_variable openStateChanged;

        std::thread h5Thread;
        uint32_t result;
    };
};

#endif // TEST_SETUP_H__