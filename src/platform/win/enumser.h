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
/*
Module : enumser.h
Purpose: Defines the interface for a class to enumerate the serial ports installed on a PC
         using a number of different approaches
Created: PJN / 03-11-1998

Copyright (c) 1998 - 2013 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


///////////////////////// Macros / Structs etc ////////////////////////////////

#pragma once

#ifndef __ENUMSER_H__
#define __ENUMSER_H__

#ifndef CENUMERATESERIAL_EXT_CLASS
#define CENUMERATESERIAL_EXT_CLASS
#endif


///////////////////////// Includes ////////////////////////////////////////////                      

#if defined CENUMERATESERIAL_USE_STL
#ifndef _VECTOR_
  #include <vector>
  #pragma message("To avoid this message, please put vector in your pre compiled header (normally stdafx.h)")
#endif  
#ifndef _STRING_
  #include <string>
  #pragma message("To avoid this message, please put string in your pre compiled header (normally stdafx.h)")
#endif  
#else
#if defined _AFX
  #ifndef __AFXTEMPL_H__
    #include <afxtempl.h> 
    #pragma message("To avoid this message, please put afxtempl.h in your pre compiled header (normally stdafx.h)")
  #endif
#else
  #ifndef __ATLSTR_H__
    #include <atlstr.h>
    #pragma message("To avoid this message, please put atlstr.h in your pre compiled header (normally stdafx.h)")
  #endif  
#endif
#endif


///////////////////////// Classes /////////////////////////////////////////////

class CENUMERATESERIAL_EXT_CLASS CEnumerateSerial
{
public:
//Methods
#ifndef NO_ENUMSERIAL_USING_CREATEFILE
  #if defined CENUMERATESERIAL_USE_STL
  	static BOOL UsingCreateFile(std::vector<UINT>& ports);
  #elif defined _AFX
  	static BOOL UsingCreateFile(CUIntArray& ports);
  #else
      static BOOL UsingCreateFile(CSimpleArray<UINT>& ports);
  #endif
#endif

#ifndef NO_ENUMSERIAL_USING_QUERYDOSDEVICE
  #if defined CENUMERATESERIAL_USE_STL
    static BOOL UsingQueryDosDevice(std::vector<UINT>& ports);
  #elif defined _AFX
    static BOOL UsingQueryDosDevice(CUIntArray& ports);
  #else
    static BOOL UsingQueryDosDevice(CSimpleArray<UINT>& ports);
  #endif
#endif

#ifndef NO_ENUMSERIAL_USING_GETDEFAULTCOMMCONFIG
  #if defined CENUMERATESERIAL_USE_STL
	static BOOL UsingGetDefaultCommConfig(std::vector<UINT>& ports);
  #elif defined _AFX
    static BOOL UsingGetDefaultCommConfig(CUIntArray& ports);
  #else
    static BOOL UsingGetDefaultCommConfig(CSimpleArray<UINT>& ports);
  #endif
#endif

#ifndef NO_ENUMSERIAL_USING_SETUPAPI1
  #if defined CENUMERATESERIAL_USE_STL
    #if defined _UNICODE
      static BOOL UsingSetupAPI1(std::vector<UINT>& ports, std::vector<std::wstring>& friendlyNames);
    #else
      static BOOL UsingSetupAPI1(std::vector<UINT>& ports, std::vector<std::string>& friendlyNames);
    #endif
  #elif defined _AFX
    static BOOL UsingSetupAPI1(CUIntArray& ports, CStringArray& friendlyNames);
  #else
    static BOOL UsingSetupAPI1(CSimpleArray<UINT>& ports, CSimpleArray<CString>& friendlyNames);
  #endif
#endif

#ifndef NO_ENUMSERIAL_USING_SETUPAPI2
  #if defined CENUMERATESERIAL_USE_STL
    #if defined _UNICODE
      static BOOL UsingSetupAPI2(std::vector<UINT>& ports, std::vector<std::wstring>& friendlyNames);
    #else
      static BOOL UsingSetupAPI2(std::vector<UINT>& ports, std::vector<std::string>& friendlyNames);
    #endif
  #elif defined _AFX
    static BOOL UsingSetupAPI2(CUIntArray& ports, CStringArray& friendlyNames);
  #else
    static BOOL UsingSetupAPI2(CSimpleArray<UINT>& ports, CSimpleArray<CString>& friendlyNames);
  #endif
#endif

#ifndef NO_ENUMSERIAL_USING_ENUMPORTS
  #if defined CENUMERATESERIAL_USE_STL
    static BOOL UsingEnumPorts(std::vector<UINT>& ports);
  #elif defined _AFX
    static BOOL UsingEnumPorts(CUIntArray& ports);
  #else
    static BOOL UsingEnumPorts(CSimpleArray<UINT>& ports);
  #endif
#endif

#ifndef NO_ENUMSERIAL_USING_WMI
  #if defined CENUMERATESERIAL_USE_STL
    #if defined _UNICODE
      static BOOL UsingWMI(std::vector<UINT>& ports, std::vector<std::wstring>& friendlyNames);
    #else
      static BOOL UsingWMI(std::vector<UINT>& ports, std::vector<std::string>& friendlyNames);
    #endif
  #elif defined _AFX
    static BOOL UsingWMI(CUIntArray& ports, CStringArray& friendlyNames);
  #else
    static BOOL UsingWMI(CSimpleArray<UINT>& ports, CSimpleArray<CString>& friendlyNames);
  #endif
#endif

#ifndef NO_ENUMSERIAL_USING_COMDB
  #if defined CENUMERATESERIAL_USE_STL
    static BOOL UsingComDB(std::vector<UINT>& ports);
  #elif defined _AFX
    static BOOL UsingComDB(CUIntArray& ports);
  #else
    static BOOL UsingComDB(CSimpleArray<UINT>& ports);
  #endif
#endif

#ifndef NO_ENUMSERIAL_USING_REGISTRY
  #if defined CENUMERATESERIAL_USE_STL
    #if defined _UNICODE
      static BOOL UsingRegistry(std::vector<std::wstring>& ports);
    #else
      static BOOL UsingRegistry(std::vector<std::string>& ports);
    #endif
  #elif defined _AFX
    static BOOL UsingRegistry(CStringArray& ports);
  #else
    static BOOL UsingRegistry(CSimpleArray<CString>& ports);
  #endif
#endif

protected:
//Methods
#if !defined(NO_ENUMSERIAL_USING_SETUPAPI1) || !defined(NO_ENUMSERIAL_USING_SETUPAPI2)
  static BOOL RegQueryValueString(HKEY kKey, LPCTSTR lpValueName, LPTSTR& pszValue);
  static BOOL QueryRegistryPortName(HKEY hDeviceKey, int& nPort);
#endif
#if !defined(NO_ENUMSERIAL_USING_SETUPAPI1) || !defined(NO_ENUMSERIAL_USING_SETUPAPI2) || !defined(NO_ENUMSERIAL_USING_COMDB)
  static HMODULE LoadLibraryFromSystem32(LPCTSTR lpFileName);
#endif
  static BOOL IsNumeric(LPCSTR pszString, BOOL bIgnoreColon);
  static BOOL IsNumeric(LPCWSTR pszString, BOOL bIgnoreColon);
};

#endif //__ENUMSER_H__
