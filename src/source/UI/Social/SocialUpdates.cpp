#include "stdafx.h"

#include "UI/Social/SocialUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Social/ChatRoomWindow.h"
#include "UI/Social/FriendWindow.h"
#include "UI/Social/SocialWindowManager.h"

#include <string>

namespace UI::Social
{
void Reset()
{
    g_pWindowMgr->Reset();
    g_pFriendList->ClearFriendList();
    g_pLetterList->ClearLetterList();
}

void ReplaceFriendList(std::span<const FriendEntry> friends)
{
    g_pWindowMgr->Reset();
    for (const FriendEntry& entry : friends)
        g_pFriendList->AddFriend(std::wstring(entry.name).c_str(), 0, entry.server);
    g_pFriendList->Sort(0);
    g_pFriendList->Sort(1);
    g_pWindowMgr->RefreshMainWndPalList();
    g_pWindowMgr->SetServerEnable(TRUE);
}

void FriendAdded(std::wstring_view name, std::uint8_t server)
{
    g_pFriendList->AddFriend(std::wstring(name).c_str(), 0, server);
    g_pFriendList->Sort();
    g_pWindowMgr->RefreshMainWndPalList();
    g_pFriendMenu->UpdateAllChatWindowInviteList();
}

void FriendRemoved(std::wstring_view name)
{
    g_pFriendList->RemoveFriend(std::wstring(name).c_str());
    g_pWindowMgr->RefreshMainWndPalList();
}

void FriendStateChanged(std::wstring_view name, std::uint8_t server, bool offline)
{
    const std::wstring friendName(name);
    g_pFriendList->UpdateFriendState(friendName.c_str(), 0, server);
    g_pFriendList->Sort();
    g_pWindowMgr->RefreshMainWndPalList();
    g_pFriendMenu->UpdateAllChatWindowInviteList();

    const DWORD dwChatRoomUIID = g_pFriendMenu->CheckChatRoomDuplication(friendName.c_str());
    if (dwChatRoomUIID > 0)
    {
        if (auto* pWindow = static_cast<CUIChatWindow*>(g_pWindowMgr->GetWindow(dwChatRoomUIID)))
            pWindow->Lock(offline ? TRUE : FALSE);
    }
}

void FriendServerLost(std::uint8_t server)
{
    g_pFriendList->UpdateAllFriendState(0, server);
    g_pFriendList->Sort();
    g_pWindowMgr->RefreshMainWndPalList();
    g_pFriendMenu->LockAllChatWindow();
    g_pWindowMgr->SetServerEnable(FALSE);
}

void ShowFriendRequest(std::wstring_view prompt, std::wstring_view requester)
{
    if (!UI::Windows::IsVisible(mu::ui::window::INTERFACE_FRIEND))
        UI::Windows::Show(mu::ui::window::INTERFACE_FRIEND);
    g_pWindowMgr->Dialogs().FriendRequest(std::wstring(prompt).c_str(), std::wstring(requester).c_str());
}

void ShowNotice(std::wstring_view text)
{
    g_pWindowMgr->Dialogs().Notice(std::wstring(text).c_str());
}
}
