// FilterInfo.cpp: implementation of the FilterInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FilterInfo.h"

#ifdef INFO_BUILDER

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace info;

FilterInfoPtr FilterInfo::MakeInfo(const std::wstring& filename)
{
    FilterInfoPtr info(new FilterInfo(filename));
    return info;
}

FilterInfo::FilterInfo(const std::wstring& filename) : InfoFile(filename), m_IsOpenFile(false)
{
    m_IsOpenFile = OpenFile(filename);

    if (!m_IsOpenFile)
    {
        assert(0);
    }
}

FilterInfo::~FilterInfo()
{
    clear();
}

bool FilterInfo::OpenFile(const std::wstring& filename)
{
    return true;
}

#endif //INFO_BUILDER