/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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
    struct udev_device *udev_dev;

    udev_list_entry_foreach(udev_entry, udev_devices)
    {
        const char *path = udev_list_entry_get_name(udev_entry);

        udev_dev = udev_device_new_from_syspath(udev_ctx, path);
        const char *devname = udev_device_get_devnode(udev_dev);

        udev_dev = udev_device_get_parent_with_subsystem_devtype(
            udev_dev,
            "usb",
            "usb_device"
        );

        const char *idVendor = udev_device_get_sysattr_value(udev_dev, "idVendor");

        // Only add SEGGER and ARM (even though VENDOR_ID is NXPs...) devices to list
        if(idVendor != NULL && ((strcmp(idVendor, SEGGER_VENDOR_ID) == 0) || (strcmp(idVendor, NXP_VENDOR_ID) == 0)))
        {
            serial_device_t *serial_device = (serial_device_t*)malloc(sizeof(serial_device_t));
            memset(serial_device, 0, sizeof(serial_device_t));

            strcpy(serial_device->vendorId, idVendor);
            strcpy(serial_device->port, devname);
            strcpy(serial_device->locationId, path);
            strcpy(serial_device->productId, udev_device_get_sysattr_value(udev_dev, "idProduct"));
            strcpy(serial_device->manufacturer, udev_device_get_sysattr_value(udev_dev,"manufacturer"));
            strcpy(serial_device->serialNumber, udev_device_get_sysattr_value(udev_dev, "serial"));

            devices->push_back(serial_device);
        }

        udev_device_unref(udev_dev);
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

        delete device;
    }

    devices->clear();
    delete devices;
}
