// SkillInfo.cpp: implementation of the SkillInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkillInfo.h"

#ifdef INFO_BUILDER

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace info;

SkillInfoPtr SkillInfo::MakeInfo(const std::wstring& filename)
{
    SkillInfoPtr info(new SkillInfo(filename));
    return info;
}

SkillInfo::SkillInfo(const std::wstring& filename) : InfoFile(filename), m_IsOpenFile(false)
{
    m_IsOpenFile = OpenFile(filename);

    if (!m_IsOpenFile)
    {
        assert(0);
    }
}

SkillInfo::~SkillInfo()
{
    clear();
}

bool SkillInfo::OpenFile(const std::wstring& filename)
{
    return true;
}

#endif //INFO_BUILDER