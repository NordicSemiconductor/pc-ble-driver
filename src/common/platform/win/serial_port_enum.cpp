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
/**
 *
 *
 *
 * Portions of this code is from the node-serialport project:
 https://github.com/voodootikigod/node-serialport
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

#include <list>
#include <string>

//#include "disphelper.h"
#include "stdafx.h"

//#include "enumser.h"
//#include "jlinkid_reg_lookup.h"

#include "serial_port_enum.h"

#include <Setupapi.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <objbase.h>
#include <regex>

#pragma comment(lib, "Setupapi.lib")


#define SEGGER_VENDOR_ID "1366"
#define NXP_VENDOR_ID "0D28"
#define NORDIC_SEMICONDUCTOR_VENDOR_ID "1915"
#define NRF52_CONNECTIVITY_DONGLE_PID "C00A"


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
 * **********************************
 * * * * * Notable VIDs & PIDs combos:
 * VID 0403 / PID 6001 - FTDI FT232
 *
 * VID 2341 / PID 0043 - Arduino Diecimila
 *
 * * * * * Enable VIDs & PIDs combos:
 * VID 1366 / PID 0105 - SEGGER JLink V9
 * 
 * VID 0D28 - NXP  
 * 
 * VIP 1915 / PID C00A - Nordic nrf52 dongle
 */


