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

const char* SEGGER_VENDOR_ID = "1366";
const char* NXP_VENDOR_ID = "0d28";

std::string to_str(const char* s)
{
    // NOLINTNEXTLINE(modernize-use-nullptr)
    return (s != NULL) ? std::string(s) : std::string();
}

std::list<SerialPortDesc> EnumSerialPorts()
{
    // Setup return value
    std::list<SerialPortDesc> devices;

    // Setup udev related variables
    struct udev *udev_ctx = udev_new();
    assert(udev_ctx != NULL); // NOLINT(modernize-use-nullptr)

    struct udev_enumerate *udev_enum = udev_enumerate_new(udev_ctx);
    assert(udev_enum != NULL); // NOLINT(modernize-use-nullptr)

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

        std::string idVendor = to_str(udev_device_get_sysattr_value(udev_usb_dev, "idVendor"));
        std::string manufacturer = to_str(udev_device_get_sysattr_value(udev_usb_dev,"manufacturer"));

        if(
          ((idVendor == SEGGER_VENDOR_ID) || (idVendor == NXP_VENDOR_ID))
          && ((manufacturer == "SEGGER")
              || (strncasecmp(manufacturer.c_str(), "arm", 3) == 0)
              || (strncasecmp(manufacturer.c_str(), "mbed", 4) == 0))
          )
        {
            std::string serialNumber = to_str(udev_device_get_sysattr_value(udev_usb_dev, "serial"));
            std::string idProduct = to_str(udev_device_get_sysattr_value(udev_usb_dev, "idProduct"));

            devices.push_back(SerialPortDesc {
              devname,
              manufacturer,
              serialNumber,
              "",
              path,
              idVendor,
              idProduct
            });
        }

        udev_device_unref(udev_tty_dev);
    }

    udev_enumerate_unref(udev_enum);
    udev_unref(udev_ctx);

    return devices;
}
