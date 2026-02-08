//////////////////////////////////////////////////////////////////////////
//  UIGuardsMan.h
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIGUARDSMAN_H__076768BF_F97F_4B80_B84C_ADFFD37F4E39__INCLUDED_)
#define AFX_UIGUARDSMAN_H__076768BF_F97F_4B80_B84C_ADFFD37F4E39__INCLUDED_

#pragma once

class CUIGuardsMan
{
public:
    CUIGuardsMan();
    virtual ~CUIGuardsMan();

protected:
    enum
    {
        BC_REQ_LEVEL = 200,
        BC_REQ_MEMBERCOUNT = 20
    };
    enum REG_STATUS
    {
        REG_STATUS_NONE = 0,
        REG_STATUS_REG = 1
    };
    REG_STATUS	m_eRegStatus;
    DWORD		m_dwRegMark;

public:
    bool IsSufficentDeclareLevel();
    bool HasRegistered() { return (m_eRegStatus == REG_STATUS_REG); }
    void SetRegStatus(BYTE byStatus) { m_eRegStatus = (REG_STATUS)byStatus; }

    DWORD GetRegMarkCount() { return m_dwRegMark; }
    void SetMarkCount(DWORD dwMarkCount) { m_dwRegMark = dwMarkCount; }

    DWORD GetMyMarkCount();
    int GetMyMarkSlotIndex();
};

extern CUIGuardsMan g_GuardsMan;

#endif // !defined(AFX_UIGUARDSMAN_H__076768BF_F97F_4B80_B84C_ADFFD37F4E39__INCLUDED_)
