/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
*
* The information contained herein is confidential property of Nordic Semiconductor. The use,
* copying, transfer or disclosure of such information is prohibited except by express written
* agreement with Nordic Semiconductor.
*
*/

#ifndef ADAPTER_H__
#define ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    void *internal;
} adapter_t;

typedef struct
{
    void *internal;
} transport_layer_t;

typedef struct
{
    void *internal;
} data_link_layer_t;

typedef struct
{
    void *internal;
} physical_layer_t;

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_H__
