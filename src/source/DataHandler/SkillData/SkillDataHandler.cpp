#include "stdafx.h"
#include "SkillDataHandler.h"

#include "SkillDataLoader.h"
#include "SkillDataSaver.h"
#include "SkillDataSaverLegacy.h"
#include "GameData/SkillData/SkillStructs.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"

#ifdef _EDITOR
#include "SkillDataExporter.h"
#endif

// External references
extern SKILL_ATTRIBUTE* SkillAttribute;

CSkillDataHandler::CSkillDataHandler()
{
}

CSkillDataHandler& CSkillDataHandler::GetInstance()
{
    static CSkillDataHandler instance;
    return instance;
}

SKILL_ATTRIBUTE* CSkillDataHandler::GetSkillAttributes()
{
    return SkillAttribute;
}

SKILL_ATTRIBUTE* CSkillDataHandler::GetSkillAttribute(int index)
{
    if (index >= 0 && index < MAX_SKILLS)
        return &SkillAttribute[index];
    return nullptr;
}

int CSkillDataHandler::GetSkillCount() const
{
    return MAX_SKILLS;
}

bool CSkillDataHandler::Load(wchar_t* fileName)
{
    return SkillDataLoader::Load(fileName);
}

#ifdef _EDITOR
bool CSkillDataHandler::Save(wchar_t* fileName, std::string* outChangeLog)
{
    return SkillDataSaver::Save(fileName, outChangeLog);
}

bool CSkillDataHandler::ExportAsS6E3(wchar_t* fileName)
{
    return SkillDataSaverLegacy::SaveLegacy(fileName);
}

bool CSkillDataHandler::ExportToCsv(wchar_t* fileName)
{
    return SkillDataExporter::ExportToCsv(fileName);
}
#endif
