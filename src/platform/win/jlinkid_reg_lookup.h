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

#ifndef JLINK_ID_REG_LOOKUP_H
#define JLINK_ID_REG_LOOKUP_H

#include <string>

using namespace std;

/* Perform a Windows registry lookup to find Jlink id from serial port name.
 *
 * param[in] port: JLink CDC serial port name string, e.g. "COM10"
 *
 * retval: Jlink serial number as string, e.g. "682944700"
 */
string portNameToJlinkId(string port);

#endif
