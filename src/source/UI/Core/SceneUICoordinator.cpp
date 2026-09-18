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

    // Uses WindowWidth/WindowHeight, not CInput's screen size, which isn't guaranteed to match
    // (see LoginWin.cpp's LoginUIScaleRatio()); g_LoginWin/g_LoginMainWin sit only ~11px apart,
    // so a mismatch here could overlap their hit-test boxes.
    g_MsgWin.Create();
    g_MsgWin.SetPosition((static_cast<int>(WindowWidth) - 352) / 2, (static_cast<int>(WindowHeight) - 113) / 2);

    g_SysMenuWin.Create();

    g_LoginMainWin.Create();

    int nBaseY = int(567.0f / 600.0f * static_cast<float>(WindowHeight));
    g_LoginMainWin.SetPosition(30, nBaseY - g_LoginMainWin.GetHeight() - 11);

    g_ServerSelWin.Create();

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
    // Uses WindowWidth/WindowHeight, not CInput's screen size -- a past mismatch here made
    // RmlUi's #panel position disagree with the real window, drifting the caret off the dialog.
    g_CharMakeWin.SetPosition(
        (static_cast<int>(WindowWidth) - 454) / 2,
        (static_cast<int>(WindowHeight) - 406) / 2);

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
    // A plain SetPosition sweep isn't enough: CSprite's cached screen height (set at Create())
    // goes stale on resize, so we must re-Create the sprites -- which also resets m_bShow, hence
    // the visibility snapshot/restore below.
    if (m_nScene == UIM_SCENE_LOGIN)
    {
        const bool wasShown_MsgWin = g_MsgWin.IsVisible();
        const bool wasShown_SysMenuWin = g_SysMenuWin.IsVisible();
        const bool wasShown_LoginMainWin = g_LoginMainWin.IsVisible();
        const bool wasShown_ServerSelWin = g_ServerSelWin.IsVisible();
        const bool wasShown_LoginWin = g_LoginWin.IsVisible();
        const bool wasShown_CreditWin = g_CreditWin.IsVisible();

        CreateLoginScene();

        // Restore each window's pre-resize visibility.
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

        // Create() clears button labels; re-populate from existing network-side data.
        g_ServerSelWin.UpdateDisplay();
    }
    else if (m_nScene == UIM_SCENE_CHARACTER)
    {
        CreateCharacterScene();
    }
    // MainScene uses the new UI system which resizes itself; nothing to do.
}

void CSceneUICoordinator::Update(double dDeltaTick)
{
    if (UIM_SCENE_NONE == m_nScene)
        return;

    // Polls for the background reconnect thread (started on Cancel) to finish, then joins it.
    if (g_LoginWin.HasPendingConnectionReconnect())
        g_LoginWin.ProcessPendingConnectionReconnect();

    // m_bCursorOnUI folds in whatever claims the mouse, so a modal like CMsgWin still blocks
    // world-click/rotation gating the same way its old full-screen rect used to.
    m_NewStyleMng.UpdateMouseEvent();
    m_NewStyleMng.UpdateKeyEvent();
    m_bCursorOnUI = m_NewStyleMng.GetActiveMouseUIObj() != nullptr;

    CInput& rInput = CInput::Instance();

    // ESC opens/closes the system menu (mouse Cancel still cancels the login form). Runs before
    // m_NewStyleMng.Update() and sets m_bSysMenuToggledByEscThisFrame -- without it, g_LoginWin's
    // own Escape-cancel check would see this frame's post-toggle IsVisible() and fire too.
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
            else if (!g_MsgWin.IsVisible() && !g_CreditWin.IsVisible() && !g_CharMakeWin.IsVisible())
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
