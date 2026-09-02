#pragma once

#include <stdio.h>

// Skill Data Loading Operations
// Loads skill data from encrypted BMD files with checksum verification
// This is the single source of truth for skill data loading (used in ALL builds)
class SkillDataLoader
{
public:
    // Load skill data from file
    // Returns true on success, false on failure
    static bool Load(wchar_t* fileName);

private:
    // Generic template method for loading any skill format
    template <typename TFileFormat> static bool LoadFormat(FILE* fp, const wchar_t* formatName);

    // Load legacy format (32-byte names)
    static bool LoadLegacyFormat(FILE* fp, long fileSize);

    // Load new format (MAX_SKILL_NAME-byte names)
    static bool LoadNewFormat(FILE* fp, long fileSize);
};
