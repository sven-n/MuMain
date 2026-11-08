#pragma once

#include <filesystem>
#include <string>

// GameConfig loads and saves config.ini beside the executable; this puts the file back as it was
// and reloads it.
class ExecutableConfigFile
{
public:
    ExecutableConfigFile();
    ~ExecutableConfigFile();
    ExecutableConfigFile(const ExecutableConfigFile&) = delete;
    ExecutableConfigFile& operator=(const ExecutableConfigFile&) = delete;

    void Write(const std::string& text) const;
    // 2 when the key is missing.
    [[nodiscard]] int ReadRender(const std::string& key) const;

private:
    std::filesystem::path m_path;
    bool m_existed = false;
    std::string m_content;
};
