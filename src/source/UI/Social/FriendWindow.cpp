
#include "stdafx.h"

#include "UI/Social/FriendWindow.h"
#include "UI/Core/WindowManager.h"
#include "UI/Core/WindowSystem.h"
#include "Audio/DSPlaySound.h"
#include "UI/Social/SocialWindowBase.h"
#include "UI/Scaling/UITransform.h"
#include "Render/RmlUi/RmlUiRuntime.h"

using mu::ui::window::CheckMouseIn;   // WindowCommon.h

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mu::ui::window::CFriendWindow::CFriendWindow() : m_pNewUIMng(NULL), m_pFriendWindowMgr(NULL)
{
    // Every coordinate this family publishes is in floating-workspace units, not the centred
    // panel space a CObject defaults to -- CUIWindowMgr clamps against FloatingWorkspaceBounds.
    SetLayoutMode(UI::Scaling::LayoutMode::FloatingWorkspace);
}

mu::ui::window::CFriendWindow::~CFriendWindow()
{
    Release();
}

bool mu::ui::window::CFriendWindow::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_FRIEND, this);

    m_pFriendWindowMgr = new CUIWindowMgr;
    m_pFriendWindowMgr->Reset();

    GetFriendList()->ClearFriendList();
    GetLetterList()->ClearLetterList();
    GetFriendMenu()->Reset();

    Show(false);

    return true;
}

void mu::ui::window::CFriendWindow::Reset()
{
    m_Dialogs.Reset();
    m_pFriendWindowMgr->Reset();

    GetFriendList()->ClearFriendList();
    GetLetterList()->ClearLetterList();
    GetFriendMenu()->Reset();
}

void mu::ui::window::CFriendWindow::Release()
{
    m_Dialogs.Reset();
    SAFE_DELETE(m_pFriendWindowMgr);
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool mu::ui::window::CFriendWindow::Render()
{
    if (m_pFriendWindowMgr)
    {
        m_pFriendWindowMgr->Render();
    }
    return true;
}

bool mu::ui::window::CFriendWindow::UpdateMouseEvent()
{
    if (m_pFriendWindowMgr)
    {
        m_pFriendWindowMgr->DoAction();

        CUIFriendWindow* pMainWnd = m_pFriendWindowMgr->GetFriendMainWindow();
        if (pMainWnd)
        {
            if (CheckMouseIn(pMainWnd->GetPosition_x(), pMainWnd->GetPosition_y(), pMainWnd->GetWidth(),
                             pMainWnd->GetHeight()) == true)
            {
                return false;
            }
            if (g_dwActiveUIID != 0 || g_dwMouseUseUIID != 0)
            {
                return false;
            }
        }
    }

    return true;
}

bool mu::ui::window::CFriendWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_FRIEND) == false)
        return true;
    if (mu::ui::window::IsPress(VK_ESCAPE) == false)
        return true;

    // Escape closes the window being typed in, and the whole family otherwise. CChatInputBox sets
    // the same precedent for the main chat line: the window owning the focused field takes the key
    // rather than letting it fall through to something larger.
    if (m_pFriendWindowMgr != nullptr)
    {
        if (auto* typing = m_pFriendWindowMgr->GetFieldFocusWindow())
        {
            m_pFriendWindowMgr->SendUIMessage(UI_MESSAGE_HIDE, typing->GetUIID(), 0);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_FRIEND);
    PlayBuffer(SOUND_CLICK01);
    return false;
}

bool mu::ui::window::CFriendWindow::Update()
{
    // The windows' RmlUi documents (FriendWindowViews.h), also while the family is hidden.
    if (m_pFriendWindowMgr)
        m_pFriendWindowMgr->SyncRmlViews(IsVisible());
    return true;
}

// The family's windows are separate documents; whichever of them has the typing field, Escape
// reaches this window.
bool mu::ui::window::CFriendWindow::TakesTypingFrom(const Rml::ElementDocument*) const
{
    return m_pFriendWindowMgr != nullptr && m_pFriendWindowMgr->GetFieldFocusWindow() != nullptr;
}

float mu::ui::window::CFriendWindow::GetLayerDepth()
{
    return 6.f;
}

CFriendList* mu::ui::window::CFriendWindow::GetFriendList()
{
    static CFriendList s_FriendList;
    return &s_FriendList;
}
CLetterList* mu::ui::window::CFriendWindow::GetLetterList()
{
    static CLetterList s_LetterList;
    return &s_LetterList;
}
CUIFriendMenu* mu::ui::window::CFriendWindow::GetFriendMenu()
{
    static CUIFriendMenu s_FriendMenu;
    return &s_FriendMenu;
}
