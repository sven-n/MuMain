#include "stdafx.h"
#include "ExecutableConfigFile.h"

#include "Core/Platform/WinCompat.h"
#include "Core/Platform/WinIni.h"
#include "Data/GameConfig/GameConfig.h"

#include <fstream>
#include <iterator>
#include <system_error>

ExecutableConfigFile::ExecutableConfigFile()
{
    wchar_t executable[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, executable, MAX_PATH);
    m_path = std::filesystem::path(executable).parent_path() / L"config.ini";
    std::ifstream file(m_path, std::ios::binary);
    m_existed = file.is_open();
    m_content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

ExecutableConfigFile::~ExecutableConfigFile()
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

void ExecutableConfigFile::Write(const std::string& text) const
{
    std::ofstream file(m_path, std::ios::trunc);
    file << text;
}

int ExecutableConfigFile::ReadRender(const std::string& key) const
{
    const std::wstring wideKey(key.begin(), key.end());
    return static_cast<int>(GetPrivateProfileIntW(L"Render", wideKey.c_str(), 2, m_path.wstring().c_str()));
}
