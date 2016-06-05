/**
 *
 *
 *
 * Portions of this code is from the node-serialport project: https://github.com/voodootikigod/node-serialport
 *
 * The license that code is release under is:
 *
 * Copyright 2010, 2011, 2012 Christopher Williams. All rights reserved.
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */


#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include <assert.h>
#include <vector>
#include <iostream>

#include <AvailabilityMacros.h>
#include <sys/param.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/serial/IOSerialKeys.h>

#include <pthread.h>

#include "serial_port_enum.h"

pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;
const char* TTY_PATH_PREFIX = "/dev/tty.";

#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
#include <errno.h>
#endif

#define EL_CAPITAN_ISSUE 1

using namespace std;

typedef struct SerialDevice {
    char port[MAXPATHLEN];
    char locationId[MAXPATHLEN];
    char vendorId[MAXPATHLEN];
    char productId[MAXPATHLEN];
    char manufacturer[MAXPATHLEN];
    char serialNumber[MAXPATHLEN];
} serial_device_t;

const char* SEGGER_VENDOR_ID = "1366";

typedef vector<serial_device_t*> adapter_list_t;

static mach_port_t masterPort;
static io_registry_entry_t root;

// Function prototypes
static void FindModems(io_iterator_t *matchingServices);
static io_registry_entry_t GetUsbDevice(char *pathName);
static adapter_list_t* GetAdapters();

static void FindModems(io_iterator_t *matchingServices)
{
    kern_return_t     kernResult;
    CFMutableDictionaryRef  classesToMatch;

    classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);

    if (classesToMatch != NULL)
    {
        CFDictionarySetValue(classesToMatch,
                             CFSTR(kIOSerialBSDTypeKey),
                             CFSTR(kIOSerialBSDAllTypes));
    }

    kern_return_t status;

    status = IOMasterPort(MACH_PORT_NULL, &masterPort);
    assert(status == kIOReturnSuccess);

    /*
    root = IORegistryGetRootEntry(masterPort);
    assert(root != MACH_PORT_NULL); */

    status = IOServiceGetMatchingServices(masterPort, classesToMatch, matchingServices);
    assert(status == kIOReturnSuccess);
}

static io_registry_entry_t GetUsbDevice(char* pathName)
{
    io_registry_entry_t device = 0;

    CFMutableDictionaryRef classesToMatch = IOServiceMatching(kIOUSBDeviceClassName);

    if (classesToMatch != NULL)
    {
        io_iterator_t matchingServices;

        kern_return_t status = IOServiceGetMatchingServices(masterPort, classesToMatch, &matchingServices);
        assert(status == kIOReturnSuccess);

        io_service_t service;
        Boolean deviceFound = false;

        while ((service = IOIteratorNext(matchingServices)) && !deviceFound)
        {
            CFStringRef bsdPathAsCFString = (CFStringRef) IORegistryEntrySearchCFProperty(service, kIOServicePlane, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, kIORegistryIterateRecursively);

            if (bsdPathAsCFString)
            {
                Boolean result;
                char    bsdPath[MAXPATHLEN];

                // Convert the path from a CFString to a C (NULL-terminated)
                result = CFStringGetCString(bsdPathAsCFString,
                                            bsdPath,
                                            sizeof(bsdPath),
                                            kCFStringEncodingUTF8);

                CFRelease(bsdPathAsCFString);

                if (result && (strcmp(bsdPath, pathName) == 0))
                {
                    deviceFound = true;
                    device = service;
                }
                else
                {
                   // Release the object which are no longer needed
                   (void) IOObjectRelease(service);
                }
            }
        }

        // Release the iterator.
        IOObjectRelease(matchingServices);
    }

    return device;
}

static void ExtractUsbInformation(serial_device_t *serialDevice, IOUSBDeviceInterface  **deviceInterface)
{
    kern_return_t kernResult;
    UInt32 locationID;

    kernResult = (*deviceInterface)->GetLocationID(deviceInterface, &locationID);

    if (KERN_SUCCESS == kernResult)
    {
        snprintf(serialDevice->locationId, MAXPATHLEN, "0x%08x", locationID);
    }

    UInt16 vendorID;
    kernResult = (*deviceInterface)->GetDeviceVendor(deviceInterface, &vendorID);

    if (KERN_SUCCESS == kernResult)
    {
        snprintf(serialDevice->vendorId, MAXPATHLEN, "0x%04x", vendorID);
    }

    UInt16 productID;
    kernResult = (*deviceInterface)->GetDeviceProduct(deviceInterface, &productID);

    if (KERN_SUCCESS == kernResult)
    {
        snprintf(serialDevice->productId, MAXPATHLEN, "0x%04x", productID);
    }
}

