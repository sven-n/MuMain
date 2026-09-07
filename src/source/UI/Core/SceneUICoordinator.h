//*****************************************************************************
// File: SceneUICoordinator.h
//*****************************************************************************
#pragma once

#include "UI/Windows/MsgWin.h"
#include "UI/Core/WindowManager.h"

#define UIM_SCENE_NONE 0
#define UIM_SCENE_TITLE 1
#define UIM_SCENE_LOGIN 2
#define UIM_SCENE_LOADING 3
#define UIM_SCENE_CHARACTER 4
#define UIM_SCENE_MAIN 5

// Creates/releases/positions the login- and character-scene g_*Win globals per scene transition,
// and forwards Update()/Render() to its own CManager instance (m_NewStyleMng below). No
// CWin-derived window list of its own -- every window that formerly needed one has migrated onto
// mu::ui::window::CObject/CManager.
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

    // A scene-scoped CManager instance (own object, not the shared g_pNewUIMng that
    // MAIN_SCENE's ~79 windows use): CManager's dispatch is only ever driven from
    // MainScene.cpp today, so a window that only exists during login/character scenes (like
    // CCreditWin) would never update/render if registered with the shared instance instead.
    // Update()/Render() below (already called unconditionally every frame, regardless of scene)
    // forward to this one, giving every login/character-scene window the same IObject
    // interface and dispatch semantics as the MAIN_SCENE-tier CObject windows, without touching
    // the shared manager at all.
    mu::ui::window::CManager m_NewStyleMng;

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
     * (g_CreditWin, g_LoginWin, g_MsgWin, etc.);
     * the MAIN_SCENE-only mu::ui::window windows are driven by g_pNewUISystem separately.
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
    mu::ui::window::CManager& GetNewStyleMng()
    {
        return m_NewStyleMng;
    }

protected:
    CSceneUICoordinator();
};
