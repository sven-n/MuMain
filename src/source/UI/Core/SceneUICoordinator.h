//*****************************************************************************
// File: SceneUICoordinator.h
//*****************************************************************************
#pragma once

#include "UI/Windows/MsgWin.h"
#include "UI/Core/NewUIManager.h"

#define UIM_SCENE_NONE 0
#define UIM_SCENE_TITLE 1
#define UIM_SCENE_LOGIN 2
#define UIM_SCENE_LOADING 3
#define UIM_SCENE_CHARACTER 4
#define UIM_SCENE_MAIN 5

// Was CUIMng (docs/newui-legacy-merger.md, Phase 4) -- that class used to own a CWin-derived
// window list of its own; every one of those windows has since migrated onto
// mu::ui::window::CNewUIObj/CNewUIManager (Phases 1-3), so all that remained of it was this class's
// actual, still-needed job: creating/releasing/positioning the login- and character-scene g_*Win
// globals per scene transition, and forwarding Update()/Render() to its own CNewUIManager instance.
// Renamed to describe that job directly, not a "CWin manager" that no longer manages any CWin.
class CSceneUICoordinator
{
protected:
    bool m_bCursorOnUI;
    int m_nScene;

    // True only during the Update() call in which the ESC-toggle-system-menu block itself opened
    // or closed g_SysMenuWin -- reset at the top of every Update(). See Update()'s own comment for
    // the race this exists to prevent: that block runs entirely before m_NewStyleMng's depth-sorted
    // dispatch (not as part of it), so a migrated window whose own Escape polling depends on
    // g_SysMenuWin::IsVisible() (CLoginWin) would otherwise see this frame's POST-toggle value and
    // could react to the very same keypress a second time.
    bool m_bSysMenuToggledByEscThisFrame = false;

    // A scene-scoped CNewUIManager instance (own object, not the shared g_pNewUIMng that
    // MAIN_SCENE's ~79 windows use): CNewUIManager's dispatch is only ever driven from
    // MainScene.cpp today, so a window that only exists during login/character scenes (like
    // CCreditWin) would never update/render if registered with the shared instance instead.
    // Update()/Render() below (already called unconditionally every frame, regardless of scene)
    // forward to this one, giving every login/character-scene window the same INewUIBase
    // interface and dispatch semantics as the MAIN_SCENE-tier CNewUIObj windows, without touching
    // the shared manager at all.
    mu::ui::window::CNewUIManager m_NewStyleMng;

public:
    virtual ~CSceneUICoordinator();

    static CSceneUICoordinator& Instance();

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
     * Only affects the login/character-scene windows this class itself drives
     * (g_CreditWin, g_LoginWin, g_MsgWin, etc. -- see docs/newui-legacy-merger.md);
     * the MAIN_SCENE-only CNewUI* windows are driven by g_pNewUISystem separately.
     */
    void RepositionSceneUI();

    void Update(double dDeltaTick);
    void Render();

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
    // See m_bSysMenuToggledByEscThisFrame's own comment.
    bool WasSysMenuToggledByEscThisFrame() const
    {
        return m_bSysMenuToggledByEscThisFrame;
    }

    // See m_NewStyleMng's own comment -- windows migrating off CWin register here instead of the
    // shared g_pNewUIMng.
    mu::ui::window::CNewUIManager& GetNewStyleMng()
    {
        return m_NewStyleMng;
    }

protected:
    CSceneUICoordinator();
};
