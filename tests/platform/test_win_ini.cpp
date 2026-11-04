#include <doctest.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "Core/Platform/WinIni.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace
{
// An .ini file in the temp folder, removed again at the end.
class TempIniFile
{
public:
    explicit TempIniFile(const char* name) : m_path(std::filesystem::temp_directory_path() / name) {}
    ~TempIniFile()
    {
        std::error_code ignored;
        std::filesystem::remove(m_path, ignored);
    }
    TempIniFile(const TempIniFile&) = delete;
    TempIniFile& operator=(const TempIniFile&) = delete;

    void Write(const std::string& text) const
    {
        std::ofstream file(m_path, std::ios::binary | std::ios::trunc);
        file << text;
    }
    [[nodiscard]] std::string Read() const
    {
        std::ifstream file(m_path, std::ios::binary);
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }
    [[nodiscard]] std::wstring Path() const
    {
        return m_path.wstring();
    }

private:
    std::filesystem::path m_path;
};
} // namespace

TEST_CASE("removing a key or a section that is not there leaves the .ini file as it is [platform][ini]")
{
    const TempIniFile file("mu_win_ini_missing.ini");
    // The comment stays only while the file is not written again.
    const std::string text = "; a player's note\r\n[Render]\r\nVSync=1\r\n";
    file.Write(text);
    const std::wstring path = file.Path();

    CHECK(WritePrivateProfileStringW(L"Render", L"Missing", nullptr, path.c_str()));
    CHECK(WritePrivateProfileStringW(L"Gone", L"Missing", nullptr, path.c_str()));
    CHECK(WritePrivateProfileStringW(L"Gone", nullptr, nullptr, path.c_str()));
    CHECK(file.Read() == text);
}

TEST_CASE("removing a key that is there keeps the other keys [platform][ini]")
{
    const TempIniFile file("mu_win_ini_present.ini");
    file.Write("[Render]\r\nVSync=1\r\nObsolete=5\r\n");
    const std::wstring path = file.Path();

    CHECK(WritePrivateProfileStringW(L"Render", L"Obsolete", nullptr, path.c_str()));
    CHECK(GetPrivateProfileIntW(L"Render", L"Obsolete", 7, path.c_str()) == 7);
    CHECK(GetPrivateProfileIntW(L"Render", L"VSync", 0, path.c_str()) == 1);
}
