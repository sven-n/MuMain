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
};
