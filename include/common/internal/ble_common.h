#pragma once

#include "adapter.h"
#include <functional>
#include <stdint.h>

typedef std::function<uint32_t(uint8_t *, uint32_t *)> encode_function_t;
typedef std::function<uint32_t(uint8_t *, uint32_t, uint32_t *)> decode_function_t;

uint32_t encode_decode(adapter_t *adapter, const encode_function_t &encode_function,
                       const decode_function_t &decode_function);

/*
 * We do not want to change the codecs provided by the SDK too much. The BLESecurityContext provides
 * a way to set the root security context before calling the codecs. Typically the root context is
 * the SerializationTransport object.
 */

class RequestReplyCodecContext
{
  public:
    explicit RequestReplyCodecContext(void *adapterId);
    ~RequestReplyCodecContext();
    RequestReplyCodecContext(const RequestReplyCodecContext &) = delete;
    RequestReplyCodecContext &operator=(const RequestReplyCodecContext &) = delete;
    RequestReplyCodecContext(RequestReplyCodecContext &&)                 = delete;
    RequestReplyCodecContext &operator=(RequestReplyCodecContext &&) = delete;
};

class EventCodecContext
{
  public:
    explicit EventCodecContext(void *adapterId);
    ~EventCodecContext();
    EventCodecContext(const EventCodecContext &) = delete;
    EventCodecContext &operator=(const EventCodecContext &) = delete;
    EventCodecContext(EventCodecContext &&)                 = delete;
    EventCodecContext &operator=(EventCodecContext &&) = delete;
};
