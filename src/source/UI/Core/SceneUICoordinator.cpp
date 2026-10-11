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

CSceneUICoordinator::CSceneUICoordinator()
{
    // The login and character scenes' windows measure and draw as the original did there: the
    // original screen stretched over the window, text at its own size.
    m_NewStyleMng.SetUnits(&UI::Scaling::LegacyUiTransform);
}

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

    g_MsgWin.Create();
    g_SysMenuWin.Create();
    g_LoginMainWin.Create();
    g_ServerSelWin.Create();
    g_LoginWin.Create();
    g_CreditWin.Create();

    m_nScene = UIM_SCENE_LOGIN;
    RepositionSceneUI();
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

    g_MsgWin.Create();
    g_ServerMsgWin.Create();
    g_SysMenuWin.Create();
    g_CharSelMainWin.Create();
    g_CharMakeWin.Create();

    g_CharSelMainWin.UpdateDisplay();
    g_CharInfoBalloonMng.UpdateDisplay();
    g_CharSelMainWin.Show(true);

    m_nScene = UIM_SCENE_CHARACTER;
    RepositionSceneUI();
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
    // RmlUi relayouts the documents; only native bookkeeping and pushed origins need refreshing.
    if (m_nScene == UIM_SCENE_LOGIN)
        RepositionLoginWindows();
    else if (m_nScene == UIM_SCENE_CHARACTER)
        RepositionCharacterWindows();
}

void CSceneUICoordinator::RepositionLoginWindows()
{
    constexpr int messageWidth = 352;
    constexpr int messageHeight = 113;
    constexpr int barMargin = 30;
    constexpr int barGap = 11;
    constexpr float barBaseline = 567.0f / 600.0f;
    constexpr int loginVerticalDivisor = 3;
    const int width = static_cast<int>(WindowWidth);
    const int height = static_cast<int>(WindowHeight);
    g_MsgWin.SetPosition((width - messageWidth) / 2, (height - messageHeight) / 2);
    g_LoginMainWin.RefreshLayout();
    g_LoginMainWin.SetPosition(barMargin,
        static_cast<int>(barBaseline * height) - g_LoginMainWin.GetHeight() - barGap);
    g_LoginWin.RefreshLayout();
    g_LoginWin.SetPosition((width - g_LoginWin.GetWidth()) / 2,
                          (height - g_LoginWin.GetHeight()) * 2 / loginVerticalDivisor);
}

void CSceneUICoordinator::RepositionCharacterWindows()
{
    constexpr int messageWidth = 352;
    constexpr int messageHeight = 113;
    constexpr int noticeMargin = 10;
    constexpr float noticeBaseline = 31.0f / 600.0f;
    constexpr int creationWidth = 454;
    constexpr int creationHeight = 406;
    const int width = static_cast<int>(WindowWidth);
    const int height = static_cast<int>(WindowHeight);
    g_MsgWin.SetPosition((width - messageWidth) / 2, (height - messageHeight) / 2);
    g_ServerMsgWin.SetPosition(noticeMargin, static_cast<int>(noticeBaseline * height) + noticeMargin);
    g_CharSelMainWin.RefreshLayout();
    g_CharMakeWin.SetPosition((width - creationWidth) / 2, (height - creationHeight) / 2);
}

void CSceneUICoordinator::Update(double dDeltaTick)
{
    if (UIM_SCENE_NONE == m_nScene)
        return;

    // Polls for the background reconnect thread (started on Cancel) to finish, then joins it.
    if (g_LoginWin.HasPendingConnectionReconnect())
        g_LoginWin.ProcessPendingConnectionReconnect();

    // m_bCursorOnUI folds in whatever claims the mouse, so a modal like CMsgWin still blocks
    // world-click/rotation gating even without a full-screen rect of its own.
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

    m_NewStyleMng.Render();
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
