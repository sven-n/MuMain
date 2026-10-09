#include "stdafx.h"
#include "UI/Social/FriendWindow.h"
#include "UI/Social/FriendShell.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Core/WindowSystem.h"

CUIFriendWindow::CUIFriendWindow() : m_Shell(std::make_unique<UI::Social::FriendShell>(*this)) {}
CUIFriendWindow::~CUIFriendWindow() = default;

void CUIFriendWindow::Init(const wchar_t* title, DWORD parent)
{
    SetTitle(title);
    SetParentUIID(parent);
    m_Shell->Build();
}

void CUIFriendWindow::Refresh()
{
    RefreshPalList();
    RefreshLetterList();
}

bool CUIFriendWindow::PointerOver() const
{
    return m_Shell && m_Shell->PointerOver();
}

BOOL CUIFriendWindow::DoAction(BOOL messageOnly)
{
    while (!m_MessageList.empty())
    {
        GetUIMessage();
        HandleMessage();
    }
    if (!messageOnly)
    {
        m_Shell->ProcessActions();
        if (g_dwMouseUseUIID == 0 && (g_dwActiveUIID == 0 || g_dwActiveUIID == GetUIID()) &&
            m_Shell->PointerOver())
            g_dwMouseUseUIID = GetUIID();
    }
    return FALSE;
}

BOOL CUIFriendWindow::HandleMessage()
{
    if (m_WorkMessage.m_iMessage == UI_MESSAGE_TXTRETURN)
    {
        const auto name = TakeReturnText();
        if (!name.empty())
            SocketClient->ToGameServer()->SendFriendAddRequest(MU_C16(name.c_str()));
    }
    else if (m_WorkMessage.m_iMessage == UI_MESSAGE_SELECTED)
        m_Shell->RequestPaneFocus();
    return TRUE;
}

void CUIFriendWindow::Reset() { m_Shell->SetTab(0); }
void CUIFriendWindow::Close()
{
    if (GetState() == UISTATE_NORMAL)
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
}
void CUIFriendWindow::Maximize() { m_Shell->Maximize(); }
void CUIFriendWindow::RefreshPalList() { m_Shell->RefreshFriends(); }
void CUIFriendWindow::RefreshLetterList() { m_Shell->RefreshLetters(); }
void CUIFriendWindow::AddWindow(DWORD id, const wchar_t* title) { m_Shell->AddWindow(id, title); }
void CUIFriendWindow::RemoveWindow(DWORD id) { m_Shell->RemoveWindow(id); }
void CUIFriendWindow::ResetWindow() { m_Shell->ResetWindows(); }
DWORD CUIFriendWindow::GetCurrentSelectedWindow() { return m_Shell->SelectedWindow(); }
LETTERLIST_TEXT* CUIFriendWindow::GetCurrentSelectedLetter() { return g_pLetterList->GetLetter(m_Shell->SelectedLetter()); }
void CUIFriendWindow::PrevNextCursorMove(int line) { m_Shell->SelectLetterLine(line); }
void CUIFriendWindow::SetTabIndex(int tab) { m_Shell->SetTab(tab); }
int CUIFriendWindow::GetTabIndex() { return m_Shell->GetTab(); }
bool CUIFriendWindow::SyncSemanticView(bool shown)
{
    return m_Shell->Sync(shown);
}
void CUIFriendWindow::PullSemanticViewToFront() { m_Shell->PullToFront(); }

void CUIFriendWindow::RestoreSemanticLayout(int x, int y, int width, int height)
{
    m_Shell->RestoreLayout(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width),
                           static_cast<float>(height), true);
}

// The manager keeps the maximize state alongside the geometry and hands both back when it
// recreates this window; without this the restored window came back un-maximized.
void CUIFriendWindow::RestoreSemanticMaximized()
{
    BOOL maximized = FALSE;
    int backPosY = 0, backHeight = 0;
    GetBackPosition(&maximized, &backPosY, &backHeight);
    m_Shell->RestoreMaximized(maximized != FALSE, static_cast<float>(backPosY), static_cast<float>(backHeight));
}
