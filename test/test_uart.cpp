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
    std::cout << "errorCode       : " << errorCode << std::endl;
    std::cout << "bytesTransferred: " << bytesTransferred << std::endl;
}

int main(int argc, char *argv[])
{
    boost::system::error_code ec;
    serial_port::baud_rate serial_port_option;
    io_service io_service;
    
    std::string serialPort(argv[1]);
    int count = atoi(argv[2]);

    auto segger = serial_port(io_service);

    std::cout << "Opening port " << serialPort;

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
