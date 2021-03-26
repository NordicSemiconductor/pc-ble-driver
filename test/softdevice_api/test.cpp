#include <iostream>
#include <mutex>
#include <test_environment.h>

#include "logging.h"
#include "spdlog/spdlog.h"
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::ostream &nrfLogStream(std::cout);
std::mutex nrfLogMutex;

#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

int main(int argc, char *argv[])
{
    Catch::Session session;

    auto serialPortA  = std::string{};
    auto serialPortB  = std::string{};
    auto hardwareInfo = std::string{};
    auto baudRate     = defaultBaudRate;

    using namespace Catch::clara;

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

    setup_logger(sinks);
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%H:%M:%S.%f] [t:%t] [%^%l%$] %v");

    const auto cli =
        session.cli() |
        Opt(serialPortA, "serial-port")["--port-a"]("serial port A, usually BLE central") |
        Opt(serialPortB, "serial-port")["--port-b"]("serial port B, usually BLE peripheral") |
        Opt(test::ConfiguredEnvironment.baudRate, "baud-rate")["--baud-rate"]("baud rate") |
        Opt(test::ConfiguredEnvironment.responseTimeout,
            "milliseconds")["--response-timeout"]("Transport response timeout") |
        Opt(test::ConfiguredEnvironment.retransmissionInterval,
            "milliseconds")["--retransmission-interval"]("Transport retransmission interval") |
        Opt(test::ConfiguredEnvironment.mtu,
            "size")["--ble-mtu"]("Default BLE MTU, may be ignored in some tests") |
        Opt(test::ConfiguredEnvironment.numberOfIterations,
            "count")["--iterations"]("number of iterations (for tests supporting that)") |
        Opt(
            [&](const std::string &value) {
                if (!value.empty())
                {
                    try
                    {
                        auto logLevel = testutil::parseSpdLogLevel(value);
                        spdlog::set_level(logLevel);
                    }
                    catch (std::invalid_argument &)
                    {
                        INFO("Log level '" << value << "' not supported.");
                    }
                }
            },
            "trace|debug|info|warning|error|fatal")["--log-level"]("Logger log level") |
        Opt(
            [&](const std::string &value) {
                if (!value.empty())
                {
                    try
                    {
                        test::ConfiguredEnvironment.driverLogLevel =
                            testutil::parseLogSeverity(value);
                        test::ConfiguredEnvironment.driverLogLevelSet = true;
                    }
                    catch (std::invalid_argument &)
                    {
                        INFO("Log level '" << value << "' not supported.");
                    }
                }
            },
            "trace|debug|info|warning|error|fatal")["--driver-log-level"]("Driver log level") |
        Opt(hardwareInfo, "text")["--hardware-info"]("hardware info text to show in test reports");

    session.cli(cli);

    const auto exitCode = session.applyCommandLine(argc, argv);

    if (exitCode != 0)
    {
        get_logger()->error("Parsing comman line arguments returned error {}", exitCode);
        return exitCode;
    }

    if (!serialPortA.empty())
    {
        test::ConfiguredEnvironment.serialPorts.emplace_back(serialPortA, baudRate);
    }

    if (!serialPortB.empty())
    {
        test::ConfiguredEnvironment.serialPorts.emplace_back(serialPortB, baudRate);
    }

    if (!hardwareInfo.empty())
    {
        test::ConfiguredEnvironment.hardwareInfo = hardwareInfo;
    }
    else
    {
        test::ConfiguredEnvironment.hardwareInfo = "No hardware info provided.";
    }

    // The return code from the Catch2 framework is a bit different then ordinary retur codes.
    // Source for this information:
    //     https://github.com/catchorg/Catch2/blob/c12170ff69ddc9a0a25ec2025783b815354c6d26/src/catch2/catch_session.cpp#L258
    const auto ret = session.run();

    switch (ret)
    {
        case 1:
            get_logger()->error("Exceptions starting test framework, session.run() returned {}",
                                ret);
            break;
        default:
            // This is correct for value of 2 also if no warning is set for no tests found
            // To do run with warning if there are no tests, speciy '-w NoTests' when running the
            // tests
            get_logger()->error("{} tests failed", ret);
            break;
    }

    spdlog::shutdown();

    return ret;
}
