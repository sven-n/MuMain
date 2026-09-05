//*****************************************************************************
// File: SceneUICoordinator.cpp
//*****************************************************************************

#include "stdafx.h"
#include "SceneUICoordinator.h"
#include "UI/Windows/CreditWin.h"
#include "UI/Windows/ServerMsgWin.h"
#include "UI/Windows/ServerSelWin.h"
#include "UI/Windows/SysMenuWin.h"
#include "UI/Windows/LoginWin.h"
#include "Character/CharSelMainWin.h"
#include "Character/CharMakeWin.h"
#include "Character/CharInfoBalloonMng.h"
#include "UI/Windows/LoginMainWin.h"
#include "Core/Globals/_enum.h"
#include "Core/Input/Input.h"
#include "Audio/DSPlaySound.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"

#include "Network/Server/ServerListManager.h"
#include "UI/Scaling/UITransform.h"

CSceneUICoordinator::CSceneUICoordinator() {}

CSceneUICoordinator::~CSceneUICoordinator() {}

CSceneUICoordinator& CSceneUICoordinator::Instance()
{
    static CSceneUICoordinator s_Coordinator;
    return s_Coordinator;
}

void CSceneUICoordinator::Create()
{
    m_bCursorOnUI = false;
    m_nScene = UIM_SCENE_NONE;

    return;
}

void CSceneUICoordinator::Release()
{
    g_CharInfoBalloonMng.Release();
    g_CreditWin.Release();
    g_ServerMsgWin.Release();
    g_ServerSelWin.Release();
    g_MsgWin.Release();
    g_SysMenuWin.Release();
    g_CharSelMainWin.Release();
    g_CharMakeWin.Release();
    g_LoginMainWin.Release();
    g_LoginWin.Release();

    m_nScene = UIM_SCENE_NONE;
}

void CSceneUICoordinator::CreateLoginScene()
{
    g_CharInfoBalloonMng.Release();
    g_CreditWin.Release();
    g_ServerMsgWin.Release();
    g_ServerSelWin.Release();
    g_MsgWin.Release();
    g_SysMenuWin.Release();
    g_CharSelMainWin.Release();
    g_CharMakeWin.Release();
    g_LoginMainWin.Release();
    g_LoginWin.Release();

    // WindowWidth/WindowHeight (ZzzOpenglUtil.cpp), not CInput::Instance().GetScreenWidth()/
    // GetScreenHeight() -- see LoginWin.cpp's LoginUIScaleRatio() for why: CInput's own copy of
    // the screen size isn't guaranteed to already match WindowWidth/WindowHeight (the exact
    // values every migrated window's own Create()/SetPosition() now uses internally). This
    // function is the one place that positions g_LoginWin and g_LoginMainWin relative to each
    // other -- their hit-test boxes sit only ~11px apart vertically at the reference resolution.
    // A drift between this function's source of screen size and each window's own internal one
    // could close that gap into an overlap, but that's no longer a starvation risk either way
    // (docs/newui-legacy-merger.md, Phase 3): both windows' dispatch now goes through the same
    // depth-sorted CNewUIManager claim, not a first-checked-wins list walk.
    g_MsgWin.Create();
    g_MsgWin.SetPosition((static_cast<int>(WindowWidth) - 352) / 2, (static_cast<int>(WindowHeight) - 113) / 2);

    g_SysMenuWin.Create();

    g_LoginMainWin.Create();

    int nBaseY = int(567.0f / 600.0f * static_cast<float>(WindowHeight));
    g_LoginMainWin.SetPosition(30, nBaseY - g_LoginMainWin.GetHeight() - 11);

    g_ServerSelWin.Create();
    g_ServerSelWin.SetPosition((static_cast<int>(WindowWidth) - g_ServerSelWin.GetWidth()) / 2,
                               (static_cast<int>(WindowHeight) - g_ServerSelWin.GetHeight()) / 2);

    g_LoginWin.Create();
    g_LoginWin.SetPosition((static_cast<int>(WindowWidth) - g_LoginWin.GetWidth()) / 2,
                           (static_cast<int>(WindowHeight) - g_LoginWin.GetHeight()) * 2 / 3);

    g_CreditWin.Create();

    m_nScene = UIM_SCENE_LOGIN;
}

