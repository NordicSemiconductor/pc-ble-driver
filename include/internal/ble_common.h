#ifndef BLE_COMMON_IMPL_H__
#define BLE_COMMON_IMPL_H__

#include <functional>
#include "adapter.h"
#include <stdint.h>
#include "app_ble_gap_sec_keys.h"

typedef std::function<uint32_t(uint8_t*, uint32_t*)> encode_function_t;
typedef std::function<uint32_t(uint8_t*, uint32_t, uint32_t*)> decode_function_t;

uint32_t encode_decode(adapter_t *adapter, encode_function_t encode_function, decode_function_t decode_function);

/*
 * We do not want to change the codecs provided by the SDK too much. The BLESecurityContext provides a way to set the root
 * security context before calling the codecs. Typically the root context is the SerializationTransport object.
*/

class BLESecurityContext
{
private:
    void *context;

public:
    explicit BLESecurityContext(void* context) : context(context)
    {
        app_ble_gap_sec_context_root_set(context);
    }

    ~BLESecurityContext()
    {
        app_ble_gap_sec_context_root_release();
    }
};

#endif

