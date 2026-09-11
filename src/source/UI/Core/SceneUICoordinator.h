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
// and forwards Update()/Render() to its own CManager instance (m_NewStyleMng below).
class CSceneUICoordinator
{
protected:
    bool m_bCursorOnUI;
    int m_nScene;

    // True only during the Update() in which the ESC-toggle-system-menu block itself opened/closed
    // g_SysMenuWin; reset at the top of every Update(). That block runs before m_NewStyleMng's
    // dispatch, so without this flag a window polling g_SysMenuWin::IsVisible() on Escape (CLoginWin)
    // would see this frame's post-toggle value and could react to the same keypress twice.
    bool m_bSysMenuToggledByEscThisFrame = false;

    // Scene-scoped CManager (not the shared g_pNewUIMng used by MAIN_SCENE) -- lets
    // login/character-scene-only windows (e.g. CCreditWin) update/render outside MAIN_SCENE.
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
     * @brief Re-layouts login/character-scene windows for the current WindowWidth/Height.
     * Call after a runtime resolution change. MAIN_SCENE windows are handled separately by
     * g_pNewUISystem.
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
