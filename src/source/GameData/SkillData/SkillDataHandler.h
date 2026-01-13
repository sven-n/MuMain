#pragma once

#include <string>

// Skill Data Handler - Singleton Facade
// Provides centralized access to skill data operations
class CSkillDataHandler
{
public:
    static CSkillDataHandler& GetInstance();

    // Data Operations - delegates to specialized classes
    bool Load(wchar_t* fileName);

#ifdef _EDITOR
    bool Save(wchar_t* fileName, std::string* outChangeLog = nullptr);
    bool ExportAsS6E3(wchar_t* fileName);
    bool ExportToCsv(wchar_t* fileName);
#endif

    // Data Access
    SKILL_ATTRIBUTE* GetSkillAttributes();
    SKILL_ATTRIBUTE* GetSkillAttribute(int index);
    int GetSkillCount() const;

private:
    CSkillDataHandler();
    ~CSkillDataHandler() = default;

    // Prevent copying
    CSkillDataHandler(const CSkillDataHandler&) = delete;
    CSkillDataHandler& operator=(const CSkillDataHandler&) = delete;
};

#define g_SkillDataHandler CSkillDataHandler::GetInstance()
