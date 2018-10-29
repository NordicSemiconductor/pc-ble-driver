#include "test_setup.h"

namespace test {
SerialPort::SerialPort(std::string port, uint32_t baudRate)
    : port(port)
    , baudRate(baudRate){};

Environment getEnvironment()
{
    Environment env;

    auto baudRate    = DEFAULT_BAUD_RATE;
    const auto envBaudRate = std::getenv("BLE_DRIVER_TEST_BAUD_RATE");

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

    auto numberOfIterations    = 100;
    auto envNumberOfIterations = std::getenv("BLE_DRIVER_TEST_OPENCLOSE_ITERATIONS");

    if (envNumberOfIterations != nullptr)
    {
        numberOfIterations = std::stoi(envNumberOfIterations);
    }

    env.numberOfIterations = numberOfIterations;

    auto driverLogLevel    = SD_RPC_LOG_INFO;
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

VirtualTransportSendSync::VirtualTransportSendSync() noexcept
    : Transport()
    , pushData(false)
{}

uint32_t VirtualTransportSendSync::open(const status_cb_t &status_callback,
                                        const data_cb_t &data_callback,
                                        const log_cb_t &log_callback)
{
    Transport::open(status_callback, data_callback, log_callback);
    pushData = true;

    auto inboundDataThread = [=]() -> void {
        std::vector<uint8_t> syncPacket{0xc0, 0x00, 0x2f, 0x00, 0xd1, 0x01, 0x7e, 0xc0};

        while (pushData)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            upperDataCallback(syncPacket.data(), syncPacket.size());
        }
    };

    dataPusher = std::thread(inboundDataThread);

    return NRF_SUCCESS;
}

uint32_t VirtualTransportSendSync::close()
{
    pushData = false;

    if (dataPusher.joinable())
    {
        dataPusher.join();
    }

    return NRF_SUCCESS;
}

uint32_t VirtualTransportSendSync::send(const std::vector<uint8_t> &data)
{
    NRF_LOG("->" << testutil::asHex(data) << " length: " << data.size());
    return NRF_SUCCESS;
}

// Since the H5Transport.open is a blocking call
// we need to run open in separate threads to
// make the two H5Transports communicate
H5TransportWrapper::H5TransportWrapper(Transport *nextTransportLayer,
                                       uint32_t retransmission_interval) noexcept
    : H5Transport(nextTransportLayer, retransmission_interval)
    , isOpenDone(false)
    , result(-1)
{}

H5TransportWrapper::~H5TransportWrapper()
{
    H5Transport::close();
}

void H5TransportWrapper::openThread(status_cb_t status_callback, data_cb_t data_callback,
                                    log_cb_t log_callback)
{
    auto lock = std::unique_lock<std::mutex>(openWait);

    result = H5Transport::open(status_callback, data_callback, log_callback);

    isOpenDone = true;

    lock.unlock();
    openStateChanged.notify_all();
};

// Runs open in a separate thread, call waitForResult to wait for the result of opening.
//
// It does not override open becase we can not provide a return value that
// is of the same type as H5Transport::open
void H5TransportWrapper::wrappedOpen(status_cb_t status_callback, data_cb_t data_callback,
                                     log_cb_t log_callback)
{
    auto lock = std::unique_lock<std::mutex>(openWait);
    h5Thread  = std::thread(std::bind(&H5TransportWrapper::openThread, this, status_callback,
                                     data_callback, log_callback));
}

uint32_t H5TransportWrapper::waitForResult()
{
    auto lock = std::unique_lock<std::mutex>(openWait);

    if (isOpenDone)
    {
        return result;
    }

    openStateChanged.wait(lock, [this] { return isOpenDone; });
    return result;
}
} // namespace test
