#pragma once

#include "Core/Platform/WinCompat.h"
#include "Core/Platform/WinIni.h"
#include "Data/GameConfig/GameConfig.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>

// GameConfig loads and saves config.ini beside the executable; this puts the file back as it was
// and reloads it.
class ExecutableConfigFile
{
public:
    ExecutableConfigFile()
    {
        wchar_t executable[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, executable, MAX_PATH);
        m_path = std::filesystem::path(executable).parent_path() / L"config.ini";
        std::ifstream file(m_path, std::ios::binary);
        m_existed = file.is_open();
        m_content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
    ~ExecutableConfigFile()
    {
        if (m_existed)
        {
            std::ofstream file(m_path, std::ios::binary | std::ios::trunc);
            file << m_content;
        }
        else
        {
            std::error_code ignored;
            std::filesystem::remove(m_path, ignored);
        }
        GameConfig::GetInstance().Load();
    }
    ExecutableConfigFile(const ExecutableConfigFile&) = delete;
    ExecutableConfigFile& operator=(const ExecutableConfigFile&) = delete;

    void Write(const std::string& text) const
    {
        std::ofstream file(m_path, std::ios::trunc);
        file << text;
    }
    // 2 when the key is missing.
    [[nodiscard]] int ReadRender(const std::string& key) const
    {
        const std::wstring wideKey(key.begin(), key.end());
        return static_cast<int>(GetPrivateProfileIntW(L"Render", wideKey.c_str(), 2, m_path.wstring().c_str()));
    }

private:
    std::filesystem::path m_path;
    bool m_existed = false;
    std::string m_content;
};
