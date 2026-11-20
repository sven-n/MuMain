#pragma once

#include <filesystem>
#include <string>

// The config.ini GameConfig loads and saves (in the user folder); this puts the file back as it was
// and reloads it.
class UserConfigFile
{
public:
    UserConfigFile();
    ~UserConfigFile();
    UserConfigFile(const UserConfigFile&) = delete;
    UserConfigFile& operator=(const UserConfigFile&) = delete;

    void Write(const std::string& text) const;
    // 2 when the key is missing.
    [[nodiscard]] int ReadRender(const std::string& key) const;

private:
    std::filesystem::path m_path;
    bool m_existed = false;
    std::string m_content;
};
