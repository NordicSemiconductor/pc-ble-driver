/*
 * Copyright (c) 2016 Nordic Semiconductor ASA
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
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>

using namespace boost::asio;

void writeHandler(const boost::system::error_code& errorCode, const size_t bytesTransferred)
{
    if (errorCode.value() != boost::system::errc::errc_t::success)
    {
        std::cout << "errorCode       : " << errorCode.value() << std::endl;
    }

    std::cout << "bytesTransferred: " << bytesTransferred << std::endl;
}

int main(int argc, char *argv[])
{
    boost::system::error_code ec;
    serial_port::baud_rate serial_port_option;
    io_service io_service;

    if (argc <= 2)
    {
        std::cout << "Usage: " << argv[0] << " SERIALPORT COUNT_TO_NUMBER" << std::endl;
        return -1;
    }
    
    std::string serialPort(argv[1]);
    int count = atoi(argv[2]);

    auto segger = serial_port(io_service);

    std::cout << "Opening port " << serialPort << std::endl;

    segger.open(serialPort, ec);

    if (ec) {
        std::cout << "error : port->open() failed...com_port_name="
        << serialPort << ", e=" << ec.message().c_str() << std::endl;
        return -1;
    }

    serial_port::baud_rate baudrate(115200);
    serial_port::stop_bits stopBits(serial_port::stop_bits::one);
    serial_port::parity parity(serial_port::parity::none);
    serial_port::flow_control flowControl(serial_port::flow_control::none);
    serial_port::character_size charSize = serial_port::character_size(8);

    segger.set_option(baudrate);
    segger.set_option(stopBits);
    segger.set_option(parity);
    segger.set_option(flowControl);
    segger.set_option(charSize);

    std::vector<uint8_t> writeBufferVector;

    for(int i = 0; i < count; i++) {
        auto value = i % 256;
        writeBufferVector.push_back(value);
    }

    mutable_buffers_1 mutableWriteBuffer = boost::asio::buffer(writeBufferVector, writeBufferVector.size());

    boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
    async_write(segger, mutableWriteBuffer, &writeHandler);
    
    std::this_thread::sleep_for(std::chrono::seconds(3));

    segger.cancel();
    segger.close();

    io_service.stop();
    io_service.reset();
}
