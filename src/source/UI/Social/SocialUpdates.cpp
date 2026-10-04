#include "stdafx.h"

#include "UI/Social/SocialUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Social/ChatRoomWindow.h"
#include "UI/Social/FriendWindow.h"
#include "UI/Social/LetterReadWindow.h"
#include "UI/Social/LetterWriteWindow.h"
#include "UI/Social/SocialWindowManager.h"
#include "I18N/All.h"

#include <string>

extern int g_iLetterReadNextPos_x, g_iLetterReadNextPos_y;

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

void LetterSent(std::uint32_t writeWindow)
{
    if (writeWindow != 0)
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, writeWindow, 0);
}

void LetterSendFailed(std::uint32_t writeWindow, std::wstring_view notice)
{
    if (writeWindow != 0)
    {
        if (auto* pWindow = static_cast<CUILetterWriteWindow*>(g_pWindowMgr->GetWindow(writeWindow)))
            pWindow->SetSendState(FALSE);
    }
    ShowNotice(notice);
}

void NewLetterArrived(const LetterSummary& letter)
{
    g_pFriendMenu->SetNewMailAlert(TRUE);
    g_pLetterList->AddLetter(letter.index, std::wstring(letter.sender).c_str(), std::wstring(letter.subject).c_str(),
                             std::wstring(letter.date).c_str(), std::wstring(letter.time).c_str(), 0x00);
    g_pLetterList->Sort();
    g_pWindowMgr->RefreshMainWndLetterList();
}

void LetterListed(const LetterSummary& letter, std::uint8_t readState)
{
    g_pLetterList->AddLetter(letter.index, std::wstring(letter.sender).c_str(), std::wstring(letter.subject).c_str(),
                             std::wstring(letter.date).c_str(), std::wstring(letter.time).c_str(), readState);
    g_pLetterList->Sort(2);
    g_pWindowMgr->RefreshMainWndLetterList();
}

void LetterDeleted(std::uint32_t index)
{
    g_pLetterList->RemoveLetter(index);
    g_pLetterList->RemoveLetterTextCache(index);
    g_pWindowMgr->RefreshMainWndLetterList();
}

int LetterCount()
{
    return g_pLetterList->GetLetterCount();
}

void LetterBodyReceived(const LetterBody& body)
{
    g_pLetterList->CacheLetterText(body.index, body);
    ShowLetter(body);
}

void ShowLetter(const LetterBody& body)
{
    LETTERLIST_TEXT* pLetterHead = g_pLetterList->GetLetter(body.index);
    if (pLetterHead == nullptr)
        return;

    pLetterHead->m_bIsRead = TRUE;
    g_pWindowMgr->RefreshMainWndLetterList();

    wchar_t title[MAX_TEXT_LENGTH + 1];
    mu_swprintf(title, I18N::Game::ReadLetterS, pLetterHead->m_szText);
    DWORD dwUIID = 0;
    if (g_iLetterReadNextPos_x == UIWND_DEFAULT)
    {
        dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_READLETTER, 100, 100, title);
    }
    else
    {
        dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_READLETTER, g_iLetterReadNextPos_x, g_iLetterReadNextPos_y, title,
                                         0, UIADDWND_FORCEPOSITION);
        g_iLetterReadNextPos_x = UIWND_DEFAULT;
    }

    auto* pWindow = static_cast<CUILetterReadWindow*>(g_pWindowMgr->GetWindow(dwUIID));
    pWindow->SetLetter(pLetterHead, body.text.c_str());

    g_pWindowMgr->SetLetterReadWindow(pLetterHead->m_dwLetterID, dwUIID);

    if (wcsnicmp(pLetterHead->m_szID, L"webzen", MAX_USERNAME_SIZE) == 0)
    {
        pWindow->m_Photo.SetWebzenMail(TRUE);
    }
    else
    {
        std::array<BYTE, 25> equipment = body.equipment;
        pWindow->m_Photo.SetClass(static_cast<CLASS_TYPE>(body.classType));
        pWindow->m_Photo.SetEquipmentPacket(equipment.data());
        pWindow->m_Photo.SetAnimation(body.animation);
        pWindow->m_Photo.SetAngle(body.angleDegrees);
        pWindow->m_Photo.SetZoom(body.zoom);
    }
    pWindow->SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);
}
}
