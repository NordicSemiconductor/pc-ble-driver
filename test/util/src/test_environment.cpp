#include "test_environment.h"
#include <algorithm>
#include <sstream>
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

    env.driverLogLevelSet      = true;
    env.baudRate               = baudRate;
    env.hardwareInfo           = ConfiguredEnvironment.hardwareInfo;
    env.retransmissionInterval = ConfiguredEnvironment.retransmissionInterval;
    env.responseTimeout        = ConfiguredEnvironment.responseTimeout;
    env.mtu                    = ConfiguredEnvironment.mtu;

    return env;
};
} // namespace test
