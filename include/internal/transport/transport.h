/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "sd_rpc_types.h"

#include <functional>
#include <string>
#include <vector>

#include <stdint.h>

typedef std::function<void(sd_rpc_app_status_t code, const char *message)> status_cb_t;
typedef std::function<void(uint8_t *data, size_t length)> data_cb_t;
typedef std::function<void(sd_rpc_log_severity_t severity, std::string message)> log_cb_t;

class Transport {
public:
    virtual ~Transport();
    virtual uint32_t open(status_cb_t status_callback, data_cb_t data_callback, log_cb_t log_callback);
    virtual uint32_t close();
    virtual uint32_t send(std::vector<uint8_t> &data) = 0;

protected:
    Transport();

    status_cb_t statusCallback;
    data_cb_t dataCallback;
    log_cb_t logCallback;
};

#endif //TRANSPORT_H
