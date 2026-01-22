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

    // Column visibility settings
    bool GetColumnVisibility(const std::string& columnName, bool defaultValue = false) const;
    void SetColumnVisibility(const std::string& columnName, bool visible);

    // Get all column visibility settings
    const std::map<std::string, bool>& GetAllColumnVisibility() const { return m_columnVisibility; }
    void SetAllColumnVisibility(const std::map<std::string, bool>& visibility) { m_columnVisibility = visibility; }

private:
    CMuEditorConfig();
    ~CMuEditorConfig();

    // Config file path
    const char* m_configPath = "MuEditor/MuEditor.ini";

    // Settings storage
    std::string m_language;
    std::map<std::string, bool> m_columnVisibility;

    // Helper functions for INI parsing
    std::string Trim(const std::string& str);
};

#define g_MuEditorConfig CMuEditorConfig::GetInstance()

#endif // _EDITOR
