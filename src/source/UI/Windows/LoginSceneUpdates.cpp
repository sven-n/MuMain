#include "stdafx.h"

#include "UI/Windows/LoginSceneUpdates.h"

#include "Character/CharInfoBalloonMng.h"
#include "Character/CharSelMainWin.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Network/Server/ServerListManager.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Windows/CreditWin.h"
#include "UI/Windows/LoginMainWin.h"
#include "UI/Windows/LoginWin.h"
#include "UI/Windows/ServerSelWin.h"

#include <cstdlib>
#include <string>

namespace UI::LoginScene
{
void ServerListReceived()
{
    if (std::getenv("MU_INPUT_DIAGNOSTICS") != nullptr)
    {
        mu::log::Get("input")->info(
            "[InputDiag] server-list groups={} selector(show={},active={}) login-main(show={},active={}) credits={}",
            g_ServerListManager->GetServerGroupSize(), g_ServerSelWin.IsVisible(), g_ServerSelWin.IsActive(),
            g_LoginMainWin.IsVisible(), g_LoginMainWin.IsActive(), g_CreditWin.IsVisible());
    }
    if (!g_CreditWin.IsVisible())
    {
        g_ServerSelWin.Show(true);
        g_ServerSelWin.UpdateDisplay();
        g_LoginMainWin.Show(true);
    }
}

void ShowLoginWindow()
{
    g_LoginWin.Show(true);
    g_LoginWin.FocusUsername();
}

void HideLoginWindow()
{
    g_LoginWin.Show(false);
}

void ShowMessage(int messageCode)
{
    CSceneUICoordinator::Instance().PopUpMsgWin(messageCode);
}

void CloseMessage()
{
    CSceneUICoordinator::Instance().CloseMsgWin();
}

void AddServerMessage(std::wstring_view text)
{
    std::wstring message(text);
    CSceneUICoordinator::Instance().AddServerMsg(message.data());
}

void CharacterCreated()
{
    CSceneUICoordinator::Instance().CloseMsgWin();
    g_CharSelMainWin.UpdateDisplay();
    g_CharInfoBalloonMng.UpdateDisplay();
}
}
