//*****************************************************************************
// File: UIMng.h
//*****************************************************************************
#pragma once

#include "Core/Utilities/PList.h"
#include "UI/Windows/MsgWin.h"
#include "UI/NewUI/NewUIManager.h"

#define UIM_SCENE_NONE 0
#define UIM_SCENE_TITLE 1
#define UIM_SCENE_LOGIN 2
#define UIM_SCENE_LOADING 3
#define UIM_SCENE_CHARACTER 4
#define UIM_SCENE_MAIN 5

class CSprite;
class CGaugeBar;
class CWin;
class CLoadingScene;

class CUIMng
{
public:
    CLoadingScene* m_pLoadingScene;

protected:
    CSprite* m_asprTitle;
    CGaugeBar* m_pgbLoding;
    CPList m_WinList;
    bool m_bCursorOnUI;
    bool m_bBlockCharMove;
    int m_nScene;
    bool m_bWinActive;
    bool m_bSysMenuWinShow;

    // True only during the Update() call in which the ESC-toggle-system-menu block itself opened
    // or closed g_SysMenuWin -- reset at the top of every Update(). See Update()'s own comment for
    // the race this exists to prevent: that block runs entirely before m_NewStyleMng's depth-sorted
    // dispatch (not as part of it), so a migrated window whose own Escape polling depends on
    // g_SysMenuWin::IsVisible() (CLoginWin) would otherwise see this frame's POST-toggle value and
    // could react to the very same keypress a second time.
    bool m_bSysMenuToggledByEscThisFrame = false;

    // CUIMng/CNewUIManager merger (docs/rmlui-ui-system) -- a scene-scoped CNewUIManager instance
    // (own object, not the shared g_pNewUIMng that MAIN_SCENE's ~79 windows use): CNewUIManager's
    // dispatch is only ever driven from MainScene.cpp today, so a window that only exists during
    // login/character scenes (like CCreditWin, the Phase 1 pilot) would never update/render if
    // registered with the shared instance instead. Update()/Render() below (already called
    // unconditionally every frame, regardless of scene) forward to this one, giving migrated
    // CUIMng windows the same INewUIBase interface and dispatch semantics as the CNewUIObj tier
    // without touching the MAIN_SCENE-only shared manager at all.
    SEASON3B::CNewUIManager m_NewStyleMng;

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

    /**
     * @brief Re-layouts the current scene's UI for the current WindowWidth/
     * Height. Call after a runtime resolution change so info boxes, menus,
     * etc. don't end up anchored to the old screen size.
     *
     * Only affects the login/character-scene windows CUIMng itself drives
     * (g_CreditWin, g_LoginWin, g_MsgWin, etc. -- CUIMng/CNewUIManager merger,
     * docs/newui-legacy-merger.md); the MAIN_SCENE-only CNewUI* windows are
     * driven by g_pNewUISystem separately.
     */
    void RepositionSceneUI();

    void Update(double dDeltaTick);
    void Render();

    void ShowWin(CWin* pWin);
    void HideWin(CWin* pWin);

    bool IsCursorOnUI()
    {
        return m_bCursorOnUI;
    }
    void PopUpMsgWin(int nMsgCode, wchar_t* pszMsg = NULL);
    void AddServerMsg(wchar_t* pszMsg);
    void CloseMsgWin()
    {
        g_MsgWin.Show(false);
    }
    void SetSysMenuWinShow(bool bShow)
    {
        m_bSysMenuWinShow = bShow;
    }
    bool IsSysMenuWinShow()
    {
        return m_bSysMenuWinShow;
    };
    // See m_bSysMenuToggledByEscThisFrame's own comment.
    bool WasSysMenuToggledByEscThisFrame() const
    {
        return m_bSysMenuToggledByEscThisFrame;
    }

    // See m_NewStyleMng's own comment -- windows migrating off CWin register here instead of the
    // shared g_pNewUIMng.
    SEASON3B::CNewUIManager& GetNewStyleMng()
    {
        return m_NewStyleMng;
    }

protected:
    CUIMng();

    void RemoveWinList();
    CWin* SetActiveWin(CWin* pWin);
    void CheckDockWin();
    bool SetDockWinPosition(CWin* pMoveWin, int nDockX, int nDockY);
};
