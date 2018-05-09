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

#ifndef SD_RPC_TYPES_H__
#define SD_RPC_TYPES_H__

#include "adapter.h"

#include "ble.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SD_RPC_MAXPATHLEN 512

/**@brief Error codes that an error callback can be associated with. */
typedef struct {
    char port[SD_RPC_MAXPATHLEN];
    char manufacturer[SD_RPC_MAXPATHLEN];
    char serialNumber[SD_RPC_MAXPATHLEN];
    char pnpId[SD_RPC_MAXPATHLEN];
    char locationId[SD_RPC_MAXPATHLEN];
    char vendorId[SD_RPC_MAXPATHLEN];
    char productId[SD_RPC_MAXPATHLEN];
} sd_rpc_serial_port_desc_t;

/**@brief Error codes that an error callback can be associated with. */
typedef enum
{
    PKT_SEND_MAX_RETRIES_REACHED,
    PKT_UNEXPECTED,
    PKT_ENCODE_ERROR,
    PKT_DECODE_ERROR,
    PKT_SEND_ERROR,
    IO_RESOURCES_UNAVAILABLE,
    RESET_PERFORMED,
    CONNECTION_ACTIVE
} sd_rpc_app_status_t;

/**@brief Levels of severity that a log message can be associated with. */
typedef enum
{
    SD_RPC_LOG_TRACE,
    SD_RPC_LOG_DEBUG,
    SD_RPC_LOG_INFO,
    SD_RPC_LOG_WARNING,
    SD_RPC_LOG_ERROR,
    SD_RPC_LOG_FATAL
} sd_rpc_log_severity_t;

/**@brief Flow control modes */
typedef enum
{
    SD_RPC_FLOW_CONTROL_NONE,
    SD_RPC_FLOW_CONTROL_HARDWARE
} sd_rpc_flow_control_t;

/**@brief Parity modes */
typedef enum
{
    SD_RPC_PARITY_NONE,
    SD_RPC_PARITY_EVEN
} sd_rpc_parity_t;

/**@brief Reset modes to specify how the connectivity firmware will perform a reset. */
typedef enum
{
    SYS_RESET,      /** System reset of the connectivity chip, all state is reset. */
    SOFT_RESET,     /** Reset transport and SoftDevice related states only. */
} sd_rpc_reset_t;

/**@brief Function pointer type for event callbacks.
*/
typedef void(*sd_rpc_status_handler_t)(adapter_t *adapter, sd_rpc_app_status_t code, const char * message);
typedef void(*sd_rpc_evt_handler_t)(adapter_t *adapter, ble_evt_t * p_ble_evt);
typedef void(*sd_rpc_log_handler_t)(adapter_t *adapter, sd_rpc_log_severity_t severity, const char * log_message);

#ifdef __cplusplus
}
#endif
#endif // SD_RPC_TYPES_H__
