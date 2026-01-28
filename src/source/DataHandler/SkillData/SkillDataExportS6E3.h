#pragma once

#ifdef _EDITOR

#include <stdio.h>

// Skill Data Legacy Format Saving Operations (32-byte name format)
class SkillDataExportS6E3
{
public:
    static bool SaveLegacy(wchar_t* fileName);
};

#endif // _EDITOR
