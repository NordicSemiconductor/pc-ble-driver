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
#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>

#include "jlinkid_reg_lookup.h"

using namespace std;

#define KEY_LENGTH 255

vector<string> getRegKeyEntries(string keyPath)
{
    vector<string> entries;
    HKEY keyHandle;
    long result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyPath.c_str(), NULL, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_READ, &keyHandle);

    if (result != ERROR_SUCCESS)
    {
        return entries;
    }

    for (int index = 0; result == ERROR_SUCCESS; index++)
    {
        TCHAR keyName[255];
        DWORD dwSize = KEY_LENGTH;
        DWORD dwIdx = 0;

        long result = RegEnumKeyEx(keyHandle, index, keyName, &dwSize, NULL, NULL, NULL, NULL);

        if (result != ERROR_SUCCESS) break;
    
        entries.push_back(string(keyName));
    }

    return entries;
}

string getRegKeyValue(string keyPath, string valueName)
{
    HKEY keyHandle;
    long result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyPath.c_str(), NULL, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_READ, &keyHandle);

    if (result != ERROR_SUCCESS)
    {
        return "";
    }

    TCHAR data[KEY_LENGTH];
    DWORD valueSize = KEY_LENGTH;
    LPCTSTR subKeyTemp = NULL;

    result = RegQueryValueEx(keyHandle, valueName.c_str(), NULL, NULL, (LPBYTE)data, &valueSize);

    if (result != ERROR_SUCCESS)
    {
        return "";
    }

    return string(data);
}

map<string, vector<string>> findSeggerSubEntries()
{
    string baseKeyPath = "SYSTEM\\CurrentControlSet\\Enum\\USB";
    vector<string> usbEntries = getRegKeyEntries(baseKeyPath);

    vector<string> seggerKeyPaths;
    for (string val : usbEntries)
    {
        if (val.find("VID_1366") == string::npos) continue;

        seggerKeyPaths.push_back(baseKeyPath + "\\" + val);
    }

    map<string, vector<string>> seggerSubEntries;
    for (string keyPath : seggerKeyPaths)
    {
        vector<string> entries = getRegKeyEntries(keyPath);
        seggerSubEntries[keyPath] = entries;
    }
    
    return seggerSubEntries;
}

bool isValidJlinkId(string jlinkId)
{
    regex jlinkIdRegex("^[0-9]+$");
    smatch match;
    regex_match(jlinkId, match, jlinkIdRegex);

    if (match.size() < 1)
    {
        return false;
    }

    return true;
}

bool matchParentIdPrefix(string value, vector<string> parentIdPrefixCandidates)
{
    for (string parentIdPrefixCandidate : parentIdPrefixCandidates)
    {
        if (parentIdPrefixCandidate.find(value) == string::npos)
        {
            continue;
        }

        return true;
    }

    return false;
}

string findJlinkId(vector<string> parentIdPrefixCandidates)
{
    // Example keypath HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\VID_1366&PID_1015\000682944700, ParentIdPrefix

    map<string, vector<string>> seggerSubEntries = findSeggerSubEntries();

    for (auto& kv : seggerSubEntries)
    {
        for (auto jlinkIdCandidate : kv.second)
        {
            string subKeyPath = kv.first + "\\" + jlinkIdCandidate;
            string value = getRegKeyValue(subKeyPath, "ParentIdPrefix");

            if (value == "")
            {
                continue;
            }

            if (!isValidJlinkId(jlinkIdCandidate))
            {
                continue;
            }

            if (!matchParentIdPrefix(value, parentIdPrefixCandidates))
            {
                continue;
            }

            return jlinkIdCandidate;
        }
    }

    return "";
}

vector<string> findParentIdPrefixCandidates(string portName)
{
    // Example keypath HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\VID_1366&PID_1015&MI_00\7&2f8ac9e0&0&0000\Device Parameters, PortName

    map<string, vector<string>> seggerSubEntries = findSeggerSubEntries();

    vector<string> parentIdPrefixCandidates;

    for (auto& kv : seggerSubEntries)
    {
        for (auto& parentIdPrefixCandidate : kv.second)
        {
            string subKeyPath = kv.first + "\\" + parentIdPrefixCandidate + "\\Device Parameters";
            string value = getRegKeyValue(subKeyPath, "PortName");

            if (value == portName)
            {
                parentIdPrefixCandidates.push_back(parentIdPrefixCandidate);
            }       
        }
    }

    return parentIdPrefixCandidates;
}

string removeLeadingZeros(string jlinkId)
{
    regex jlinkIdRegex("[0]*([1-9][0-9]+)");
    smatch match;
    regex_match(jlinkId, match, jlinkIdRegex);

    // The first match is the whole string, the next is the grouped match
    if (match.size() < 2)
    {
        return jlinkId;
    }

    return match[1];
}

string portNameToJlinkId(string port) {
    vector<string> parentIdPrefixCandidates = findParentIdPrefixCandidates(port);

    if (parentIdPrefixCandidates.size() == 0)
    {
        return "";
    }

    string jlinkId = findJlinkId(parentIdPrefixCandidates);

    if (jlinkId == "")
    {
        return "";
    }

    string cleanedJlinkId = removeLeadingZeros(jlinkId);

    return cleanedJlinkId;
}
