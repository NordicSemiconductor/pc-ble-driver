/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of other
 *   contributors to this software may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *   4. This software must only be used in or with a processor manufactured by Nordic
 *   Semiconductor ASA, or in or with a processor manufactured by a third party that
 *   is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *   5. Any software provided in binary or object form under this license must not be
 *   reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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
