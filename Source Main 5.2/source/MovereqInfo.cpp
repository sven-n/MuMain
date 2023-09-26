// MovereqInfo.cpp: implementation of the MovereqInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MovereqInfo.h"

#ifdef INFO_BUILDER

using namespace info;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MovereqInfoPtr MovereqInfo::MakeInfo(const std::wstring& filename)
{
    MovereqInfoPtr info(new MovereqInfo(filename));
    return info;
}

MovereqInfo::MovereqInfo(const std::wstring& filename) : InfoFile(filename), m_IsOpenFile(false)
{
    m_IsOpenFile = OpenFile(filename);

    if (!m_IsOpenFile)
    {
        assert(0);
    }
}

MovereqInfo::~MovereqInfo()
{
    clear();
}

bool MovereqInfo::OpenFile(const std::wstring& filename)
{
    return true;
}

#endif //INFO_BUILDER