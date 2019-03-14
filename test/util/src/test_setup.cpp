#include "test_setup.h"
#include <logger.h>
#include <algorithm>
#include <utility>

namespace test {

Environment ConfiguredEnvironment;

SerialPort::SerialPort(std::string port, uint32_t baudRate)
    : port(std::move(port))
    , baudRate(baudRate){};

std::string getEnvironmentAsText(const Environment &env)
{
    std::stringstream ss;

    if (env.serialPorts.empty())
    {
        ss << "No serial ports setup.\n";
    }
    else
    {
        ss << "serial-ports:\n";
        for (auto port : env.serialPorts)
        {
            ss << "  port:" << port.port << " baud-rate:" << port.baudRate << "\n";
        }
    }

    ss << "retransmission-interval:" << env.retransmissionInterval << "\n";
    ss << "response-timeout:" << env.responseTimeout << "\n";
    ss << "ble-mtu:" << env.mtu << "\n";

    ss << "hardware-info:" << env.hardwareInfo << "\n";
    ss << "log-level:" << testutil::asText(env.driverLogLevel) << "\n";
    ss << "iterations(if relevant):" << env.numberOfIterations << "\n";

    return ss.str();
}

Environment getEnvironment()
{
    Environment env;

    auto baudRate          = defaultBaudRate;
    const auto envBaudRate = std::getenv("BLE_DRIVER_TEST_BAUD_RATE");

    if (envBaudRate != nullptr)
    {
        baudRate = std::stoi(envBaudRate);
    }

    // Command line argument override environment variable
    if (ConfiguredEnvironment.baudRate != 0)
    {
        baudRate = ConfiguredEnvironment.baudRate;
    }

    // Command line argument override environment variable
    if (std::count_if(ConfiguredEnvironment.serialPorts.begin(),
                      ConfiguredEnvironment.serialPorts.end(),
                      [](const SerialPort &port) -> bool { return !port.port.empty(); }) >= 2)
    {
        for (auto port : ConfiguredEnvironment.serialPorts)
        {
            if (!port.port.empty())
            {
                env.serialPorts.emplace_back(port.port, baudRate);
            }
        }
    }
    else
    {
        auto envPortA = std::getenv("BLE_DRIVER_TEST_SERIAL_PORT_A");

        if (envPortA != nullptr)
        {
            env.serialPorts.emplace_back(envPortA, baudRate);
        }

        auto envPortB = std::getenv("BLE_DRIVER_TEST_SERIAL_PORT_B");

        if (envPortB != nullptr)
        {
            env.serialPorts.emplace_back(envPortB, baudRate);
        }
    }

    auto numberOfIterations          = 100;
    const auto envNumberOfIterations = std::getenv("BLE_DRIVER_TEST_OPENCLOSE_ITERATIONS");

    if (envNumberOfIterations != nullptr)
    {
        numberOfIterations = std::stoi(envNumberOfIterations);
    }

    // Command line argument override environment variable
    if (ConfiguredEnvironment.numberOfIterations != 0)
    {
        numberOfIterations = ConfiguredEnvironment.numberOfIterations;
    }

    env.numberOfIterations = numberOfIterations;

    env.driverLogLevel = SD_RPC_LOG_INFO;

    // Command line argument override environment variable
    if (ConfiguredEnvironment.driverLogLevelSet)
    {
        env.driverLogLevel = ConfiguredEnvironment.driverLogLevel;
    }
    else
    {
        const auto envDriverLogLevel = std::getenv("BLE_DRIVER_TEST_LOGLEVEL");

        if (envDriverLogLevel != nullptr)
        {
            env.driverLogLevel = testutil::parseLogSeverity(envDriverLogLevel);
        }
    }

    env.driverLogLevelSet = true;
    env.baudRate          = baudRate;
    env.hardwareInfo      = ConfiguredEnvironment.hardwareInfo;
    env.retransmissionInterval  = ConfiguredEnvironment.retransmissionInterval;
    env.responseTimeout = ConfiguredEnvironment.responseTimeout;
    env.mtu = ConfiguredEnvironment.mtu;

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
    get_logger()->debug("->{} length: {}", testutil::asHex(data), data.size());
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
// It does not override open because we can not provide a return value that
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
