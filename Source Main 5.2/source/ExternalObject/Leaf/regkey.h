#pragma once
#include <string>

namespace leaf {
    class CRegKey {
    public:
        enum _HKEY {
            _HKEY_CLASSES_ROOT = (LONG)(ULONG_PTR)HKEY_CLASSES_ROOT,
            _HKEY_CURRENT_CONFIG = (LONG)(ULONG_PTR)HKEY_CURRENT_CONFIG,
            _HKEY_CURRENT_USER = (LONG)(ULONG_PTR)HKEY_CURRENT_USER,
            _HKEY_LOCAL_MACHINE = (LONG)(ULONG_PTR)HKEY_LOCAL_MACHINE,
            _HKEY_USERS = (LONG)(ULONG_PTR)HKEY_USERS
        };

        CRegKey() = default;
        ~CRegKey() = default;

        void SetKey(_HKEY hKey, const std::wstring& subkey) {
            m_hKey = reinterpret_cast<HKEY>(hKey);
            m_subkey = subkey;
        }

        bool ReadDword(const std::wstring& name, DWORD& value) {
            return OpenKey([&](HKEY hKey) {
                DWORD type = REG_DWORD;
                DWORD size = sizeof(DWORD);
                return RegQueryValueEx(hKey, name.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(&value), &size) == ERROR_SUCCESS;
                });
        }

        bool ReadString(const std::wstring& name, std::wstring& value) {
            return OpenKey([&](HKEY hKey) {
                wchar_t buffer[256];
                DWORD type = REG_EXPAND_SZ;
                DWORD size = sizeof(buffer);
                if (RegQueryValueEx(hKey, name.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(buffer), &size) != ERROR_SUCCESS)
                    return false;
                value = buffer;
                return true;
                });
        }

        bool WriteDword(const std::wstring& name, DWORD value) {
            return OpenKey([&](HKEY hKey) {
                return RegSetValueEx(hKey, name.c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)) == ERROR_SUCCESS;
                });
        }

        bool WriteString(const std::wstring& name, const std::wstring& value) {
            return OpenKey([&](HKEY hKey) {
                DWORD size = static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)); 
                return RegSetValueEx(hKey, name.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()), size) == ERROR_SUCCESS;
                });
        }

        bool DeleteKey() {
            return RegDeleteKey(m_hKey, m_subkey.c_str()) == ERROR_SUCCESS;
        }

        bool DeleteValue(const std::wstring& name) {
            return OpenKey([&](HKEY hKey) {
                return RegDeleteValue(hKey, name.c_str()) == ERROR_SUCCESS;
                });
        }

    private:
        HKEY m_hKey = nullptr;
        std::wstring m_subkey;

        template<typename Func>
        bool OpenKey(Func&& func) {
            HKEY hKey = nullptr;
            DWORD disp;
            if (RegCreateKeyEx(m_hKey, m_subkey.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hKey, &disp) != ERROR_SUCCESS)
                return false;

            bool result = func(hKey);
            RegCloseKey(hKey);
            return result;
        }
    };
}