//////////////////////////////////////////////////////////////////////////
//
//  UIGateKeeper.h
//
//  내  용 : 문지기 인터페이스
//
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIGATEKEEPER_H__1EA5EC0A_1A31_4614_98D6_BC982443141B__INCLUDED_)
#define AFX_UIGATEKEEPER_H__1EA5EC0A_1A31_4614_98D6_BC982443141B__INCLUDED_

#pragma once

enum
{
    TOUCH_TYPE_NONE = 0,
    TOUCH_TYPE_PERSON,
    TOUCH_TYPE_GUILD_STAFF,
    TOUCH_TYPE_GUILD_MASTER
};

class CUIGateKeeper
{
public:
    CUIGateKeeper();
    virtual ~CUIGateKeeper();

protected:
    bool		m_bPublic;
    BYTE        m_byType;
    int			m_nEntranceFee;
    int         m_iViewEntranceFee;
    int         m_iAddEntranceFee;
    int         m_iMaxEnteranceFee;

public:
    void SetPublic(bool bPublic) { m_bPublic = bPublic; }
    void SetEntranceFee(int nEntranceFee) { m_nEntranceFee = nEntranceFee; }
    void SetInfo(BYTE byType, bool bPublic, int iEntranceFee, int iAddEntranceFee, int iMaxEntranceFee)
    {
        m_byType = byType;
        m_bPublic = bPublic;
        m_nEntranceFee = iEntranceFee;
        m_iViewEntranceFee = m_nEntranceFee;
        m_iAddEntranceFee = iAddEntranceFee;
        m_iMaxEnteranceFee = iMaxEntranceFee;
    }
    BYTE GetType() { return m_byType; }
    BOOL IsPublic() { return m_bPublic; }
    int GetEnteranceFee() { return m_nEntranceFee; }
    int GetViewEnteranceFee() { return m_iViewEntranceFee; }
    int GetAddEnteranceFee() { return m_iAddEntranceFee; }
    int GetMaxEnteranceFee() { return m_iMaxEnteranceFee; }

    void SendPublicSetting();
    void SendEnteranceFee();
    void SendEnter();
    void EnteranceFeeUp();
    void EnteranceFeeDown();
};

#endif // !defined(AFX_UIGATEKEEPER_H__1EA5EC0A_1A31_4614_98D6_BC982443141B__INCLUDED_)
