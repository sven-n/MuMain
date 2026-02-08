#pragma once

#ifdef _EDITOR

// Skill Data Export Operations
// Exports skill data to CSV format for external editing
class SkillDataExportAsCSV
{
public:
    // Export all skills to CSV file
    // Returns true on success, false on failure
    static bool ExportToCsv(wchar_t* fileName);
};

#endif // _EDITOR
