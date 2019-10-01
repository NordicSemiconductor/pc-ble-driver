#include "logging.h"

#include <memory>
#include <vector>

#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> setup_logger(std::vector<spdlog::sink_ptr> sinks)
{
    auto logger = spdlog::get(logger_name);

    if (logger == nullptr)
    {
        if (sinks.size() > 0)
        {
            logger =
                std::make_shared<spdlog::logger>(logger_name, std::begin(sinks), std::end(sinks));
            spdlog::register_logger(logger);
        }
        else
        {
            logger = spdlog::stdout_color_mt(logger_name);
        }
    }

    return logger;
}

std::shared_ptr<spdlog::logger> get_logger()
{
    return spdlog::get(logger_name);
}