void CSceneUICoordinator::CreateCharacterScene()
{
    g_CreditWin.Release();
    g_ServerMsgWin.Release();
    g_ServerSelWin.Release();
    g_MsgWin.Release();
    g_SysMenuWin.Release();
    g_CharSelMainWin.Release();
    g_CharMakeWin.Release();
    g_LoginMainWin.Release();
    g_LoginWin.Release();

    g_CharInfoBalloonMng.Create();

    CInput& rInput = CInput::Instance();

    g_MsgWin.Create();
    g_MsgWin.SetPosition((rInput.GetScreenWidth() - 352) / 2, (rInput.GetScreenHeight() - 113) / 2);

    g_ServerMsgWin.Create();
    int nBaseY = int(31.0f / 600.0f * (float)rInput.GetScreenHeight());
    g_ServerMsgWin.SetPosition(10, nBaseY + 10);

    g_SysMenuWin.Create();

    g_CharSelMainWin.Create();

    g_CharMakeWin.Create();
    g_CharMakeWin.SetPosition((rInput.GetScreenWidth() - 454) / 2, (rInput.GetScreenHeight() - 406) / 2);

    g_CharSelMainWin.UpdateDisplay();
    g_CharInfoBalloonMng.UpdateDisplay();

    g_CharSelMainWin.Show(true);

    m_nScene = UIM_SCENE_CHARACTER;
}

void CSceneUICoordinator::CreateMainScene()
{
    g_CharInfoBalloonMng.Release();
    g_CreditWin.Release();
    g_ServerMsgWin.Release();
    g_ServerSelWin.Release();
    g_MsgWin.Release();
    g_SysMenuWin.Release();
    g_CharSelMainWin.Release();
    g_CharMakeWin.Release();
    g_LoginMainWin.Release();
    g_LoginWin.Release();

    m_nScene = UIM_SCENE_MAIN;
}

void CSceneUICoordinator::RepositionSceneUI()
{
    // A lightweight SetPosition sweep isn't enough: CSprite caches
    // m_fScrHeight = WindowHeight at Create() time and uses it for the
    // Y-flipped coordinate math in SetPosition(). When the window resizes,
    // every sprite's cached screen height is stale, so a pure SetPosition
    // call lands the windows in the wrong place.
    //
    // The only clean way to refresh that cache is to re-Create the sprites,
    // which is exactly what the scene's Create*Scene() function does. But
    // that also resets each window's m_bShow flag, so we snapshot the
    // current visibility here and restore it right after.
    if (m_nScene == UIM_SCENE_LOGIN)
    {
        const bool wasShown_MsgWin = g_MsgWin.IsVisible();
        const bool wasShown_SysMenuWin = g_SysMenuWin.IsVisible();
        const bool wasShown_LoginMainWin = g_LoginMainWin.IsVisible();
        const bool wasShown_ServerSelWin = g_ServerSelWin.IsVisible();
        const bool wasShown_LoginWin = g_LoginWin.IsVisible();
        const bool wasShown_CreditWin = g_CreditWin.IsVisible();

        CreateLoginScene();

        // Restore visibility BEFORE re-populating dynamic windows: child
        // elements like server/group buttons read `CWin::m_bShow` of their
        // parent when `UpdateDisplay()` decides which sub-elements to show.
        // If the parent is still hidden at that moment, nothing renders.
        if (wasShown_MsgWin)
            g_MsgWin.Show(true);
        if (wasShown_SysMenuWin)
            g_SysMenuWin.Show(true);
        if (wasShown_LoginMainWin)
            g_LoginMainWin.Show(true);
        if (wasShown_ServerSelWin)
            g_ServerSelWin.Show(true);
        if (wasShown_LoginWin)
            g_LoginWin.Show(true);
        if (wasShown_CreditWin)
            g_CreditWin.Show(true);

        // Re-populate the server / server-group buttons from the existing
        // network-side data. Create() clears the button labels, so without
        // this the server list and groups render empty after a resolution
        // change.
        g_ServerSelWin.UpdateDisplay();
    }
    else if (m_nScene == UIM_SCENE_CHARACTER)
    {
        // CreateCharacterScene() ends with an explicit g_CharSelMainWin.Show(true)
        // so visibility of the main panel is already preserved. Other character-
        // scene windows (msg box, server msg, char make) are shown on demand
        // by game events, matching the fresh-scene state.
        CreateCharacterScene();
    }
    // MainScene uses the new UI system which resizes itself; nothing to do.
}

