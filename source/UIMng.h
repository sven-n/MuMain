//*****************************************************************************
// File: UIMng.h
//
// Desc: interface for the CUIMng class.
//		 새로 적용된 UI 매니저 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_UIMNG_H__9C8B9D12_9584_47C7_8F6E_6E9F402ACB65__INCLUDED_)
#define AFX_UIMNG_H__9C8B9D12_9584_47C7_8F6E_6E9F402ACB65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PList.h"

#include "MsgWin.h"
#include "SysMenuWin.h"
#include "OptionWin.h"
#include "LoginMainWin.h"
#include "ServerSelWin.h"
#include "LoginWin.h"
#include "CreditWin.h"
#include "CharSelMainWin.h"
#include "CharMakeWin.h"
#include "CharInfoBalloonMng.h"
#include "ServerMsgWin.h"
#ifdef LJH_ADD_ONETIME_PASSWORD
#include "OneTimePassword.h"
#endif //LJH_ADD_ONETIME_PASSWORD


// m_nScene에 들어갈 내용.
#define	UIM_SCENE_NONE			0		// 씬 없음.
#define	UIM_SCENE_TITLE			1		// 타이틀 씬.
#define UIM_SCENE_LOGIN			2		// 로그인 씬.
#define	UIM_SCENE_LOADING		3		// 로딩 씬.
#define	UIM_SCENE_CHARACTER		4		// 캐릭터 선택 씬.
#define	UIM_SCENE_MAIN			5		// 주 게임 씬.

class CSprite;
class CGaugeBar;
class CWin;
class CLoadingScene;

class CUIMng  
{
public:
// 공통
	CMsgWin			m_MsgWin;			// 메시지 창.
	CSysMenuWin		m_SysMenuWin;		// 시스템 메뉴 창.
	COptionWin		m_OptionWin;		// 옵션 창.

// 로그인 씬
	CLoginMainWin	m_LoginMainWin;		// 로그인 씬 메인 메뉴.
	CServerSelWin	m_ServerSelWin;		// 서버 선택 창.
	CLoginWin		m_LoginWin;			// 로그인 창.
	CCreditWin		m_CreditWin;		// 크레딧 창.
#ifdef LJH_ADD_LOCALIZED_BTNS
	CMsgWin			m_LocalizedYESNOMsgWin;				// (localized)예, 아니오 메시지 창
	CMsgWin			m_LocalizedConfirmCancelMsgWin;		// (localized)확인, 취소 메시지 창
#endif //LJH_ADD_LOCALIZED_BTNS
#ifdef LJH_ADD_ONETIME_PASSWORD
	COneTimePassword	m_OneTimePasswordWin;	// 원타임 패스워드 창.
#endif //LJH_ADD_ONETIME_PASSWORD


// 캐릭터 선택 씬.
	CCharSelMainWin	m_CharSelMainWin;	// 캐릭터 선택 씬 메인 메뉴.
	CCharMakeWin	m_CharMakeWin;		// 캐릭터 생성 창.
	CCharInfoBalloonMng	m_CharInfoBalloonMng;// 캐릭터 정보 말풍선 매니저.
	CServerMsgWin	m_ServerMsgWin;		// 서버 메시지 윈도우.

// 로딩 씬
	CLoadingScene*	m_pLoadingScene;	// 로딩 씬.

protected:
// 타이틀 씬
	CSprite*		m_asprTitle;		// 스프라이트들.
	CGaugeBar*		m_pgbLoding;		// 로딩 게이지바.

	// m_WinList는 이 Class에서 생성된 모든 윈도우가 등록. 따라서 보이던 안보이
	//던 모두 m_WinList에 연결되어 있음. list 나열 방법은 보이는 순과 활성화 순
	//으로 나열. 단 활성화 윈도우는 단 하나이거나 없음.
	CPList			m_WinList;			// 윈도우 리스트.
	bool			m_bCursorOnUI;		// UI 위에 커서가 있는가?
	bool			m_bBlockCharMove;	// 캐릭터 이동을 막는가?(퀵슬롯에서 아이콘을 버릴때 이용)
	int				m_nScene;			// 씬.
	bool			m_bWinActive;		// true이면 m_WinList의 head 윈도우가 Active(포커스).
	bool			m_bSysMenuWinShow;	// 시스템 메뉴 창이 보여져야 되는가?
										//(ESC키 토글이므로 추가)

// 로그인 씬
#ifdef MOVIE_DIRECTSHOW
	bool			m_bMoving;			// 로그인 씬에서 무비 플레이 중인지 검사
#endif // MOVIE_DIRECTSHOW

public:
	virtual ~CUIMng();

	static CUIMng& Instance();

// 타이틀 씬 부분.
	void CreateTitleSceneUI();
	void ReleaseTitleSceneUI();
	void RenderTitleSceneUI(HDC hDC, DWORD dwNow, DWORD dwTotal);
// ~타이틀 씬 부분.

	void Create();
	void Release();
	void CreateLoginScene();
	void CreateCharacterScene();
	void CreateMainScene();
	void Update(double dDeltaTick);
	void Render();

	void ShowWin(CWin* pWin);
	void HideWin(CWin* pWin);

	bool IsCursorOnUI() { return m_bCursorOnUI; }	// UI 위에 커서가 있는가?
	void PopUpMsgWin(int nMsgCode, char* pszMsg = NULL);
	void AddServerMsg(char* pszMsg);
	void CloseMsgWin() { HideWin(&m_MsgWin); }
	void SetSysMenuWinShow(bool bShow) { m_bSysMenuWinShow = bShow; }
	bool IsSysMenuWinShow() { return m_bSysMenuWinShow; };	// 시스템 메뉴 창이 보여져야 되는가?

#ifdef MOVIE_DIRECTSHOW
	void SetMoving(bool bMoving) { m_bMoving = bMoving; };
	bool IsMoving() { return m_bMoving; };
#endif // MOVIE_DIRECTSHOW

protected:
	CUIMng();

	void RemoveWinList();
	CWin* SetActiveWin(CWin* pWin);
	void CheckDockWin();
	bool SetDockWinPosition(CWin* pMoveWin, int nDockX, int nDockY);
};

#endif // !defined(AFX_UIMNG_H__9C8B9D12_9584_47C7_8F6E_6E9F402ACB65__INCLUDED_)
