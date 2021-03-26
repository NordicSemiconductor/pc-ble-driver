/** @file
 *
 * @brief Helper class for logging.
 *
 */

#pragma once

#include "sd_rpc_types.h"

#include <spdlog/common.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include <unordered_map>

// NOLINTNEXTLINE
constexpr char NRF_BLE_DRIVER_LOGGER_NAME[] = "nrf_ble_driver";

static std::shared_ptr<spdlog::logger> nrfdl_logger;

auto getLogger() noexcept -> std::shared_ptr<spdlog::logger>;

/**@brief Log helper.
 *
 * This class are static helper functions related to logging.
 */
class LogHelper
{
  private:
    static std::unordered_map<sd_rpc_log_severity_t, spdlog::level::level_enum> apilevel_map;
    static std::unordered_map<spdlog::level::level_enum, sd_rpc_log_severity_t> spdloglevel_map;
    static void initLogLevelMaps();

  public:
    /**@brief Convert to Spd log level.
     *
     * Convert a Nordic Semiconductor log level to a Spd log level.
     *
     * @param[in]   level  Nordic Semiconductor log level.
     * @return      Pair of bool and spd log level. Bool is true if conversion was successful and
     * false otherwise.
     */
    static auto levelToSpdlog(sd_rpc_log_severity_t level)
        -> std::pair<bool, spdlog::level::level_enum>;

    /**@brief Convert to API log level.
     *
     * Convert a SPD log level to a API log level.
     *
     * @param[in]   level  spdlog level.
     * @return      Pair of bool and spd log level. Bool is true if conversion was successful and
     * false otherwise.
     */
    static auto levelToAPI(spdlog::level::level_enum level)
        -> std::pair<bool, sd_rpc_log_severity_t>;

    static void tryToLog(const std::shared_ptr<spdlog::logger> &logger,
                         const spdlog::level::level_enum &level, const std::string &msg) noexcept;
    static void tryToLog(const spdlog::level::level_enum &level, const std::string &msg) noexcept;

    static void tryToLogException(const std::shared_ptr<spdlog::logger> &logger,
                                  const spdlog::level::level_enum &level,
                                  const std::exception &exc) noexcept;

    static void tryToLogException(const spdlog::level::level_enum &level, const std::exception &exc,
                                  const char *msg = "") noexcept;
};
