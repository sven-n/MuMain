// ExplanationInfo.h: interface for the ExplanationInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

#if !defined(AFX_EXPLANATIONINFO_H__67A9648C_8432_403E_A150_B811447EAAE4__INCLUDED_)
#define AFX_EXPLANATIONINFO_H__67A9648C_8432_403E_A150_B811447EAAE4__INCLUDED_

#include "interface.h"
#include "ZzzInfomation.h"

#pragma once

namespace info
{
    SmartPointer(ExplanationInfo);
    class ExplanationInfo : public InfoFile
    {
    public:
        static ExplanationInfoPtr MakeInfo(const std::wstring& filename);
        virtual ~ExplanationInfo();

    protected:
        virtual bool isopenfile();
        virtual void clear();

    public:
        const wchar_t* GetData(int index);

    private:
        bool OpenFile(const std::wstring& filename);
        ExplanationInfo(const std::wstring& filename);

    private:
        bool m_IsOpenFile;
        wchar_t m_Info[MAX_TEXTS][MAX_GLOBAL_TEXT_STRING];
    };
};
inline
bool info::ExplanationInfo::isopenfile()
{
    return m_IsOpenFile;
}

inline
void info::ExplanationInfo::clear()
{
}

inline
const wchar_t* info::ExplanationInfo::GetData(int index)
{
    if (index >= MAX_TEXTS)
        assert(0);
    return m_Info[index];
}

#endif // !defined(AFX_EXPLANATIONINFO_H__67A9648C_8432_403E_A150_B811447EAAE4__INCLUDED_)

#endif //INFO_BUILDER