static adapter_list_t* GetAdapters()
{
    // Setup return value
    adapter_list_t* devices = new adapter_list_t();

    kern_return_t kernResult;
    io_iterator_t serialPortIterator;
    char bsdPath[MAXPATHLEN];

    FindModems(&serialPortIterator);

    io_service_t modemService;
    kernResult = KERN_FAILURE;
    Boolean modemFound = false;

    // Initialize the returned path
    *bsdPath = '\0';

    int length = 0;

    while ((modemService = IOIteratorNext(serialPortIterator)))
    {
        CFTypeRef bsdPathAsCFString;

        bsdPathAsCFString = IORegistryEntrySearchCFProperty(modemService, kIOServicePlane, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, kIORegistryIterateRecursively);

        if (bsdPathAsCFString)
        {
            Boolean result;

            // Convert the path from a CFString to a C (NUL-terminated)

            result = CFStringGetCString((CFStringRef) bsdPathAsCFString,
                                        bsdPath,
                                        sizeof(bsdPath),
                                        kCFStringEncodingUTF8);
            CFRelease(bsdPathAsCFString);

            assert(result);

            serial_device_t *serial_device = (serial_device_t*)malloc(sizeof(serial_device_t));
            memset(serial_device, 0, sizeof(serial_device_t));

            // Apparently the cu.X prefix is wrong, it should be tty.X. This is a hack to do that.
            strcpy(serial_device->port, TTY_PATH_PREFIX);
            strcpy(serial_device->port + strlen(TTY_PATH_PREFIX), bsdPath + strlen("/dev/cu."));

            // Wait until this is fixed in OS X 10.11: https://forums.developer.apple.com/message/19670#19670
#if EL_CAPITAN_ISSUE
            modemFound = true;
            kernResult = KERN_SUCCESS;

            pthread_mutex_lock(&list_mutex);

            io_registry_entry_t device = GetUsbDevice(bsdPath);

            if (device) {
                CFStringRef manufacturerAsCFString = (CFStringRef) IORegistryEntrySearchCFProperty(device,
                                      kIOServicePlane,
                                      CFSTR(kUSBVendorString),
                                      kCFAllocatorDefault,
                                      kIORegistryIterateRecursively);

                if (manufacturerAsCFString)
                {
                    Boolean result;
                    char    manufacturer[MAXPATHLEN];

                    // Convert from a CFString to a C (NUL-terminated)
                    result = CFStringGetCString(manufacturerAsCFString,
                                                manufacturer,
                                                sizeof(manufacturer),
                                                kCFStringEncodingUTF8);

                    if (result) {
                        strcpy(serial_device->manufacturer, manufacturer);
                    }

                    CFRelease(manufacturerAsCFString);
                }

                CFStringRef serialNumberAsCFString = (CFStringRef) IORegistryEntrySearchCFProperty(device,
                                      kIOServicePlane,
                                      CFSTR(kUSBSerialNumberString),
                                      kCFAllocatorDefault,
                                      kIORegistryIterateRecursively);

                if (serialNumberAsCFString)
                {
                    Boolean result;
                    char    serialNumber[MAXPATHLEN];

                    // Convert from a CFString to a C (NUL-terminated)
                    result = CFStringGetCString(serialNumberAsCFString,
                                                serialNumber,
                                                sizeof(serialNumber),
                                                kCFStringEncodingUTF8);

                    if (result) {
                      strcpy(serial_device->serialNumber, serialNumber);
                    }

                    CFRelease(serialNumberAsCFString);
                }

                IOCFPlugInInterface **plugInInterface = NULL;
                SInt32        score;
                HRESULT       res;

                IOUSBDeviceInterface  **deviceInterface = NULL;

                kernResult = IOCreatePlugInInterfaceForService(
                    device,
                    kIOUSBDeviceUserClientTypeID,
                    kIOCFPlugInInterfaceID,
                    &plugInInterface, &score);

                if ((kIOReturnSuccess != kernResult) || !plugInInterface) {
                    continue;
                }

                // Use the plugin interface to retrieve the device interface.
                res = (*plugInInterface)->QueryInterface(
                    plugInInterface,
                    CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
                    (LPVOID*) &deviceInterface);

                // Now done with the plugin interface.
                (*plugInInterface)->Release(plugInInterface);

                if (res || deviceInterface == NULL) {
                    continue;
                }

                // Extract the desired Information
                ExtractUsbInformation(serial_device, deviceInterface);

                // Release the Interface
                (*deviceInterface)->Release(deviceInterface);

                // Release the device
                (void) IOObjectRelease(device);

            }
#endif

            // Add the device to the result
            devices->push_back(serial_device);

            pthread_mutex_unlock(&list_mutex);
        }
    }

    // Release the io_service_t now that we are done with it.
    (void) IOObjectRelease(modemService);

    IOObjectRelease(serialPortIterator);  // Release the iterator.

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
            resultItem->locationId = device->locationId;
            resultItem->vendorId = device->vendorId;
            resultItem->productId = device->productId;
            resultItem->manufacturer = device->manufacturer;
            resultItem->serialNumber = device->serialNumber;

            descs.push_back(resultItem);
        }

        delete device;
    }

    devices->clear();
    delete devices;
}
