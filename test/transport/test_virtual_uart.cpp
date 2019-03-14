// Logging support
#include <logger.h>

#include <test_setup.h>
#include <test_util.h>

// Test framework
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <nrf_error.h>
#include <virtual_uart.h>

#if defined(_MSC_VER)
// Disable warning "This function or variable may be unsafe. Consider using _dupenv_s instead."
#pragma warning(disable: 4996)
#endif

TEST_CASE("virtual_uart")
{
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

        const auto dataCallback = [](const std::string name, payload_t &payloadReceived)
        {
            return [name, &payloadReceived](const uint8_t *data, const size_t length) -> void
            {
                payloadReceived.assign(data, data + length);
                get_logger()->debug("[{}][data]<- {} length: {}", name, testutil::asHex(payloadReceived), length);
            };
        };

        const std::string uartAName = "uartA";

        uartA->open(
            [&uartAName](const sd_rpc_app_status_t code, const std::string &message) -> void
            {
                get_logger()->debug("[{}][status] code: {} message: {}", uartAName, code, message);
            },
            dataCallback(uartAName, payloadFromB),
            [&uartAName](const sd_rpc_log_severity_t severity, const std::string &message) -> void
            {
                get_logger()->debug("[{}][log] severity: {} message: {}", uartAName, severity, message);
            }
        );

        const std::string uartBName = "uartB";

        uartB->open(
            [&uartBName](const sd_rpc_app_status_t code, const std::string &message) -> void
            {
                get_logger()->debug("[{}][status] code: {} message: {}", uartBName, code, message);
            },
            dataCallback(uartBName, payloadFromA),
            [&uartBName](const sd_rpc_log_severity_t severity, const std::string &message) -> void
            {
                get_logger()->debug("[{}][log] severity: {} message: {}", uartBName, severity, message);
            }
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
