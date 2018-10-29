// Logging support

#include <iostream>
#include <mutex>
#include <sstream>

std::ostream &nrfLogStream(std::cout);
std::mutex nrfLogMutex;

#include <internal/log.h>

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
