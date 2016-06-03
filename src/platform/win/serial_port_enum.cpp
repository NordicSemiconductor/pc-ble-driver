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

#include "serial_port_enum.h"

#include <list>
#include <string>

#include "disphelper.h"
#include "stdafx.h"
#include "enumser.h"
#include "jlinkid_reg_lookup.h"

#define MAX_BUFFER_SIZE 1000

/*
 * listComPorts.c -- list COM ports
 *
 * http://github.com/todbot/usbSearch/
 *
 * 2012, Tod E. Kurt, http://todbot.com/blog/
 *
 *
 * Uses DispHealper : http://disphelper.sourceforge.net/
 *
 * Notable VIDs & PIDs combos:
 * VID 0403 - FTDI
 *
 * VID 0403 / PID 6001 - Arduino Diecimila
 *
 */
uint32_t EnumSerialPorts(std::list<SerialPortDesc*>& descs);
{
    DISPATCH_OBJ(wmiSvc);
    DISPATCH_OBJ(colDevices);

    dhInitialize(TRUE);
    dhToggleExceptions(FALSE);

    dhGetObject(L"winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2", nullptr, &wmiSvc);
    dhGetValue(L"%o", &colDevices, wmiSvc, L".ExecQuery(%S)", L"Select * from Win32_PnPEntity");

    FOR_EACH(objDevice, colDevices, NULL) {
        char* name = nullptr;
        char* pnpid = nullptr;
        char* manu = nullptr;
        char* match;

        dhGetValue(L"%s", &name,  objDevice, L".Name");
        dhGetValue(L"%s", &pnpid, objDevice, L".PnPDeviceID");

        if( name != nullptr && ((match = strstr( name, "(COM" )) != nullptr) ) { // look for "(COM23)"
            // 'Manufacturuer' can be null, so only get it if we need it
            dhGetValue(L"%s", &manu, objDevice,  L".Manufacturer");

            if((strcmp("SEGGER", manu) == 0)
                || (_stricmp("arm", manu) == 0)
                || (_stricmp("mbed", manu) == 0))
            {
                auto comname = strtok( match, "()");
                auto resultItem = new SerialPortDesc();
                resultItem->comName = comname;
                resultItem->manufacturer = manu;
                resultItem->pnpId = pnpid;
                data->results.push_back(resultItem);

                string jlinkId = portNameToJlinkId(string(comname));
                if (jlinkId != "") {
                    resultItem->serialNumber = jlinkId.c_str();
                }
            }

            dhFreeString(manu);
          }

        dhFreeString(name);
        dhFreeString(pnpid);
    } NEXT(objDevice);

    SAFE_RELEASE(colDevices);
    SAFE_RELEASE(wmiSvc);

    dhUninitialize(TRUE);
}
