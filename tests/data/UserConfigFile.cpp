#include "stdafx.h"
#include "UserConfigFile.h"

#include "Core/Platform/WinIni.h"
#include "Data/GameConfig/GameConfig.h"

#include <fstream>
#include <iterator>
#include <system_error>

UserConfigFile::UserConfigFile() : m_path(GameConfig::GetInstance().GetConfigPath())
{
    std::ifstream file(m_path, std::ios::binary);
    m_existed = file.is_open();
    m_content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

UserConfigFile::~UserConfigFile()
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

void UserConfigFile::Write(const std::string& text) const
{
    std::ofstream file(m_path, std::ios::trunc);
    file << text;
}

int UserConfigFile::ReadRender(const std::string& key) const
{
    const std::wstring wideKey(key.begin(), key.end());
    return static_cast<int>(GetPrivateProfileIntW(L"Render", wideKey.c_str(), 2, m_path.wstring().c_str()));
}
