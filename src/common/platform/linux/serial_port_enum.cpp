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

#include <libudev.h>

#include <vector>
#include <cstring>
#include <cassert>

#include <sys/param.h>

#include "serial_port_enum.h"

using namespace std;

typedef struct serial_device_t {
    char port[MAXPATHLEN];
    char locationId[MAXPATHLEN];
    char vendorId[MAXPATHLEN];
    char productId[MAXPATHLEN];
    char manufacturer[MAXPATHLEN];
    char serialNumber[MAXPATHLEN];
} serial_device_t;

const char* SEGGER_VENDOR_ID = "1366";
const char* NXP_VENDOR_ID = "0d28";

typedef vector<serial_device_t*> adapter_list_t;

static adapter_list_t* GetAdapters()
{
    // Setup return value
    adapter_list_t* devices = new adapter_list_t();

    // Setup udev related variables
    struct udev *udev_ctx = udev_new();
    assert(udev_ctx != NULL);

    struct udev_enumerate *udev_enum = udev_enumerate_new(udev_ctx);
    assert(udev_enum != NULL);

    udev_enumerate_add_match_subsystem(udev_enum, "tty");
    udev_enumerate_scan_devices(udev_enum);

    struct udev_list_entry *udev_devices = udev_enumerate_get_list_entry(udev_enum);
    struct udev_list_entry *udev_entry;
    struct udev_device *udev_tty_dev, *udev_usb_dev;

    udev_list_entry_foreach(udev_entry, udev_devices)
    {
        const char *path = udev_list_entry_get_name(udev_entry);

        udev_tty_dev = udev_device_new_from_syspath(udev_ctx, path);
        const char *devname = udev_device_get_devnode(udev_tty_dev);

        udev_usb_dev = udev_device_get_parent_with_subsystem_devtype(
            udev_tty_dev,
            "usb",
            "usb_device"
        );

        const char *idVendor = udev_device_get_sysattr_value(udev_usb_dev, "idVendor");

        // Only add SEGGER and ARM (even though VENDOR_ID is NXPs...) devices to list
        if(idVendor != NULL && ((strcmp(idVendor, SEGGER_VENDOR_ID) == 0) || (strcmp(idVendor, NXP_VENDOR_ID) == 0)))
        {
            serial_device_t *serial_device = (serial_device_t*)malloc(sizeof(serial_device_t));
            memset(serial_device, 0, sizeof(serial_device_t));

            strcpy(serial_device->vendorId, idVendor);
            strcpy(serial_device->port, devname);
            strcpy(serial_device->locationId, path);
            strcpy(serial_device->productId, udev_device_get_sysattr_value(udev_usb_dev, "idProduct"));
            strcpy(serial_device->manufacturer, udev_device_get_sysattr_value(udev_usb_dev,"manufacturer"));
            strcpy(serial_device->serialNumber, udev_device_get_sysattr_value(udev_usb_dev, "serial"));

            devices->push_back(serial_device);
        }

        udev_device_unref(udev_tty_dev);
    }

    udev_enumerate_unref(udev_enum);
    udev_unref(udev_ctx);

    return devices;
}
uint32_t EnumSerialPorts(std::list<SerialPortDesc*>& descs)
{

    adapter_list_t* devices = GetAdapters();

    for(auto device : *devices)
    {
        if((strcmp(device->manufacturer,"SEGGER") == 0)
            || (strcasecmp(device->manufacturer, "arm") == 0)
            || (strcasecmp(device->manufacturer, "mbed") == 0))
        {
            SerialPortDesc* resultItem = new SerialPortDesc();

            resultItem->comName = device->port;

            if (device->locationId != NULL) {
                resultItem->locationId = device->locationId;
            }

            if (device->vendorId != NULL) {
                resultItem->vendorId = device->vendorId;
            }

            if (device->productId != NULL) {
                resultItem->productId = device->productId;
            }

            if (device->manufacturer != NULL) {
                resultItem->manufacturer = device->manufacturer;
            }

            if (device->serialNumber != NULL) {
                resultItem->serialNumber = device->serialNumber;
            }

            descs.push_back(resultItem);
        }

        free(device);
    }

    devices->clear();
    delete devices;
    
    return 0;
}