void CSceneUICoordinator::Update(double dDeltaTick)
{
    if (UIM_SCENE_NONE == m_nScene)
        return;

    // New-style (CNewUIObj-tier) windows (docs/newui-legacy-merger.md) -- the only dispatch this
    // class still drives; every window it used to own via a CWin list has migrated onto
    // mu::ui::window::CNewUIObj/CNewUIManager. m_bCursorOnUI folds in whatever this claimed, so a
    // migrated modal like CMsgWin still blocks CharacterScene.cpp's world-click/rotation gating
    // the same way its old full-screen CWin::CursorInWin(WA_ALL) rect used to.
    m_NewStyleMng.UpdateMouseEvent();
    m_NewStyleMng.UpdateKeyEvent();
    m_bCursorOnUI = m_NewStyleMng.GetActiveMouseUIObj() != nullptr;

    CInput& rInput = CInput::Instance();

    // ESC toggles system menu in login/character scenes. Checked before m_NewStyleMng.Update()
    // (below) runs -- a migrated window's own ESC handling (e.g. CMsgWin closing itself on ESC)
    // must not also flip g_MsgWin.IsVisible() to false in time to fool this same frame's check.
    //
    // Also resets/sets m_bSysMenuToggledByEscThisFrame (own comment): closing the menu here, then
    // letting m_NewStyleMng.Update() (below) run g_LoginWin's own Escape-cancel gate in the same
    // frame, would make a single ESC press both close the menu AND cancel the login form behind
    // it. Unlike CCreditWin (depth 100) and CMsgWin (depth 50), both handled entirely inside
    // m_NewStyleMng's own depth-sorted dispatch (so a lower-depth g_LoginWin's Update() always runs
    // BEFORE theirs, seeing pre-close state for free), g_SysMenuWin's ESC close happens here,
    // completely outside that dispatch and unconditionally before it -- g_LoginWin has no ordering
    // protection against it on its own.
    m_bSysMenuToggledByEscThisFrame = false;
    if (rInput.IsKeyDown(VK_ESCAPE))
    {
        extern EGameScene SceneFlag;
        if (SceneFlag == LOG_IN_SCENE || SceneFlag == CHARACTER_SCENE)
        {
            if (g_SysMenuWin.IsVisible())
            {
                g_SysMenuWin.Show(false);
                m_bSysMenuToggledByEscThisFrame = true;
            }
            else if (!g_MsgWin.IsVisible() && !g_LoginWin.IsVisible() && !g_CreditWin.IsVisible() &&
                     !g_CharMakeWin.IsVisible())
            {
                ::PlayBuffer(SOUND_CLICK01);
                g_SysMenuWin.Show(true);
                m_bSysMenuToggledByEscThisFrame = true;
            }
        }
    }

    m_NewStyleMng.Update();
}

void CSceneUICoordinator::Render()
{
    if (UIM_SCENE_NONE == m_nScene)
        return;

    const auto previousTransform = UI::Scaling::GetActiveTransform();
    UI::Scaling::SetActiveTransform(UI::Scaling::LegacyUiTransform(WindowWidth, WindowHeight));

    m_NewStyleMng.Render();

    UI::Scaling::SetActiveTransform(previousTransform);
}

void CSceneUICoordinator::PopUpMsgWin(int nMsgCode, wchar_t* pszMsg)
{
    if (UIM_SCENE_NONE == m_nScene || UIM_SCENE_TITLE == m_nScene || UIM_SCENE_LOADING == m_nScene)
        return;

    if (UIM_SCENE_MAIN == m_nScene)
        return;

    g_MsgWin.PopUp(nMsgCode, pszMsg);
}

void CSceneUICoordinator::AddServerMsg(wchar_t* pszMsg)
{
    if (UIM_SCENE_CHARACTER != m_nScene)
        return;

    g_ServerMsgWin.AddMsg(pszMsg);
}
