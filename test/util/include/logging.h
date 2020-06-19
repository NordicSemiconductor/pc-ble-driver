#pragma once

#include "spdlog/logger.h"
#include <memory>
#include <vector>

static const std::string logger_name = "test";
std::shared_ptr<spdlog::logger> setup_logger(std::vector<spdlog::sink_ptr> sinks);
std::shared_ptr<spdlog::logger> get_logger();
