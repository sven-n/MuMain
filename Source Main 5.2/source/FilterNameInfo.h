// FilterNameInfo.h: interface for the FilterNameInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

#if !defined(AFX_FILTERNAMEINFO_H__2827381F_AD6C_467A_9086_4A7EA9F8EE42__INCLUDED_)
#define AFX_FILTERNAMEINFO_H__2827381F_AD6C_467A_9086_4A7EA9F8EE42__INCLUDED_

#include "interface.h"

#pragma once

namespace info
{
    SmartPointer(FilterNameInfo);
    class FilterNameInfo : public InfoFile
    {
    private:
        bool OpenFile(const std::wstring& filename);
        FilterNameInfo(const std::wstring& filename);

    public:
        static FilterNameInfoPtr MakeInfo(const std::wstring& filename);
        virtual ~FilterNameInfo();

    protected:
        virtual bool isopenfile();
        virtual void clear();

    public:
        const wchar_t* GetData(int index) const;

    private:
        bool		m_IsOpenFile;
        wchar_t		m_Info[2048][20];
    };
};

inline
bool info::FilterNameInfo::isopenfile()
{
    return m_IsOpenFile;
}

inline
void info::FilterNameInfo::clear()
{
}

inline
const wchar_t* info::FilterNameInfo::GetData(int index) const
{
    if (index >= 2048)
        assert(0);
    return m_Info[index];
}

#endif // !defined(AFX_FILTERNAMEINFO_H__2827381F_AD6C_467A_9086_4A7EA9F8EE42__INCLUDED_)

#endif //INFO_BUILDER