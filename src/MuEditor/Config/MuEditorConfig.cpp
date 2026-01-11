#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorConfig.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

CMuEditorConfig& CMuEditorConfig::GetInstance()
{
    static CMuEditorConfig instance;
    return instance;
}

CMuEditorConfig::CMuEditorConfig()
    : m_language("en")
{
}

CMuEditorConfig::~CMuEditorConfig()
{
    Save();
}

std::string CMuEditorConfig::Trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

void CMuEditorConfig::Load()
{
    std::ifstream file(m_configPath);
    if (!file.is_open())
        return; // File doesn't exist yet, use defaults

    std::string currentSection;
    std::string line;

    while (std::getline(file, line))
    {
        line = Trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;

        // Check for section header
        if (line[0] == '[' && line[line.length() - 1] == ']')
        {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }

        // Parse key=value
        size_t pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string key = Trim(line.substr(0, pos));
            std::string value = Trim(line.substr(pos + 1));

            if (currentSection == "General")
            {
                if (key == "Language")
                {
                    m_language = value;
                }
            }
            else if (currentSection == "ColumnVisibility")
            {
                m_columnVisibility[key] = (value == "1" || value == "true");
            }
        }
    }

    file.close();
}

void CMuEditorConfig::Save()
{
    // Create MuEditor directory if it doesn't exist
    std::filesystem::create_directory("MuEditor");

    std::ofstream file(m_configPath);
    if (!file.is_open())
        return;

    // Write header
    file << "; MU Editor Configuration File\n";
    file << "; Generated automatically - Edit with care\n\n";

    // Write [General] section
    file << "[General]\n";
    file << "Language=" << m_language << "\n\n";

    // Write [ColumnVisibility] section
    file << "[ColumnVisibility]\n";
    for (const auto& col : m_columnVisibility)
    {
        file << col.first << "=" << (col.second ? "1" : "0") << "\n";
    }

    file.close();
}

std::string CMuEditorConfig::GetLanguage() const
{
    return m_language;
}

void CMuEditorConfig::SetLanguage(const std::string& language)
{
    m_language = language;
}

bool CMuEditorConfig::GetColumnVisibility(const std::string& columnName, bool defaultValue) const
{
    auto it = m_columnVisibility.find(columnName);
    if (it != m_columnVisibility.end())
        return it->second;
    return defaultValue;
}

void CMuEditorConfig::SetColumnVisibility(const std::string& columnName, bool visible)
{
    m_columnVisibility[columnName] = visible;
}

#endif // _EDITOR
