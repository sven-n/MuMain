#include "stdafx.h"
#include "UI/Social/ChatRoomWindow.h"
#include "UI/Social/ChatRoom.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Core/WindowSystem.h"
#include "I18N/All.h"

// What is left of CUIChatWindow once the room is an RmlUi document of its own: the chat-server
// connection, the room's identity, and forwarding. Presentation, participants, the draft line and
// the invitation panel all live in UI::Social::ChatRoomView.

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
    SetLimitSize(250, 150);
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
            mu::ui::window::CheckMouseIn(GetPosition_x(), GetPosition_y(), GetWidth(), GetHeight()))
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
    const int count = m_View->AddPal(pszID, Number);
    RefreshRoomTitle();
    if (count >= 2)
        Lock(FALSE);
    return count;
}

void CUIChatWindow::RemoveChatPal(const wchar_t* pszID)
{
    if (m_View->PalCount() > 2)
    {
        m_View->RemovePal(pszID);
        RefreshRoomTitle();
    }
    else
        m_View->RemovePal(pszID);
}

// CUIChatPalListBox::MakeTitleText fed the window title from the room's members.
void CUIChatWindow::RefreshRoomTitle()
{
    wchar_t szTitle[128] = {0};
    wcsncpy(szTitle, I18N::Game::Talking, 127);
    m_View->MakeTitleText(szTitle, 128);
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
    return m_View->PalCount();
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
    return m_View->ChatFriend(piResult);
}

// Native prefixed the title while the room had nobody to talk to, and locked the line.
void CUIChatWindow::Lock(BOOL bFlag)
{
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
