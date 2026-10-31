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

    // The address of the OpenMU admin panel which the Item Sync window updates from
    // ([OpenMU] AdminPanelUrl).
    const std::string& GetOpenMUAdminPanelUrl() const;
    void SetOpenMUAdminPanelUrl(const std::string& url);

    // Skill Editor column visibility settings
    const std::map<std::string, bool>& GetSkillEditorColumnVisibility() const { return m_skillEditorColumnVisibility; }
    void SetSkillEditorColumnVisibility(const std::map<std::string, bool>& visibility) { m_skillEditorColumnVisibility = visibility; }

private:
    CMuEditorConfig();

    // Config file path
    const char* m_configPath = "MuEditor/MuEditor.ini";

    // Settings storage
    std::string m_language;
    std::map<std::string, bool> m_skillEditorColumnVisibility;  // Skill Editor columns
    std::string m_openMUAdminPanelUrl;

    // Helper functions for INI parsing
    std::string Trim(const std::string& str);
};

#define g_MuEditorConfig CMuEditorConfig::GetInstance()

#endif // _EDITOR
