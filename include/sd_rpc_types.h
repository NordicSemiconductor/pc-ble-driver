/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
*
* The information contained herein is confidential property of Nordic Semiconductor. The use,
* copying, transfer or disclosure of such information is prohibited except by express written
* agreement with Nordic Semiconductor.
*
*/

#ifndef SD_RPC_TYPES_H__
#define SD_RPC_TYPES_H__

#include "adapter.h"

#include "ble.h"

#include <stdint.h>

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

/**@brief Function pointer type for event callbacks.
*/
typedef void(*sd_rpc_status_handler_t)(adapter_t *adapter, sd_rpc_app_status_t code, const char * message);
typedef void(*sd_rpc_evt_handler_t)(adapter_t *adapter, ble_evt_t * p_ble_evt);
typedef void(*sd_rpc_log_handler_t)(adapter_t *adapter, sd_rpc_log_severity_t severity, const char * log_message);

#endif // SD_RPC_TYPES_H__
