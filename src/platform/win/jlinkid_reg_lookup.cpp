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

string findJlinkId(string parentIdPrefix)
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

			if (parentIdPrefix.find(value) == string::npos)
			{
				continue;
			}

			return jlinkIdCandidate;
		}
	}

	return "";

}

string findParentIdPrefix(string portName)
{
	// Example keypath HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\VID_1366&PID_1015&MI_00\7&2f8ac9e0&0&0000\Device Parameters, PortName

	map<string, vector<string>> seggerSubEntries = findSeggerSubEntries();

	for (auto& kv : seggerSubEntries)
	{
		for (auto& parentIdPrefixCandidate : kv.second)
		{
			string subKeyPath = kv.first + "\\" + parentIdPrefixCandidate + "\\Device Parameters";
			string value = getRegKeyValue(subKeyPath, "PortName");

			if (value == portName)
			{
				return parentIdPrefixCandidate;
			}		
		}
	}

	return "";
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
	string parentIdPrefix = findParentIdPrefix(port);

	if (parentIdPrefix == "")
	{
		return "";
	}

	string jlinkId = findJlinkId(parentIdPrefix);

	if (jlinkId == "")
	{
		return "";
	}

	string cleanedJlinkId = removeLeadingZeros(jlinkId);

	return cleanedJlinkId;
}
