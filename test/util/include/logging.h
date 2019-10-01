#pragma once

#include <memory>
#include <vector>
#include "spdlog/logger.h"

static const std::string logger_name = "test";
std::shared_ptr<spdlog::logger> setup_logger(std::vector<spdlog::sink_ptr> sinks);
std::shared_ptr<spdlog::logger> get_logger();
