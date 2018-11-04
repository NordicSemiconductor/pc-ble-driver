// Logging support

#include <iostream>
#include <mutex>
#include <test_setup.h>

std::ostream &nrfLogStream(std::cout);
std::mutex nrfLogMutex;

#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

int main(int argc, char *argv[])
{
    Catch::Session session;

    auto serialPortA = std::string{};
    auto serialPortB = std::string{};
    auto iterations  = 0;
    auto baudRate    = DEFAULT_BAUD_RATE;
    auto logLevel    = SD_RPC_LOG_FATAL;

    using namespace Catch::clara;

    const auto cli = session.cli() | Opt(serialPortA, "serial-port")["--port-a"]("serial port A, usually BLE central") |
                     Opt(serialPortB, "serial-port")["--port-b"]("serial port B, usually BLE peripheral") |
                     Opt(baudRate, "baud-rate")["--baud-rate"]("baud rate") |
                         Opt(iterations, "count")["--iterations"](
                             "number of iterations (for tests supporting that)") |
                     Opt(
                         [&](const std::string &value) {
                             if (value.compare("trace"))
                             {
                                 logLevel = SD_RPC_LOG_TRACE;
                             }
                             else if (value.compare("debug"))
                             {
                                 logLevel = SD_RPC_LOG_DEBUG;
                             }
                             else if (value.compare("info"))
                             {
                                 logLevel = SD_RPC_LOG_INFO;
                             }
                             else if (value.compare("warning"))
                             {
                                 logLevel = SD_RPC_LOG_WARNING;
                             }
                             else if (value.compare("error"))
                             {
                                 logLevel = SD_RPC_LOG_ERROR;
                             }
                             else if (value.compare("fatal"))
                             {
                                 logLevel = SD_RPC_LOG_FATAL;
                             }
                         },
                         "trace|debug|info|warning|error|fatal")["--log-level"]("pc-ble-driver log level");

    session.cli(cli);

    const auto exitCode = session.applyCommandLine(argc, argv);

    if (exitCode != 0)
        return exitCode;

    test::ConfiguredEnvironment.serialPorts.emplace_back(serialPortA, 0);
    test::ConfiguredEnvironment.serialPorts.emplace_back(serialPortB, 0);
    test::ConfiguredEnvironment.numberOfIterations = iterations;
    test::ConfiguredEnvironment.baudRate           = baudRate;

    return session.run();
}
