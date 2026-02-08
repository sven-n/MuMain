// PortalMgr.h: interface for the CPortalMgr class.
//////////////////////////////////////////////////////////////////////
#pragma once

class CPortalMgr
{
public:
    CPortalMgr();
    virtual ~CPortalMgr();

    void Reset();
    void ResetPortalPosition();
    void ResetRevivePosition();

    void SavePortalPosition();
    void SaveRevivePosition();

    BOOL IsPortalPositionSaved();
    BOOL IsRevivePositionSaved();

    void GetPortalPositionText(wchar_t* pszOut);
    void GetRevivePositionText(wchar_t* pszOut);

    BOOL IsPortalUsable();

protected:
    int m_iPortalWorld;
    int m_iPortalPosition_x;
    int m_iPortalPosition_y;

    int m_iReviveWorld;
    int m_iRevivePosition_x;
    int m_iRevivePosition_y;
};

extern CPortalMgr g_PortalMgr;
