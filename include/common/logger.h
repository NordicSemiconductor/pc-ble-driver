#pragma once

#include <memory>
#include <vector>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

static const std::string logger_name = "driver";
std::shared_ptr<spdlog::logger> setup_logger(std::vector<spdlog::sink_ptr> sinks);
std::shared_ptr<spdlog::logger> get_logger();
