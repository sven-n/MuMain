#pragma once

#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>
#include "LauncherHelper.h"

#pragma comment(lib, "Winmm.lib")

class CRegKey{
public:
// 	enum _HKEY{ _HKEY_CLASSES_ROOT = HKEY_CLASSES_ROOT, 
// 		_HKEY_CURRENT_CONFIG = HKEY_CURRENT_CONFIG, 
// 		_HKEY_CURRENT_USER = HKEY_CURRENT_USER,
// 		_HKEY_LOCAL_MACHINE = HKEY_LOCAL_MACHINE,
// 		_HKEY_USERS = HKEY_USERS
// 	};
	enum _HKEY{ _HKEY_CLASSES_ROOT		= (DWORD)HKEY_CLASSES_ROOT, 
				_HKEY_CURRENT_CONFIG	= (DWORD)HKEY_CURRENT_CONFIG, 
				_HKEY_CURRENT_USER		= (DWORD)HKEY_CURRENT_USER,
				_HKEY_LOCAL_MACHINE		= (DWORD)HKEY_LOCAL_MACHINE,
				_HKEY_USERS				= (DWORD)HKEY_USERS			};

	CRegKey(){}
	~CRegKey(){}
	
	void SetKey(_HKEY hKey, const std::string& subkey) {
		m_hKey = (HKEY)hKey;
		m_subkey = subkey;
	}
	bool ReadDword(const std::string& name, DWORD& value) {
		HKEY	hKey = NULL;
		DWORD	dwDisp;
		DWORD	dwType = REG_DWORD;
		DWORD	dwValue = 0;
		DWORD	dwSize = sizeof(DWORD);
		
		if (ERROR_SUCCESS != RegCreateKeyEx(m_hKey, m_subkey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp))
		{
			return false;
		}
		
		if (ERROR_SUCCESS != RegQueryValueEx(hKey, name.c_str(), NULL, &dwType, (LPBYTE)&value, &dwSize)){
			RegCloseKey(hKey); 
			return false;
		}
		
		RegCloseKey(hKey);
		return true;
	}
	bool ReadString(const std::string& name, std::string& value) {
		char	szTempKey[256];
		HKEY	hKey = NULL;
		DWORD	dwDisp;
		DWORD	dwType = REG_EXPAND_SZ;
		DWORD	dwValue = 0;
		DWORD	dwSize = 256;
		
		if (ERROR_SUCCESS != RegCreateKeyEx(m_hKey, m_subkey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp))
			return false;
		
		if (ERROR_SUCCESS != RegQueryValueEx(hKey, name.c_str(), NULL, &dwType, (LPBYTE )szTempKey, &dwSize)){
			RegCloseKey(hKey);
			return false;
		}
		if(dwSize > 0) {
			value = szTempKey;
		}
		RegCloseKey(hKey);
		return true;
	}
	bool WriteDword(const std::string& name, DWORD value) {
		HKEY	hKey = NULL;
		DWORD	dwDisp;
		DWORD	dwType = REG_DWORD;
		DWORD	dwSize = sizeof(DWORD);
		
		if (ERROR_SUCCESS != RegCreateKeyEx(m_hKey, m_subkey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp))
			return false;
		
		RegSetValueEx(hKey, name.c_str(), 0L, REG_DWORD, (BYTE*)&value, dwSize);
		RegCloseKey(hKey);
		return true;
	}
	bool WriteString(const std::string& name, const std::string& value) {
		HKEY	hKey = NULL;
		DWORD	dwDisp;
		DWORD	dwType = REG_DWORD;
		DWORD	dwSize = value.size();
		
		if (ERROR_SUCCESS != RegCreateKeyEx(m_hKey, m_subkey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp))
			return false;
		
		RegSetValueEx(hKey, name.c_str(), 0L, REG_SZ, (CONST BYTE*)value.c_str(), dwSize);
		RegCloseKey(hKey);
		return true;
	}
	bool DeleteKey() {
		if (ERROR_SUCCESS != RegDeleteKey(m_hKey, m_subkey.c_str()))
			return false;
		return true;
	}
	bool DeleteValue(const std::string& name) {
		HKEY	hKey = NULL;
		if (ERROR_SUCCESS != RegOpenKeyEx(m_hKey, m_subkey.c_str(), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &hKey))
			return false;
		
		RegDeleteValue(hKey, name.c_str());
		RegCloseKey(hKey);
		return true;
	}
	
private:
	HKEY	m_hKey;
	std::string	m_subkey;
};

