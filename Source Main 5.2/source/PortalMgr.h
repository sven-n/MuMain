// PortalMgr.h: interface for the CPortalMgr class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PORTALMGR_H__55104483_515C_4618_A100_F3BD8533591D__INCLUDED_)
#define AFX_PORTALMGR_H__55104483_515C_4618_A100_F3BD8533591D__INCLUDED_

#pragma once

#ifdef YDG_ADD_CS5_PORTAL_CHARM

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

	void GetPortalPositionText(char * pszOut);
	void GetRevivePositionText(char * pszOut);

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

#endif	// YDG_ADD_CS5_PORTAL_CHARM

#endif // !defined(AFX_PORTALMGR_H__55104483_515C_4618_A100_F3BD8533591D__INCLUDED_)
