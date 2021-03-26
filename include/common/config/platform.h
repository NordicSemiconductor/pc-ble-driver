#ifndef PLATFORM_H__
#define PLATFORM_H__

#ifdef __cplusplus
#define SD_RPC_EXTERN_C extern "C"
#else
#define SD_RPC_EXTERN_C
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifdef PC_BLE_DRIVER_STATIC // Static build of the library
#define SD_RPC_API
#else
#ifdef SD_RPC_EXPORTS
#define SD_RPC_API SD_RPC_EXTERN_C __declspec(dllexport)
#else
#define SD_RPC_API SD_RPC_EXTERN_C __declspec(dllimport)
#endif
#endif // PC_BLE_DRIVER_STATIC
#else
#ifdef PC_BLE_DRIVER_STATIC
#define SD_RPC_API
#else
#ifdef SD_RPC_EXPORTS
#define SD_RPC_API SD_RPC_EXTERN_C __attribute__((visibility("default")))
#else
#define SD_RPC_API SD_RPC_EXTERN_C
#endif
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_H__