bool wzRegisterConnectionKey()
{
	CRegKey RegKey;
#ifdef PBG_ADD_LAUNCHER_BLUE
	RegKey.SetKey(CRegKey::_HKEY_LOCAL_MACHINE, "SOFTWARE\\Webzen\\Mu_Blue\\Connection");
#else //PBG_ADD_LAUNCHER_BLUE
	RegKey.SetKey(CRegKey::_HKEY_LOCAL_MACHINE, "SOFTWARE\\Webzen\\Mu\\Connection");
#endif //PBG_ADD_LAUNCHER_BLUE

	if(!RegKey.WriteDword("Key", timeGetTime()))
	{
		
		return false;
	}

	return true;
}
void wzUnregisterConnectionKey()
{
	CRegKey RegKey;
#ifdef PBG_ADD_LAUNCHER_BLUE
	RegKey.SetKey(CRegKey::_HKEY_LOCAL_MACHINE, "SOFTWARE\\Webzen\\Mu_Blue\\Connection");
#else //PBG_ADD_LAUNCHER_BLUE
	RegKey.SetKey(CRegKey::_HKEY_LOCAL_MACHINE, "SOFTWARE\\Webzen\\Mu\\Connection");
#endif //PBG_ADD_LAUNCHER_BLUE

	RegKey.DeleteKey();
}

unsigned long wzGetConnectionKey()
{
	CRegKey RegKey;
#ifdef PBG_ADD_LAUNCHER_BLUE
	RegKey.SetKey(CRegKey::_HKEY_LOCAL_MACHINE, "SOFTWARE\\Webzen\\Mu_Blue\\Connection");
#else //PBG_ADD_LAUNCHER_BLUE
	RegKey.SetKey(CRegKey::_HKEY_LOCAL_MACHINE, "SOFTWARE\\Webzen\\Mu\\Connection");
#endif //PBG_ADD_LAUNCHER_BLUE

	DWORD dwKey;
	if(!RegKey.ReadDword("Key", dwKey)) {
		return 0xFFFFFFFF;
	}
	return dwKey;
}

bool wzPushLaunchInfo(const WZLAUNCHINFO& LaunchInfo) {

	if(timeGetTime() - wzGetConnectionKey() > 5000) {
		wzUnregisterConnectionKey();
		return false;
	}
	wzUnregisterConnectionKey();
	
	//. encryption
	const BYTE bySuffle[] = { 0x0C, 0x07, 0x03, 0x13 };
	char szEncodedIP[256];
	int iEncodedPort;

	for(unsigned int i=0; i<LaunchInfo.ip.size(); i++)
		szEncodedIP[i] = LaunchInfo.ip[i] + bySuffle[i%4];
	szEncodedIP[LaunchInfo.ip.size()] = '\0';
	iEncodedPort = LaunchInfo.port ^ bySuffle[LaunchInfo.ip.size()%4];

	CRegKey RegKey;
#ifdef PBG_ADD_LAUNCHER_BLUE
	RegKey.SetKey(CRegKey::_HKEY_LOCAL_MACHINE, "SOFTWARE\\Webzen\\Mu_Blue\\Connection");
#else //PBG_ADD_LAUNCHER_BLUE
	RegKey.SetKey(CRegKey::_HKEY_LOCAL_MACHINE, "SOFTWARE\\Webzen\\Mu\\Connection");
#endif //PBG_ADD_LAUNCHER_BLUE

	if(!RegKey.WriteString("ParameterA", szEncodedIP))
	{
		return false;
	}
	if(!RegKey.WriteDword("ParameterB", iEncodedPort))
	{
		return false;
	}


	wzRegisterConnectionKey();
	
	return true;
}
bool wzPopLaunchInfo(WZLAUNCHINFO& LaunchInfo) {

	if(timeGetTime() - wzGetConnectionKey() > 26000) {
		wzUnregisterConnectionKey();
		return false;
	}
	
	//. get endcoded data
	std::string strEncodedIP;
	DWORD dwEncodedPort;
	CRegKey RegKey;
#ifdef PBG_ADD_LAUNCHER_BLUE
	RegKey.SetKey(CRegKey::_HKEY_LOCAL_MACHINE, "SOFTWARE\\Webzen\\Mu_Blue\\Connection");
#else //PBG_ADD_LAUNCHER_BLUE
	RegKey.SetKey(CRegKey::_HKEY_LOCAL_MACHINE, "SOFTWARE\\Webzen\\Mu\\Connection");
#endif //PBG_ADD_LAUNCHER_BLUE
	if(!RegKey.ReadString("ParameterA", strEncodedIP))
	{
		return false;
	}
	if(!RegKey.ReadDword("ParameterB", dwEncodedPort))
	{
		return false;
	}

	//. decryption
	const BYTE bySuffle[] = { 0x0C, 0x07, 0x03, 0x13 };
	char szDecodedIP[256];
	DWORD dwDecodedPort;

	for(unsigned int i=0; i<strEncodedIP.size(); i++)
		szDecodedIP[i] = strEncodedIP[i] - bySuffle[i%4];
	szDecodedIP[strEncodedIP.size()] = '\0';
	dwDecodedPort = dwEncodedPort ^ bySuffle[strEncodedIP.size()%4];

	LaunchInfo.ip = szDecodedIP;
	LaunchInfo.port = dwDecodedPort;

	wzUnregisterConnectionKey();

	return true;
}
