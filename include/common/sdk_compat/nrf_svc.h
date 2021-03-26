#ifndef NRF_SVC_H__
#define NRF_SVC_H__

#include "platform.h"
#define SVCALL(number, return_type, signature) SD_RPC_API return_type signature

#include "adapter.h"

#endif // NRF_SVC_H__