std::list<SerialPortDesc> EnumSerialPorts()
{
    std::list<SerialPortDesc> descs;
    
    GUID Guids[16]      = {0};
    GUID PortsGUIDs[8] = {0};
    GUID ModemsGUIDs[8] = {0};
    DWORD ports_guids_size, modems_guids_size;
    BOOL ret =
        SetupDiClassGuidsFromName("Ports", PortsGUIDs, sizeof(PortsGUIDs), &ports_guids_size);
    if (ret == FALSE) 
        return descs;
    ret = SetupDiClassGuidsFromName("Modem", ModemsGUIDs, sizeof(ModemsGUIDs), &modems_guids_size);
    if (ret == FALSE)
        return descs;

    for (DWORD i = 0; i < ports_guids_size; i++)
        Guids[i] = PortsGUIDs[i];
    for (DWORD i = 0; i < modems_guids_size; i++)
        Guids[i + ports_guids_size] = ModemsGUIDs[i];
    
    for (DWORD i = 0; i < ports_guids_size + modems_guids_size; i++)
    {
        SP_DEVINFO_DATA devinfo = {0};
        devinfo.cbSize          = sizeof(devinfo);
    
        HDEVINFO p_hdi = SetupDiGetClassDevs(&Guids[i], NULL, NULL, DIGCF_PRESENT);
        for (int j = 0; TRUE == SetupDiEnumDeviceInfo(p_hdi, j, &devinfo); j++)
        {
            DWORD dwPropertyRegDataType, dwSize;
            char szData[512] = {0};
            
            //read friendlyname
            BOOL ret         = SetupDiGetDeviceRegistryProperty(p_hdi, &devinfo, SPDRP_FRIENDLYNAME,
                                                        &dwPropertyRegDataType, (BYTE *)szData,
                                                        sizeof(szData), &dwSize);
            if (TRUE == ret && strstr(szData, "(COM") != nullptr)
            {
                SerialPortDesc resultItem = {""};
                std::string s             = szData;
                
                std::regex pattern(".*\\((COM[0-9]+)\\).*");
                std::smatch match_result;
                if (regex_match(s, match_result, pattern))
                {
                    resultItem.comName = match_result.str(1);
                }
                else
                {
                   continue;
                }

                // read manufacturer
                memset(szData, 0, sizeof(szData));
                if (TRUE == SetupDiGetDeviceRegistryProperty(p_hdi, &devinfo, SPDRP_MFG,
                                                             &dwPropertyRegDataType, (BYTE *)szData,
                                                             sizeof(szData), &dwSize))
                {
                    resultItem.manufacturer = szData;
                }

                // read HardwareID
                memset(szData, 0, sizeof(szData));
                if (FALSE ==
                    SetupDiGetDeviceInstanceId(p_hdi, &devinfo, szData, sizeof(szData), &dwSize))
                {
                    memset(szData, 0, sizeof(szData));
                    if (FALSE == SetupDiGetDeviceRegistryProperty(
                                     p_hdi, &devinfo, SPDRP_HARDWAREID, &dwPropertyRegDataType,
                                     (BYTE *)szData, sizeof(szData), &dwSize))
                    {
                        memset(szData, 0, sizeof(szData));
                    }
                }
                resultItem.pnpId = szData;

                // get vid, pid, serial number
                if (strnicmp(szData, "USB", 3) == 0)
                {
                    //"USB\\VID_1915&PID_C00A&MI_01\\6&142BFA11&0&0001"
                    std::string s = szData;
                    std::regex pattern(".+VID_([0-9a-fA-F]{4})(&PID_([0-9a-fA-F]{4}))?(&MI_(\\d{"
                                       "2}))?(\\\\(.*))?");
                    std::smatch match_result;
                    if (regex_match(s, match_result, pattern))
                    {
                        // get vid
                        resultItem.vendorId = match_result.str(1);
                        // get pid
                        resultItem.productId = match_result.str(3);
                        
                        // get serial nubmer
                        if (match_result[7].matched &&
                            regex_match(match_result.str(7), std::regex("^\\w+$")))
                        {
                            resultItem.serialNumber = match_result.str(7);
                        }
                        else
                        { // read parent serial nubmer
                            memset(szData, 0, sizeof(szData));
                            DWORD devinst;
                            CONFIGRET status =
                                CM_Get_Parent(&devinst, devinfo.DevInst, CM_LOCATE_DEVNODE_NORMAL);
                            if (status == CR_SUCCESS)
                            {
                                status = CM_Get_Device_ID(devinst, szData, sizeof(szData),
                                                          CM_LOCATE_DEVNODE_NORMAL);
                                if (status == CR_SUCCESS)
                                {
                                    std::string s = szData;
                                    resultItem.serialNumber =
                                        s.substr(s.find_last_of("\\") + 1,
                                                 s.length() - s.find_last_of("\\") - 1);
                                }
                            }
                        }
                    }
                    //remove segger serial number zeros
                    if (resultItem.vendorId == SEGGER_VENDOR_ID &&
                        resultItem.serialNumber.length() > 0)
                    {
                        std::regex pattern("[0]*([1-9][0-9]+)");
                        std::smatch match_result;
                        if (regex_match(resultItem.serialNumber, match_result, pattern))
                        {
                            resultItem.serialNumber = match_result.str(1);
                        }
                    }
                }
                else if (strnicmp(szData, "FTDIBUS", 7) == 0)
                {
                    //"FTDIBUS\\VID_0403+PID_6001+AB0MH7ATA\\0000"
                    std::string s = szData;
                    std::regex pattern(".+VID_([0-9a-fA-F]{4})\\+PID_([0-9a-fA-F]{4})(\\+(\\w+))?(\\\\(.*))?");
                    std::smatch match_result;
                    if (regex_match(s, match_result, pattern))
                    {
                        // get vid
                        resultItem.vendorId = match_result.str(1);
                        // get pid
                        resultItem.productId = match_result.str(2);
                        // get serial nubmer
                        resultItem.serialNumber = match_result.str(4);   
                    }
                }
                //read locationid
                /*memset(szData, 0, sizeof(szData));
                if (TRUE == SetupDiGetDeviceRegistryProperty(p_hdi, &devinfo, SPDRP_LOCATION_PATHS,
                                                             &dwPropertyRegDataType, (BYTE *)szData,
                                                             sizeof(szData), &dwSize))
                {
                    
                }*/

                if ((resultItem.vendorId == SEGGER_VENDOR_ID ||
                      resultItem.vendorId == NXP_VENDOR_ID ||
                     resultItem.vendorId == NORDIC_SEMICONDUCTOR_VENDOR_ID) &&
                     (resultItem.manufacturer == "SEGGER" ||
                      strnicmp(resultItem.manufacturer.c_str(), "arm", 3) == 0 ||
                      strnicmp(resultItem.manufacturer.c_str(), "mbed", 4) == 0 ||
                     resultItem.productId == NRF52_CONNECTIVITY_DONGLE_PID)
                    )
                {
                    descs.push_back(resultItem);
                }
            }
        }
        SetupDiDestroyDeviceInfoList(p_hdi);
    }

    return descs;
}