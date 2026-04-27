// UIDefaultBase.h: interface for the CUIDefaultBase class.
//
//////////////////////////////////////////////////////////////////////

#ifdef UIDEFAULTBASE

#if !defined(AFX_UIDEFAULTBASE_H__9F7419C7_E7DA_4EED_B32B_A40D3F6932F1__INCLUDED_)
#define AFX_UIDEFAULTBASE_H__9F7419C7_E7DA_4EED_B32B_A40D3F6932F1__INCLUDED_

#pragma once

#define UIInitializeFunction( classname ) \
	public: \
		classname( const string& uiname ); \
		virtual ~classname();

#define UIDefaultVariable \
	private: \
		bool		m_IsOpen; \
		string		m_UIName;

#define UIDefaultFunction \
	public: \
		void Open(); \
		void Close(); \
		bool IsOpen(); \
	UIDefaultVariable;

namespace SEASON3A
{
    class CUIDefaultBase
    {
        UIInitializeFunction(CUIDefaultBase);
        UIDefaultFunction;
    };

    inline
        void CUIDefaultBase::Open()
    {
        m_IsOpen = true;
    }

    inline
        void CUIDefaultBase::Close()
    {
        m_IsOpen = false;
    }

    inline
        bool CUIDefaultBase::IsOpen()
    {
        return m_IsOpen;
    }
};

#endif // !defined(AFX_UIDEFAULTBASE_H__9F7419C7_E7DA_4EED_B32B_A40D3F6932F1__INCLUDED_)

#endif //UIDEFAULTBASE