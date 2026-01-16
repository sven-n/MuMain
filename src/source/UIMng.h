//*****************************************************************************
// File: UIMng.h
//*****************************************************************************
#pragma once

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

#define	UIM_SCENE_NONE			0
#define	UIM_SCENE_TITLE			1
#define UIM_SCENE_LOGIN			2
#define	UIM_SCENE_LOADING		3
#define	UIM_SCENE_CHARACTER		4
#define	UIM_SCENE_MAIN			5

class CSprite;
class CGaugeBar;
class CWin;
class CLoadingScene;

class CUIMng
{
public:
    CMsgWin			m_MsgWin;
    CSysMenuWin		m_SysMenuWin;
    COptionWin		m_OptionWin;
    CLoginMainWin	m_LoginMainWin;
    CServerSelWin	m_ServerSelWin;
    CLoginWin		m_LoginWin;
    CCreditWin		m_CreditWin;
    CCharSelMainWin	m_CharSelMainWin;
    CCharMakeWin	m_CharMakeWin;
    CCharInfoBalloonMng	m_CharInfoBalloonMng;
    CServerMsgWin	m_ServerMsgWin;
    CLoadingScene* m_pLoadingScene;

protected:
    CSprite* m_asprTitle;
    CGaugeBar* m_pgbLoding;
    CPList			m_WinList;
    bool			m_bCursorOnUI;
    bool			m_bBlockCharMove;
    int				m_nScene;
    bool			m_bWinActive;
    bool			m_bSysMenuWinShow;

public:
    virtual ~CUIMng();

    static CUIMng& Instance();

    void CreateTitleSceneUI();
    void ReleaseTitleSceneUI();
    void RenderTitleSceneUI(HDC hDC, DWORD dwNow, DWORD dwTotal);
    void Create();
    void Release();
    void CreateLoginScene();
    void CreateCharacterScene();
    void CreateMainScene();
    void Update(double dDeltaTick);
    void Render();

    void ShowWin(CWin* pWin);
    void HideWin(CWin* pWin);

    bool IsCursorOnUI() { return m_bCursorOnUI; }
    void PopUpMsgWin(int nMsgCode, wchar_t* pszMsg = NULL);
    void AddServerMsg(wchar_t* pszMsg);
    void CloseMsgWin() { HideWin(&m_MsgWin); }
    void SetSysMenuWinShow(bool bShow) { m_bSysMenuWinShow = bShow; }
    bool IsSysMenuWinShow() { return m_bSysMenuWinShow; };

protected:
    CUIMng();

    void RemoveWinList();
    CWin* SetActiveWin(CWin* pWin);
    void CheckDockWin();
    bool SetDockWinPosition(CWin* pMoveWin, int nDockX, int nDockY);
};
