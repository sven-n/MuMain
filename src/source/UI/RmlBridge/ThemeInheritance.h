#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace UI::RmlBridge
{
// Resolves whole files and individual settings from child to parent. Paths remain virtual
// in RmlUi so a child's overrides also apply to inherited documents and templates.
class ThemeInheritance
{
public:
    struct Setting
    {
        std::string value;
        std::filesystem::path directory;
    };

    explicit ThemeInheritance(std::filesystem::path directory);
    static bool IsValidName(const std::string& name);
    bool IsValid() const { return m_Error.empty(); }
    const std::string& Error() const { return m_Error; }
    const std::vector<std::filesystem::path>& Directories() const { return m_Directories; }
    std::optional<std::filesystem::path> ResolveFile(const std::filesystem::path& relative,
                                                  bool texture = false) const;
    std::optional<Setting> GetSetting(const std::string& section, const std::string& key) const;
    std::map<std::string, Setting> Section(const std::string& section) const;
    static std::optional<std::string> ReadSetting(const std::filesystem::path& directory,
                                                const std::string& section, const std::string& key);

private:
    using Values = std::map<std::string, std::map<std::string, std::string>>;
    static Values ReadValues(const std::filesystem::path& directory);
    void Build(std::filesystem::path directory);
    std::vector<std::filesystem::path> m_Directories;
    std::vector<Values> m_Values;
    std::string m_Error;
};

std::optional<std::filesystem::path> ThemeDirectoryForResource(const std::string& path);
std::string ThemePathUtf8(const std::filesystem::path& path);
std::string ResolveThemeResource(const std::string& path, bool texture = false);
} // namespace UI::RmlBridge
