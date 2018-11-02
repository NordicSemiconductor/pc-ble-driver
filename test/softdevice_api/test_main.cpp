// Logging support

#include <iostream>
#include <mutex>

std::ostream &nrfLogStream(std::cout);
std::mutex nrfLogMutex;

#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

int main(int argc, char *argv[])
{
    Catch::Session session;

    std::string serialPortA;
    std::string serialPortB;
    uint32_t iterations;

    using namespace Catch::clara;

    const auto cli = session.cli() |
                     Opt(serialPortA, "serial port A")["-pa"]["--porta"]("Serial port A") |
                     Opt(serialPortB, "serial port B")["-pb"]["--portb"]("Serial port B") |
                     Opt(iterations, "iterations")["-it"]["--iterations"](
                         "Number of iterations. "
                         "Ignored for tests not having iterations.");

    session.cli(cli);

    const auto exitCode = session.applyCommandLine(argc, argv);

    if (exitCode != 0)
        return exitCode;
}
