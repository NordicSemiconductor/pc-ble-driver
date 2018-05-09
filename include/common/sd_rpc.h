/*
 * Copyright (c) 2016 Nordic Semiconductor ASA
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
* @brief Type definitions and API calls for SoftDevice RPC module.
*
*/

#ifndef SD_RPC_H__
#define SD_RPC_H__

#include "config/platform.h"
#include "sd_rpc_types.h"
#include "adapter.h"

#include "ble.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**@brief Enumerate available serial ports.
 *
 * @param[out] serial_port_descs  The array of serial port descriptors to be filled in.
 * @param[in,out]  size The size of the array. The number of ports found is stored here.
 *
 * @retval NRF_SUCCESS  The serial ports were enumerated successfully.
 * @retval NRF_ERROR    There was an error enumerating the serial ports.
 */
SD_RPC_API uint32_t sd_rpc_serial_port_enum(sd_rpc_serial_port_desc_t serial_port_descs[], uint32_t *size);

/**@brief Create a new serial physical layer.
 *
 * @param[in]  port_name  The serial port name.
 * @param[in]  baud_rate  The serial port speed.
 * @param[in]  flow_control  The flow control scheme to use.
 * @param[in]  parity  The parity scheme to use.
 *
 * @retval The physical layer or NULL.
 */
SD_RPC_API physical_layer_t *sd_rpc_physical_layer_create_uart(const char * port_name, uint32_t baud_rate, sd_rpc_flow_control_t flow_control, sd_rpc_parity_t parity);

/**@brief Create a new data link layer.
 *
 * @param[in]  physical_layer  The physical layer to use with this data link layer.
 * @param[in]  response_timeout  Response timeout of the data link layer.
 *
 * @retval The data link layer or NULL.
 */
SD_RPC_API data_link_layer_t *sd_rpc_data_link_layer_create_bt_three_wire(physical_layer_t *physical_layer, uint32_t retransmission_interval);

/**@brief Create a new transport layer.
 *
 * @param[in]  data_link_layer  The data linkk layer to use with this transport.
 * @param[in]  response_timeout  Response timeout.
 *
 * @retval The transport layer or NULL.
 */
SD_RPC_API transport_layer_t *sd_rpc_transport_layer_create(data_link_layer_t *data_link_layer, uint32_t response_timeout);

/**@brief Create a new transport adapter.
 *
 * @param[in]  transport_layer  The transport layer to use with this adapter.
 *
 * @retval The adapter or NULL.
 */
SD_RPC_API adapter_t *sd_rpc_adapter_create(transport_layer_t* transport_layer);

/**@brief Delete a transport adapter.
 *
 * @param[in]  adapter  The transport adapter.
 *
 */
SD_RPC_API void sd_rpc_adapter_delete(adapter_t *adapter);

/**@brief Initialize the SoftDevice RPC module.
 *
 * @note This function must be called prior to the sd_ble_* API commands.
 *       The serial port will be attempted opened with the configured serial port settings.
 *
 * @param[in]  adapter  The transport adapter.
 * @param[in]  status_handler  The status handler callback.
 * @param[in]  evt_handler  The event handler callback.
 * @param[in]  log_handler  The log handler callback.
 *
 * @retval NRF_SUCCESS  The module was opened successfully.
 * @retval NRF_ERROR    There was an error opening the module.
 */
SD_RPC_API uint32_t sd_rpc_open(adapter_t *adapter, sd_rpc_status_handler_t status_handler, sd_rpc_evt_handler_t event_handler, sd_rpc_log_handler_t log_handler);

/**@brief Close the SoftDevice RPC module.
 *
 * @note This function will close the serial port and release allocated resources.
 *
 * @param[in]  adapter  The transport adapter.
 *
 * @retval NRF_SUCCESS  The module was closed successfully.
 * @retval NRF_ERROR    There was an error closing the module.
 */
SD_RPC_API uint32_t sd_rpc_close(adapter_t *adapter);

/**@brief Set the lowest log level for messages to be logged to handler.
 *        Default log handler severity filter is LOG_INFO.
 *
 * @param[in]  adapter  The transport adapter.
 * @param[in]  severity_filter  The lowest severity level messages should be logged.
 *
 * @retval NRF_SUCCESS              severity_filter is valid.
 * @retval NRF_ERROR_INVALID_PARAM  severity_filter is not one of the valid enum values
 *                                  in app_log_severity_t
 */
SD_RPC_API uint32_t sd_rpc_log_handler_severity_filter_set(adapter_t *adapter, sd_rpc_log_severity_t severity_filter);

/**@brief Reset the connectivity firmware.
 *
 * @param[in]  adapter      The transport adapter.
 * @param[in]  reset_mode   The reset mode to perform in connectivity firmware.
 *
 * @retval NRF_SUCCESS  The connectivity chip was reset successfully.
 * @retval NRF_ERROR    There was an error reset the connectivity chip.
 */
SD_RPC_API uint32_t sd_rpc_conn_reset(adapter_t *adapter, sd_rpc_reset_t reset_mode);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SD_RPC_H__
