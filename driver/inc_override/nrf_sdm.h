/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */ 

#ifndef NRF_SDM_OVERRIDE_H__
#define NRF_SDM_OVERRIDE_H__

#include "nrf_svc.h"
#include "nrf_soc.h"

#undef SVCALL
#define SVCALL(number, return_type, signature) EXTERN_C return_type signature

#include "headers/nrf_sdm.h"

#undef SVCALL
#define SVCALL(number, return_type, signature) SD_RPC_API return_type signature

#endif // NRF_SDM_OVERRIDE_H__
