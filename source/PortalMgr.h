// PortalMgr.h: interface for the CPortalMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PORTALMGR_H__55104483_515C_4618_A100_F3BD8533591D__INCLUDED_)
#define AFX_PORTALMGR_H__55104483_515C_4618_A100_F3BD8533591D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef YDG_ADD_CS5_PORTAL_CHARM

// 이동의 부적, 부활의 부적 좌표 정보 출력용
// 클라이언트 종료시 부적의 정보가 사라지므로 좌표는 클라이언트 독자 처리하며, 따로 저장해둘 필요가 없다.
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

	BOOL IsPortalUsable();	// 이동 부적 사용 가능한가

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
