#ifndef _REGKEY_H_
#define _REGKEY_H_

//. soyaviper

#include <windows.h>

#pragma warning(disable : 4786)
#include <string>

namespace leaf{
	class CRegKey{
	public:
		enum _HKEY{ _HKEY_CLASSES_ROOT = (LONG)(ULONG_PTR)HKEY_CLASSES_ROOT, 
					_HKEY_CURRENT_CONFIG = (LONG)(ULONG_PTR)HKEY_CURRENT_CONFIG, 
					_HKEY_CURRENT_USER = (LONG)(ULONG_PTR)HKEY_CURRENT_USER,
					_HKEY_LOCAL_MACHINE = (LONG)(ULONG_PTR)HKEY_LOCAL_MACHINE,
					_HKEY_USERS = (LONG)(ULONG_PTR)HKEY_USERS
		};
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
			DWORD	dwSize = sizeof(DWORD);
			
			if (ERROR_SUCCESS != RegCreateKeyEx(m_hKey, m_subkey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp))
				return false;
			
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
			DWORD	dwSize = 256;
			
			if (ERROR_SUCCESS != RegCreateKeyEx(m_hKey, m_subkey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp))
				return false;
			
			if (ERROR_SUCCESS != RegQueryValueEx(hKey, name.c_str(), NULL, &dwType, (LPBYTE )szTempKey, &dwSize)){
				RegCloseKey(hKey);
				return false;
			}
			value = szTempKey;
			RegCloseKey(hKey);
			return true;
		}
		bool WriteDword(const std::string& name, DWORD value) {
			HKEY	hKey = NULL;
			DWORD	dwDisp;
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
}

#endif /* _REGKEY_H_ */