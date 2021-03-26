// clang-format off

#ifndef SD_RPC_TYPES_H__
#define SD_RPC_TYPES_H__

#include "adapter.h"

#include "ble.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Error codes that an error callback can be associated with. */
typedef enum {
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
typedef enum {
    SD_RPC_LOG_TRACE,
    SD_RPC_LOG_DEBUG,
    SD_RPC_LOG_INFO,
    SD_RPC_LOG_WARNING,
    SD_RPC_LOG_ERROR,
    SD_RPC_LOG_FATAL
} sd_rpc_log_severity_t;

/**@brief Flow control modes */
typedef enum { SD_RPC_FLOW_CONTROL_NONE, SD_RPC_FLOW_CONTROL_HARDWARE } sd_rpc_flow_control_t;

/**@brief Parity modes */
typedef enum { SD_RPC_PARITY_NONE, SD_RPC_PARITY_EVEN } sd_rpc_parity_t;

/**@brief Reset modes to specify how the connectivity firmware will perform a reset. */
typedef enum {
    SYS_RESET,  /** System reset of the connectivity chip, all state is reset. */
    SOFT_RESET, /** Reset transport and SoftDevice related states only. */
} sd_rpc_reset_t;

/**@bref Error codes for SD_RPC related errors */
#define NRF_ERROR_SD_RPC_BASE_NUM (NRF_ERROR_BASE_NUM + 0x8000)

#define NRF_ERROR_SD_RPC_ENCODE (NRF_ERROR_SD_RPC_BASE_NUM + 1)
#define NRF_ERROR_SD_RPC_DECODE (NRF_ERROR_SD_RPC_BASE_NUM + 2)
#define NRF_ERROR_SD_RPC_SEND (NRF_ERROR_SD_RPC_BASE_NUM + 3)
#define NRF_ERROR_SD_RPC_INVALID_ARGUMENT (NRF_ERROR_SD_RPC_BASE_NUM + 4)
#define NRF_ERROR_SD_RPC_NO_RESPONSE (NRF_ERROR_SD_RPC_BASE_NUM + 5)
#define NRF_ERROR_SD_RPC_INVALID_STATE (NRF_ERROR_SD_RPC_BASE_NUM + 6)

#define NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT (NRF_ERROR_SD_RPC_BASE_NUM + 20)
#define NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT_INVALID_STATE (NRF_ERROR_SD_RPC_BASE_NUM + 21)
#define NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT_NO_RESPONSE (NRF_ERROR_SD_RPC_BASE_NUM + 22)
#define NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT_ALREADY_OPEN (NRF_ERROR_SD_RPC_BASE_NUM + 23)
#define NRF_ERROR_SD_RPC_SERIALIZATION_TRANSPORT_ALREADY_CLOSED (NRF_ERROR_SD_RPC_BASE_NUM + 24)

#define NRF_ERROR_SD_RPC_H5_TRANSPORT (NRF_ERROR_SD_RPC_BASE_NUM + 40)
#define NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE (NRF_ERROR_SD_RPC_BASE_NUM + 41)
#define NRF_ERROR_SD_RPC_H5_TRANSPORT_NO_RESPONSE (NRF_ERROR_SD_RPC_BASE_NUM + 42)
#define NRF_ERROR_SD_RPC_H5_TRANSPORT_SLIP_PAYLOAD_SIZE (NRF_ERROR_SD_RPC_BASE_NUM + 43)
#define NRF_ERROR_SD_RPC_H5_TRANSPORT_SLIP_CALCULATED_PAYLOAD_SIZE (NRF_ERROR_SD_RPC_BASE_NUM + 44)
#define NRF_ERROR_SD_RPC_H5_TRANSPORT_SLIP_DECODING (NRF_ERROR_SD_RPC_BASE_NUM + 45)
#define NRF_ERROR_SD_RPC_H5_TRANSPORT_HEADER_CHECKSUM (NRF_ERROR_SD_RPC_BASE_NUM + 46)
#define NRF_ERROR_SD_RPC_H5_TRANSPORT_PACKET_CHECKSUM (NRF_ERROR_SD_RPC_BASE_NUM + 47)
#define NRF_ERROR_SD_RPC_H5_TRANSPORT_ALREADY_OPEN (NRF_ERROR_SD_RPC_BASE_NUM + 48)
#define NRF_ERROR_SD_RPC_H5_TRANSPORT_ALREADY_CLOSED (NRF_ERROR_SD_RPC_BASE_NUM + 49)
#define NRF_ERROR_SD_RPC_H5_TRANSPORT_INTERNAL_ERROR (NRF_ERROR_SD_RPC_BASE_NUM + 50)

#define NRF_ERROR_SD_RPC_SERIAL_PORT (NRF_ERROR_SD_RPC_BASE_NUM + 60)
#define NRF_ERROR_SD_RPC_SERIAL_PORT_STATE (NRF_ERROR_SD_RPC_BASE_NUM + 61)
#define NRF_ERROR_SD_RPC_SERIAL_PORT_ALREADY_OPEN (NRF_ERROR_SD_RPC_BASE_NUM + 62)
#define NRF_ERROR_SD_RPC_SERIAL_PORT_ALREADY_CLOSED (NRF_ERROR_SD_RPC_BASE_NUM + 63)
#define NRF_ERROR_SD_RPC_SERIAL_PORT_INTERNAL_ERROR (NRF_ERROR_SD_RPC_BASE_NUM + 64)


typedef struct
{
    sd_rpc_log_severity_t severity;
    const char* message;
} sd_rpc_log_t;

typedef struct 
{
    sd_rpc_app_status_t code;
    const char *message;
} sd_rpc_status_t;

/**@brief Function pointer type for event callbacks. */
typedef void (*sd_rpc_status_handler_t)(adapter_t *adapter, const sd_rpc_app_status_t code, const char *message, const void *user_data);
typedef void (*sd_rpc_log_handler_t)(adapter_t *adapter, const sd_rpc_log_t *p_log, const void *user_data);

typedef void (*sd_rpc_evt_handler_t)(adapter_t *adapter, const ble_evt_t *p_ble_evt, const void *user_data);

#ifdef __cplusplus
}
#endif
#endif // SD_RPC_TYPES_H__

// clang-format on
