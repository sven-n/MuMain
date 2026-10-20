#include "stdafx.h"
#include "UI/Social/ChatRoomWindow.h"
#include "UI/Social/ChatRoomView.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Core/WindowSystem.h"
#include "I18N/All.h"
#include <algorithm>

namespace
{
constexpr BYTE SystemSpeaker = 255;
constexpr size_t RoomTitleCapacity = 128;
constexpr size_t MaximumRoomParticipants = 30;
}

CUIChatWindow::CUIChatWindow() : m_dwRoomNumber(0), m_View(std::make_unique<UI::Social::ChatRoomView>(*this))
{
}

CUIChatWindow::~CUIChatWindow()
{
    DisconnectToChatServer();
}

void CUIChatWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    SetTitle(pszTitle);
    SetParentUIID(dwParentID);
    SetPosition(50, 50);
    SetSize(250, 170);
    m_View->Build();
}

void CUIChatWindow::Refresh()
{
    m_bHaveTextBox = TRUE;
}

BOOL CUIChatWindow::DoAction(BOOL messageOnly)
{
    while (!m_MessageList.empty())
    {
        GetUIMessage();
        HandleMessage();
    }
    if (!messageOnly)
    {
        // Enter on this room's own field, the way every other RmlUi field in this client takes it.
        if (m_View->FieldHasFocus() && mu::ui::window::IsPress(VK_RETURN))
            m_View->SubmitDraft();
        m_View->ProcessActions();
        if (g_dwMouseUseUIID == 0 && (g_dwActiveUIID == 0 || g_dwActiveUIID == GetUIID()) &&
            m_View->PointerOver())
            g_dwMouseUseUIID = GetUIID();
    }
    return FALSE;
}

BOOL CUIChatWindow::HandleMessage()
{
    if (m_WorkMessage.m_iMessage == UI_MESSAGE_SELECTED)
        m_View->FocusField();
    return TRUE;
}

void CUIChatWindow::FocusReset()
{
    m_View->FocusField();
}

int CUIChatWindow::AddChatPal(const wchar_t* pszID, BYTE Number, BYTE Server)
{
    (void)Server;
    const auto it = std::find_if(m_Participants.begin(), m_Participants.end(),
                                 [pszID](const Participant& pal) { return pal.name == pszID; });
    if (it != m_Participants.end())
        it->number = Number;
    else
        m_Participants.push_back({pszID, Number});
    m_View->AddPal(pszID, Number);
    RefreshRoomTitle();
    const int count = GetUserCount();
    if (count >= 2)
        Lock(FALSE);
    return count;
}

void CUIChatWindow::RemoveChatPal(const wchar_t* pszID)
{
    const bool refreshTitle = m_Participants.size() > 2;
    std::erase_if(m_Participants, [pszID](const Participant& pal) { return pal.name == pszID; });
    m_View->RemovePal(pszID);
    if (refreshTitle)
        RefreshRoomTitle();
}

// CUIChatPalListBox::MakeTitleText fed the window title from the room's members.
void CUIChatWindow::RefreshRoomTitle()
{
    wchar_t szTitle[RoomTitleCapacity] = {0};
    wcsncpy(szTitle, I18N::Game::Talking, RoomTitleCapacity - 1);
    int named = 0;
    for (const auto& pal : m_Participants)
    {
        if (pal.name == Hero->ID)
            continue;
        std::wstring suffix = named > 0 ? L", " : L"";
        suffix += pal.name;
        if (++named >= 3)
            suffix += L"...";
        const size_t used = wcslen(szTitle);
        if (used + 1 < RoomTitleCapacity)
            wcsncat(szTitle, suffix.c_str(), RoomTitleCapacity - used - 1);
        if (named >= 3)
            break;
    }
    SetTitle(szTitle);
    g_pWindowMgr->RefreshMainWndChatRoomList();
}

void CUIChatWindow::AddChatText(BYTE byIndex, const wchar_t* pszText, int iType, int iColor)
{
    (void)iColor;
    m_View->AddLine(byIndex, pszText, iType);
}

int CUIChatWindow::GetUserCount()
{
    return static_cast<int>(m_Participants.size());
}

int CUIChatWindow::GetShowType()
{
    return m_View->InviteShown() ? 2 : 1;
}

void CUIChatWindow::UpdateInvitePalList()
{
    m_View->RefreshInviteList();
}

const wchar_t* CUIChatWindow::GetCurrentInvitePal()
{
    return m_View->SelectedInvite();
}

const wchar_t* CUIChatWindow::GetChatFriend(int* piResult)
{
    if (piResult)
        *piResult = 0;
    if (m_Participants.size() > 2)
    {
        if (piResult)
            *piResult = 2;
        return nullptr;
    }
    for (const auto& pal : m_Participants)
    {
        if (pal.name != Hero->ID)
        {
            if (piResult)
                *piResult = 1;
            return pal.name.c_str();
        }
    }
    return nullptr;
}

void CUIChatWindow::InviteSelected(const std::wstring& name)
{
    if (m_Locked || name.empty() || m_Participants.size() <= 1)
        return;
    if (m_Participants.size() >= MaximumRoomParticipants)
    {
        AddChatText(SystemSpeaker, I18N::Game::YouHaveReachedTheMaximumNumberOfFriendsYouCanList, 1, 0);
        return;
    }
    SocketClient->ToGameServer()->SendChatRoomInvitationRequest(MU_C16(name.c_str()), GetRoomNumber(), GetUIID());
}

void CUIChatWindow::SubmitLine(const std::wstring& line)
{
    // The chat server should not receive the same line twice in succession.
    if (line != m_LastSent)
    {
        m_LastSent = line;
        if (!m_Locked && !line.empty())
        {
            if (auto* connection = GetCurrentSocket())
                connection->ToChatServer()->SendChatMessageExt(0, line.c_str());
        }
    }
    if (m_Participants.size() < 2)
        Lock(TRUE);
}

// Native prefixed the title while the room had nobody to talk to, and locked the line.
void CUIChatWindow::Lock(BOOL bFlag)
{
    m_Locked = bFlag != FALSE;
    m_View->SetLocked(bFlag != FALSE);
    const size_t offlineLength = wcslen(I18N::Game::Offline);
    if (bFlag == TRUE)
    {
        if (wcsncmp(GetTitle(), I18N::Game::Offline, offlineLength) != 0)
        {
            wchar_t szTitle[128] = {0};
            wcsncpy(szTitle, I18N::Game::Offline, 127);
            wcsncat(szTitle, GetTitle(), 127 - wcslen(szTitle));
            SetTitle(szTitle);
        }
    }
    else if (wcsncmp(GetTitle(), I18N::Game::Offline, offlineLength) == 0)
    {
        wchar_t szTitle[128] = {0};
        wcsncpy(szTitle, GetTitle() + offlineLength, 127);
        SetTitle(szTitle);
    }
}

bool CUIChatWindow::SyncSemanticView(bool shown)
{
    return m_View->Sync(shown);
}

void CUIChatWindow::PullSemanticViewToFront()
{
    m_View->PullToFront();
}

void CUIChatWindow::Maximize()
{
    m_View->Maximize();
}

bool CUIChatWindow::SemanticFieldHasFocus() const
{
    return m_View->FieldHasFocus();
}
