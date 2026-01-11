#pragma once

#ifdef _EDITOR

#include <string>
#include <map>

// Centralized configuration manager for MU Editor
// Handles all editor settings in a unified MuEditor.ini file
class CMuEditorConfig
{
public:
    static CMuEditorConfig& GetInstance();

    // Load/Save all configuration
    void Load();
    void Save();

    // General settings
    std::string GetLanguage() const;
    void SetLanguage(const std::string& language);

    // Item Editor column visibility settings
    bool GetColumnVisibility(const std::string& columnName, bool defaultValue = false) const;
    void SetColumnVisibility(const std::string& columnName, bool visible);

    // Get all item editor column visibility settings
    const std::map<std::string, bool>& GetAllColumnVisibility() const { return m_columnVisibility; }
    void SetAllColumnVisibility(const std::map<std::string, bool>& visibility) { m_columnVisibility = visibility; }

    // Skill Editor column visibility settings
    const std::map<std::string, bool>& GetSkillEditorColumnVisibility() const { return m_skillEditorColumnVisibility; }
    void SetSkillEditorColumnVisibility(const std::map<std::string, bool>& visibility) { m_skillEditorColumnVisibility = visibility; }

private:
    CMuEditorConfig();

    // Config file path
    const char* m_configPath = "MuEditor/MuEditor.ini";

    // Settings storage
    std::string m_language;
    std::map<std::string, bool> m_columnVisibility;  // Item Editor columns
    std::map<std::string, bool> m_skillEditorColumnVisibility;  // Skill Editor columns

    // Helper functions for INI parsing
    std::string Trim(const std::string& str);
};

#define g_MuEditorConfig CMuEditorConfig::GetInstance()

#endif // _EDITOR
