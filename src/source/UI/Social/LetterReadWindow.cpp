#include "stdafx.h"
#include "UI/Social/UIWindows.h"
#include "UI/Social/LetterRead.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Core/WindowSystem.h"
#include "Render/Text/TextWrap.h"
#include "I18N/All.h"

extern void ReceiveLetterText(std::span<const BYTE> packet, bool cached);
extern int g_iLetterReadNextPos_x, g_iLetterReadNextPos_y;

// Owns the letter, sender's portrait, and actions. LetterReadView owns its RmlUi document.

CUILetterReadWindow::CUILetterReadWindow() : m_View(std::make_unique<UI::Social::LetterReadView>(*this))
{
}

CUILetterReadWindow::~CUILetterReadWindow()
{
    g_pWindowMgr->CloseLetterRead(m_LetterHead.m_dwLetterID);
}

void CUILetterReadWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    SetTitle(pszTitle);
    SetParentUIID(dwParentID);
    SetPosition(50, 50);
    // Native added the portrait's own 120 to its 250x182 before anything was laid out.
    SetSize(250 + 120, 182);
    SetLimitSize(250 + 120, 182);
    SetOption(UIWINDOWSTYLE_TITLEBAR | UIWINDOWSTYLE_FRAME | UIWINDOWSTYLE_MOVEABLE | UIWINDOWSTYLE_MINBUTTON);
    m_Photo.Init(0);
    m_Photo.SetOption(UIPHOTOVIEWER_CANCONTROL);
    m_Photo.SetParentUIID(GetUIID());
    m_View->Build();
}

void CUILetterReadWindow::Refresh() {}

void CUILetterReadWindow::SetLetter(LETTERLIST_TEXT* pLetterHead, const wchar_t* pLetterText)
{
    memcpy(&m_LetterHead, pLetterHead, sizeof(LETTERLIST_TEXT));
    m_View->SetLetter(m_LetterHead.m_szID, m_LetterHead.m_szDate, m_LetterHead.m_szTime, pLetterText);
}

BOOL CUILetterReadWindow::DoAction(BOOL messageOnly)
{
    while (!m_MessageList.empty())
    {
        GetUIMessage();
        HandleMessage();
    }
    if (!messageOnly)
    {
        m_View->ProcessActions();
        m_Photo.SetShowType(2);
        m_Photo.DoAction(messageOnly);
        if (g_dwMouseUseUIID == 0 && (g_dwActiveUIID == 0 || g_dwActiveUIID == GetUIID()) &&
            mu::ui::window::CheckMouseIn(GetPosition_x(), GetPosition_y(), GetWidth(), GetHeight()))
            g_dwMouseUseUIID = GetUIID();
    }
    return FALSE;
}

BOOL CUILetterReadWindow::HandleMessage()
{
    if (m_WorkMessage.m_iMessage == UI_MESSAGE_YNRETURN && m_WorkMessage.m_iParam2 == 1)
    {
        SocketClient->ToGameServer()->SendLetterDeleteRequest(m_LetterHead.m_dwLetterID);
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
    }
    return TRUE;
}

void CUILetterReadWindow::Reply()
{
    wchar_t temp[MAX_TEXT_LENGTH + 1] = {0};
    mu_swprintf(temp, I18N::Game::WriteLetterCostDZen, g_cdwLetterCost);
    const DWORD id = g_pWindowMgr->AddWindow(UIWNDTYPE_WRITELETTER, 100, 100, temp);
    auto* window = dynamic_cast<CUILetterWriteWindow*>(g_pWindowMgr->GetWindow(id));
    if (!window)
        return;
    window->SetMailtoText(m_LetterHead.m_szID);
    wchar_t subject[MAX_TEXT_LENGTH + 1] = {0};
    mu_swprintf(subject, I18N::Game::ReS, m_LetterHead.m_szText);
    constexpr int SubjectLength = 32;
    wchar_t trimmed[SubjectLength + 1] = {0};
    CutText4(subject, trimmed, nullptr, SubjectLength);
    window->SetMainTitleText(trimmed);
}

void CUILetterReadWindow::AskDelete()
{
    g_pWindowMgr->Dialogs().Confirm(I18N::Game::AreYouSureYouWantToDeleteTheLetter, GetUIID());
}

// Previous and Next were two copies of the same twenty lines; the only difference is which
// neighbour the list is asked for. With no neighbour, native still moved the shell's cursor onto
// the letter already open and left this window alone.
void CUILetterReadWindow::StepLetter(int direction)
{
    const DWORD current = m_LetterHead.m_dwLetterID;
    const DWORD target = direction < 0 ? g_pLetterList->GetPrevLetterID(current)
                                       : g_pLetterList->GetNextLetterID(current);
    auto* shell = g_pWindowMgr->GetFriendMainWindow();
    if (target == 0)
    {
        if (shell)
            shell->PrevNextCursorMove(g_pLetterList->GetLineNum(current));
        return;
    }
    if (shell)
        shell->PrevNextCursorMove(g_pLetterList->GetLineNum(target));
    g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
    g_pWindowMgr->CloseLetterRead(current);
    // The replacement opens where this one stood.
    g_iLetterReadNextPos_x = GetPosition_x();
    g_iLetterReadNextPos_y = GetPosition_y();
    if (g_pWindowMgr->LetterReadCheck(target))
    {
        if (const DWORD open = g_pWindowMgr->GetLetterReadWindow(target))
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, open, 0);
        return;
    }
    if (auto* cached = g_pLetterList->GetLetterText(target))
        ReceiveLetterText(std::span(reinterpret_cast<const BYTE*>(cached), sizeof(FS_LETTER_TEXT)), true);
    else
        SocketClient->ToGameServer()->SendLetterReadRequest(target);
}

bool CUILetterReadWindow::SyncSemanticView(bool shown)
{
    return m_View->Sync(shown);
}

void CUILetterReadWindow::PullSemanticViewToFront()
{
    m_View->PullToFront();
}

void CUILetterReadWindow::Maximize()
{
    m_View->Maximize();
}

// After RmlUi's main context, not before it -- see CUIWindowMgr::RenderOverlay3D().
void CUILetterReadWindow::RenderAboveRmlUi()
{
    m_Photo.Render();
}
