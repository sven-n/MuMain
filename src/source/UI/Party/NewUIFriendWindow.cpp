// NewUIFriendWindow.cpp: implementation of the CFriendWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UI/Party/NewUIFriendWindow.h"
#include "UI/Core/NewUIManager.h"
#include "UI/Core/NewUISystem.h"
#include "Audio/DSPlaySound.h"
#include "UI/Widgets/UIControls.h"
#include "UI/Scaling/UITransform.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mu::ui::window::CFriendWindow::CFriendWindow() : m_pNewUIMng(NULL), m_pFriendWindowMgr(NULL) {}

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
    m_pFriendWindowMgr->Reset();

    GetFriendList()->ClearFriendList();
    GetLetterList()->ClearLetterList();
    GetFriendMenu()->Reset();
}

void mu::ui::window::CFriendWindow::Release()
{
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
        // GetFriendMenu()->Render();
        // GetFriendMenu()->RenderFriendButton();
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
            const int maxY = std::max(static_cast<int>(UI::Scaling::FloatingWorkspaceContentHeight(
                                          WindowWidth, WindowHeight))
                                          - pMainWnd->GetHeight(),
                                      0);
            if (pMainWnd->GetPosition_y() < 0)
            {
                pMainWnd->SetPosition(pMainWnd->GetPosition_x(), 0);
            }
            if (pMainWnd->GetPosition_y() > maxY)
            {
                pMainWnd->SetPosition(pMainWnd->GetPosition_x(), maxY);
            }

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
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_FRIEND) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_FRIEND);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool mu::ui::window::CFriendWindow::Update()
{
    return true;
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
