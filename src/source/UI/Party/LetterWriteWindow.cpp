#include "stdafx.h"
#include "UI/Party/UIWindows.h"
#include "UI/Party/LetterWrite.h"
#include "UI/Party/LetterWriteModel.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Core/WindowSystem.h"
#include "I18N/All.h"

// What is left of CUILetterWriteWindow once the draft is two documents of its own: the send, its
// validation, the player's portrait and the close check. UI::Party::LetterWriteView owns the rest.

namespace
{
// Native folded the field's CRLF pairs back into single newlines before sending, and inserted a
// space where a blank line would otherwise collapse.
std::wstring NormaliseBody(const std::wstring& typed)
{
    std::wstring out;
    out.reserve(typed.size());
    for (size_t i = 0; i < typed.size(); ++i)
    {
        if (typed[i] == L'\r')
        {
            out.push_back(L'\n');
            if (i + 3 < typed.size() && typed[i + 1] == L'\n' && typed[i + 2] == L'\r' && typed[i + 3] == L'\n')
                out.push_back(L' ');
            ++i;
        }
        else
            out.push_back(typed[i]);
    }
    if (out.size() > MAX_LETTERTEXT_LENGTH)
        out.resize(MAX_LETTERTEXT_LENGTH);
    return out;
}
} // namespace

CUILetterWriteWindow::CUILetterWriteWindow()
    : m_bIsSend(FALSE), m_View(std::make_unique<UI::Party::LetterWriteView>(*this))
{
}

CUILetterWriteWindow::~CUILetterWriteWindow() = default;

void CUILetterWriteWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    SetTitle(pszTitle);
    SetParentUIID(dwParentID);
    SetPosition(50, 50);
    // Native added the portrait's own 120 to its 250x216 before anything was laid out.
    SetSize(250 + 120, 216);
    SetLimitSize(250 + 120, 150);
    SetOption(UIWINDOWSTYLE_TITLEBAR | UIWINDOWSTYLE_FRAME | UIWINDOWSTYLE_MOVEABLE | UIWINDOWSTYLE_MINBUTTON);
    m_Photo.Init(0);
    m_Photo.SetOption(UIPHOTOVIEWER_CANCONTROL);
    m_Photo.SetParentUIID(GetUIID());
    m_Photo.CopyPlayer();
    m_Photo.SetAutoupdatePlayer(TRUE);
    m_Photo.SetAnimation(AT_STAND1);
    m_Photo.SetAngle(90);
    m_View->Build();
}

void CUILetterWriteWindow::Refresh() {}

void CUILetterWriteWindow::SetMailtoText(const wchar_t* pszText)
{
    m_View->SetMailto(pszText);
}

void CUILetterWriteWindow::SetMainTitleText(const wchar_t* pszText)
{
    m_View->SetSubject(pszText);
}

void CUILetterWriteWindow::SetMailContextText(const wchar_t* pszText)
{
    m_View->SetBody(pszText);
}

void CUILetterWriteWindow::SetSendState(BOOL bFlag)
{
    m_bIsSend = bFlag;
    m_View->SetSending(bFlag != FALSE);
}

BOOL CUILetterWriteWindow::DoAction(BOOL messageOnly)
{
    while (!m_MessageList.empty())
    {
        GetUIMessage();
        HandleMessage();
    }
    if (!messageOnly)
    {
        m_View->ProcessActions();
        m_Photo.SetShowType(1);
        m_Photo.DoAction(messageOnly);
        if (g_dwMouseUseUIID == 0 && (g_dwActiveUIID == 0 || g_dwActiveUIID == GetUIID()) &&
            mu::ui::window::CheckMouseIn(GetPosition_x(), GetPosition_y(), GetWidth(), GetHeight()))
            g_dwMouseUseUIID = GetUIID();
    }
    return FALSE;
}

BOOL CUILetterWriteWindow::HandleMessage()
{
    if (m_WorkMessage.m_iMessage == UI_MESSAGE_SELECTED)
        m_View->RestoreFocus();
    else if (m_WorkMessage.m_iMessage == UI_MESSAGE_YNRETURN && m_WorkMessage.m_iParam2 == 1)
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
    return TRUE;
}

// Each field is checked in turn and the caret put back in the first empty one, as native did.
void CUILetterWriteWindow::Send()
{
    if (m_bIsSend != FALSE)
        return;
    const std::wstring mailto = m_View->Mailto();
    if (mailto.empty())
    {
        g_pWindowMgr->Dialogs().Notice(I18N::Game::EnterTheNameOfTheReceiver);
        m_View->FocusField(0);
        return;
    }
    const std::wstring subject = m_View->Subject();
    if (subject.empty())
    {
        g_pWindowMgr->Dialogs().Notice(I18N::Game::EnterTheTitle);
        m_View->FocusField(1);
        return;
    }
    const std::wstring typed = m_View->Body();
    if (typed.empty())
    {
        g_pWindowMgr->Dialogs().Notice(I18N::Game::EnterYourMessage);
        m_View->FocusField(2);
        return;
    }

    const std::wstring body = NormaliseBody(typed);
    SetSendState(TRUE);
    // The portrait's pose travels with the letter: angle and zoom packed into one byte, the
    // animation into the next.
    const int angle = static_cast<int>(m_Photo.GetCurrentAngle()) / 6;
    const int zoom = static_cast<int>((m_Photo.GetCurrentZoom() * 100.0f - 80 + 5) / 10);
    const BYTE data1 = static_cast<BYTE>(((zoom << 6) & 0xC0) | (angle & 0x3F));
    const BYTE data2 = static_cast<BYTE>(m_Photo.GetCurrentAction() - AT_ATTACK1);
    const WORD length = static_cast<WORD>(std::min<size_t>(MAX_LETTERTEXT_LENGTH, body.size()));
    SocketClient->ToGameServer()->SendLetterSendRequest(GetUIID(), MU_C16(mailto.c_str()),
                                                       MU_C16(subject.c_str()), data1, data2, length,
                                                       MU_C16(body.c_str()));
}

void CUILetterWriteWindow::RequestClose()
{
    if (CloseCheck() == TRUE)
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
}

// A draft with nothing in its title or body closes without asking.
BOOL CUILetterWriteWindow::CloseCheck()
{
    if (m_View->Subject().empty() && m_View->Body().empty())
        return TRUE;
    g_pWindowMgr->Dialogs().Confirm(I18N::Game::DoYouWishToQuitWritingThisLetter, GetUIID());
    return FALSE;
}

bool CUILetterWriteWindow::SyncSemanticView(bool shown)
{
    return m_View->Sync(shown);
}

void CUILetterWriteWindow::PullSemanticViewToFront()
{
    m_View->PullToFront();
}

void CUILetterWriteWindow::Maximize()
{
    m_View->Maximize();
}

bool CUILetterWriteWindow::SemanticFieldHasFocus() const
{
    return m_View->AnyFieldHasFocus();
}

// After RmlUi's main context, not before it -- see CUIWindowMgr::RenderOverlay3D().
void CUILetterWriteWindow::RenderAboveRmlUi()
{
    m_Photo.Render();
}
