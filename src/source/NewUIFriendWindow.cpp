// NewUIFriendWindow.cpp: implementation of the CNewUIFriendWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIFriendWindow.h"
#include "NewUIManager.h"
#include "NewUISystem.h"
#include "DSPlaySound.h"
#include "UIControls.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIFriendWindow::CNewUIFriendWindow() : m_pNewUIMng(NULL), m_pFriendWindowMgr(NULL)
{
}

SEASON3B::CNewUIFriendWindow::~CNewUIFriendWindow()
{
    Release();
}

bool SEASON3B::CNewUIFriendWindow::Create(CNewUIManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_FRIEND, this);

    m_pFriendWindowMgr = new CUIWindowMgr;
    m_pFriendWindowMgr->Reset();

    GetFriendList()->ClearFriendList();
    GetLetterList()->ClearLetterList();
    GetFriendMenu()->Reset();

    Show(false);

    return true;
}

void SEASON3B::CNewUIFriendWindow::Reset()
{
    m_pFriendWindowMgr->Reset();

    GetFriendList()->ClearFriendList();
    GetLetterList()->ClearLetterList();
    GetFriendMenu()->Reset();
}

void SEASON3B::CNewUIFriendWindow::Release()
{
    SAFE_DELETE(m_pFriendWindowMgr);
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool SEASON3B::CNewUIFriendWindow::Render()
{
    if (m_pFriendWindowMgr)
    {
        m_pFriendWindowMgr->Render();
        //GetFriendMenu()->Render();
        //GetFriendMenu()->RenderFriendButton();
    }
    return true;
}

bool SEASON3B::CNewUIFriendWindow::UpdateMouseEvent()
{
    if (m_pFriendWindowMgr)
    {
        m_pFriendWindowMgr->DoAction();

        CUIFriendWindow* pMainWnd = m_pFriendWindowMgr->GetFriendMainWindow();
        if (pMainWnd)
        {
            if (pMainWnd->GetPosition_y() < 0)
            {
                pMainWnd->SetPosition(pMainWnd->GetPosition_x(), 0);
            }
            if (pMainWnd->GetPosition_y() > 259)
            {
                pMainWnd->SetPosition(pMainWnd->GetPosition_x(), 259);
            }

            if (CheckMouseIn(pMainWnd->GetPosition_x(), pMainWnd->GetPosition_y(), pMainWnd->GetWidth(), pMainWnd->GetHeight()) == true)
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

bool SEASON3B::CNewUIFriendWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_FRIEND) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_FRIEND);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool SEASON3B::CNewUIFriendWindow::Update()
{
    return true;
}

float SEASON3B::CNewUIFriendWindow::GetLayerDepth()
{
    return 6.f;
}

CFriendList* SEASON3B::CNewUIFriendWindow::GetFriendList()
{
    static CFriendList s_FriendList;
    return &s_FriendList;
}
CLetterList* SEASON3B::CNewUIFriendWindow::GetLetterList()
{
    static CLetterList s_LetterList;
    return &s_LetterList;
}
CUIFriendMenu* SEASON3B::CNewUIFriendWindow::GetFriendMenu()
{
    static CUIFriendMenu s_FriendMenu;
    return &s_FriendMenu;
}