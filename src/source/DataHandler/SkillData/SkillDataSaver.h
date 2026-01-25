#pragma once

#ifdef _EDITOR

#include <string>

// Skill Data Saving Operations
// Saves skill data to encrypted BMD files with checksum generation
class SkillDataSaver
{
public:
    // Save skill data to file
    // Returns true on success, false on failure
    // outChangeLog: Optional parameter to receive a detailed change log
    static bool Save(wchar_t* fileName, std::string* outChangeLog = nullptr);
};

#endif // _EDITOR
