// Logging support
#include "internal/log.h"
#include "test_setup.h"
#include "test_util.h"

// Test framework
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "transport.h"
#include "h5_transport.h"
#include "h5.h"
#include "nrf_error.h"
#include "virtual_uart.h"

#if defined(_MSC_VER)
// Disable warning "This function or variable may be unsafe. Consider using _dupenv_s instead."
#pragma warning(disable: 4996)
#endif

using namespace std::chrono_literals;
using std::chrono::system_clock;

std::fstream logFile(
    "test_virtual_uart.txt",
    std::fstream::out | std::fstream::trunc
);

TEST_CASE("virtual_uart")
{
    setLogCallback([](const char *message) -> void
    {
        // You should not use stdout/stderr on Windows since this will have a huge impact on
        // the application since this logger is not offloading the displaying of data
        // to a separate thread. 
        //
        // This stack overflow thread contains more info in regards to cmd.exe output:
        //   https://stackoverflow.com/questions/7404551/why-is-console-output-so-slow

        logFile << message << std::flush;
    });

    SECTION("open close")
    {
        auto uartA = new VirtualUart("uartA");
        auto uartB = new VirtualUart("uartB");

        uartA->setPeer(uartB);
        uartB->setPeer(uartA);

        // Check if implementation return error if user try to send before UART is opened
        REQUIRE(uartA->send(payload_t{ 0xaa }) == NRF_ERROR_INTERNAL);
        REQUIRE(uartB->send(payload_t{ 0xbb }) == NRF_ERROR_INTERNAL);

        payload_t payloadFromB;
        payload_t payloadFromA;

        auto statusCallback = [=](const std::string name)
        {
            return [=](sd_rpc_app_status_t code, const char *message) -> void
            {
                DEBUG("[" << name << "][status] code: " << code << " message: " << message);
            };
        };

        auto logCallback = [=](const std::string name)
        {
            return [=](sd_rpc_log_severity_t severity, std::string &message) -> void
            {
                DEBUG("[" << name << "][log] severity: " << severity << " message: " << message);
            };
        };

        auto dataCallback = [](const std::string name, payload_t &payloadReceived)
        {
            return [name, &payloadReceived](uint8_t *data, size_t length) -> void
            {
                payloadReceived.assign(data, data + length);
                DEBUG("[" << name << "][data]<- " << testutil::convertToString(payloadReceived) << " length: " << length);
            };
        };

        const std::string uartAName = "uartA";

        uartA->open(
            statusCallback(uartAName),
            dataCallback(uartAName, payloadFromB),
            logCallback(uartAName)
        );

        const std::string uartBName = "uartB";

        uartB->open(
            statusCallback(uartBName),
            dataCallback(uartBName, payloadFromA),
            logCallback(uartBName)
        );

        auto payloadToB = payload_t{ 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa };
        auto payloadToA = payload_t{ 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb };

        REQUIRE(uartA->send(payloadToB) == NRF_SUCCESS);
        REQUIRE(uartB->send(payloadToA) == NRF_SUCCESS);

        // Wait for data to be sent between transports
        std::this_thread::sleep_for(std::chrono::seconds(1));

        REQUIRE(std::equal(payloadToB.begin(), payloadToB.end(), payloadFromA.begin()) == true);
        REQUIRE(std::equal(payloadToA.begin(), payloadToA.end(), payloadFromB.begin()) == true);

        delete uartA;
        delete uartB;
    }
}
