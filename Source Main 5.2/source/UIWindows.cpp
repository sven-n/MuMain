///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzAI.h"
#include "GOBoid.h"
#include "UIManager.h"
#include "CSParts.h"
#include "SummonSystem.h"
#include "MapManager.h"
#include "CharacterManager.h"
#include "DSPlaySound.h"
#include "NewUISystem.h"



extern int	 g_iChatInputType;
extern DWORD g_dwActiveUIID;
extern DWORD g_dwMouseUseUIID;
extern CUITextInputBox* g_pSingleTextInputBox;
extern DWORD g_dwTopWindow;
extern DWORD g_dwKeyFocusUIID;
extern void ReceiveLetterText(std::span<const BYTE> ReceiveBuffer, bool isCached);

int g_iLetterReadNextPos_x, g_iLetterReadNextPos_y;

CUIWindowMgr::CUIWindowMgr()
{
    m_bWindowsEnable = FALSE;
    memset(&m_WorkMessage, 0, sizeof(UI_MESSAGE));
    m_bRenderFrame = TRUE;
    m_dwMainWindowUIID = 0;

    m_iMainWindowPos_x = 0;
    m_iMainWindowPos_y = 0;
    m_iMainWindowWidth = 0;
    m_iMainWindowHeight = 0;
    m_iMainWindowBackPos_y = 0;
    m_iMainWindowBackHeight = 0;
    m_bIsMainWindowMaximize = FALSE;
    m_bChatReject = FALSE;
    m_iLastFriendWindowTabIndex = 0;

    SetTimer(g_hWnd, CHATCONNECT_TIMER, 15 * 1000, 0);

    g_iLetterReadNextPos_x = UIWND_DEFAULT;
    g_iLetterReadNextPos_y = UIWND_DEFAULT;
}

CUIWindowMgr::~CUIWindowMgr()
{
    Reset();
}

void CUIWindowMgr::Reset()
{
    m_dwMainWindowUIID = 0;
    for (m_WindowMapIter = m_WindowMap.begin(); m_WindowMapIter != m_WindowMap.end(); ++m_WindowMapIter)
    {
        if (m_WindowMapIter->second != NULL)
        {
            delete m_WindowMapIter->second;
            m_WindowMapIter->second = NULL;
        }
    }
    m_WindowMap.clear();
    m_WindowArrangeList.clear();
    m_LetterReadMap.clear();

    HideAllWindowClear();
    m_ForceTopWindowList.clear();
    g_dwTopWindow = 0;
    m_iLastFriendWindowTabIndex = 0;
    m_bServerEnable = TRUE;
    m_iFriendMainWindowTitleNumber = 990;
    m_dwAddWindowUIID = 0;
    SetChatReject(FALSE);
    if (GetFriendMainWindow() != NULL)
    {
        GetFriendMainWindow()->Reset();
    }
    if (g_iChatInputType == 0)
    {
        // for OpenMU the following line is not required.
        // 2 probably means logging out.
        SocketClient->ToGameServer()->SendSetFriendOnlineState(2);
    }
}

DWORD CUIWindowMgr::AddWindow(int iWindowType, int iPos_x, int iPos_y, const wchar_t* pszTitle, DWORD dwParentID, int iOption)
{
    if (g_iChatInputType == 0/* || g_dwTopWindow != 0*/) return 0;
    CUIBaseWindow* pbw = NULL;

    switch (iWindowType)
    {
    case UIWNDTYPE_EMPTY:
        pbw = new CUIDefaultWindow;
        break;
    case UIWNDTYPE_CHAT:
    case UIWNDTYPE_CHAT_READY:
        pbw = new CUIChatWindow;
        if (m_dwMainWindowUIID != 0)
        {
            auto* pMainWnd = (CUIFriendWindow*)GetWindow(m_dwMainWindowUIID);
            if (pMainWnd != NULL)
                pMainWnd->AddWindow(pbw->GetUIID(), pszTitle);
        }
        break;
    case UIWNDTYPE_FRIENDMAIN:
        if (m_dwMainWindowUIID == 0)
        {
            pbw = new CUIFriendWindow;
            m_dwMainWindowUIID = pbw->GetUIID();
            if (g_pFriendMenu->IsNewMailAlert() == TRUE)
            {
                ((CUIFriendWindow*)pbw)->SetTabIndex(1);
            }
            else
            {
                ((CUIFriendWindow*)pbw)->SetTabIndex(m_iLastFriendWindowTabIndex);
            }
            g_pFriendMenu->SetNewMailAlert(FALSE);
            if (IsServerEnable() == FALSE)
            {
                SocketClient->ToGameServer()->SendFriendListRequest();
            }
        }
        else return 0;
        break;
    case UIWNDTYPE_TEXTINPUT:
        if (g_dwTopWindow != 0) return 0;
        pbw = new CUITextInputWindow;
        {
            auto* pMainWnd = (CUIFriendWindow*)GetWindow(m_dwMainWindowUIID);
            if (pMainWnd != NULL)
                pMainWnd->AddWindow(pbw->GetUIID(), pszTitle);
        }
        g_dwTopWindow = pbw->GetUIID();
        break;
    case UIWNDTYPE_QUESTION:
    case UIWNDTYPE_QUESTION_FORCE:
        pbw = new CUIQuestionWindow(0);
        {
            auto* pMainWnd = (CUIFriendWindow*)GetWindow(m_dwMainWindowUIID);

            if (pMainWnd != NULL)
                pMainWnd->AddWindow(pbw->GetUIID(), GlobalText[991]);
        }
        if (iWindowType == UIWNDTYPE_QUESTION) g_dwTopWindow = pbw->GetUIID();
        else AddForceTopWindowList(pbw->GetUIID());
        break;
    case UIWNDTYPE_OK:
        if (g_dwTopWindow != 0) return 0;
    case UIWNDTYPE_OK_FORCE:
        pbw = new CUIQuestionWindow(1);
        {
            auto* pMainWnd = (CUIFriendWindow*)GetWindow(m_dwMainWindowUIID);
            if (pMainWnd != NULL)
                pMainWnd->AddWindow(pbw->GetUIID(), GlobalText[228]);
        }
        if (iWindowType == UIWNDTYPE_OK) g_dwTopWindow = pbw->GetUIID();
        else AddForceTopWindowList(pbw->GetUIID());
        break;
    case UIWNDTYPE_READLETTER:
        pbw = new CUILetterReadWindow;
        if (m_dwMainWindowUIID != 0)
        {
            auto* pMainWnd = (CUIFriendWindow*)GetWindow(m_dwMainWindowUIID);
            if (pMainWnd != NULL)
                pMainWnd->AddWindow(pbw->GetUIID(), pszTitle);
        }
        break;
    case UIWNDTYPE_WRITELETTER:
        if (g_dwTopWindow != 0) return 0;
        pbw = new CUILetterWriteWindow;
        if (m_dwMainWindowUIID != 0)
        {
            auto* pMainWnd = (CUIFriendWindow*)GetWindow(m_dwMainWindowUIID);
            if (pMainWnd != NULL)
                pMainWnd->AddWindow(pbw->GetUIID(), pszTitle);
        }
        break;
    default:
        return 0;
        break;
    };

    if (iPos_x == UIWND_DEFAULT) iPos_x = 0;
    if (iPos_y == UIWND_DEFAULT) iPos_y = 332;
    if (!pbw) return 0;

    pbw->Init(pszTitle, dwParentID);

    if (!(iOption & UIADDWND_FORCEPOSITION))
    {
        for (m_WindowMapIter = m_WindowMap.begin(); m_WindowMapIter != m_WindowMap.end(); ++m_WindowMapIter)
        {
            if (m_WindowMapIter->second->GetPosition_x() == iPos_x && m_WindowMapIter->second->GetPosition_y() == iPos_y)
            {
                if (iPos_x + pbw->GetWidth() + 20 <= 640) iPos_x += 20;
                if (iPos_y + pbw->GetHeight() + 20 <= 480) iPos_y += 20;
                if (iPos_x + pbw->GetWidth() + 20 > 640 && iPos_y + pbw->GetHeight() + 20 > 480)
                {
                    if (iPos_y % 10 == 9)
                    {
                        delete pbw;
                        return 0;
                    }
                    iPos_x = iPos_y = iPos_y % 10 + 1;
                }
                m_WindowMapIter = m_WindowMap.begin();
            }
        }
    }
    pbw->SetPosition(iPos_x, iPos_y);

    DWORD dwUIID = pbw->GetUIID();

    m_WindowMap.insert(std::pair<DWORD, CUIBaseWindow*>(dwUIID, pbw));
    m_WindowArrangeList.push_back(dwUIID);
    if (iWindowType == UIWNDTYPE_CHAT || iWindowType == UIWNDTYPE_CHAT_READY) g_pFriendMenu->AddWindow(dwUIID, pbw);

    pbw->Refresh();
    if (iWindowType == UIWNDTYPE_CHAT)
    {
        ((CUIChatWindow*)pbw)->FocusReset();
    }

    return dwUIID;
}

void CUIWindowMgr::RemoveWindow(DWORD dwUIID)
{
    if (m_dwMainWindowUIID == dwUIID)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_FRIEND);

        if (g_dwTopWindow != 0)
        {
            return;
        }

        CUIBaseWindow* pWindow = GetWindow(m_dwMainWindowUIID);
        if (pWindow != NULL)
        {
            m_iMainWindowPos_x = pWindow->GetPosition_x();
            m_iMainWindowPos_y = pWindow->GetPosition_y();
            m_iMainWindowWidth = pWindow->GetWidth();
            m_iMainWindowHeight = pWindow->GetHeight();
            pWindow->GetBackPosition(&m_bIsMainWindowMaximize, &m_iMainWindowBackPos_y, &m_iMainWindowBackHeight);
            m_iLastFriendWindowTabIndex = ((CUIFriendWindow*)pWindow)->GetTabIndex();
        }
        m_dwMainWindowUIID = 0;
    }

    if (GetAddFriendWindow() == dwUIID)
    {
        SetAddFriendWindow(0);
    }

    m_WindowMapIter = m_WindowMap.find(dwUIID);
    if (m_WindowMapIter == m_WindowMap.end())
    {
        return;
    }

    RemoveForceTopWindowList(dwUIID);

    if (m_WindowMapIter->second != NULL)
    {
        delete m_WindowMapIter->second;
        m_WindowMapIter->second = NULL;
    }
    m_WindowMap.erase(m_WindowMapIter);
    m_WindowArrangeList.remove(dwUIID);
    if (m_WindowArrangeList.empty())
        SetWindowsEnable(FALSE);

    if (m_dwMainWindowUIID != 0)
    {
        auto* pMainWnd = (CUIFriendWindow*)GetWindow(m_dwMainWindowUIID);
        if (pMainWnd != NULL)
            pMainWnd->RemoveWindow(dwUIID);
    }
    g_pFriendMenu->RemoveWindow(dwUIID);

    if (g_dwTopWindow == dwUIID)
    {
        g_dwTopWindow = 0;
    }
}

void RenderWindowVLine(float pos_x, float pos_y, float height);
void RenderColor(float x, float y, float Width, float Height, float Alpha, int Flag);

void CUIWindowMgr::Render()
{
    for (m_WindowArrangeListIter = m_WindowArrangeList.begin(); m_WindowArrangeListIter != m_WindowArrangeList.end(); ++m_WindowArrangeListIter)
    {
        m_WindowMapIter = m_WindowMap.find(*m_WindowArrangeListIter);
        if (m_WindowMapIter != m_WindowMap.end())
        {
            if (m_WindowMapIter->second->GetState() != UISTATE_HIDE &&
                m_WindowMapIter->second->GetState() != UISTATE_READY)
                m_WindowMapIter->second->Render();
        }
    }
    m_bRenderFrame = TRUE;
}

void CUIWindowMgr::DoAction()
{
    if (g_dwTopWindow != 0)
    {
        if (g_pWindowMgr->GetWindow(g_dwTopWindow) == NULL)
        {
            g_dwTopWindow = 0;
        }
    }

    if (GetFocus() == g_hWnd && PressKey(VK_F5))
    {
        g_pFriendMenu->ShowMenu(TRUE);
    }

    if (PressKey(VK_F6))
    {
        if ((m_bCurrentHideWindowState == FALSE || m_HideWindowList.empty() == FALSE))
        {
            g_pWindowMgr->HideAllWindow(TRUE, TRUE);
        }
    }

    if (m_dwMainWindowUIID > 0 && GetFriendMainWindow()->GetState() == UISTATE_HIDE)
    {
        CloseMainWnd();
    }

    for (m_WindowReverseArrangeListIter = m_WindowArrangeList.rbegin(); m_WindowReverseArrangeListIter != m_WindowArrangeList.rend(); ++m_WindowReverseArrangeListIter)
    {
        m_WindowMapIter = m_WindowMap.find(*m_WindowReverseArrangeListIter);
        if (m_WindowMapIter != m_WindowMap.end())
        {
            if (m_WindowMapIter->second->GetState() != UISTATE_HIDE &&
                m_WindowMapIter->second->GetState() != UISTATE_READY)
                m_WindowMapIter->second->DoAction();
        }
    }

    while (m_MessageList.empty() == FALSE)
    {
        GetUIMessage();
        HandleMessage();
    }

    m_bRenderFrame = FALSE;
}

void CUIWindowMgr::ShowHideWindow(DWORD dwUIID, BOOL bShowWindow)
{
    CUIBaseWindow* pWindow = GetWindow(dwUIID);
    if (pWindow != NULL)
    {
        if (bShowWindow == TRUE)
        {
            pWindow->SetState(UISTATE_NORMAL);
        }
        else pWindow->SetState(UISTATE_HIDE);
    }
}

void CUIWindowMgr::HideAllWindow(BOOL bHide, BOOL bMainClose)
{
    int iCount = 0;
    for (m_WindowMapIter = m_WindowMap.begin(); m_WindowMapIter != m_WindowMap.end(); ++m_WindowMapIter)
    {
        if ((bMainClose == TRUE || m_WindowMapIter->first != m_dwMainWindowUIID) && m_WindowMapIter->first != g_dwTopWindow)
        {
            if (bHide == TRUE)
            {
                if (m_WindowMapIter->second->GetState() == UISTATE_NORMAL)
                {
                    if (bMainClose == TRUE) m_HideWindowList.push_back(m_WindowMapIter->first);
                    m_WindowMapIter->second->SetState(UISTATE_HIDE);
                }
            }
            else
            {
                for (std::list<DWORD>::iterator iter = m_HideWindowList.begin(); iter != m_HideWindowList.end(); ++iter)
                {
                    if (m_WindowMapIter->first == *iter)
                    {
                        ++iCount;
                        m_WindowMapIter->second->SetState(UISTATE_NORMAL);
                    }
                }
            }
        }
    }
    if (bHide == FALSE)
    {
        int iHideSize = m_HideWindowList.size();
        if (iHideSize - iCount > 0)
        {
            OpenMainWnd(640 - 250, 432 - 170);
        }
        if (iCount > 0 && GetTopNotMainWindowUIID() > 0)
        {
            SendUIMessage(UI_MESSAGE_SELECT, GetTopNotMainWindowUIID(), 0);
        }
        m_HideWindowList.clear();
    }
    else
    {
        SetWindowsEnable(FALSE);
        SetFocus(g_hWnd);
    }
}

void CUIWindowMgr::HideAllWindowClear()
{
    m_bCurrentHideWindowState = FALSE;
    m_HideWindowList.clear();
}

DWORD CUIWindowMgr::GetTopNotMainWindowUIID()
{
    if (m_WindowArrangeList.empty() == TRUE) return 0;
    m_WindowReverseArrangeListIter = m_WindowArrangeList.rbegin();
    DWORD dwResult = *m_WindowReverseArrangeListIter;
    if (dwResult == m_dwMainWindowUIID)
    {
        if (m_WindowArrangeList.size() == 1) return 0;
        else dwResult = *(++m_WindowReverseArrangeListIter);
    }
    return dwResult;
}

void CUIWindowMgr::AddWindowFinder(CUIBaseWindow* pWindow)
{
    if (pWindow == NULL) return;
    DWORD dwUIID = pWindow->GetUIID();
    m_WindowFindMap.insert(std::pair<DWORD, CUIBaseWindow*>(dwUIID, pWindow));
}

void CUIWindowMgr::RemoveWindowFinder(DWORD dwUIID)
{
    m_WindowMapIter = m_WindowFindMap.find(dwUIID);
    m_WindowFindMap.erase(m_WindowMapIter);
}

CUIBaseWindow* CUIWindowMgr::GetWindow(DWORD dwUIID)
{
    m_WindowMapIter = m_WindowMap.find(dwUIID);
    if (m_WindowMapIter == m_WindowMap.end())
    {
        m_WindowMapIter = m_WindowFindMap.find(dwUIID);
        if (m_WindowMapIter == m_WindowFindMap.end())
        {
            return NULL;
        }
        else
        {
            return m_WindowMapIter->second;
        }
    }
    else
        return m_WindowMapIter->second;
}

BOOL CUIWindowMgr::IsWindow(DWORD dwUIID)
{
    m_WindowMapIter = m_WindowMap.find(dwUIID);
    if (m_WindowMapIter == m_WindowMap.end())
    {
        m_WindowMapIter = m_WindowFindMap.find(dwUIID);
        if (m_WindowMapIter == m_WindowFindMap.end())
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else
        return TRUE;
}

void CUIWindowMgr::SendUIMessageToWindow(DWORD dwUIID, int iMessage, int iParam1, int iParam2)
{
    CUIBaseWindow* pWindow = GetWindow(dwUIID);
    if (pWindow != NULL)
    {
        pWindow->SendUIMessage(iMessage, iParam1, iParam2);
    }
}

void CUIWindowMgr::HandleMessage()
{
    assert(m_WorkMessage.m_iParam1 != 0 && "Error Handle message");

    switch (m_WorkMessage.m_iMessage)
    {
    case UI_MESSAGE_SELECT:
        if (m_WorkMessage.m_iParam1 != 0 && GetWindow(m_WorkMessage.m_iParam1) != NULL)
        {
            if (GetWindow(m_WorkMessage.m_iParam1)->HaveTextBox() == FALSE)
            {
                if ((GetWindow(GetTopWindowUIID()) != NULL && GetWindow(GetTopWindowUIID())->HaveTextBox() == TRUE)
                    || g_pSingleTextInputBox->HaveFocus() == TRUE)
                    SaveIMEStatus();

                SetFocus(g_hWnd);
            }
            if (GetWindow(m_WorkMessage.m_iParam1)->GetState() == UISTATE_HIDE)
                ShowHideWindow(m_WorkMessage.m_iParam1, TRUE);

            m_WindowArrangeListIter = m_WindowArrangeList.end();
            --m_WindowArrangeListIter;

            if ((int)(*m_WindowArrangeListIter) != m_WorkMessage.m_iParam1 || GetFocus() == g_hWnd)
            {
                m_WindowArrangeList.remove(m_WorkMessage.m_iParam1);
                m_WindowArrangeList.push_back(m_WorkMessage.m_iParam1);
                SendUIMessageToWindow(m_WorkMessage.m_iParam1, UI_MESSAGE_SELECTED, 0, 0);
            }

            SetWindowsEnable(m_WorkMessage.m_iParam1);
            g_pFriendMenu->SetNewChatAlertOff(m_WorkMessage.m_iParam1);
            g_pFriendMenu->HideMenu();
        }
        break;
    case UI_MESSAGE_HIDE:
        if (m_WorkMessage.m_iParam1 != 0)
        {
            if (g_dwTopWindow != 0 && m_dwMainWindowUIID != 0 && m_WorkMessage.m_iParam1 == (int)m_dwMainWindowUIID) break;

            PlayBuffer(SOUND_CLICK01);
            ShowHideWindow(m_WorkMessage.m_iParam1, FALSE);
            m_WindowArrangeList.remove(m_WorkMessage.m_iParam1);
            m_WindowArrangeList.push_front(m_WorkMessage.m_iParam1);
            if (GetTopWindowUIID() != 0)
            {
                CUIBaseWindow* pWindow = GetWindow(GetTopWindowUIID());
                if (pWindow != NULL)
                {
                    if (pWindow->GetState() != UISTATE_HIDE && pWindow->GetState() != UISTATE_READY)
                    {
                        //GetWindow(GetTopWindowUIID())->SetState(UISTATE_NORMAL);
                        SendUIMessageToWindow(GetTopWindowUIID(), UI_MESSAGE_SELECTED, 0, 0);
                    }
                    else
                    {
                        g_pWindowMgr->SetWindowsEnable(FALSE);
                        SetFocus(g_hWnd);
                    }
                }
            }
            else
            {
                g_pWindowMgr->SetWindowsEnable(FALSE);
                SetFocus(g_hWnd);
            }
        }
        break;
    case UI_MESSAGE_MAXIMIZE:
        if (m_WorkMessage.m_iParam1 != 0)
        {
            if (GetWindow(m_WorkMessage.m_iParam1) != NULL)
            {
                GetWindow(m_WorkMessage.m_iParam1)->Maximize();
                PlayBuffer(SOUND_CLICK01);
            }
        }
        break;
    case UI_MESSAGE_CLOSE:
        if (m_WorkMessage.m_iParam1 != 0)
        {
            PlayBuffer(SOUND_CLICK01);

            if (GetWindow(m_WorkMessage.m_iParam1) != NULL && GetWindow(m_WorkMessage.m_iParam1)->HaveTextBox() == TRUE)
            {
                SaveIMEStatus();
            }
            RemoveWindow(m_WorkMessage.m_iParam1);
            if (GetTopWindowUIID() != 0)
            {
                CUIBaseWindow* pWindow = GetWindow(GetTopWindowUIID());
                if (pWindow != NULL)
                {
                    if (pWindow->GetState() != UISTATE_HIDE && pWindow->GetState() != UISTATE_READY)
                    {
                        //GetWindow(GetTopWindowUIID())->SetState(UISTATE_NORMAL);
                        SendUIMessageToWindow(GetTopWindowUIID(), UI_MESSAGE_SELECTED, 0, 0);
                    }
                    else
                    {
                        g_pWindowMgr->SetWindowsEnable(FALSE);
                        SetFocus(g_hWnd);
                    }
                }
            }
            else
            {
                g_pWindowMgr->SetWindowsEnable(FALSE);
                SetFocus(g_hWnd);
            }
        }
        break;
    case UI_MESSAGE_BOTTOM:
        if (m_WorkMessage.m_iParam1 != 0 && GetWindow(m_WorkMessage.m_iParam1) != NULL)
        {
            if ((int)(*m_WindowArrangeList.begin()) != m_WorkMessage.m_iParam1)
            {
                m_WindowArrangeList.remove(m_WorkMessage.m_iParam1);
                m_WindowArrangeList.push_front(m_WorkMessage.m_iParam1);
            }
        }
        break;
    default:
        break;
    }
}

void CUIWindowMgr::OpenMainWnd(int iPos_x, int iPos_y)
{
    g_pWindowMgr->HideAllWindowClear();
    if (g_iChatInputType == 0)
    {
        if (g_pSystemLogBox->CheckChatRedundancy(GlobalText[992], 2) == FALSE)
            g_pSystemLogBox->AddText(GlobalText[992], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return;
    }
    int iLevel = CharacterAttribute->Level;

    if (iLevel < 6)
    {
        if (g_pSystemLogBox->CheckChatRedundancy(GlobalText[1067]) == FALSE)
            g_pSystemLogBox->AddText(GlobalText[1067], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return;
    }

    if (m_dwMainWindowUIID != 0)
    {
        if (GetWindow(m_dwMainWindowUIID)->GetState() == UISTATE_HIDE)
        {
            ShowHideWindow(m_dwMainWindowUIID, TRUE);
            PlayBuffer(SOUND_CLICK01);
            PlayBuffer(SOUND_INTERFACE01);
        }
        else if (GetWindow(m_dwMainWindowUIID)->GetState() == UISTATE_NORMAL)
        {
            CloseMainWnd();
        }
        return;
    }
    if (m_iMainWindowWidth == 0)
    {
        AddWindow(UIWNDTYPE_FRIENDMAIN, iPos_x, iPos_y, GlobalText[m_iFriendMainWindowTitleNumber]);
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, m_dwMainWindowUIID, 0);
        RefreshMainWndChatRoomList();
        PlayBuffer(SOUND_CLICK01);
        PlayBuffer(SOUND_INTERFACE01);
    }
    else
    {
        AddWindow(UIWNDTYPE_FRIENDMAIN, m_iMainWindowPos_x, m_iMainWindowPos_y, GlobalText[m_iFriendMainWindowTitleNumber]);
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, m_dwMainWindowUIID, 0);
        CUIBaseWindow* pWindow = GetWindow(m_dwMainWindowUIID);
        if (pWindow != NULL)
        {
            pWindow->SetSize(m_iMainWindowWidth, m_iMainWindowHeight);
            pWindow->SetBackPosition(m_bIsMainWindowMaximize, m_iMainWindowBackPos_y, m_iMainWindowBackHeight);
            // 윈도우 목록 복구
            RefreshMainWndChatRoomList();
            pWindow->Refresh();
            //			((CUIFriendWindow *)pWindow)->SetTabIndex(m_iLastFriendWindowTabIndex);
            PlayBuffer(SOUND_CLICK01);
            PlayBuffer(SOUND_INTERFACE01);
        }
        else
        {
            return;
        }
        DoAction();
    }
}

void CUIWindowMgr::CloseMainWnd()
{
    if (m_dwMainWindowUIID == 0) return;
    RemoveWindow(m_dwMainWindowUIID);
    //SendUIMessage(UI_MESSAGE_CLOSE, m_dwMainWindowUIID, 0);
}

void CUIWindowMgr::RefreshMainWndChatRoomList()
{
    CUIBaseWindow* pWindow = GetWindow(m_dwMainWindowUIID);
    if (pWindow == NULL) return;
    ((CUIFriendWindow*)pWindow)->ResetWindow();
    for (m_WindowMapIter = m_WindowMap.begin(); m_WindowMapIter != m_WindowMap.end(); ++m_WindowMapIter)
    {
        if (m_dwMainWindowUIID != m_WindowMapIter->first && m_WindowMapIter->second->GetState() != UISTATE_READY)
            ((CUIFriendWindow*)pWindow)->AddWindow(m_WindowMapIter->first, m_WindowMapIter->second->GetTitle());
    }
}

BOOL CUIWindowMgr::LetterReadCheck(DWORD dwLetterID)
{
    m_LetterReadMapIter = m_LetterReadMap.find(dwLetterID);
    if (m_LetterReadMapIter == m_LetterReadMap.end())
    {
        m_LetterReadMap.insert(std::pair<DWORD, DWORD>(dwLetterID, 0));
        return FALSE;
    }
    return TRUE;
}

void CUIWindowMgr::CloseLetterRead(DWORD dwLetterID)
{
    m_LetterReadMapIter = m_LetterReadMap.find(dwLetterID);
    if (m_LetterReadMapIter != m_LetterReadMap.end())
    {
        m_LetterReadMap.erase(m_LetterReadMapIter);
    }
}

void CUIWindowMgr::SetLetterReadWindow(DWORD dwLetterID, DWORD dwWindowUIID)
{
    m_LetterReadMapIter = m_LetterReadMap.find(dwLetterID);
    if (m_LetterReadMapIter != m_LetterReadMap.end())
    {
        m_LetterReadMapIter->second = dwWindowUIID;
    }
}

DWORD CUIWindowMgr::GetLetterReadWindow(DWORD dwLetterID)
{
    m_LetterReadMapIter = m_LetterReadMap.find(dwLetterID);
    if (m_LetterReadMapIter != m_LetterReadMap.end())
    {
        return m_LetterReadMapIter->second;
    }
    return 0;
}

void CUIWindowMgr::AddForceTopWindowList(DWORD dwWindowUIID)
{
    if (IsForceTopWindow(dwWindowUIID) == FALSE)
        m_ForceTopWindowList.push_back(dwWindowUIID);
}

void CUIWindowMgr::RemoveForceTopWindowList(DWORD dwWindowUIID)
{
    m_ForceTopWindowList.remove(dwWindowUIID);
}

BOOL CUIWindowMgr::IsForceTopWindow(DWORD dwWindowUIID)
{
    for (std::list<DWORD>::iterator iter = m_ForceTopWindowList.begin(); iter != m_ForceTopWindowList.end(); ++iter)
    {
        if (*iter == dwWindowUIID) return TRUE;
    }
    return FALSE;
}

void CUIWindowMgr::SetServerEnable(BOOL bFlag)
{
    m_bServerEnable = bFlag;
    if (bFlag == TRUE)
    {
        if (m_iFriendMainWindowTitleNumber != 990)
        {
            m_iFriendMainWindowTitleNumber = 990;
            if (GetFriendMainWindow() != NULL)
                GetFriendMainWindow()->SetTitle(GlobalText[m_iFriendMainWindowTitleNumber]);
        }
    }
    else
    {
        if (m_iFriendMainWindowTitleNumber != 1066)
        {
            m_iFriendMainWindowTitleNumber = 1066;
            if (GetFriendMainWindow() != NULL)
                GetFriendMainWindow()->SetTitle(GlobalText[m_iFriendMainWindowTitleNumber]);
        }
    }
}
void SetLineColor(int iType, float fAlphaRate = 1.0f)
{
    GLubyte ubWindowAlpha = 255 * fAlphaRate;

    switch (iType)
    {
    case 0:
        glColor4ub(146, 134, 121, ubWindowAlpha);	break;
    case 1:
        glColor4ub(37, 37, 37, ubWindowAlpha);		break;
    case 2:
        glColor4ub(106, 97, 88, ubWindowAlpha);		break;
    case 3:
        glColor4ub(0, 0, 0, 179 * fAlphaRate);		break;
    case 4:
        glColor4ub(173, 167, 150, ubWindowAlpha);	break;
    case 5:
        glColor4ub(53, 49, 48, ubWindowAlpha);		break;
    case 6:
        glColor4ub(26, 22, 21, ubWindowAlpha);		break;
    case 7:
        glColor4ub(0, 0, 0, 255 * fAlphaRate);		break;
    case 8:
        glColor4ub(153, 156, 166, ubWindowAlpha);	break;
    case 9:
        glColor4ub(136, 138, 147, ubWindowAlpha);	break;
    case 10:
        glColor4ub(83, 85, 93, ubWindowAlpha);		break;
    case 11:
        glColor4ub(102, 104, 112, ubWindowAlpha);	break;
    case 12:
        glColor4ub(0, 0, 8, ubWindowAlpha);			break;
    case 13:
        glColor4ub(0, 0, 0, ubWindowAlpha);			break;
    case 14:
        glColor4ub(185, 185, 185, ubWindowAlpha);	break;
    case 15:
        glColor4ub(194, 194, 194, ubWindowAlpha);	break;
    case 16:
        glColor4ub(194, 194, 194, ubWindowAlpha);	break;
    case 17:
        glColor4ub(209, 188, 134, ubWindowAlpha);	break;
    case 18:
        glColor4ub(205, 209, 133, ubWindowAlpha);	break;
    default:	break;
    }
}

void RenderWindowVLine(float pos_x, float pos_y, float height)
{
    SetLineColor(2);
    RenderColor(pos_x, pos_y, 1.0f, height);
    RenderColor(pos_x + 4, pos_y, 1.0f, height);
    SetLineColor(1);
    RenderColor(pos_x + 1, pos_y, 3.0f, height);
}

void RenderWindowHLine(float pos_x, float pos_y, float width)
{
    SetLineColor(2);
    RenderColor(pos_x, pos_y, width, 1.0f);
    RenderColor(pos_x, pos_y + 4, width, 1.0f);
    SetLineColor(1);
    RenderColor(pos_x, pos_y + 1, width, 3.0f);
}

CUIBaseWindow::CUIBaseWindow()
{
    memset(&m_WorkMessage, 0, sizeof(UI_MESSAGE));
    m_iMouseClickPos_x = 0;
    m_iMouseClickPos_y = 0;
    m_iMinWidth = 100;
    m_iMinHeight = 100;
    m_iMaxWidth = 0;
    m_iMaxHeight = 0;
    SetOption(UIWINDOWSTYLE_NORMAL);
    m_bHaveTextBox = FALSE;
    m_iControlButtonClick = 0;
    m_bIsMaximize = FALSE;
    m_iBackPos_y = 0;
    m_iBackHeight = 0;
    m_iResizeDir = 0;
}

CUIBaseWindow::~CUIBaseWindow()
{
}

void CUIBaseWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    SetTitle(pszTitle);
    SetParentUIID(dwParentID);

    SetPosition(50, 50);
    SetSize(213, 170);
}

void CUIBaseWindow::SetTitle(const wchar_t* pszTitle)
{
    if (!pszTitle)
        return;

    m_strTitle = std::wstring(pszTitle);
}

void CUIBaseWindow::DrawOutLine(int iPos_x, int iPos_y, int iWidth, int iHeight)
{
    SetLineColor(0);
    RenderColor((float)iPos_x, (float)iPos_y, (float)iWidth, (float)1);
    SetLineColor(1);
    RenderColor((float)iPos_x, (float)iPos_y + 1, (float)iWidth, (float)3);
    SetLineColor(2);
    RenderColor((float)iPos_x, (float)iPos_y + 4, (float)iWidth, (float)1);

    SetLineColor(2);
    RenderColor((float)iPos_x, (float)iPos_y + iHeight - 5, (float)iWidth, (float)1);
    SetLineColor(1);
    RenderColor((float)iPos_x, (float)iPos_y + iHeight - 4, (float)iWidth, (float)3);
    SetLineColor(0);
    RenderColor((float)iPos_x, (float)iPos_y + iHeight - 1, (float)iWidth, (float)1);

    SetLineColor(0);
    RenderColor((float)iPos_x, (float)iPos_y, (float)1, (float)iHeight);
    SetLineColor(1);
    RenderColor((float)iPos_x + 1, (float)iPos_y + 5, (float)3, (float)iHeight - 10);
    SetLineColor(2);
    RenderColor((float)iPos_x + 4, (float)iPos_y + 1, (float)1, (float)iHeight - 2);

    SetLineColor(2);
    RenderColor((float)iPos_x + iWidth - 5, (float)iPos_y + 1, (float)1, (float)iHeight - 2);
    SetLineColor(1);
    RenderColor((float)iPos_x + iWidth - 4, (float)iPos_y + 5, (float)3, (float)iHeight - 10);
    SetLineColor(0);
    RenderColor((float)iPos_x + iWidth - 1, (float)iPos_y, (float)1, (float)iHeight);

    SetLineColor(4);
    RenderColor((float)iPos_x + 2, (float)iPos_y + 2, (float)1, (float)1);
    RenderColor((float)iPos_x + iWidth - 3, (float)iPos_y + 2, (float)1, (float)1);
    RenderColor((float)iPos_x + 2, (float)iPos_y + iHeight - 3, (float)1, (float)1);
    if (!CheckOption(UIWINDOWSTYLE_RESIZEABLE))
        RenderColor((float)iPos_x + iWidth - 3, (float)iPos_y + iHeight - 3, (float)1, (float)1);
}

void CUIBaseWindow::SetControlButtonColor(int iSelect)
{
    if (m_iControlButtonClick == iSelect && CheckMouseIn(m_iPos_x + m_iWidth - 38, m_iPos_y + 8, 38, 9) == TRUE)
    {
        if (MouseLButtonPush == true)
            glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
        else
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
    }
}

void CUIBaseWindow::Render()
{
    std::call_once(_controlsInitialized, [this]() 
        { 
            InitControls(); 
        }
    );

    EnableAlphaTest();

    if (m_iOptions == UIWINDOWSTYLE_NULL);
    else if (CheckOption(UIWINDOWSTYLE_FRAME))
    {
        SetLineColor(3);
        RenderColor((float)m_iPos_x, (float)m_iPos_y + 5, (float)m_iWidth, (float)m_iHeight - 10);
    }
    else
    {
        SetLineColor(3);
        RenderColor((float)m_iPos_x, (float)m_iPos_y + 5, (float)m_iWidth, (float)m_iHeight);
    }
    EndRenderColor();

    g_pRenderText->SetFont(g_hFont);

    RenderSub();

    BOOL bBackWindow = FALSE;
    if (g_pWindowMgr->GetTopWindowUIID() != GetUIID()
        )
    {
        bBackWindow = TRUE;
    }
    if (bBackWindow == TRUE)
        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    if (CheckOption(UIWINDOWSTYLE_FRAME))
    {
        if (CheckOption(UIWINDOWSTYLE_TITLEBAR))
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 8, (float)m_iPos_x + 6, (float)m_iPos_y + 5, (float)m_iWidth - 12, (float)15,
                0.f, 0.f, 4.f / 4.f, 15.f / 16.f);
            if (bBackWindow == TRUE) SetLineColor(10);
            else SetLineColor(8);
            RenderColor((float)m_iPos_x + 5, (float)m_iPos_y + 5, (float)1, (float)1);
            if (bBackWindow == TRUE) SetLineColor(10);
            else SetLineColor(9);
            RenderColor((float)m_iPos_x + 5, (float)m_iPos_y + 6, (float)1, (float)13);
            SetLineColor(10);
            RenderColor((float)m_iPos_x + 5, (float)m_iPos_y + 19, (float)1, (float)1);
            SetLineColor(11);
            RenderColor((float)m_iPos_x + m_iWidth - 6, (float)m_iPos_y + 5, (float)1, (float)1);
            SetLineColor(12);
            RenderColor((float)m_iPos_x + m_iWidth - 6, (float)m_iPos_y + 6, (float)1, (float)13);
            SetLineColor(13);
            RenderColor((float)m_iPos_x + m_iWidth - 6, (float)m_iPos_y + 19, (float)1, (float)1);
        }

        DrawOutLine(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight);
        EndRenderColor();
    }
    if (CheckOption(UIWINDOWSTYLE_TITLEBAR))
    {
        EnableAlphaTest();
        SetLineColor(2);
        RenderColor((float)m_iPos_x + 5, (float)m_iPos_y + 20, (float)m_iWidth - 10, (float)1);
        EndRenderColor();

        g_pRenderText->SetFont(g_hFontBold);
        if (bBackWindow == FALSE)
        {
            g_pRenderText->SetTextColor(230, 220, 200, 255);
        }
        else
        {
            g_pRenderText->SetTextColor(115, 110, 100, 255);
        }
        g_pRenderText->SetBgColor(0);

        wchar_t szTempTitle[256] = { 0 };
        CutText3(m_strTitle.c_str(), szTempTitle, m_iWidth - 50, 1, 256);
        g_pRenderText->RenderText(m_iPos_x + 9, m_iPos_y + 8, szTempTitle);
        if (CheckOption(UIWINDOWSTYLE_MINBUTTON))
        {
            SetControlButtonColor(1);
            RenderBitmap(BITMAP_INTERFACE_EX + 10, (float)m_iPos_x + m_iWidth - (CheckOption(UIWINDOWSTYLE_MAXBUTTON) ? 38 : 27),
                (float)m_iPos_y + 8, (float)9, (float)9, 0.f, 0.f, 9.f / 32.f, 9.f / 32.f);
        }
        if (CheckOption(UIWINDOWSTYLE_MAXBUTTON))
        {
            SetControlButtonColor(2);
            if (m_bIsMaximize == FALSE)
                RenderBitmap(BITMAP_INTERFACE_EX + 10, (float)m_iPos_x + m_iWidth - 27, (float)m_iPos_y + 8, (float)9, (float)9, 9.f / 32.f, 0.f, 9.f / 32.f, 9.f / 32.f);
            else
                RenderBitmap(BITMAP_INTERFACE_EX + 10, (float)m_iPos_x + m_iWidth - 27, (float)m_iPos_y + 8, (float)9, (float)9, 9.f / 32.f, 9.f / 32.f, 9.f / 32.f, 9.f / 32.f);
        }
        SetControlButtonColor(3);
        RenderBitmap(BITMAP_INTERFACE_EX + 10, (float)m_iPos_x + m_iWidth - 16, (float)m_iPos_y + 8, (float)9, (float)9, 0.f, 9.f / 32.f, 9.f / 32.f, 9.f / 32.f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        g_pRenderText->SetFont(g_hFont);
    }
    if (CheckOption(UIWINDOWSTYLE_RESIZEABLE))
    {
        RenderBitmap(BITMAP_INTERFACE_EX + 11, (float)m_iPos_x + m_iWidth - 10, (float)m_iPos_y + m_iHeight - 10, (float)9, (float)9,
            0.f, 0.f, 9.f / 16.f, 9.f / 16.f);
    }
    if (g_pWindowMgr->GetTopWindowUIID() != GetUIID() || !g_pUIManager->IsOpen(INTERFACE_FRIEND))
    {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        DisableAlphaBlend();
    }

    RenderOver();
}

BOOL CUIBaseWindow::DoMouseAction()
{
    m_iControlButtonClick = 0;
    if (CheckMouseIn(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight))
    {
        if (CheckOption(UIWINDOWSTYLE_TITLEBAR) &&
            CheckMouseIn(m_iPos_x + m_iWidth - 16, m_iPos_y + 8, 9, 9))
        {
            m_iControlButtonClick = 3;
            if (MouseOnWindow == false && MouseLButtonPop == true)
            {
                if (CloseCheck() == TRUE)
                {
                    g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
                }
                MouseLButtonPop = false;
            }
        }
        else if (CheckOption(UIWINDOWSTYLE_MAXBUTTON) &&
            CheckMouseIn(m_iPos_x + m_iWidth - 27, m_iPos_y + 8, 9, 9))
        {
            m_iControlButtonClick = 2;
            if (MouseOnWindow == false && MouseLButtonPop == true)
            {
                g_pWindowMgr->SendUIMessage(UI_MESSAGE_MAXIMIZE, GetUIID(), 0);
                MouseLButtonPop = false;
            }
        }
        else if (CheckOption(UIWINDOWSTYLE_MINBUTTON) &&
            CheckMouseIn(m_iPos_x + m_iWidth - (CheckOption(UIWINDOWSTYLE_MAXBUTTON) ? 38 : 27),
                m_iPos_y + 8, 9, 9))
        {
            m_iControlButtonClick = 1;
            if (MouseOnWindow == false && MouseLButtonPop == true)
            {
                g_pWindowMgr->SendUIMessage(UI_MESSAGE_HIDE, GetUIID(), 0);
                MouseLButtonPop = false;
            }
        }
        else if (CheckOption(UIWINDOWSTYLE_RESIZEABLE) &&
            CheckMouseIn(m_iPos_x, m_iPos_y, 7, 7))
        {
            if (MouseLButton == true && GetState() != UISTATE_RESIZE && g_dwActiveUIID == 0)
            {
                g_dwActiveUIID = GetUIID();
                SetState(UISTATE_RESIZE);
                m_iResizeDir = 315;
                m_iMouseClickPos_x = MouseX;
                m_iMouseClickPos_y = MouseY;
                m_bIsMaximize = FALSE;
            }
        }
        else if (CheckOption(UIWINDOWSTYLE_MOVEABLE) && CheckOption(UIWINDOWSTYLE_TITLEBAR) &&
            CheckMouseIn(m_iPos_x, m_iPos_y, m_iWidth, 20))
        {
            if (MouseLButton == true && GetState() != UISTATE_MOVE && g_dwActiveUIID == 0)
            {
                g_dwActiveUIID = GetUIID();
                SetState(UISTATE_MOVE);
                m_iMouseClickPos_x = MouseX;
                m_iMouseClickPos_y = MouseY;
            }
        }
        else if (CheckOption(UIWINDOWSTYLE_RESIZEABLE) &&
            CheckMouseIn(m_iPos_x + m_iWidth - 10, m_iPos_y + m_iHeight - 10, 10, 10))
        {
            if (MouseLButton == true && GetState() != UISTATE_RESIZE && g_dwActiveUIID == 0)
            {
                g_dwActiveUIID = GetUIID();
                SetState(UISTATE_RESIZE);
                m_iResizeDir = 135;
                m_iMouseClickPos_x = MouseX;
                m_iMouseClickPos_y = MouseY;
                m_bIsMaximize = FALSE;
            }
        }
        else if (g_dwActiveUIID == GetUIID() && GetState() != UISTATE_MOVE && GetState() != UISTATE_RESIZE)
        {
            MouseLButton = false;
            MouseLButtonPush = false;
        }
        MouseOnWindow = true;
    }
    if (GetState() == UISTATE_MOVE)
    {
        if (MouseLButton == true)
        {
            if (g_dwMouseUseUIID == 0) g_dwMouseUseUIID = GetUIID();
            MouseOnWindow = true;

            if (m_iPos_x + MouseX - m_iMouseClickPos_x < 0) m_iPos_x = 0;
            else if (m_iPos_x + m_iWidth + MouseX - m_iMouseClickPos_x > 640) m_iPos_x = 640 - m_iWidth;
            else m_iPos_x += MouseX - m_iMouseClickPos_x;

            if (m_iPos_y + MouseY - m_iMouseClickPos_y < 0)
            {
                m_iPos_y = 0;
            }
            else if (m_iPos_y + m_iHeight + MouseY - m_iMouseClickPos_y > 480)
            {
                m_iPos_y = 480 - m_iHeight;
            }
            else
            {
                m_iPos_y += MouseY - m_iMouseClickPos_y;
            }

            m_iMouseClickPos_x = MouseX;
            m_iMouseClickPos_y = MouseY;
        }
        else
        {
            SetState(UISTATE_NORMAL);
            if (g_dwActiveUIID == GetUIID()) g_dwActiveUIID = 0;
        }
    }
    else if (GetState() == UISTATE_RESIZE)
    {
        if (MouseLButton == true)
        {
            if (m_iResizeDir == 135)
            {
                if (g_dwMouseUseUIID == 0) g_dwMouseUseUIID = GetUIID();
                MouseOnWindow = true;

                if (m_iPos_x + m_iWidth + MouseX - m_iMouseClickPos_x > 640) m_iWidth = 640 - m_iPos_x;
                else m_iWidth += MouseX - m_iMouseClickPos_x;

                if (m_iWidth < m_iMinWidth)
                {
                    m_iWidth = m_iMinWidth;
                }
                else if (m_iMaxWidth > 0 && m_iWidth > m_iMaxWidth)
                {
                    m_iWidth = m_iMaxWidth;
                }
                else m_iMouseClickPos_x = MouseX;

                if (m_iPos_y + m_iHeight + MouseY - m_iMouseClickPos_y > 480) m_iHeight = 480 - m_iPos_y;
                else m_iHeight += MouseY - m_iMouseClickPos_y;

                if (m_iHeight < m_iMinHeight)
                {
                    m_iHeight = m_iMinHeight;
                }
                else if (m_iMaxHeight > 0 && m_iHeight > m_iMaxHeight)
                {
                    m_iHeight = m_iMaxHeight;
                }
                else m_iMouseClickPos_y = MouseY;
            }
            else if (m_iResizeDir == 315)
            {
                if (g_dwMouseUseUIID == 0) g_dwMouseUseUIID = GetUIID();
                MouseOnWindow = true;

                if (m_iWidth + m_iMouseClickPos_x - MouseX >= m_iMinWidth &&
                    m_iPos_x - (m_iMouseClickPos_x - MouseX) >= 0)
                {
                    m_iPos_x -= m_iMouseClickPos_x - MouseX;
                    m_iWidth += m_iMouseClickPos_x - MouseX;
                }
                m_iMouseClickPos_x = MouseX;

                if (m_iHeight + m_iMouseClickPos_y - MouseY >= m_iMinHeight &&
                    m_iPos_y - (m_iMouseClickPos_y - MouseY) >= 0)
                {
                    m_iPos_y -= m_iMouseClickPos_y - MouseY;
                    m_iHeight += m_iMouseClickPos_y - MouseY;
                }
                m_iMouseClickPos_y = MouseY;
            }
        }
        else
        {
            SetState(UISTATE_NORMAL);
            if (g_dwActiveUIID == GetUIID()) g_dwActiveUIID = 0;
        }
    }
    DoMouseActionSub();

    return TRUE;
}

void CUIBaseWindow::Maximize()
{
    if (m_bIsMaximize == FALSE)
    {
        m_iBackPos_y = m_iPos_y;
        m_iBackHeight = m_iHeight;
        m_iPos_y = 0;
        m_iHeight = 480 - 48;
        Refresh();
        m_bIsMaximize = TRUE;
    }
    else
    {
        m_iPos_y = m_iBackPos_y;
        m_iHeight = m_iBackHeight;
        Refresh();
        m_bIsMaximize = FALSE;
    }
}

CUIChatWindow::CUIChatWindow()
    : m_iShowType(1),
    m_dwRoomNumber(0)
{}

CUIChatWindow::~CUIChatWindow()
{
    DisconnectToChatServer();
}

void CUIChatWindow::InitControls()
{
    m_TextInputBox.Init(g_hWnd, 238, 14, 50);
    m_TextInputBox.SetSize(180, 14);
    m_TextInputBox.SetParentUIID(m_dwUIID);
    m_TextInputBox.SetFont(g_hFont);

    m_TextInputBox.SetOption(UIOPTION_ENTERIMECHKOFF);
    m_TextInputBox.SetBackColor(0, 0, 0, 0);

    m_TextInputBox.SetParentUIID(GetUIID());
    m_TextInputBox.SetArrangeType(2, 2, 12);
    m_TextInputBox.SetState(UISTATE_NORMAL);
    m_TextInputBox.SetTextLimit(MAX_CHATROOM_TEXT_LENGTH - 1);

    m_InviteButton.Init(1, GlobalText[993]);
    m_InviteButton.SetParentUIID(GetUIID());
    m_InviteButton.SetSize(53, 13);
    m_InviteButton.SetArrangeType(3, 54, 14);

    m_CloseInviteButton.Init(2, GlobalText[993]);
    m_CloseInviteButton.SetParentUIID(GetUIID());
    m_CloseInviteButton.SetSize(73, 13);
    m_CloseInviteButton.SetArrangeType(3, 74, 14);
    Refresh();
}

void CUIChatWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    memset(m_szLastText, 0, MAX_CHATROOM_TEXT_LENGTH);

    SetTitle(pszTitle);
    SetParentUIID(dwParentID);

    SetPosition(50, 50);
    SetSize(250, 170);
    SetLimitSize(250, 150);

    m_ChatListBox.SetParentUIID(GetUIID());
    m_ChatListBox.SetArrangeType(2, 0, 16);
    m_ChatListBox.SetResizeType(3, 0, -16);

    m_PalListBox.SetParentUIID(GetUIID());
    m_PalListBox.SetArrangeType(3, 75, 16);
    m_PalListBox.SetResizeType(2, 75, -16);

    //	m_PalListBox.AddText(L"이름네자", 1, 1);
    //	m_PalListBox.AddText(L"이름넉자", 1, 1);
    //	m_PalListBox.AddText(L"이름수넷", 1, 1);
    //	m_PalListBox.AddText(L"이름1자", 1, 1);
    //	m_PalListBox.AddText(L"이름2자", 1, 1);
    //	m_PalListBox.AddText(L"이름3넷", 1, 1);
    //	m_PalListBox.AddText(L"이름4자", 1, 1);
    //	m_PalListBox.AddText(L"이름5자", 1, 1);
    //	m_PalListBox.AddText(L"이름6넷", 1, 1);
    //	m_PalListBox.AddText(L"이름7넷", 1, 1);
    //	m_PalListBox.AddText(L"이름8넷", 1, 1);
    //	m_PalListBox.AddText(L"이름9넷", 1, 1);

    

    m_InvitePalListBox.SetParentUIID(GetUIID());
    m_InvitePalListBox.SetArrangeType(3, 75, 16);
    m_InvitePalListBox.SetResizeType(2, 75, -16);

    m_iPrevWidth = 0;
}

void CUIChatWindow::Refresh()
{
    m_ChatListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_InvitePalListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_InviteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_CloseInviteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_TextInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

    m_ChatListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_ChatListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_InvitePalListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_InvitePalListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_PalListBox.SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);
    m_InvitePalListBox.SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);

    m_bHaveTextBox = TRUE;
    g_dwKeyFocusUIID = m_PalListBox.GetUIID();
}


void TranslateChattingProtocol(DWORD dwWindowUIID, const BYTE* ReceiveBuffer, int Size);

void CUIChatWindow::HandlePacketS(int32_t handle, const BYTE* ReceiveBuffer, int32_t Size)
{
    if (const auto uuid = ConnectionHandleToWindowUuid.find(handle)->second)
    {
        TranslateChattingProtocol(uuid, ReceiveBuffer, Size);
    }
}

void CUIChatWindow::ConnectToChatServer(const wchar_t* pszIP, DWORD dwRoomNumber, DWORD dwTicket)
{
    m_dwRoomNumber = dwRoomNumber;

    _connection = new Connection(pszIP, 55980, true, &HandlePacketS);

    if (!_connection->IsConnected())
    {
        // todo: write message, that it failed?
        return;
    }

    ConnectionHandleToWindowUuid[_connection->GetHandle()] = this->GetUIID();
    _connection->ToChatServer()->SendAuthenticateExt(dwRoomNumber, dwTicket);
}

void CUIChatWindow::DisconnectToChatServer()
{
    if (_connection != nullptr)
    {
        if (_connection->IsConnected())
        {
            _connection->ToChatServer()->SendLeaveChatRoom();
            _connection->Close();
        }

        _connection = nullptr;
    }
}

int CUIChatWindow::AddChatPal(const wchar_t* pszID, BYTE Number, BYTE Server)
{
    BOOL bFind = FALSE;
    for (std::deque<GUILDLIST_TEXT>::iterator iter = m_PalListBox.GetFriendList().begin(); iter != m_PalListBox.GetFriendList().end(); ++iter)
    {
        wchar_t* n = iter->m_szID;

        if (wcscmp(iter->m_szID, pszID) == 0)
        {
            iter->m_Number = Number;
            iter->m_Server = Server;
            bFind = TRUE;
            break;
        }
    }
    if (bFind == FALSE)
        m_PalListBox.AddText(pszID, Number, Server);

    wchar_t szTitle[128] = { 0 };
    wcsncpy(szTitle, GlobalText[994], GlobalText.GetStringSize(994));
    m_PalListBox.MakeTitleText(szTitle);
    SetTitle(szTitle);
    g_pWindowMgr->RefreshMainWndChatRoomList();

    if (m_PalListBox.GetLineNum() >= 2)
    {
        Lock(FALSE);
    }

    if (m_PalListBox.GetLineNum() > 2)
    {
        if (m_iShowType >= 2)
            m_ChatListBox.SetResizeType(3, -80 - 80, -16);
        else m_ChatListBox.SetResizeType(3, -80, -16);
        m_ChatListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    }

    return m_PalListBox.GetLineNum();
}

void CUIChatWindow::RemoveChatPal(const wchar_t* pszID)
{
    if (m_PalListBox.GetLineNum() > 2)
    {
        m_PalListBox.DeleteText(pszID);

        wchar_t szTitle[128] = { 0 };
        wcsncpy(szTitle, GlobalText[994], GlobalText.GetStringSize(994));
        m_PalListBox.MakeTitleText(szTitle);
        SetTitle(szTitle);
        g_pWindowMgr->RefreshMainWndChatRoomList();
    }

    if (m_PalListBox.GetLineNum() <= 2)
    {
        if (m_iShowType >= 2)
            m_ChatListBox.SetResizeType(3, -80 - 80, -16);
        else m_ChatListBox.SetResizeType(3, 0, -16);
        m_ChatListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    }
}

void CUIChatWindow::AddChatText(BYTE byIndex, const wchar_t* pszText, int iType, int iColor)
{
    const wchar_t* pszID = m_PalListBox.GetNameByNumber(byIndex);
    m_ChatListBox.AddText((pszID != NULL ? pszID : L""), pszText, iType, iColor);
}

void CUIChatWindow::RenderSub()
{
    EnableAlphaTest();
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_ChatListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_InvitePalListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_InviteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_CloseInviteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_TextInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

        if (GetState() == UISTATE_RESIZE)
        {
            m_ChatListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
            m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
            m_InvitePalListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
        }
    }

    m_ChatListBox.Render();
    if (m_PalListBox.GetLineNum() > 2 || m_iShowType >= 2)
    {
        m_PalListBox.Render();
    }
    if (m_iShowType >= 2)
    {
        m_InvitePalListBox.Render();
        EnableAlphaTest();
        RenderWindowVLine((float)(RPos_x(0) + RWidth() - 160), (float)RPos_y(0), (float)RHeight() - 16);
    }
    if (m_PalListBox.GetLineNum() > 2 || m_iShowType >= 2)
    {
        RenderWindowVLine((float)(RPos_x(0) + RWidth() - 80), (float)RPos_y(0), (float)RHeight() - 16);
    }

    EnableAlphaTest();
    SetLineColor(2);
    RenderColor((float)RPos_x(0), (float)RPos_y(0) + RHeight() - 16, (float)RWidth(), 1.0f);
    SetLineColor(7);
    RenderColor((float)RPos_x(0), (float)RPos_y(0) + RHeight() - 15, (float)RWidth(), 15);
    EndRenderColor();

    m_InviteButton.Render();
    m_TextInputBox.Render();
    if (m_iShowType >= 2) m_CloseInviteButton.Render();
    DisableAlphaBlend();
}

void CUIChatWindow::UpdateInvitePalList()
{
    g_pFriendList->UpdateFriendList(m_InvitePalListBox.GetFriendList(), NULL);

    std::deque<GUILDLIST_TEXT>::iterator iter;

    for (iter = m_PalListBox.GetFriendList().begin(); iter != m_PalListBox.GetFriendList().end(); ++iter)
    {
        if (wcscmp(iter->m_szID, Hero->ID) != 0)
            m_InvitePalListBox.DeleteText(iter->m_szID);
    }

    for (iter = m_InvitePalListBox.GetFriendList().begin(); iter != m_InvitePalListBox.GetFriendList().end();)
    {
        if (iter->m_Server >= 253)
        {
            m_InvitePalListBox.DeleteText(iter->m_szID);
            iter = m_InvitePalListBox.GetFriendList().begin();
        }
        else
        {
            ++iter;
        }
    }
    m_InvitePalListBox.Scrolling(0);
}

BOOL CUIChatWindow::HandleMessage()
{
    if (m_WorkMessage.m_iMessage == UI_MESSAGE_LISTDBLCLICK)
    {
        if (m_WorkMessage.m_iParam1 == (int)(m_InvitePalListBox.GetUIID()))
        {
            PlayBuffer(SOUND_CLICK01);
            m_WorkMessage.m_iMessage = UI_MESSAGE_BTNLCLICK;
            m_WorkMessage.m_iParam1 = 2;
        }
    }
    switch (m_WorkMessage.m_iMessage)
    {
    case UI_MESSAGE_SELECTED:
        m_TextInputBox.GiveFocus();
        g_dwKeyFocusUIID = m_PalListBox.GetUIID();
        break;
    case UI_MESSAGE_TEXTINPUT:
    {
        wchar_t	pszText[MAX_CHATROOM_TEXT_LENGTH] = { };

        m_TextInputBox.GetText(pszText, MAX_CHATROOM_TEXT_LENGTH);
        //if (CheckAbuseFilter(pszText, false))
        //{
        //    wcsncpy(pszText, GlobalText[570], sizeof pszText);
        //}

        if (wcsncmp(m_szLastText, pszText, MAX_CHATROOM_TEXT_LENGTH) != 0)
        {
            wcsncpy(m_szLastText, pszText, MAX_CHATROOM_TEXT_LENGTH);

            if (pszText[0] != L'\0')
            {
                int iSize = wcslen(pszText);
                if (_connection != nullptr)
                {
                    _connection->ToChatServer()->SendChatMessageExt(0, pszText);
                }
            }
        }

        m_TextInputBox.SetText(NULL);
        if (m_PalListBox.GetLineNum() < 2)
        {
            Lock(TRUE);
        }
    }
    break;
    case UI_MESSAGE_BTNLCLICK:
    {
        if (g_dwTopWindow != 0) break;
        DWORD dwUIID = 0;
        switch (m_WorkMessage.m_iParam1)
        {
        case 1:
            if (m_iShowType == 1)
            {
                m_iShowType = 2;
                SetSize(GetWidth() + 80, GetHeight());
                SetLimitSize(250 + 80, 150, 540 / g_fScreenRate_x + 80);
                m_ChatListBox.SetResizeType(3, -80 - 80, -16);
                m_PalListBox.SetArrangeType(3, 75 + 80, 16);
                m_InviteButton.SetArrangeType(3, 54 + 80, 14);
                m_ChatListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
                m_CloseInviteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
                m_InvitePalListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
                m_InviteButton.SetCaption(GlobalText[996]);

                UpdateInvitePalList();

                if (m_iPos_x + m_iWidth > 640) m_iPos_x = 640 - m_iWidth;
                Refresh();
            }
            else if (m_iShowType >= 2)
            {
                m_iShowType = 1;
                SetSize(GetWidth() - 80, GetHeight());
                SetLimitSize(250, 150, 540 / g_fScreenRate_x);
                if (m_PalListBox.GetLineNum() > 2) m_ChatListBox.SetResizeType(3, -80, -16);
                else m_ChatListBox.SetResizeType(3, 0, -16);
                m_PalListBox.SetArrangeType(3, 75, 16);
                m_InviteButton.SetArrangeType(3, 54, 14);
                m_ChatListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
                m_CloseInviteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
                m_InvitePalListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
                m_InviteButton.SetCaption(GlobalText[993]);
            }
            break;
        case 2:
            if (m_TextInputBox.IsLocked() == FALSE && m_InvitePalListBox.GetSelectedText() != NULL)
            {
                if (m_PalListBox.GetLineNum() <= 1);
                else if (m_PalListBox.GetLineNum() >= 30)
                {
                    AddChatText(255, GlobalText[1074], 1, 0);
                }
                else
                {
                    SocketClient->ToGameServer()->SendChatRoomInvitationRequest(
                        m_InvitePalListBox.GetSelectedText()->m_szID,
                        m_dwRoomNumber,
                        GetUIID());
                }
            }
            break;
        default:
            break;
        }
    }
    break;
    default:
        break;
    }
    return FALSE;
}

void CUIChatWindow::DoActionSub(BOOL bMessageOnly)
{
    if (m_iPrevWidth != 0 && m_iPrevWidth != m_iWidth)
    {
        int iNewWidth = RWidth() - m_InviteButton.GetWidth() - 7;
        if (m_iShowType == 2)
            iNewWidth -= m_CloseInviteButton.GetWidth() + 7;
        m_TextInputBox.SetSize(iNewWidth, 14);
    }
    m_iPrevWidth = m_iWidth;

    m_InviteButton.DoAction(bMessageOnly);
    m_ChatListBox.DoAction(bMessageOnly);
    m_PalListBox.DoAction(bMessageOnly);
    m_InvitePalListBox.DoAction(bMessageOnly);
    m_TextInputBox.DoAction(bMessageOnly);
    m_CloseInviteButton.DoAction(bMessageOnly);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIChatWindow::DoMouseActionSub()
{
    //	if (g_dwMouseUseUIID == GetUIID() && MouseLButton == true)
    //	{
    //		m_TextInputBox.GiveFocus();
    //	}
}

const wchar_t* CUIChatWindow::GetChatFriend(int* piResult)
{
    if (m_PalListBox.GetFriendList().size() > 2)
    {
        if (piResult != NULL) *piResult = 2;
        return NULL;
    }
    else
    {
        std::deque<GUILDLIST_TEXT>& pPalList = m_PalListBox.GetFriendList();
        for (std::deque<GUILDLIST_TEXT>::iterator PalListIter = pPalList.begin(); PalListIter != pPalList.end(); ++PalListIter)
        {
            if (wcsncmp(PalListIter->m_szID, Hero->ID, MAX_ID_SIZE) != 0)
            {
                if (piResult != NULL) *piResult = 1;
                return PalListIter->m_szID;
            }
        }
        if (piResult != NULL) *piResult = 0;
        return NULL;
    }
}

void CUIChatWindow::Lock(BOOL bFlag)
{
    if (bFlag == TRUE)
    {
        m_TextInputBox.Lock(TRUE);
        wchar_t szTitle[128] = { 0 };
        if (wcsncmp(GetTitle(), GlobalText[995], GlobalText.GetStringSize(995)) != 0)
        {
            wcsncpy(szTitle, GlobalText[995], GlobalText.GetStringSize(995));
        }
        wcsncat(szTitle, GetTitle(), 128);
        SetTitle(szTitle);
    }
    else
    {
        m_TextInputBox.Lock(FALSE);
        if (wcsncmp(GetTitle(), GlobalText[995], GlobalText.GetStringSize(995)) == 0)
        {
            wchar_t szTitle[128] = { 0 };
            wcsncpy(szTitle, GetTitle() + GlobalText.GetStringSize(995), 128);
            SetTitle(szTitle);
        }
    }
}

extern int gix, giy;
extern void MoveCharacter(CHARACTER* c, OBJECT* o);
extern void MoveCharacterVisual(CHARACTER* c, OBJECT* o);

void CUIPhotoViewer::RenderPhotoCharacter()
{
    float fPos_x = m_iPos_x * 1.2f + m_iWidth / 2 - 50;
    float fPos_y = m_iPos_y * 1.2f + m_iHeight * 1.2f - 62;

    CHARACTER* c = &m_PhotoChar;
    OBJECT* o = &c->Object;
    int WorldBackup = gMapManager.WorldActive;
    gMapManager.WorldActive = WD_0LORENCIA;
    MoveCharacter(c, o);
    MoveCharacterVisual(c, o);
    MoveMount(&m_PhotoHelper, TRUE);
    gMapManager.WorldActive = WorldBackup;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glViewport2(m_iPos_x * g_fScreenRate_x, m_iPos_y * g_fScreenRate_y, m_iWidth * g_fScreenRate_x, 141 * g_fScreenRate_y);
    gluPerspective2(1.f, (float)(m_iWidth * g_fScreenRate_x) / (float)(141 * g_fScreenRate_y), 2000, 20000);//CameraViewNear,CameraViewFar);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    GetOpenGLMatrix(CameraMatrix);
    EnableDepthTest();
    EnableDepthMask();

    glRotatef(-90.0f, 1.f, 0.f, 0.f);
    glRotatef(-90.0f, 0.f, 0.f, 1.f);
    glTranslatef(-10000.0f, 0.0f, -75.f);

    if (c->Helper.Type == MODEL_DARK_HORSE_ITEM)
        glTranslatef(-o->Position[0], -o->Position[1], -o->Position[2] - 50.0f);
    else
        glTranslatef(-o->Position[0], -o->Position[1], -o->Position[2]);

    Vector(0.0f, 0.0f, m_fCurrentAngle, o->Angle);

    glDisable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
    EnableDepthTest();
    EnableCullFace();
    EnableDepthMask();
    bool AlphaTestEnable = false;

    TextureEnable = true;
    DepthTestEnable = true;
    CullFaceEnable = true;
    DepthMaskEnable = true;
    glDepthFunc(GL_LEQUAL);
    glAlphaFunc(GL_GREATER, 0.25f);
    glDisable(GL_FOG);
    glClear(GL_DEPTH_BUFFER_BIT);
    o->Scale = 0.7f * m_fCurrentZoom;
    m_PhotoHelper.Scale = m_fPhotoHelperScale * m_fCurrentZoom;
    Vector(1, 1, 1, o->Light);
    Vector(1, 1, 1, m_PhotoHelper.Light);

    c->HideShadow = true;
    if (c->Wing.Type != -1 && m_iSettingAnimation > AT_HEALING1)
        c->SafeZone = true;
    else c->SafeZone = false;
    if (c->Helper.Type == MODEL_HORN_OF_UNIRIA)
        m_PhotoHelper.Position[2] += 10;
    else if (c->Helper.Type == MODEL_HORN_OF_DINORANT)
        m_PhotoHelper.Position[2] += 25;
    RenderMount(&m_PhotoHelper, TRUE);
    if (c->Helper.Type == MODEL_HORN_OF_UNIRIA)
        m_PhotoHelper.Position[2] -= 10;
    else if (c->Helper.Type == MODEL_HORN_OF_DINORANT)
        m_PhotoHelper.Position[2] -= 25;
    RenderCharacter(c, o);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glViewport2(0, 0, WindowWidth, WindowHeight);
}

int CUIPhotoViewer::SetPhotoPose(int iCurrentAni, int iMoveDir)
{
    if (m_PhotoHelper.Live == true &&
        (m_PhotoHelper.Type == MODEL_UNICON || m_PhotoHelper.Type == MODEL_PEGASUS || m_PhotoHelper.Type == MODEL_DARK_HORSE || (m_PhotoHelper.Type >= MODEL_FENRIR_BLACK && m_PhotoHelper.Type <= MODEL_FENRIR_GOLD)))
    {
        static const int MAX_POSE_NUM = 3;
        static int siPose[MAX_POSE_NUM] = { AT_STAND1, AT_MOVE1, AT_ATTACK1 };

        int iCurrentAniArray = 0;

        for (int i = 0; i < MAX_POSE_NUM; ++i)
        {
            iCurrentAniArray = i;
            if (iCurrentAni == siPose[i]) break;
        }

        iCurrentAniArray += iMoveDir;
        if (iCurrentAniArray < 0) iCurrentAniArray = MAX_POSE_NUM * 100 + iCurrentAniArray;
        iCurrentAniArray %= MAX_POSE_NUM;
        iCurrentAni = siPose[iCurrentAniArray];
    }
    else
    {
        static const int MAX_POSE_NUM = 24;
        static int siPose[MAX_POSE_NUM] = {
            AT_STAND1, AT_GREETING1, AT_CLAP1, AT_GESTURE1, AT_DIRECTION1, AT_AWKWARD1, AT_CRY1, AT_SEE1,
            AT_CHEER1, AT_UNKNOWN1, AT_WIN1, AT_SMILE1, AT_SLEEP1, AT_COLD1, AT_AGAIN1, AT_RESPECT1,
            AT_SALUTE1, AT_GOODBYE1, AT_MOVE1, AT_RUSH1,AT_SIT1, AT_POSE1, AT_HEALING1, AT_ATTACK1
        };

        int iCurrentAniArray = 0;
        for (int i = 0; i < MAX_POSE_NUM; ++i)
        {
            iCurrentAniArray = i;
            if (iCurrentAni == siPose[i])
                break;
        }

        iCurrentAniArray += iMoveDir;
        if (iCurrentAniArray < 0) iCurrentAniArray = MAX_POSE_NUM * 100 + iCurrentAniArray;
        iCurrentAniArray %= MAX_POSE_NUM;
        iCurrentAni = siPose[iCurrentAniArray];
    }

    CHARACTER* c = &m_PhotoChar;
    OBJECT* o = &c->Object;
    int WorldBackup = gMapManager.WorldActive;
    switch (iCurrentAni)
    {
    case AT_STAND1:
        gMapManager.WorldActive = WD_0LORENCIA;
        SetPlayerStop(c);
        gMapManager.WorldActive = WorldBackup;
        break;
    case AT_ATTACK1:
        gMapManager.WorldActive = WD_0LORENCIA;
        SetPlayerAttack(c);
        gMapManager.WorldActive = WorldBackup;
        c->AttackTime = 1;
        c->Object.AnimationFrame = 0;
        c->TargetCharacter = -1;
        break;
    case AT_MOVE1:
        gMapManager.WorldActive = WD_0LORENCIA;
        SetPlayerWalk(c);
        gMapManager.WorldActive = WorldBackup;
        break;
    case AT_SIT1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(&c->Object, PLAYER_SIT1);
        else
            SetAction(&c->Object, PLAYER_SIT_FEMALE1);
        break;
    case AT_POSE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(&c->Object, PLAYER_POSE1);
        else
            SetAction(&c->Object, PLAYER_POSE_FEMALE1);
        break;
    case AT_HEALING1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(&c->Object, PLAYER_HEALING1);
        else
            SetAction(&c->Object, PLAYER_HEALING_FEMALE1);
        break;
    case AT_GREETING1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_GREETING1);
        else
            SetAction(o, PLAYER_GREETING_FEMALE1);
        break;
    case AT_GOODBYE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_GOODBYE1);
        else
            SetAction(o, PLAYER_GOODBYE_FEMALE1);
        break;
    case AT_CLAP1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_CLAP1);
        else
            SetAction(o, PLAYER_CLAP_FEMALE1);
        break;
    case AT_GESTURE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_GESTURE1);
        else
            SetAction(o, PLAYER_GESTURE_FEMALE1);
        break;
    case AT_DIRECTION1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_DIRECTION1);
        else
            SetAction(o, PLAYER_DIRECTION_FEMALE1);
        break;
    case AT_UNKNOWN1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_UNKNOWN1);
        else
            SetAction(o, PLAYER_UNKNOWN_FEMALE1);
        break;
    case AT_CRY1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_CRY1);
        else
            SetAction(o, PLAYER_CRY_FEMALE1);
        break;
    case AT_AWKWARD1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_AWKWARD1);
        else
            SetAction(o, PLAYER_AWKWARD_FEMALE1);
        break;
    case AT_SEE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_SEE1);
        else
            SetAction(o, PLAYER_SEE_FEMALE1);
        break;
    case AT_CHEER1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_CHEER1);
        else
            SetAction(o, PLAYER_CHEER_FEMALE1);
        break;
    case AT_WIN1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_WIN1);
        else
            SetAction(o, PLAYER_WIN_FEMALE1);
        break;
    case AT_SMILE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_SMILE1);
        else
            SetAction(o, PLAYER_SMILE_FEMALE1);
        break;
    case AT_SLEEP1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_SLEEP1);
        else
            SetAction(o, PLAYER_SLEEP_FEMALE1);
        break;
    case AT_COLD1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_COLD1);
        else
            SetAction(o, PLAYER_COLD_FEMALE1);
        break;
    case AT_AGAIN1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_AGAIN1);
        else
            SetAction(o, PLAYER_AGAIN_FEMALE1);
        break;
    case AT_RESPECT1:
        SetAction(o, PLAYER_RESPECT1);
        break;
    case AT_SALUTE1:
        SetAction(o, PLAYER_SALUTE1);
        break;
    case AT_RUSH1:
        SetAction(o, PLAYER_RUSH1);
        break;
    default:
        break;
    }
    MoveCharacter(c, o);
    MoveCharacterVisual(c, o);
    m_iCurrentAnimation = iCurrentAni;
    return iCurrentAni;
}

CUIPhotoViewer::CUIPhotoViewer()
{
    m_bIsInitialized = FALSE;
    m_iSettingAnimation = 0;
    m_iCurrentFrame = 0;
    m_bActionRepeatCheck = FALSE;
    m_fSettingAngle = 0;
    m_fCurrentAngle = 0;
    m_fRotateClickPos_x = 0;
    m_fSettingZoom = 1.0f;
    m_fCurrentZoom = 1.0f;
    m_bHelpEnable = FALSE;
    m_bUpdatePlayer = FALSE;
    m_fPhotoHelperScale = 0;
    m_iCurrentAnimation = 0;
    m_bIsWebzenMail = FALSE;
}

CUIPhotoViewer::~CUIPhotoViewer()
{
    g_SummonSystem.RemoveEquipEffects(&m_PhotoChar);
    DeleteCloth(&m_PhotoChar, &m_PhotoChar.Object);
}

void CUIPhotoViewer::Init(int iInitType)
{
    if (iInitType < 0)		return;

    m_PhotoHelper.Initialize();

    m_PhotoChar.Initialize();

    CreateCharacterPointer(&m_PhotoChar, MODEL_PLAYER, (Hero->PositionX), (Hero->PositionY), 0);

    // 이동
    Vector(-300, -300, -300, m_PhotoChar.Object.Position);

    m_bIsInitialized = TRUE;
}

BOOL CompareItemEqual(const PART_t* item1, const PART_t* item2)
{
    return (item1->Type == item2->Type &&
        item1->Level == item2->Level &&
        item1->ExcellentFlags == item2->ExcellentFlags);
}

BOOL CompareItemEqual(const PART_t* item1, const ITEM* item2, int iDefaultValue)
{
    if (item2->Type == -1)
    {
        return (item1->Type == iDefaultValue &&
            item1->Level == item2->Level &&
            item1->ExcellentFlags == item2->ExcellentFlags);
    }
    else
    {
        return (item1->Type == item2->Type + MODEL_ITEM &&
            item1->Level == item2->Level &&
            item1->ExcellentFlags == item2->ExcellentFlags);
    }
}

void SetItemToPhoto(PART_t* itemDest, const ITEM* itemSrc, int iDefaultValue)
{
    if (itemSrc->Type == -1)
    {
        itemDest->Type = iDefaultValue;
        itemDest->Level = itemSrc->Level;
        itemDest->ExcellentFlags = itemSrc->ExcellentFlags;
    }
    else
    {
        itemDest->Type = itemSrc->Type + MODEL_ITEM;
        itemDest->Level = itemSrc->Level;
        itemDest->ExcellentFlags = itemSrc->ExcellentFlags;
    }
}

void CUIPhotoViewer::CopyPlayer()
{
    if (m_bIsInitialized == FALSE) return;

    if (m_PhotoChar.Class != Hero->Class)
    {
        m_PhotoChar.Class = Hero->Class;
        SetChangeClass(&m_PhotoChar);
    }

    int i;
    int maxClass = MAX_CLASS;

    BOOL bChangeArmor = FALSE;
    BOOL bChangeWeapon = FALSE;
    BOOL bChangeWing = FALSE;
    BOOL bChangeHelper = FALSE;
    if (Hero->Change == FALSE)
    {
        for (i = 0; i < MAX_BODYPART; ++i)
        {
            if (CompareItemEqual(&m_PhotoChar.BodyPart[i], &Hero->BodyPart[i]) == FALSE)
            {
                bChangeArmor = TRUE;
                break;
            }
        }
        for (i = 0; i < 2; ++i)
        {
            if (CompareItemEqual(&m_PhotoChar.Weapon[i], &Hero->Weapon[i]) == FALSE)
            {
                bChangeWeapon = TRUE;
                break;
            }
        }
        if (CompareItemEqual(&m_PhotoChar.Wing, &Hero->Wing) == FALSE)
            bChangeWing = TRUE;
        if (CompareItemEqual(&m_PhotoChar.Helper, &Hero->Helper) == FALSE)
            bChangeHelper = TRUE;
    }
    else	// 변신 상태
    {
        if (CompareItemEqual(&m_PhotoChar.BodyPart[BODYPART_HELM], &CharacterMachine->Equipment[EQUIPMENT_HELM],
            MODEL_BODY_HELM + Hero->SkinIndex) == FALSE) bChangeArmor = TRUE;
        else if (CompareItemEqual(&m_PhotoChar.BodyPart[BODYPART_ARMOR], &CharacterMachine->Equipment[EQUIPMENT_ARMOR],
            MODEL_BODY_ARMOR + Hero->SkinIndex) == FALSE) bChangeArmor = TRUE;
        else if (CompareItemEqual(&m_PhotoChar.BodyPart[BODYPART_PANTS], &CharacterMachine->Equipment[EQUIPMENT_PANTS],
            MODEL_BODY_PANTS + Hero->SkinIndex) == FALSE) bChangeArmor = TRUE;
        else if (CompareItemEqual(&m_PhotoChar.BodyPart[BODYPART_GLOVES], &CharacterMachine->Equipment[EQUIPMENT_GLOVES],
            MODEL_BODY_GLOVES + Hero->SkinIndex) == FALSE) bChangeArmor = TRUE;
        else if (CompareItemEqual(&m_PhotoChar.BodyPart[BODYPART_BOOTS], &CharacterMachine->Equipment[EQUIPMENT_BOOTS],
            MODEL_BODY_BOOTS + Hero->SkinIndex) == FALSE) bChangeArmor = TRUE;

        for (i = 0; i < 2; ++i)
        {
            if (CompareItemEqual(&m_PhotoChar.Weapon[i], &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT + i], -1) == FALSE)
            {
                bChangeWeapon = TRUE;
                break;
            }
        }
        if (CompareItemEqual(&m_PhotoChar.Wing, &CharacterMachine->Equipment[EQUIPMENT_WING], -1) == FALSE)
            bChangeWing = TRUE;
        if (CompareItemEqual(&m_PhotoChar.Helper, &CharacterMachine->Equipment[EQUIPMENT_HELPER], -1) == FALSE)
            bChangeHelper = TRUE;
    }

    if (bChangeArmor == FALSE && bChangeWeapon == FALSE && bChangeWing == FALSE && bChangeHelper == FALSE)
        return;

    if (Hero->Change == FALSE)
    {
        if (bChangeArmor == TRUE)
        {
            DeleteCloth(&m_PhotoChar, NULL, NULL);
            memcpy(&m_PhotoChar.BodyPart, &Hero->BodyPart, sizeof(PART_t) * MAX_BODYPART);
            for (i = 0; i < MAX_BODYPART; ++i)
            {
                m_PhotoChar.BodyPart[i].m_pCloth[0] = NULL;
                m_PhotoChar.BodyPart[i].m_pCloth[1] = NULL;
                m_PhotoChar.BodyPart[i].m_byNumCloth = 0;
            }
        }
        if (bChangeWeapon == TRUE)
        {
            memcpy(&m_PhotoChar.Weapon, &Hero->Weapon, sizeof(PART_t) * 2);
        }
        if (bChangeWing == TRUE)
        {
            memcpy(&m_PhotoChar.Wing, &Hero->Wing, sizeof(PART_t));
            DeleteCloth(NULL, &m_PhotoChar.Object, NULL);
        }
        if (bChangeHelper == TRUE)
        {
            memcpy(&m_PhotoChar.Helper, &Hero->Helper, sizeof(PART_t));
        }
    }
    else	// 변신 상태
    {
        if (bChangeArmor == TRUE)
        {
            DeleteCloth(&m_PhotoChar, NULL, NULL);

            m_PhotoChar.BodyPart[BODYPART_HEAD].Type = MODEL_BODY_HELM + Hero->SkinIndex;
            SetItemToPhoto(&m_PhotoChar.BodyPart[BODYPART_HELM], &CharacterMachine->Equipment[EQUIPMENT_HELM],
                MODEL_BODY_HELM + Hero->SkinIndex);
            SetItemToPhoto(&m_PhotoChar.BodyPart[BODYPART_ARMOR], &CharacterMachine->Equipment[EQUIPMENT_ARMOR],
                MODEL_BODY_ARMOR + Hero->SkinIndex);
            SetItemToPhoto(&m_PhotoChar.BodyPart[BODYPART_PANTS], &CharacterMachine->Equipment[EQUIPMENT_PANTS],
                MODEL_BODY_PANTS + Hero->SkinIndex);
            SetItemToPhoto(&m_PhotoChar.BodyPart[BODYPART_GLOVES], &CharacterMachine->Equipment[EQUIPMENT_GLOVES],
                MODEL_BODY_GLOVES + Hero->SkinIndex);
            SetItemToPhoto(&m_PhotoChar.BodyPart[BODYPART_BOOTS], &CharacterMachine->Equipment[EQUIPMENT_BOOTS],
                MODEL_BODY_BOOTS + Hero->SkinIndex);

            for (i = 0; i < MAX_BODYPART; ++i)
            {
                m_PhotoChar.BodyPart[i].m_pCloth[0] = NULL;
                m_PhotoChar.BodyPart[i].m_pCloth[1] = NULL;
                m_PhotoChar.BodyPart[i].m_byNumCloth = 0;
            }
        }
        if (bChangeWeapon == TRUE)
        {
            for (i = 0; i < 2; ++i)
            {
                SetItemToPhoto(&m_PhotoChar.Weapon[i], &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT + i], -1);
            }
        }
        if (bChangeWing == TRUE)
        {
            SetItemToPhoto(&m_PhotoChar.Wing, &CharacterMachine->Equipment[EQUIPMENT_WING], -1);
        }
        if (bChangeHelper == TRUE)
        {
            SetItemToPhoto(&m_PhotoChar.Helper, &CharacterMachine->Equipment[EQUIPMENT_HELPER], -1);
        }
    }

    if (bChangeHelper == TRUE)
    {
        SetPhotoPose(AT_STAND1);
        m_iSettingAnimation = AT_STAND1;
    }
    else
    {
        SetPhotoPose(m_iCurrentAnimation);
    }

    if (bChangeHelper == TRUE || bChangeWeapon == TRUE)
    {
        m_PhotoHelper.Live = false;
        switch (m_PhotoChar.Helper.Type - MODEL_HELPER)
        {
        case 0:CreateMountSub(MODEL_HELPER, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper); break;
        case 2:CreateMountSub(MODEL_UNICON, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper); break;
        case 3:CreateMountSub(MODEL_PEGASUS, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper); break;
        case 4:CreateMountSub(MODEL_DARK_HORSE, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper); break;
        case 37:	//^ 펜릴 편지 관련
            if (m_PhotoChar.Helper.ExcellentFlags == 0x01)
            {
                CreateMountSub(MODEL_FENRIR_BLACK, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper);
            }
            else if (m_PhotoChar.Helper.ExcellentFlags == 0x02)
            {
                CreateMountSub(MODEL_FENRIR_BLUE, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper);
            }
            else if (m_PhotoChar.Helper.ExcellentFlags == 0x04)
            {
                CreateMountSub(MODEL_FENRIR_GOLD, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper);
            }
            else
            {
                CreateMountSub(MODEL_FENRIR_RED, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper);
            }
            break;
        }
        m_PhotoHelper.Alpha = 0;
        m_fPhotoHelperScale = m_PhotoHelper.Scale * 0.7f / m_PhotoChar.Object.Scale;
    }
}

void CUIPhotoViewer::SetClass(CLASS_TYPE byClass)
{
    if (m_bIsInitialized == FALSE) return;
    CHARACTER* c = CharactersClient;
    CharactersClient = &m_PhotoChar;
    CharactersClient->Class = byClass;
    SetChangeClass(CharactersClient);
    CharactersClient = c;
}

void CUIPhotoViewer::SetEquipmentPacket(BYTE* pbyEquip)
{
    if (m_bIsInitialized == FALSE) return;
    //CHARACTER *c = CharactersClient;
    //CharactersClient = &m_PhotoChar;

    ReadEquipmentExtended(0, 0, pbyEquip, &m_PhotoChar, &m_PhotoHelper);

    m_fPhotoHelperScale = m_PhotoHelper.Scale * 0.7f / Hero->Object.Scale;

    if (m_PhotoChar.Wing.Type != -1 && m_iSettingAnimation > AT_HEALING1)
        m_PhotoChar.SafeZone = true;
    else
        m_PhotoChar.SafeZone = false;
}

void CUIPhotoViewer::SetAngle(float fDegree)
{
    if (m_bIsInitialized == FALSE) return;
    OBJECT* o = &m_PhotoChar.Object;
    Vector(-20.f, 5.f, 60.f, o->Angle);

    m_fSettingAngle = fDegree;
    m_fCurrentAngle = fDegree;
}

void CUIPhotoViewer::SetZoom(float fZoom)
{
    m_fSettingZoom = fZoom;
    m_fCurrentZoom = fZoom;
}

void CUIPhotoViewer::SetAnimation(int iAnimationType)
{
    m_iSettingAnimation = iAnimationType;
    SetPhotoPose(m_iSettingAnimation);
}

void CUIPhotoViewer::ChangeAnimation(int iMoveDir)
{
    m_iSettingAnimation = SetPhotoPose(m_iSettingAnimation, iMoveDir);
    m_iCurrentFrame = 0;
    m_bActionRepeatCheck = TRUE;
}

void CUIPhotoViewer::SetID(const wchar_t* pszID)
{
    if (pszID == NULL) return;
    swprintf(m_PhotoChar.ID, pszID);
}

extern bool EquipmentSuccess;

BOOL CUIPhotoViewer::DoMouseAction()
{
    if (m_bIsWebzenMail == TRUE)
    {
        if (CheckMouseIn(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight) == TRUE)
        {
            MouseOnWindow = true;
        }
        return TRUE;
    }

    if (m_bUpdatePlayer == TRUE && EquipmentSuccess == true)
    {
        CopyPlayer();
    }

    m_PhotoChar.EtcPart = Hero->EtcPart;

    if (CheckOption(UIPHOTOVIEWER_CANCONTROL))
    {
        if (GetState() == UISTATE_NORMAL && CheckMouseIn(m_iPos_x + 1, m_iPos_y + m_iHeight - 17, 16, 16) == TRUE)
        {
            MouseOnWindow = true;
            if (MouseLButtonPush)
            {
                m_bHelpEnable = (m_bHelpEnable + 1) % 2;
                MouseLButtonPush = FALSE;
                MouseLButton = FALSE;
            }
        }
        else if (CheckMouseIn(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight) == TRUE)
        {
            MouseOnWindow = true;
            if (MouseLButtonPush)
            {
                m_bHelpEnable = FALSE;
                if (GetState() == UISTATE_NORMAL && g_dwActiveUIID == 0)
                {
                    g_dwActiveUIID = GetUIID();
                    SetState(UISTATE_SCROLL);
                    m_fRotateClickPos_x = MouseX;
                    SetFocus(g_hWnd);
                }
            }
            else if (MouseRButtonPush)
            {
                m_bHelpEnable = FALSE;
                m_fCurrentAngle = m_fSettingAngle;
                m_fCurrentZoom = m_fSettingZoom;
            }
            else if (MouseWheel != 0)
            {
                m_bHelpEnable = FALSE;
                m_fCurrentZoom += MouseWheel / 50.0f;
                if (m_fCurrentZoom > 1.1f) m_fCurrentZoom = 1.1f;
                else if (m_fCurrentZoom < 0.8f) m_fCurrentZoom = 0.8f;
                MouseWheel = 0;
            }
        }
        if (GetState() == UISTATE_SCROLL)
        {
            if (MouseLButtonPush)
            {
                MouseOnWindow = true;
                m_fCurrentAngle += (MouseX - m_fRotateClickPos_x);
                m_fRotateClickPos_x = MouseX;
            }
            else
            {
                SetState(UISTATE_NORMAL);
                if (g_dwActiveUIID == GetUIID()) g_dwActiveUIID = 0;
            }
        }
    }
    else
    {
        if (GetState() == UISTATE_NORMAL && CheckMouseIn(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight) == TRUE)
        {
            MouseOnWindow = true;
            if (MouseLButtonPush)
            {
                MouseLButtonPush = FALSE;
                MouseLButton = FALSE;
            }
        }
    }
    return TRUE;
}

extern int TextNum;
extern wchar_t TextList[50][100];
extern int  TextListColor[50];
extern int  TextBold[50];
extern SIZE Size[50];

void CUIPhotoViewer::Render()
{
    if (m_bIsWebzenMail == TRUE)
    {
        glColor4f(0.f, 0.f, 0.f, 1.0f);
        RenderColor(m_iPos_x, m_iPos_y, 119.f, 141.f);
        EndRenderColor();
        RenderBitmap(BITMAP_INTERFACE_EX + 22, m_iPos_x + 20, m_iPos_y + 38, 80.f, 62.f, 0.f, 0.f, 256.f / 256.f, 195.f / 256.f);
        return;
    }

    CHARACTER* c = &m_PhotoChar;
    OBJECT* o = &c->Object;

    if (o->AnimationFrame < m_iCurrentFrame)
    {
        if (m_bActionRepeatCheck == FALSE && rand_fps_check(4))
        {
            m_bActionRepeatCheck = TRUE;
            SetPhotoPose(m_iSettingAnimation);
        }
        else
        {
            if (m_iSettingAnimation >= AT_STAND1 && m_iSettingAnimation <= AT_HEALING1);
            else
            {
                m_bActionRepeatCheck = FALSE;
                SetPhotoPose(AT_STAND1);
            }
        }
        m_iCurrentFrame = 0;
    }
    else m_iCurrentFrame = o->AnimationFrame;

    if (c->EtcPart < PARTS_LION)
    {
        DeleteParts(&m_PhotoChar);
    }
    RenderPhotoCharacter();

    if (CheckOption(UIPHOTOVIEWER_CANCONTROL))
    {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        DisableAlphaBlend();
        if (m_bHelpEnable == FALSE)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 20, m_iPos_x + 1, m_iPos_y + m_iHeight - 17, 16.0f, 16.0f, 0.f, 0.f, 16.f / 16.f, 16.f / 16.f);
        }
        else
        {
            glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 20, m_iPos_x + 2, m_iPos_y + m_iHeight - 16, 15.0f, 15.0f, 0.f, 0.f, 15.f / 16.f, 15.f / 16.f);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            TextNum = 0;
            swprintf(TextList[TextNum], GlobalText[997]); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[998]); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[999]); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++;
            SIZE TextSize;
            GetTextExtentPoint32(g_pRenderText->GetFontDC(), L"Z", 1, &TextSize);
            TextSize.cy /= g_fScreenRate_y;
            RenderTipTextList(m_iPos_x + m_iWidth / 2, m_iPos_y + m_iHeight - TextNum * (TextSize.cy + 2), TextNum, 0, RT3_SORT_LEFT);
        }
    }
}

void CUILetterWriteWindow::InitControls()
{
    SIZE size;
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), GlobalText[1000], GlobalText.GetStringSize(1000), &size);

    size.cx = (size.cx / g_fScreenRate_x) + 0.5f;

    m_MailtoInputBox.Init(g_hWnd, 238, 14, 50);
    m_MailtoInputBox.SetParentUIID(m_dwUIID);
    m_MailtoInputBox.SetFont(g_hFont);
    m_MailtoInputBox.SetOption(UIOPTION_NULL);
    m_MailtoInputBox.SetBackColor(0, 0, 0, 0);
    m_MailtoInputBox.SetTextLimit(MAX_ID_SIZE);
    m_MailtoInputBox.SetParentUIID(GetUIID());
    m_MailtoInputBox.SetArrangeType(0, size.cx, 3);
    m_MailtoInputBox.SetState(UISTATE_NORMAL);

    m_TitleInputBox.Init(g_hWnd, 238, 14, 50);
    m_TitleInputBox.SetParentUIID(m_dwUIID);
    m_TitleInputBox.SetFont(g_hFont);
    m_TitleInputBox.SetOption(UIOPTION_NULL);
    m_TitleInputBox.SetBackColor(0, 0, 0, 0);
    m_TitleInputBox.SetTextLimit(MAX_LETTER_TITLE_LENGTH - 1);
    m_TitleInputBox.SetParentUIID(GetUIID());
    m_TitleInputBox.SetArrangeType(0, size.cx, 18);
    m_TitleInputBox.SetState(UISTATE_NORMAL);

    m_TextInputBox.SetMultiline(TRUE);
    m_TextInputBox.Init(g_hWnd, 238, 135, 50);
    m_TextInputBox.SetTextLimit(MAX_LETTERTEXT_LENGTH - 1);
    m_TextInputBox.SetParentUIID(m_dwUIID);
    m_TextInputBox.SetFont(g_hFont);
    m_TextInputBox.SetOption(UIOPTION_NULL);
    m_TextInputBox.SetBackColor(0, 0, 0, 0);
    m_TextInputBox.SetParentUIID(GetUIID());
    m_TextInputBox.SetArrangeType(0, 5, 33);
    m_TextInputBox.SetState(UISTATE_NORMAL);

    m_MailtoInputBox.SetTabTarget(&m_TitleInputBox);
    m_TitleInputBox.SetTabTarget(&m_TextInputBox);
    m_TextInputBox.SetTabTarget(&m_MailtoInputBox);

    m_SendButton.Init(1, GlobalText[1001]);
    m_SendButton.SetParentUIID(GetUIID());
    m_SendButton.SetArrangeType(2, 12, 16);
    m_SendButton.SetSize(50, 14);

    m_CloseButton.Init(2, GlobalText[1002]);
    m_CloseButton.SetParentUIID(GetUIID());
    m_CloseButton.SetArrangeType(2, 63, 16);
    m_CloseButton.SetSize(50, 14);

    //	m_PhotoShowButton.Init(3, GlobalText[1064]);
    //	m_PhotoShowButton.SetParentUIID(GetUIID());
    //	m_PhotoShowButton.SetArrangeType(2, 114, 16);
    //	m_PhotoShowButton.SetSize(50, 14);

    m_PrevPoseButton.Init(4, GlobalText[1003]);
    m_PrevPoseButton.SetParentUIID(GetUIID());
    m_PrevPoseButton.SetArrangeType(2, 250, 16);
    m_PrevPoseButton.SetSize(50, 14);

    m_NextPoseButton.Init(5, GlobalText[1004]);
    m_NextPoseButton.SetParentUIID(GetUIID());
    m_NextPoseButton.SetArrangeType(2, 301, 16);
    m_NextPoseButton.SetSize(50, 14);

    m_MailtoInputBox.GiveFocus();
    m_iLastTabIndex = 0;
    m_bHaveTextBox = TRUE;

    m_Photo.Init(0);
    m_Photo.SetOption(UIPHOTOVIEWER_CANCONTROL);
    m_Photo.SetParentUIID(GetUIID());
    m_Photo.SetArrangeType(1, 119, 30);
    m_Photo.SetSize(119, RHeight() - 49);
    m_Photo.CopyPlayer();
    m_Photo.SetAutoupdatePlayer(TRUE);
    m_Photo.SetAnimation(AT_STAND1);
    m_Photo.SetAngle(90);
    Refresh();
}

void CUILetterWriteWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    SetTitle(pszTitle);
    SetParentUIID(dwParentID);

    SetPosition(50, 50);
    SetSize(250, 216);
    SetLimitSize(250, 150);
    SetOption(UIWINDOWSTYLE_TITLEBAR | UIWINDOWSTYLE_FRAME | UIWINDOWSTYLE_MOVEABLE | UIWINDOWSTYLE_MINBUTTON);

    m_iShowType = 1;
    SetSize(GetWidth() + 120, GetHeight());
    SetLimitSize(250 + 120, 150);
}

void CUILetterWriteWindow::Refresh()
{
    m_MailtoInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_TitleInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_TextInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_SendButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_CloseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    //m_PhotoShowButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_PrevPoseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_NextPoseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_Photo.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
}

void CUILetterWriteWindow::SetMailtoText(const wchar_t* pszText)
{
    m_MailtoInputBox.SetText(pszText);
    m_TitleInputBox.GiveFocus();
    m_iLastTabIndex = 1;
}

void CUILetterWriteWindow::SetMainTitleText(const wchar_t* pszText)
{
    m_TitleInputBox.SetText(pszText);
    m_TextInputBox.GiveFocus();
    m_iLastTabIndex = 2;
}

void CUILetterWriteWindow::SetMailContextText(const wchar_t* pszText)
{
    m_TextInputBox.SetText(pszText);
}

void CUILetterWriteWindow::RenderSub()
{
    EnableAlphaTest();
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_SendButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_CloseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_MailtoInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_TitleInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_TextInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        //m_PhotoShowButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_PrevPoseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_NextPoseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_Photo.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

        //		if (GetState() == UISTATE_RESIZE)
        //		{
        //			m_TextInputBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
        //		}
    }

    SetLineColor(7);
    RenderColor((float)RPos_x(0), (float)RPos_y(0), (float)RWidth(), 29.0f);
    SetLineColor(2);
    RenderColor((float)RPos_x(0), (float)RPos_y(14), (float)RWidth(), 1.0f);
    RenderColor((float)RPos_x(0), (float)RPos_y(29), (float)RWidth(), 1.0f);

    SetLineColor(7);
    RenderColor((float)RPos_x(0), (float)RPos_y(0) + RHeight() - 19, (float)RWidth(), 19.0f);
    SetLineColor(2);
    RenderColor((float)RPos_x(0), (float)RPos_y(0) + RHeight() - 19, (float)RWidth(), 1.0f);
    EndRenderColor();

    SIZE size;

    g_pRenderText->SetTextColor(230, 220, 200, 255);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), GlobalText[1000], GlobalText.GetStringSize(1000), &size);
    g_pRenderText->RenderText(RPos_x(3), RPos_y(3), GlobalText[1000], size.cx / g_fScreenRate_x, 0, RT3_SORT_RIGHT);
    g_pRenderText->RenderText(RPos_x(3), RPos_y(18), GlobalText[1005], size.cx / g_fScreenRate_x, 0, RT3_SORT_RIGHT);

    m_MailtoInputBox.Render();
    m_TitleInputBox.Render();
    m_TextInputBox.Render();
    DisableAlphaBlend();

    m_SendButton.Render();
    m_CloseButton.Render();
    //m_PhotoShowButton.Render();
    if (m_iShowType == 1)
    {
        m_PrevPoseButton.Render();
        m_NextPoseButton.Render();
    }
}

void CUILetterWriteWindow::RenderOver()
{
    if (m_iShowType == 1)
    {
        m_Photo.Render();
    }
}

BOOL CUILetterWriteWindow::HandleMessage()
{
    switch (m_WorkMessage.m_iMessage)
    {
    case UI_MESSAGE_SELECTED:
        switch (m_iLastTabIndex)
        {
        case 0:
            m_MailtoInputBox.GiveFocus();
            break;
        case 1:
            m_TitleInputBox.GiveFocus();
            m_iLastTabIndex = 0;
            break;
        case 2:
            m_TextInputBox.GiveFocus();
            m_iLastTabIndex = 0;
            break;
        default:
            break;
        }
        break;
    case UI_MESSAGE_BTNLCLICK:
    {
        if (g_dwTopWindow != 0) break;

        DWORD dwUIID = 0;

        switch (m_WorkMessage.m_iParam1)
        {
        case 1:
            if (m_bIsSend == FALSE)
            {
                wchar_t	szMailto[MAX_ID_SIZE + 1] = { '\0' };
                wchar_t	szTitle[MAX_LETTER_TITLE_LENGTH] = { '\0' };
                wchar_t	szTempText[MAX_LETTERTEXT_LENGTH] = { '\0' };

                //std::wstring	strTitle = "", strText = "";
                std::wstring	wstrTitle = L"", wstrText = L"";
                int k = 0;

                m_MailtoInputBox.GetText(szMailto, MAX_ID_SIZE + 1);
                m_TitleInputBox.GetText(szTitle, MAX_LETTER_TITLE_LENGTH);
                m_TextInputBox.GetText(szTempText, MAX_LETTERTEXT_LENGTH);

                //for (k = 0; k < MAX_LETTER_TITLE_LENGTH_UTF16 + 1; k++)
                //    szTitleUTF16[k] = g_pMultiLanguage->ConvertFulltoHalfWidthChar(szTitleUTF16[k]);
                //for (k = 0; k < MAX_LETTER_TEXT_LENGTH_UTF16 + 1; k++)
                //    szTextUTF16[k] = g_pMultiLanguage->ConvertFulltoHalfWidthChar(szTextUTF16[k]);

                //wstrTitle = szTitleUTF16;
                //wstrText = szTextUTF16;

                // delete memory
                //delete[] szTitleUTF16;	delete[] szTextUTF16;

                //g_pMultiLanguage->ConvertWideCharToStr(strTitle, wstrTitle.c_str(), g_pMultiLanguage->GetCodePage());
                //g_pMultiLanguage->ConvertWideCharToStr(strText, wstrText.c_str(), g_pMultiLanguage->GetCodePage());
                //wcsncpy(szTitle, strTitle.c_str(), sizeof szTitle);
                //wcsncpy(szTempText, strText.c_str(), sizeof szTempText);

                //if (CheckAbuseFilter(wstrTitle, false))
                //    g_pMultiLanguage->ConvertCharToWideStr(wstrTitle, GlobalText[570]);
                //if (CheckAbuseFilter(wstrText, false))
                //    g_pMultiLanguage->ConvertCharToWideStr(wstrText, GlobalText[570]);

                //g_pMultiLanguage->ConvertWideCharToStr(strTitle, wstrTitle.c_str(), CP_UTF8);
                //g_pMultiLanguage->ConvertWideCharToStr(strText, wstrText.c_str(), CP_UTF8);
                /*wcsncpy(szTitle, strTitle.c_str(), sizeof szTitle);
                wcsncpy(szTempText, strText.c_str(), sizeof szTempText);*/

                if (szMailto[0] == '\0' || wcslen(szMailto) == 0)
                {
                    g_pWindowMgr->AddWindow(UIWNDTYPE_OK, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1006]);
                    m_MailtoInputBox.GiveFocus();
                    m_iLastTabIndex = 0;
                    break;
                }

                if (szTitle[0] == '\0' || wcslen(szTitle) == 0)
                {
                    g_pWindowMgr->AddWindow(UIWNDTYPE_OK, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1007]);
                    m_TitleInputBox.GiveFocus();
                    m_iLastTabIndex = 1;
                    break;
                }

                if (szTempText[0] == '\0' || wcslen(szTempText) == 0)
                {
                    g_pWindowMgr->AddWindow(UIWNDTYPE_OK, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1008]);
                    m_TextInputBox.GiveFocus();
                    m_iLastTabIndex = 2;
                    break;
                }

                wchar_t szText[1024] = { 0 };

                for (int i = 0, j = 0; i <= (int)wcslen(szTempText); ++i, ++j)
                {
                    if (j > MAX_LETTERTEXT_LENGTH || i > MAX_LETTERTEXT_LENGTH) break;

                    if (szTempText[i] == '\r')
                    {
                        szText[j] = '\n';
                        if (szTempText[i + 1] == '\n' && szTempText[i + 2] == '\r' && szTempText[i + 3] == '\n')
                        {
                            szText[++j] = ' ';
                        }
                        ++i;
                    }
                    else
                    {
                        szText[j] = szTempText[i];
                    }
                }

                szText[MAX_LETTERTEXT_LENGTH] = '\0';
                WORD len = min(MAX_LETTERTEXT_LENGTH, wcslen(szText));
                m_bIsSend = TRUE;
                int iAngle = m_Photo.GetCurrentAngle() / 6;
                int iZoom = (m_Photo.GetCurrentZoom() * 100.0f - 80 + 5) / 10;
                BYTE Data1 = (iZoom << 6) & 0xC0 | iAngle & 0x3F;
                BYTE Data2 = m_Photo.GetCurrentAction() - AT_ATTACK1;
                SocketClient->ToGameServer()->SendLetterSendRequest(GetUIID(), szMailto, szTitle, Data1, Data2, len, szText);
            }
            break;
        case 2:
            if (CloseCheck() == TRUE)
                g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
            break;
        case 3:
            break;
        case 4:
            m_Photo.ChangeAnimation(-1);
            break;
        case 5:
            m_Photo.ChangeAnimation(1);
            break;
        default:
            break;
        }
    }
    break;
    case UI_MESSAGE_YNRETURN:
        if (m_WorkMessage.m_iParam2 == 1)
        {
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
        }
        break;
    default:
        break;
    }
    return FALSE;
}

void CUILetterWriteWindow::DoActionSub(BOOL bMessageOnly)
{
    m_MailtoInputBox.DoAction(bMessageOnly);
    m_TitleInputBox.DoAction(bMessageOnly);
    m_TextInputBox.DoAction(bMessageOnly);
    m_SendButton.DoAction(bMessageOnly);
    m_CloseButton.DoAction(bMessageOnly);
    //m_PhotoShowButton.DoAction(bMessageOnly);
    if (m_iShowType == 1)
    {
        m_PrevPoseButton.DoAction(bMessageOnly);
        m_NextPoseButton.DoAction(bMessageOnly);
        m_Photo.SetShowType(m_iShowType);
        m_Photo.DoAction(bMessageOnly);
    }
}

void CUILetterWriteWindow::DoMouseActionSub()
{
}

BOOL CUILetterWriteWindow::CloseCheck()
{
    wchar_t szTest[16] = { 0 };
    m_TitleInputBox.GetText(szTest, 15);
    int iTextSize = (szTest[0] == '\0' ? 0 : wcslen(szTest));
    m_TextInputBox.GetText(szTest, 15);
    iTextSize += (szTest[0] == '\0' ? 0 : wcslen(szTest));
    if (iTextSize == 0)
    {
        return TRUE;
    }
    else
    {
        g_pWindowMgr->AddWindow(UIWNDTYPE_QUESTION, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1009], GetUIID());
        return FALSE;
    }
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
    //SetSize(213, 170);
    SetSize(250, 182);
    SetLimitSize(250, 182);
    SetOption(UIWINDOWSTYLE_TITLEBAR | UIWINDOWSTYLE_FRAME | UIWINDOWSTYLE_MOVEABLE | UIWINDOWSTYLE_MINBUTTON);

    m_LetterTextBox.SetParentUIID(GetUIID());
    m_LetterTextBox.SetArrangeType(2, 0, 20);
    m_LetterTextBox.SetResizeType(3, 0, -36);

    m_ReplyButton.Init(1, GlobalText[1010]);
    m_ReplyButton.SetParentUIID(GetUIID());
    m_ReplyButton.SetArrangeType(2, 2, 16);
    m_ReplyButton.SetSize(50, 14);

    m_DeleteButton.Init(2, GlobalText[1011]);
    m_DeleteButton.SetParentUIID(GetUIID());
    m_DeleteButton.SetArrangeType(2, 53, 16);
    m_DeleteButton.SetSize(50, 14);

    m_CloseButton.Init(3, GlobalText[1002]);
    m_CloseButton.SetParentUIID(GetUIID());
    m_CloseButton.SetArrangeType(2, 186, 16);
    m_CloseButton.SetSize(50, 14);

    //	m_PhotoButton.Init(4, L">>");
    //	m_PhotoButton.SetParentUIID(GetUIID());
    //	m_PhotoButton.SetArrangeType(2, 186, 16);
    //	m_PhotoButton.SetSize(50, 14);

    m_PrevButton.Init(5, GlobalText[1012]);
    m_PrevButton.SetParentUIID(GetUIID());
    m_PrevButton.SetArrangeType(2, 104, 16);
    m_PrevButton.SetSize(40, 14);

    m_NextButton.Init(6, GlobalText[1013]);
    m_NextButton.SetParentUIID(GetUIID());
    m_NextButton.SetArrangeType(2, 145, 16);
    m_NextButton.SetSize(40, 14);

    m_Photo.Init(0);
    m_Photo.SetOption(UIPHOTOVIEWER_CANCONTROL);
    m_Photo.SetParentUIID(GetUIID());
    m_Photo.SetArrangeType(1, 119, 15);
    m_Photo.SetResizeType(2, 119, -15);

    m_LetterTextBox.SetResizeType(3, 0 - 120, -36);
    SetSize(GetWidth() + 120, GetHeight());
    SetLimitSize(250 + 120, 182);
}

void CUILetterReadWindow::Refresh()
{
    m_LetterTextBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_ReplyButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_DeleteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_CloseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    //m_PhotoButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_PrevButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_NextButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_Photo.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

    m_LetterTextBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_LetterTextBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_Photo.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
}

void CUILetterReadWindow::SetLetter(LETTERLIST_TEXT* pLetterHead, const wchar_t* pLetterText)
{
    memcpy(&m_LetterHead, pLetterHead, sizeof(LETTERLIST_TEXT));

    wchar_t* temp = new wchar_t[wcslen(pLetterText) + 20];
    wcsncpy(temp, pLetterText, wcslen(pLetterText) + 1);
    wchar_t* token = _wcstok(temp, L"\n");
    while (token != NULL)
    {
        m_LetterTextBox.AddText(token);
        token = _wcstok(NULL, L"\n");
    }
    m_LetterTextBox.SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);
    delete[] temp;
}

void CUILetterReadWindow::RenderSub()
{
    EnableAlphaTest();
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_LetterTextBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_ReplyButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_DeleteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_CloseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_PrevButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_NextButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_Photo.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

        if (GetState() == UISTATE_RESIZE)
        {
            m_LetterTextBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
            m_Photo.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
        }
    }

    m_LetterTextBox.Render();

    if (m_iShowType >= 2)
    {
        SetLineColor(2);
        RenderColor((float)(RPos_x(0) + RWidth() - 120), (float)RPos_y(0) + RHeight() - 19, 1, 19);
        RenderColor((float)RPos_x(0), (float)RPos_y(0) + RHeight() - 20, (float)RWidth() - 120, 1.0f);
        SetLineColor(7);
        RenderColor((float)RPos_x(0), (float)RPos_y(0) + RHeight() - 19, (float)RWidth() - 120, 18.0f);
    }
    else
    {
        SetLineColor(2);
        RenderColor((float)RPos_x(0), (float)RPos_y(0) + RHeight() - 20, (float)RWidth(), 1.0f);
        SetLineColor(7);
        RenderColor((float)RPos_x(0), (float)RPos_y(0) + RHeight() - 19, (float)RWidth(), 18.0f);
    }

    SetLineColor(2);
    RenderColor((float)RPos_x(0), (float)RPos_y(14), (float)RWidth(), 1.0f);
    SetLineColor(7);
    RenderColor((float)RPos_x(0), (float)RPos_y(0), (float)RWidth(), 14.0f);
    EndRenderColor();

    wchar_t szMailFrom[256] = { 0 };
    swprintf(szMailFrom, GlobalText[1014], m_LetterHead.m_szID, m_LetterHead.m_szDate, m_LetterHead.m_szTime);
    g_pRenderText->RenderText(RPos_x(3), RPos_y(3), szMailFrom);

    m_ReplyButton.Render();
    m_DeleteButton.Render();
    m_CloseButton.Render();
    m_PrevButton.Render();
    m_NextButton.Render();

    DisableAlphaBlend();
}

void CUILetterReadWindow::RenderOver()
{
    if (m_iShowType >= 2)
    {
        m_Photo.Render();
    }
}

BOOL CUILetterReadWindow::HandleMessage()
{
    switch (m_WorkMessage.m_iMessage)
    {
    case UI_MESSAGE_SELECTED:
        break;
    case UI_MESSAGE_BTNLCLICK:
    {
        if (g_dwTopWindow != 0) break;
        DWORD dwUIID = 0;
        switch (m_WorkMessage.m_iParam1)
        {
        case 1:
        {
            wchar_t temp[MAX_TEXT_LENGTH + 1];
            swprintf(temp, GlobalText[1071], g_cdwLetterCost);

            dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_WRITELETTER, 100, 100, temp);
            if (dwUIID == 0) break;
            ((CUILetterWriteWindow*)g_pWindowMgr->GetWindow(dwUIID))->SetMailtoText(m_LetterHead.m_szID);
            wchar_t szMailTitle[MAX_TEXT_LENGTH + 1] = { 0 };
            swprintf(szMailTitle, GlobalText[1016], m_LetterHead.m_szText);
            wchar_t szMailTitleResult[32 + 1] = { 0 };
            CutText4(szMailTitle, szMailTitleResult, NULL, 32);
            ((CUILetterWriteWindow*)g_pWindowMgr->GetWindow(dwUIID))->SetMainTitleText(szMailTitleResult);
        }
        break;
        case 2:
        {
            wchar_t tempTxt[MAX_TEXT_LENGTH + 1] = { 0 };
            wcscat(tempTxt, GlobalText[1017]);
            dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_QUESTION, UIWND_DEFAULT, UIWND_DEFAULT, tempTxt, GetUIID());
        }
        break;
        case 3:
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
            break;
        case 4:
            break;
        case 5:
        {
            DWORD dwPrevID = g_pLetterList->GetPrevLetterID(m_LetterHead.m_dwLetterID);
            if (dwPrevID != 0)
            {
                if (g_pWindowMgr->GetFriendMainWindow() != NULL)
                {
                    g_pWindowMgr->GetFriendMainWindow()->PrevNextCursorMove(g_pLetterList->GetLineNum(dwPrevID));
                }
                g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
                g_pWindowMgr->CloseLetterRead(m_LetterHead.m_dwLetterID);
                g_iLetterReadNextPos_x = GetPosition_x();
                g_iLetterReadNextPos_y = GetPosition_y();
                if (g_pWindowMgr->LetterReadCheck(dwPrevID) == FALSE)
                {
                    if (g_pLetterList->GetLetterText(dwPrevID) == NULL)
                    {
                        SocketClient->ToGameServer()->SendLetterReadRequest(dwPrevID);
                    }
                    else
                    {
                        auto data = reinterpret_cast<const BYTE*>(g_pLetterList->GetLetterText(dwPrevID));
                        ReceiveLetterText(std::span(data, sizeof(FS_LETTER_TEXT)), true);
                    }
                }
                else
                {
                    DWORD dwFindUIID = g_pWindowMgr->GetLetterReadWindow(dwPrevID);
                    if (dwFindUIID != 0)
                        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, dwFindUIID, 0);
                }
            }
            else
            {
                if (g_pWindowMgr->GetFriendMainWindow() != NULL)
                {
                    g_pWindowMgr->GetFriendMainWindow()->PrevNextCursorMove(g_pLetterList->GetLineNum(m_LetterHead.m_dwLetterID));
                }
            }
        }
        break;
        case 6:
        {
            DWORD dwNextID = g_pLetterList->GetNextLetterID(m_LetterHead.m_dwLetterID);
            if (dwNextID != 0)
            {
                if (g_pWindowMgr->GetFriendMainWindow() != NULL)
                {
                    g_pWindowMgr->GetFriendMainWindow()->PrevNextCursorMove(g_pLetterList->GetLineNum(dwNextID));
                }
                g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
                g_pWindowMgr->CloseLetterRead(m_LetterHead.m_dwLetterID);
                g_iLetterReadNextPos_x = GetPosition_x();
                g_iLetterReadNextPos_y = GetPosition_y();
                if (g_pWindowMgr->LetterReadCheck(dwNextID) == FALSE)
                {
                    if (g_pLetterList->GetLetterText(dwNextID) == NULL)
                    {
                        SocketClient->ToGameServer()->SendLetterReadRequest(dwNextID);
                    }
                    else
                    {
                        auto data = reinterpret_cast<const BYTE*>(g_pLetterList->GetLetterText(dwNextID));
                        ReceiveLetterText(std::span(data, sizeof(FS_LETTER_TEXT)), true);
                    }
                }
                else
                {
                    DWORD dwFindUIID = g_pWindowMgr->GetLetterReadWindow(dwNextID);
                    if (dwFindUIID != 0)
                        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, dwFindUIID, 0);
                }
            }
            else
            {
                if (g_pWindowMgr->GetFriendMainWindow() != NULL)
                {
                    g_pWindowMgr->GetFriendMainWindow()->PrevNextCursorMove(g_pLetterList->GetLineNum(m_LetterHead.m_dwLetterID));
                }
            }
        }
        break;
        default:
            break;
        }
    }
    break;
    case UI_MESSAGE_YNRETURN:
        if (m_WorkMessage.m_iParam2 == 1)
        {
            SocketClient->ToGameServer()->SendLetterDeleteRequest(m_LetterHead.m_dwLetterID);
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
        }
        break;
    default:
        break;
    }
    return FALSE;
}

void CUILetterReadWindow::DoActionSub(BOOL bMessageOnly)
{
    m_LetterTextBox.DoAction(bMessageOnly);
    m_ReplyButton.DoAction(bMessageOnly);
    m_DeleteButton.DoAction(bMessageOnly);
    m_CloseButton.DoAction(bMessageOnly);
    //m_PhotoButton.DoAction(bMessageOnly);
    m_PrevButton.DoAction(bMessageOnly);
    m_NextButton.DoAction(bMessageOnly);
    m_Photo.SetShowType(m_iShowType);
    m_Photo.DoAction(bMessageOnly);
}

void CUILetterReadWindow::DoMouseActionSub()
{
}

void CFriendList::AddFriend(const wchar_t* pszID, BYTE Number, BYTE Server)
{
    static GUILDLIST_TEXT FriendData;
    wcsncpy(FriendData.m_szID, pszID, MAX_ID_SIZE);
    FriendData.m_szID[MAX_ID_SIZE] = '\0';
    FriendData.m_Number = Number;
    FriendData.m_Server = Server;

    m_FriendList.insert(m_FriendList.end(), FriendData);
}

void CFriendList::RemoveFriend(const wchar_t* pszID)
{
    for (m_FriendListIter = m_FriendList.begin(); m_FriendListIter != m_FriendList.end(); ++m_FriendListIter)
    {
        if (wcsncmp(m_FriendListIter->m_szID, pszID, MAX_ID_SIZE) == 0)
        {
            m_FriendList.erase(m_FriendListIter);
            break;
        }
    }
}

void CFriendList::ClearFriendList()
{
    m_FriendList.clear();
    m_FriendListIter = m_FriendList.begin();
}

int CFriendList::UpdateFriendList(std::deque<GUILDLIST_TEXT>& pDestData, const wchar_t* pszID)
{
    pDestData.clear();
    int i = 1, iResult = 0;
    for (m_FriendListIter = m_FriendList.begin(); m_FriendListIter != m_FriendList.end(); ++m_FriendListIter, ++i)
    {
        pDestData.push_back(*m_FriendListIter);
        if (pszID != NULL && wcsncmp(m_FriendListIter->m_szID, pszID, MAX_ID_SIZE) == 0) iResult = i;
    }
    return iResult;
}

void CFriendList::UpdateFriendState(const wchar_t* pszID, BYTE Number, BYTE Server)
{
    for (m_FriendListIter = m_FriendList.begin(); m_FriendListIter != m_FriendList.end(); ++m_FriendListIter)
    {
        if (wcsncmp(m_FriendListIter->m_szID, pszID, MAX_ID_SIZE) == 0)
        {
            m_FriendListIter->m_Number = Number;
            m_FriendListIter->m_Server = Server;
            break;
        }
    }
}

void CFriendList::UpdateAllFriendState(BYTE Number, BYTE Server)
{
    for (m_FriendListIter = m_FriendList.begin(); m_FriendListIter != m_FriendList.end(); ++m_FriendListIter)
    {
        m_FriendListIter->m_Number = Number;
        m_FriendListIter->m_Server = Server;
    }
}

bool TestAlphabeticOrder(const wchar_t* pszText1, const wchar_t* pszText2, BOOL* pbEqual = FALSE)
{
    if (pbEqual != NULL) *pbEqual = FALSE;
    int iLength = min(wcslen(pszText1), wcslen(pszText2));
    for (int i = 0; i < iLength; ++i)
    {
        if (pszText1[i] == pszText2[i]);
        else if (pszText1[i] > pszText2[i]) return true;
        else return false;
    }
    if (pbEqual != NULL) *pbEqual = TRUE;
    return false;	// 완전히 동일
}

bool FriendListSortByID(const GUILDLIST_TEXT& lhs, const GUILDLIST_TEXT& rhs)
{
    return TestAlphabeticOrder(lhs.m_szID, rhs.m_szID);
}

bool FriendListSortByServer(const GUILDLIST_TEXT& lhs, const GUILDLIST_TEXT& rhs)
{
    return (lhs.m_Server > rhs.m_Server);
}

void CFriendList::Sort(int iType)
{
    if (iType != -1) m_iCurrentSortType = iType;
    switch (m_iCurrentSortType)
    {
    case 0:
        sort(m_FriendList.begin(), m_FriendList.end(), FriendListSortByID);
        break;
    case 1:
        sort(m_FriendList.begin(), m_FriendList.end(), FriendListSortByServer);
        break;
    default:
        return;
        break;
    }
}

void CUIFriendListTabWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    SetTitle(pszTitle);
    SetParentUIID(dwParentID);
    SetOption(UIWINDOWSTYLE_NULL);

    SetPosition(50, 50);
    SetSize(250, 170);
    SetLimitSize(250, 150);

    RefreshPalList();

    m_PalListBox.SetParentUIID(GetUIID());
    m_PalListBox.SetArrangeType(2, 0, 22);
    m_PalListBox.SetResizeType(3, 0, -39);
    m_PalListBox.SetLayout(1);

    m_AddFriendButton.Init(1, GlobalText[1018]);
    m_AddFriendButton.SetParentUIID(GetUIID());
    m_AddFriendButton.SetArrangeType(2, 2, 17);
    m_AddFriendButton.SetSize(50, 14);

    m_DelFriendButton.Init(2, GlobalText[1019]);
    m_DelFriendButton.SetParentUIID(GetUIID());
    m_DelFriendButton.SetArrangeType(2, 53, 17);
    m_DelFriendButton.SetSize(50, 14);

    m_TalkButton.Init(3, GlobalText[1020]);
    m_TalkButton.SetParentUIID(GetUIID());
    m_TalkButton.SetArrangeType(2, 104, 17);
    m_TalkButton.SetSize(50, 14);

    m_LetterButton.Init(4, GlobalText[1015]);
    m_LetterButton.SetParentUIID(GetUIID());
    m_LetterButton.SetArrangeType(2, 155, 17);
    m_LetterButton.SetSize(50, 14);
}

void CUIFriendListTabWindow::Refresh()
{
    m_AddFriendButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_DelFriendButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_TalkButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_LetterButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

    m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_PalListBox.SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);
}

const wchar_t* CUIFriendListTabWindow::GetCurrentSelectedFriend(BYTE* pNumber, BYTE* pServer)
{
    if (m_PalListBox.GetSelectedText() == NULL) return NULL;
    else
    {
        if (pNumber != NULL) *pNumber = m_PalListBox.GetSelectedText()->m_Number;
        if (pServer != NULL) *pServer = m_PalListBox.GetSelectedText()->m_Server;
        return m_PalListBox.GetSelectedText()->m_szID;
    }
}

void CUIFriendListTabWindow::RenderSub()
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

        m_AddFriendButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_DelFriendButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_TalkButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_LetterButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

        if (GetState() == UISTATE_RESIZE)
        {
            m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
        }
    }

    EnableAlphaTest();
    SetLineColor(7);
    RenderColor((float)RPos_x(0), (float)RPos_y(18 + m_PalListBox.GetHeight()),
        (float)RWidth(), (float)RHeight() - m_PalListBox.GetHeight() - 18);
    EndRenderColor();
    DisableAlphaBlend();

    m_PalListBox.Render();

    m_AddFriendButton.Render();
    m_DelFriendButton.Render();
    m_TalkButton.Render();
    m_LetterButton.Render();

    EnableAlphaTest();
    SetLineColor(2);
    RenderColor((float)RPos_x(0), (float)RPos_y(16), (float)RWidth(), 1.0f);
    SetLineColor(5);
    RenderColor((float)RPos_x(0), (float)RPos_y(0), (float)RWidth(), 16);
    SetLineColor(2);
    RenderColor((float)RPos_x(0), (float)RPos_y(17 + m_PalListBox.GetHeight()), (float)RWidth(), 1.0f);
    RenderColor((float)RPos_x(0) + m_PalListBox.GetColumnPos_x(1), (float)RPos_y(17), 1.0f, (float)RHeight() - 22 - 17);
    SetLineColor(14);
    RenderColor((float)RPos_x(0) + m_PalListBox.GetColumnPos_x(1), (float)RPos_y(3), 1.0f, 10);
    EndRenderColor();

    g_pRenderText->SetBgColor(0);

    if (CheckMouseIn(RPos_x(0) + m_PalListBox.GetColumnPos_x(0), RPos_y(0), m_PalListBox.GetColumnWidth(0), 19) == TRUE || g_pFriendList->GetCurrentSortType() == 0)
    {
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(RPos_x(4) + m_PalListBox.GetColumnPos_x(0), RPos_y(3), GlobalText[1021]);
        g_pRenderText->SetTextColor(230, 220, 200, 255);
    }
    else
        g_pRenderText->RenderText(RPos_x(4) + m_PalListBox.GetColumnPos_x(0), RPos_y(3), GlobalText[1021]);

    if (CheckMouseIn(RPos_x(0) + m_PalListBox.GetColumnPos_x(1), RPos_y(0), m_PalListBox.GetColumnWidth(1), 19) == TRUE || g_pFriendList->GetCurrentSortType() == 1)
    {
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(RPos_x(4) + m_PalListBox.GetColumnPos_x(1), RPos_y(3), GlobalText[1022]);
        g_pRenderText->SetTextColor(230, 220, 200, 255);
    }
    else
        g_pRenderText->RenderText(RPos_x(4) + m_PalListBox.GetColumnPos_x(1), RPos_y(3), GlobalText[1022]);

    DisableAlphaBlend();
}

BOOL CUIFriendListTabWindow::HandleMessage()
{
    if (m_WorkMessage.m_iMessage == UI_MESSAGE_LISTDBLCLICK)
    {
        PlayBuffer(SOUND_CLICK01);
        m_WorkMessage.m_iMessage = UI_MESSAGE_BTNLCLICK;
        m_WorkMessage.m_iParam1 = 3;
    }

    switch (m_WorkMessage.m_iMessage)
    {
    case UI_MESSAGE_SELECTED:
        break;
    case UI_MESSAGE_BTNLCLICK:
    {
        if (g_dwTopWindow != 0) break;
        DWORD dwUIID = 0;
        switch (m_WorkMessage.m_iParam1)
        {
        case 1:
            dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_TEXTINPUT, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1023], GetUIID());
            g_pWindowMgr->SetAddFriendWindow(dwUIID);
            break;
        case 2:
        {
            if (GetCurrentSelectedFriend() == NULL) break;
            wchar_t tempTxt[MAX_TEXT_LENGTH + 1] = { 0 };
            swprintf(tempTxt, L"%s %s", GlobalText[1024], GetCurrentSelectedFriend()); // "Do you really wish to delete this friend?"
            dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_QUESTION, UIWND_DEFAULT, UIWND_DEFAULT, tempTxt, GetUIID());
        }
        break;
        case 3:
        {
            if (GetCurrentSelectedFriend() == NULL) break;
            wchar_t pszName[MAX_ID_SIZE] = { 0 };
            BYTE Server;
            wcsncpy(pszName, GetCurrentSelectedFriend(NULL, &Server), MAX_ID_SIZE);
            if (Server <= 0xFC)
            {
                DWORD dwDuplicationCheck = g_pFriendMenu->CheckChatRoomDuplication(pszName);
                if (dwDuplicationCheck == 0)
                {
                    if (g_pWindowMgr->GetChatReject() == FALSE && g_pFriendMenu->IsRequestWindow(pszName) == FALSE)
                    {
                        g_pFriendMenu->AddRequestWindow(pszName);
                        SocketClient->ToGameServer()->SendChatRoomCreateRequest(pszName);
                    }
                }
                else if (dwDuplicationCheck == -1);
                else
                {
                    g_pWindowMgr->GetWindow(dwDuplicationCheck)->SetState(UISTATE_HIDE);
                    g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, dwDuplicationCheck, 0);
                }
            }
        }
        break;
        case 4:		// 편지쓰기
        {
            wchar_t temp[MAX_TEXT_LENGTH + 1];
            swprintf(temp, GlobalText[1071], g_cdwLetterCost);
            dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_WRITELETTER, 100, 100, temp);	// "편지쓰기"
            if (dwUIID == 0) break;
            if (GetCurrentSelectedFriend() != NULL)
                ((CUILetterWriteWindow*)g_pWindowMgr->GetWindow(dwUIID))->SetMailtoText((const wchar_t*)GetCurrentSelectedFriend());
        }
        break;
        default:
            break;
        }
        if (dwUIID != 0)
        {
            CUIBaseWindow* pWindow = g_pWindowMgr->GetWindow(dwUIID);
            if (pWindow != NULL)
            {
                pWindow->SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
                pWindow->SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
            }
        }
    }
    break;
    case UI_MESSAGE_TXTRETURN:
        if (m_WorkMessage.m_iParam2 != 0)
        {
            wchar_t* pText = (wchar_t*)m_WorkMessage.m_iParam2;
            SocketClient->ToGameServer()->SendFriendAddRequest(pText);
            delete[] pText;
        }
        break;
    case UI_MESSAGE_YNRETURN:
        if (m_WorkMessage.m_iParam2 == 1)
        {
            if (GetCurrentSelectedFriend() == NULL) break;
            SocketClient->ToGameServer()->SendFriendDelete(GetCurrentSelectedFriend());
        }
        break;
    default:
        break;
    }
    return FALSE;
}

void CUIFriendListTabWindow::DoActionSub(BOOL bMessageOnly)
{
    m_PalListBox.DoAction(bMessageOnly);

    m_AddFriendButton.DoAction(bMessageOnly);
    m_DelFriendButton.DoAction(bMessageOnly);
    m_TalkButton.DoAction(bMessageOnly);
    m_LetterButton.DoAction(bMessageOnly);
}

void CUIFriendListTabWindow::DoMouseActionSub()
{
    if (MouseLButton)
    {
        if (CheckMouseIn(RPos_x(0) + m_PalListBox.GetColumnPos_x(0), RPos_y(0), m_PalListBox.GetColumnWidth(0), 19) == TRUE)
        {
            if (g_pFriendList->GetCurrentSortType() != 0)
            {
                PlayBuffer(SOUND_CLICK01);
                g_pFriendList->Sort(0);
                RefreshPalList();
            }
            MouseLButton = FALSE;
        }
        else if (CheckMouseIn(RPos_x(0) + m_PalListBox.GetColumnPos_x(1), RPos_y(0), m_PalListBox.GetColumnWidth(1), 19) == TRUE)
        {
            if (g_pFriendList->GetCurrentSortType() != 1)
            {
                PlayBuffer(SOUND_CLICK01);
                g_pFriendList->Sort(0);
                g_pFriendList->Sort(1);
                RefreshPalList();
            }
            MouseLButton = FALSE;
        }
    }
}

void CUIFriendListTabWindow::RefreshPalList()
{
    wchar_t szID[MAX_ID_SIZE + 1] = { 0 };
    if (m_PalListBox.SLGetSelectLineNum() > 0)
        wcsncpy(szID, m_PalListBox.SLGetSelectLine()->m_szID, MAX_ID_SIZE);
    int iSelectNum = g_pFriendList->UpdateFriendList(m_PalListBox.GetFriendList(), (wchar_t*)&szID);
    m_PalListBox.SLSetSelectLine(iSelectNum);
    m_PalListBox.Scrolling(0);
}

/*
BOOL CChatRoomSocketList::AddChatRoomSocket(DWORD dwRoomID, DWORD dwWindowUIID, const std::wstring pszIP, void(*packetHandler)(int32_t Handle, const BYTE*, int32_t))
{
    int32_t usPort = 55980;

    DWORD dwSocketID = CreateChatRoomSocketID(dwRoomID);
    if (dwSocketID == -1) return FALSE;

    CHATROOM_SOCKET * pCRSocket;
    pCRSocket = new CHATROOM_SOCKET;
    pCRSocket->m_dwRoomID = dwRoomID;
    pCRSocket->m_dwWindowUIID = dwWindowUIID;
    pCRSocket->m_WSClient = new Connection(pszIP, usPort, packetHandler);

    if (!pCRSocket->m_WSClient->IsConnected())
    {
        assert(!"RemoveChatRoomSocket Connect!!!");
        return FALSE;
    }

    m_ChatRoomSocketMap.insert(std::pair<DWORD, CHATROOM_SOCKET *>(dwRoomID, pCRSocket));

    return TRUE;
}

void CChatRoomSocketList::RemoveChatRoomSocket(DWORD dwRoomID)
{
    m_ChatRoomSocketMapIter = m_ChatRoomSocketMap.find(dwRoomID);
    if (m_ChatRoomSocketMapIter == m_ChatRoomSocketMap.end())
    {
        assert(!"RemoveChatRoomSocket!!!");
        return;
    }
    m_ChatRoomSocketMapIter->second->m_WSClient.Close();
    delete m_ChatRoomSocketMapIter->second;
    m_ChatRoomSocketMapIter->second = NULL;
    m_ChatRoomSocketMap.erase(m_ChatRoomSocketMapIter);

    BOOL bFind = FALSE;
    for (m_ChatRoomSocketStatusMapIter = m_ChatRoomSocketStatusMap.begin(); m_ChatRoomSocketStatusMapIter != m_ChatRoomSocketStatusMap.end(); ++m_ChatRoomSocketStatusMapIter)
    {
        if (m_ChatRoomSocketStatusMapIter->second == dwRoomID)
        {
            bFind = TRUE;
            break;
        }
    }
    if (bFind == FALSE)
    {
        assert(!"RemoveChatRoomSocket bFind!!!");
        return;
    }
    m_bChatRoomSocketStatus[m_ChatRoomSocketStatusMapIter->first] = FALSE;
    m_ChatRoomSocketStatusMap.erase(m_ChatRoomSocketStatusMapIter);
}

void CChatRoomSocketList::ClearChatRoomSocketList()
{
    for (m_ChatRoomSocketMapIter = m_ChatRoomSocketMap.begin(); m_ChatRoomSocketMapIter != m_ChatRoomSocketMap.end(); ++m_ChatRoomSocketMapIter)
    {
        delete m_ChatRoomSocketMapIter->second;
        m_ChatRoomSocketMapIter->second = NULL;
    }
    m_ChatRoomSocketMap.clear();

    memset(m_bChatRoomSocketStatus, 0, 256 * sizeof(BYTE));
    m_ChatRoomSocketStatusMap.clear();
    m_bCurrectCreateID = 0;
}

DWORD CChatRoomSocketList::CreateChatRoomSocketID(DWORD dwRoomID)
{
//	++m_bCurrectCreateID;

    int iWhileCount = 0;
    while (m_bChatRoomSocketStatus[m_bCurrectCreateID] == TRUE)
    {
        ++iWhileCount;
        if (m_bCurrectCreateID == 255) m_bCurrectCreateID = 0;
        else ++m_bCurrectCreateID;

        if (iWhileCount > 256)
        {
            return MCI_SEQ_MAPPER;
        }
    };
    m_bChatRoomSocketStatus[m_bCurrectCreateID] = TRUE;
    m_ChatRoomSocketStatusMap.insert(std::pair < DWORD, DWORD > (m_bCurrectCreateID, dwRoomID));
    return m_bCurrectCreateID;
}

DWORD CChatRoomSocketList::GetChatRoomSocketID(DWORD dwSocketID)
{
    m_ChatRoomSocketStatusMapIter = m_ChatRoomSocketStatusMap.find(dwSocketID);
    if (m_ChatRoomSocketStatusMapIter == m_ChatRoomSocketStatusMap.end())
    {
        return MCI_SEQ_MAPPER;
    }
    else return m_ChatRoomSocketStatusMapIter->second;
}

CHATROOM_SOCKET * CChatRoomSocketList::GetChatRoomSocketData(DWORD dwRoomID)
{
    m_ChatRoomSocketMapIter = m_ChatRoomSocketMap.find(dwRoomID);
    if (m_ChatRoomSocketMapIter == m_ChatRoomSocketMap.end())
    {
        return NULL;
    }
    return m_ChatRoomSocketMapIter->second;
}*/

void ReceiveChatRoomConnectResult(DWORD dwWindowUIID, const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_CHAT_JOIN_RESULT)ReceiveBuffer;
    switch (Data->Result)
    {
    case 0x00:
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1058]);
        break;
    case 0x01:
        break;
    default:
        break;
    };
}

void ReceiveChatRoomUserStateChange(DWORD dwWindowUIID, const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_CHAT_CHANGE_STATE)ReceiveBuffer;
    auto* pChatWindow = (CUIChatWindow*)g_pWindowMgr->GetWindow(dwWindowUIID);
    if (pChatWindow == NULL) return;
    wchar_t szName[MAX_ID_SIZE + 1] = { 0 };
    CMultiLanguage::ConvertFromUtf8(szName, Data->Name, MAX_ID_SIZE);
    szName[MAX_ID_SIZE] = '\0';
    wchar_t szText[MAX_TEXT_LENGTH + 1] = { 0 };
    CMultiLanguage::ConvertFromUtf8(szText, Data->Name, MAX_ID_SIZE);
    szText[MAX_ID_SIZE] = '\0';
    switch (Data->Type)
    {
    case 0x00:
        if (pChatWindow->AddChatPal(szName, Data->Index, 0) >= 3)
        {
            wcscat(szText, GlobalText[1059]);
            pChatWindow->AddChatText(255, szText, 1, 0);
        }
        break;
    case 0x01:
        if (pChatWindow->GetUserCount() >= 3)
        {
            wcscat(szText, GlobalText[1060]);
            pChatWindow->AddChatText(255, szText, 1, 0);
        }
        pChatWindow->RemoveChatPal(szName);
        break;
    default:
        return;
        break;
    };
    if (pChatWindow->GetShowType() == 2)
        pChatWindow->UpdateInvitePalList();
}

void ReceiveChatRoomUserList(DWORD dwWindowUIID, const BYTE* ReceiveBuffer)
{
    auto Header = (LPFS_CHAT_USERLIST_HEADER)ReceiveBuffer;
    int iMoveOffset = sizeof(FS_CHAT_USERLIST_HEADER);
    wchar_t szName[MAX_ID_SIZE + 1] = { 0 };
    for (int i = 0; i < Header->Count; ++i)
    {
        auto Data = (LPFS_CHAT_USERLIST_DATA)(ReceiveBuffer + iMoveOffset);
        CMultiLanguage::ConvertFromUtf8(szName, Data->Name, MAX_ID_SIZE);
        szName[MAX_ID_SIZE] = '\0';
        ((CUIChatWindow*)g_pWindowMgr->GetWindow(dwWindowUIID))->AddChatPal(szName, Data->Index, 0);
        iMoveOffset += sizeof(FS_CHAT_USERLIST_DATA);
    }
}

void ReceiveChatRoomChatText(DWORD dwWindowUIID, const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_CHAT_TEXT)ReceiveBuffer;
    auto* pChatWindow = (CUIChatWindow*)g_pWindowMgr->GetWindow(dwWindowUIID);
    if (pChatWindow == NULL) return;

    char temp[MAX_CHATROOM_TEXT_LENGTH] = { };
    if (Data->MsgSize >= MAX_CHATROOM_TEXT_LENGTH) return;

    memcpy(temp, Data->Msg, Data->MsgSize);
    BuxConvert((LPBYTE)temp, Data->MsgSize);

    wchar_t chatMessage[MAX_CHATROOM_TEXT_LENGTH] = { };
    CMultiLanguage::ConvertFromUtf8(chatMessage, temp, MAX_CHATROOM_TEXT_LENGTH);

    if (pChatWindow->GetState() == UISTATE_READY)
    {
        g_pFriendMenu->SetNewChatAlert(dwWindowUIID);
        g_pSystemLogBox->AddText(GlobalText[1063], SEASON3B::TYPE_SYSTEM_MESSAGE);
        pChatWindow->SetState(UISTATE_HIDE);
        if (g_pWindowMgr->GetFriendMainWindow() != NULL)
        {
            g_pWindowMgr->GetFriendMainWindow()->AddWindow(dwWindowUIID, g_pWindowMgr->GetWindow(dwWindowUIID)->GetTitle());
        }
    }
    else if (pChatWindow->GetState() == UISTATE_HIDE || g_pWindowMgr->GetTopWindowUIID() != dwWindowUIID)
    {
        g_pFriendMenu->SetNewChatAlert(dwWindowUIID);
    }
    pChatWindow->AddChatText(Data->Index, chatMessage, 3, 0);
}

void ReceiveChatRoomNoticeText(DWORD dwWindowUIID, const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_CHAT_TEXT)ReceiveBuffer;
    Data->Msg[99] = '\0';
    if (Data->Msg[0] == '\0')
    {
        return;
    }

    wchar_t message[sizeof Data->Msg]{};
    CMultiLanguage::ConvertFromUtf8(message, Data->Msg, sizeof Data->Msg);
    g_pSystemLogBox->AddText(message, SEASON3B::TYPE_SYSTEM_MESSAGE);
}

void TranslateChattingProtocol(DWORD dwWindowUIID, const BYTE* ReceiveBuffer, int Size)
{
    if (Size < 4)
    {
        return;
    }

    int HeadCode;
    BOOL bIsC1C3 = ReceiveBuffer[0] % 2 == 1;
    if (bIsC1C3) // C1 and C3
    {
        HeadCode = ReceiveBuffer[2];
    }
    else
    {
        HeadCode = ReceiveBuffer[3];
    }

    switch (HeadCode)
    {
    case 0x00:
        ReceiveChatRoomConnectResult(dwWindowUIID, ReceiveBuffer);
        break;
    case 0x01:
        ReceiveChatRoomUserStateChange(dwWindowUIID, ReceiveBuffer);
        break;
    case 0x02:
        ReceiveChatRoomUserList(dwWindowUIID, ReceiveBuffer);
        break;
    case 0x04:
        ReceiveChatRoomChatText(dwWindowUIID, ReceiveBuffer);
        break;
    case 0x0D:
        ReceiveChatRoomNoticeText(dwWindowUIID, ReceiveBuffer);
        break;
    default:
        break;
    }
}

/*
void CChatRoomSocketList::ProcessSocketMessage(DWORD dwSocketID, WORD wMessage)
{
    CHATROOM_SOCKET * pChatroomSocket = GetChatRoomSocketData(GetChatRoomSocketID(dwSocketID));
    if (pChatroomSocket == NULL) return;
    Connection* pSocketClient = &pChatroomSocket->m_WSClient;

    if (pSocketClient == NULL)
    {
        return;
    }
    switch(wMessage)
    {
    case FD_CONNECT:
        break;
    case FD_READ :
        // pSocketClient->nRecv();
        break;
    case FD_WRITE :
        // pSocketClient->FDWriteSend();
        break;
    case FD_CLOSE :
        CUIChatWindow * pWindow = (CUIChatWindow *)g_pWindowMgr->GetWindow(pChatroomSocket->m_dwWindowUIID);
        if (pWindow != NULL)
            pWindow->AddChatText(255, GlobalText[402], 1, 0);
        pSocketClient->Close();
        break;
    }
}

//void CChatRoomSocketList::ProtocolCompile()
//{
//	// TODO: Change that
//	for (m_ChatRoomSocketMapIter = m_ChatRoomSocketMap.begin(); m_ChatRoomSocketMapIter != m_ChatRoomSocketMap.end(); ++m_ChatRoomSocketMapIter)
//	{
//		ProtocolCompiler(&m_ChatRoomSocketMapIter->second->m_WSClient, 1, m_ChatRoomSocketMapIter->second->m_dwWindowUIID);
//	}
//}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIChatRoomListTabWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    SetTitle(pszTitle);
    SetParentUIID(dwParentID);
    SetOption(UIWINDOWSTYLE_NULL);

    SetPosition(50, 50);
    //SetSize(213, 170);
    SetSize(250, 170);
    SetLimitSize(250, 150);

    m_WindowListBox.SetParentUIID(GetUIID());
    m_WindowListBox.SetArrangeType(2, 0, 22);
    m_WindowListBox.SetResizeType(3, 0, -39);

    m_HideAllButton.Init(1, GlobalText[1025]);
    m_HideAllButton.SetParentUIID(GetUIID());
    m_HideAllButton.SetArrangeType(2, 2, 17);
    m_HideAllButton.SetSize(50, 14);
}

void CUIChatRoomListTabWindow::Refresh()
{
    m_HideAllButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_WindowListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_WindowListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_WindowListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_WindowListBox.SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);
}

void CUIChatRoomListTabWindow::AddWindow(DWORD dwUIID, const wchar_t* pszTitle)
{
    m_WindowListBox.AddText(dwUIID, pszTitle);
}

void CUIChatRoomListTabWindow::RemoveWindow(DWORD dwUIID)
{
    m_WindowListBox.DeleteText(dwUIID);
    m_WindowListBox.Scrolling(0);
}

DWORD CUIChatRoomListTabWindow::GetCurrentSelectedWindow()
{
    if (m_WindowListBox.GetSelectedText() == NULL) return NULL;
    else return m_WindowListBox.GetSelectedText()->m_dwUIID;
}

void CUIChatRoomListTabWindow::RenderSub()
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_WindowListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_HideAllButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

        if (GetState() == UISTATE_RESIZE)
        {
            m_WindowListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
        }
    }

    EnableAlphaTest();
    SetLineColor(7);
    RenderColor((float)RPos_x(0), (float)RPos_y(18 + m_WindowListBox.GetHeight()),
        (float)RWidth(), (float)RHeight() - m_WindowListBox.GetHeight() - 18);
    EndRenderColor();

    m_WindowListBox.Render();
    m_HideAllButton.Render();

    EnableAlphaTest();
    SetLineColor(2);
    RenderColor((float)RPos_x(0), (float)RPos_y(0) + 16, (float)RWidth(), 1.0f);
    SetLineColor(5);
    RenderColor((float)RPos_x(0), (float)RPos_y(0), (float)RWidth(), 16);
    SetLineColor(2);
    RenderColor((float)RPos_x(0), (float)RPos_y(17 + m_WindowListBox.GetHeight()), (float)RWidth(), 1.0f);
    EndRenderColor();

    EnableAlphaTest();

    g_pRenderText->SetTextColor(230, 220, 200, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->RenderText(RPos_x(8), RPos_y(3), GlobalText[1026]);
    DisableAlphaBlend();
}

BOOL CUIChatRoomListTabWindow::HandleMessage()
{
    switch (m_WorkMessage.m_iMessage)
    {
    case UI_MESSAGE_SELECTED:
        break;
    case UI_MESSAGE_LISTDBLCLICK:
        if (g_pWindowMgr->GetWindow(GetCurrentSelectedWindow())->GetState() == UISTATE_HIDE ||
            g_pWindowMgr->GetTopNotMainWindowUIID() != GetCurrentSelectedWindow())
        {
            PlayBuffer(SOUND_CLICK01);
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, GetCurrentSelectedWindow(), 0);
            g_pWindowMgr->HideAllWindowClear();
            MouseLButton = false;
        }
        else
        {
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_HIDE, GetCurrentSelectedWindow(), 0);
        }
        break;
    case UI_MESSAGE_BTNLCLICK:
    {
        if (g_dwTopWindow != 0) break;
        switch (m_WorkMessage.m_iParam1)
        {
        case 1:
            g_pWindowMgr->HideAllWindow(TRUE);
            break;
        default:
            break;
        }
    }
    default:
        break;
    }
    return FALSE;
}

void CUIChatRoomListTabWindow::DoActionSub(BOOL bMessageOnly)
{
    m_WindowListBox.DoAction(bMessageOnly);
    m_HideAllButton.DoAction(bMessageOnly);
}

void CUIChatRoomListTabWindow::DoMouseActionSub()
{
}

void CLetterList::AddLetter(DWORD dwLetterID, const wchar_t* pszID, const wchar_t* pszText, const wchar_t* pszDate, const wchar_t* pszTime, BOOL bIsRead)
{
    for (m_LetterListIter = m_LetterList.begin(); m_LetterListIter != m_LetterList.end(); ++m_LetterListIter)
    {
        if (m_LetterListIter->m_dwLetterID == dwLetterID)
        {
            return;
        }
    }

    static LETTERLIST_TEXT text;
    wcsncpy(text.m_szID, pszID, MAX_ID_SIZE);
    text.m_szID[MAX_ID_SIZE] = '\0';
    wcsncpy(text.m_szText, pszText, 32);
    text.m_szText[32] = '\0';
    wcsncpy(text.m_szDate, pszDate, 16);
    wcsncpy(text.m_szTime, pszTime, 16);
    text.m_bIsRead = bIsRead;
    text.m_dwLetterID = dwLetterID;

    //m_LetterList.insert(m_LetterList.end(), text);
    m_LetterList.push_back(text);
}

void CLetterList::RemoveLetter(DWORD dwLetterID)
{
    for (m_LetterListIter = m_LetterList.begin(); m_LetterListIter != m_LetterList.end(); ++m_LetterListIter)
    {
        if (m_LetterListIter->m_dwLetterID == dwLetterID)
        {
            m_LetterList.erase(m_LetterListIter);
            break;
        }
    }
}

void CLetterList::ClearLetterList()
{
    m_LetterList.clear();
    m_LetterListIter = m_LetterList.begin();
    ClearLetterTextCache();
}

int CLetterList::UpdateLetterList(std::deque<LETTERLIST_TEXT>& pDestData, DWORD dwSelectLineNum)
{
    pDestData.clear();
    int i = 1, iResult = 0;
    for (m_LetterListIter = m_LetterList.begin(); m_LetterListIter != m_LetterList.end(); ++m_LetterListIter, ++i)
    {
        pDestData.push_back(*m_LetterListIter);
        if (m_LetterListIter->m_dwLetterID == dwSelectLineNum) iResult = i;
    }
    return iResult;
}

bool LetterListSortByRead(const LETTERLIST_TEXT& lhs, const LETTERLIST_TEXT& rhs)
{
    return (lhs.m_bIsRead == TRUE && rhs.m_bIsRead == FALSE);
}

bool LetterListSortByID(const LETTERLIST_TEXT& lhs, const LETTERLIST_TEXT& rhs)
{
    return TestAlphabeticOrder(lhs.m_szID, rhs.m_szID);
}

bool LetterListSortByTime(const LETTERLIST_TEXT& lhs, const LETTERLIST_TEXT& rhs)
{
    BOOL bEqual = FALSE;
    bool bResult = TestAlphabeticOrder(rhs.m_szDate, lhs.m_szDate, &bEqual);
    if (bEqual == TRUE) return TestAlphabeticOrder(rhs.m_szTime, lhs.m_szTime);
    else return bResult;
}

bool LetterListSortByTitle(const LETTERLIST_TEXT& lhs, const LETTERLIST_TEXT& rhs)
{
    return TestAlphabeticOrder(lhs.m_szText, rhs.m_szText);
}

void CLetterList::Sort(int iType)
{
    if (iType != -1) m_iCurrentSortType = iType;
    switch (m_iCurrentSortType)
    {
    case 0:
        sort(m_LetterList.begin(), m_LetterList.end(), LetterListSortByRead);
        break;
    case 1:
        sort(m_LetterList.begin(), m_LetterList.end(), LetterListSortByID);
        break;
    case 2:
        sort(m_LetterList.begin(), m_LetterList.end(), LetterListSortByTime);
        break;
    case 3:
        sort(m_LetterList.begin(), m_LetterList.end(), LetterListSortByTitle);
        break;
    default:
        return;
        break;
    }
}

DWORD CLetterList::GetPrevLetterID(DWORD dwLetterID)
{
    for (m_LetterListIter = m_LetterList.begin(); m_LetterListIter != m_LetterList.end(); ++m_LetterListIter)
    {
        if (m_LetterListIter->m_dwLetterID == dwLetterID) break;
    }
    if (m_LetterListIter == m_LetterList.end()) return 0;
    ++m_LetterListIter;
    if (m_LetterListIter == m_LetterList.end()) return 0;
    return m_LetterListIter->m_dwLetterID;
}

DWORD CLetterList::GetNextLetterID(DWORD dwLetterID)
{
    for (m_LetterListIter = m_LetterList.begin(); m_LetterListIter != m_LetterList.end(); ++m_LetterListIter)
    {
        if (m_LetterListIter->m_dwLetterID == dwLetterID) break;
    }
    if (m_LetterListIter == m_LetterList.end()) return 0;
    if (m_LetterListIter == m_LetterList.begin()) return 0;
    --m_LetterListIter;
    return m_LetterListIter->m_dwLetterID;
}

LETTERLIST_TEXT* CLetterList::GetLetter(DWORD dwLetterID)
{
    for (m_LetterListIter = m_LetterList.begin(); m_LetterListIter != m_LetterList.end(); ++m_LetterListIter)
    {
        if (m_LetterListIter->m_dwLetterID == dwLetterID) break;
    }
    if (m_LetterListIter == m_LetterList.end()) return NULL;
    return &(*m_LetterListIter);
}

void CLetterList::ResetLetterSelect(BOOL bFlag)
{
    for (m_LetterListIter = m_LetterList.begin(); m_LetterListIter != m_LetterList.end(); ++m_LetterListIter)
    {
        m_LetterListIter->m_bIsSelected = bFlag;
    }
}

BOOL CLetterList::CheckNoReadLetter()
{
    for (m_LetterListIter = m_LetterList.begin(); m_LetterListIter != m_LetterList.end(); ++m_LetterListIter)
    {
        if (m_LetterListIter->m_bIsRead == FALSE) return TRUE;
    }
    return FALSE;
}

void CLetterList::CacheLetterText(DWORD dwIndex, LPFS_LETTER_TEXT pLetterText)
{
    m_LetterCache.insert(std::pair<DWORD, FS_LETTER_TEXT>(dwIndex, *pLetterText));
}

LPFS_LETTER_TEXT CLetterList::GetLetterText(DWORD dwIndex)
{
    m_LetterCacheIter = m_LetterCache.find(dwIndex);
    if (m_LetterCacheIter == m_LetterCache.end()) return NULL;
    return &m_LetterCacheIter->second;
}

void CLetterList::RemoveLetterTextCache(DWORD dwIndex)
{
    m_LetterCacheIter = m_LetterCache.find(dwIndex);
    if (m_LetterCacheIter != m_LetterCache.end())
    {
        auto letter = &m_LetterCacheIter->second;
        delete letter;
        m_LetterCache.erase(m_LetterCacheIter);
    }
}

void CLetterList::ClearLetterTextCache()
{
    for (auto cachedLetter : m_LetterCache)
    {
        auto letter = &cachedLetter.second;
        delete letter;
    }

    m_LetterCache.clear();
}

int CLetterList::GetLineNum(DWORD dwLetterID)
{
    int iCount = 0;
    for (m_LetterListIter = m_LetterList.begin(); m_LetterListIter != m_LetterList.end(); ++m_LetterListIter)
    {
        ++iCount;
        if (m_LetterListIter->m_dwLetterID == dwLetterID)
        {
            return iCount;
        }
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUILetterBoxTabWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    SetTitle(pszTitle);
    SetParentUIID(dwParentID);
    SetOption(UIWINDOWSTYLE_NULL);

    SetPosition(50, 50);
    //SetSize(213, 170);
    SetSize(250, 170);
    SetLimitSize(250, 150);

    RefreshLetterList();

    m_LetterListBox.SetParentUIID(GetUIID());
    m_LetterListBox.SetArrangeType(2, 0, 22);
    m_LetterListBox.SetResizeType(3, 0, -39);

    m_WriteButton.Init(1, GlobalText[1015]);
    m_WriteButton.SetParentUIID(GetUIID());
    m_WriteButton.SetArrangeType(2, 2, 17);
    m_WriteButton.SetSize(50, 14);

    m_ReadButton.Init(2, GlobalText[1027]);
    m_ReadButton.SetParentUIID(GetUIID());
    m_ReadButton.SetArrangeType(2, 53, 17);
    m_ReadButton.SetSize(50, 14);

    m_ReplyButton.Init(3, GlobalText[1010]);
    m_ReplyButton.SetParentUIID(GetUIID());
    m_ReplyButton.SetArrangeType(2, 104, 17);
    m_ReplyButton.SetSize(50, 14);

    m_DeleteButton.Init(4, GlobalText[1011]);
    m_DeleteButton.SetParentUIID(GetUIID());
    m_DeleteButton.SetArrangeType(2, 155, 17);
    m_DeleteButton.SetSize(50, 14);
}

void CUILetterBoxTabWindow::Refresh()
{
    m_WriteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_ReadButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_ReplyButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_DeleteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_LetterListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_LetterListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_LetterListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_LetterListBox.SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);
}

LETTERLIST_TEXT* CUILetterBoxTabWindow::GetCurrentSelectedLetter()
{
    if (m_LetterListBox.GetSelectedText() == NULL) return NULL;
    else return m_LetterListBox.GetSelectedText();
}

void CUILetterBoxTabWindow::RefreshLetterList()
{
    int iSelectNum = g_pLetterList->UpdateLetterList(m_LetterListBox.GetLetterList(),
        (m_LetterListBox.SLGetSelectLineNum() > 0 ? m_LetterListBox.SLGetSelectLine()->m_dwLetterID : -1));
    m_LetterListBox.SLSetSelectLine(iSelectNum);
    m_LetterListBox.Scrolling(0);
    CheckAll(FALSE);
}

void CUILetterBoxTabWindow::CheckAll(BOOL bCheck)
{
    m_LetterListBox.ResetCheckedLine(bCheck);
    g_pLetterList->ResetLetterSelect(bCheck);
    m_bCheckAllState = bCheck;
}

void CUILetterBoxTabWindow::PrevNextCursorMove(int iMove)
{
    m_LetterListBox.SLSetSelectLine(iMove);
}

void CUILetterBoxTabWindow::RenderSub()
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_LetterListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

        m_WriteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_ReadButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_ReplyButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_DeleteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        //		m_DeliveryButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

        if (GetState() == UISTATE_RESIZE)
        {
            m_LetterListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
        }
    }

    EnableAlphaTest();
    SetLineColor(7);
    RenderColor((float)RPos_x(0), (float)RPos_y(18 + m_LetterListBox.GetHeight()), (float)RWidth(), (float)RHeight() - m_LetterListBox.GetHeight() - 18);
    EndRenderColor();
    DisableAlphaBlend();

    m_LetterListBox.Render();

    EnableAlphaTest();
    SetLineColor(2);
    RenderColor((float)RPos_x(0), (float)RPos_y(16), (float)RWidth(), 1.0f);
    SetLineColor(5);
    RenderColor((float)RPos_x(0), (float)RPos_y(0), (float)RWidth(), 16);
    SetLineColor(2);
    RenderColor((float)RPos_x(0), (float)RPos_y(17 + m_LetterListBox.GetHeight()), (float)RWidth(), 1.0f);

    RenderColor((float)RPos_x(0) + m_LetterListBox.GetColumnPos_x(1), (float)RPos_y(17), 1.0f, (float)RHeight() - 22 - 17);
    RenderColor((float)RPos_x(0) + m_LetterListBox.GetColumnPos_x(2), (float)RPos_y(17), 1.0f, (float)RHeight() - 22 - 17);
    RenderColor((float)RPos_x(0) + m_LetterListBox.GetColumnPos_x(3), (float)RPos_y(17), 1.0f, (float)RHeight() - 22 - 17);
    SetLineColor(14);
    RenderColor((float)RPos_x(0) + m_LetterListBox.GetColumnPos_x(1), (float)RPos_y(3), 1.0f, 10);
    RenderColor((float)RPos_x(0) + m_LetterListBox.GetColumnPos_x(2), (float)RPos_y(3), 1.0f, 10);
    RenderColor((float)RPos_x(0) + m_LetterListBox.GetColumnPos_x(3), (float)RPos_y(3), 1.0f, 10);
    EndRenderColor();

    g_pRenderText->SetTextColor(230, 220, 200, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    RenderCheckBox(RPos_x(1), RPos_y(3), m_bCheckAllState);
    RenderBitmap(BITMAP_INTERFACE_EX + 14, RPos_x(1 + 10), RPos_y(3), 13.0f, 9.0f, 0.f, 0.f, 13.f / 16.f, 9.f / 32.f);

    if (CheckMouseIn(RPos_x(0) + m_LetterListBox.GetColumnPos_x(1), RPos_y(0), m_LetterListBox.GetColumnWidth(1), 19) == TRUE || g_pLetterList->GetCurrentSortType() == 1)
    {
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(RPos_x(4) + m_LetterListBox.GetColumnPos_x(1), RPos_y(3), GlobalText[1028]);
        g_pRenderText->SetTextColor(230, 220, 200, 255);
    }
    else
    {
        g_pRenderText->RenderText(RPos_x(4) + m_LetterListBox.GetColumnPos_x(1), RPos_y(3), GlobalText[1028]);
    }

    if (CheckMouseIn(RPos_x(0) + m_LetterListBox.GetColumnPos_x(2), RPos_y(0), m_LetterListBox.GetColumnWidth(2), 19) == TRUE || g_pLetterList->GetCurrentSortType() == 2)
    {
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(RPos_x(4) + m_LetterListBox.GetColumnPos_x(2), RPos_y(3), GlobalText[1029]);
        g_pRenderText->SetTextColor(230, 220, 200, 255);
    }
    else
    {
        g_pRenderText->RenderText(RPos_x(4) + m_LetterListBox.GetColumnPos_x(2), RPos_y(3), GlobalText[1029]);
    }

    if (CheckMouseIn(RPos_x(0) + m_LetterListBox.GetColumnPos_x(3), RPos_y(0), m_LetterListBox.GetColumnWidth(3), 19) == TRUE || g_pLetterList->GetCurrentSortType() == 3)
    {
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(RPos_x(4) + m_LetterListBox.GetColumnPos_x(3), RPos_y(3), GlobalText[1030]);
        g_pRenderText->SetTextColor(230, 220, 200, 255);
    }
    else
    {
        g_pRenderText->RenderText(RPos_x(4) + m_LetterListBox.GetColumnPos_x(3), RPos_y(3), GlobalText[1030]);
    }

    DisableAlphaBlend();

    m_WriteButton.Render();
    m_ReadButton.Render();
    m_ReplyButton.Render();
    m_DeleteButton.Render();
    //	m_DeliveryButton.Render();
}

BOOL CUILetterBoxTabWindow::HandleMessage()
{
    if (m_WorkMessage.m_iMessage == UI_MESSAGE_LISTDBLCLICK)
    {
        PlayBuffer(SOUND_CLICK01);
        m_WorkMessage.m_iMessage = UI_MESSAGE_BTNLCLICK;
        m_WorkMessage.m_iParam1 = 2;
    }
    switch (m_WorkMessage.m_iMessage)
    {
    case UI_MESSAGE_SELECTED:
        break;
    case UI_MESSAGE_BTNLCLICK:
    {
        if (g_dwTopWindow != 0) break;
        DWORD dwUIID = 0;
        switch (m_WorkMessage.m_iParam1)
        {
        case 1:
        {
            wchar_t temp[MAX_TEXT_LENGTH + 1];
            swprintf(temp, GlobalText[1071], g_cdwLetterCost);
            dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_WRITELETTER, 100, 100, temp);
        }
        break;
        case 2:
        {
            if (GetCurrentSelectedLetter() == NULL) break;
            DWORD dwLetterID = GetCurrentSelectedLetter()->m_dwLetterID;
            if (g_pWindowMgr->LetterReadCheck(dwLetterID) == FALSE)
            {
                // 캐시
                if (g_pLetterList->GetLetterText(dwLetterID) == NULL)
                {
                    SocketClient->ToGameServer()->SendLetterReadRequest(dwLetterID);
                }
                else
                {
                    auto data = reinterpret_cast<const BYTE*>(g_pLetterList->GetLetterText(dwLetterID));
                    ReceiveLetterText(std::span(data, sizeof(FS_LETTER_TEXT)), true);
                }
            }
            else
            {
                DWORD dwFindUIID = g_pWindowMgr->GetLetterReadWindow(dwLetterID);
                if (dwFindUIID != 0)
                    g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, dwFindUIID, 0);
            }
        }
        break;
        case 3:
        {
            if (GetCurrentSelectedLetter() == NULL) break;
            wchar_t temp[MAX_TEXT_LENGTH + 1];
            swprintf(temp, GlobalText[1071], g_cdwLetterCost);
            dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_WRITELETTER, 100, 100, temp);
            if (dwUIID == 0) break;
            ((CUILetterWriteWindow*)g_pWindowMgr->GetWindow(dwUIID))->SetMailtoText(GetCurrentSelectedLetter()->m_szID);
            wchar_t szMailTitle[MAX_TEXT_LENGTH + 1] = { 0 };
            swprintf(szMailTitle, GlobalText[1016], GetCurrentSelectedLetter()->m_szText);
            wchar_t szMailTitleResult[32 + 1] = { 0 };
            CutText4(szMailTitle, szMailTitleResult, NULL, 32);
            ((CUILetterWriteWindow*)g_pWindowMgr->GetWindow(dwUIID))->SetMainTitleText(szMailTitleResult);
        }
        break;
        case 4:
        {
            if (m_LetterListBox.HaveCheckedLine() == FALSE)
            {
                dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_OK, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1031]);
                break;
            }
            dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_QUESTION, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1017], GetUIID());
        }
        break;
        case 5:
        {
            SocketClient->ToGameServer()->SendLetterListRequest();
        }
        break;
        default:
            break;
        }
        if (dwUIID != 0)
        {
            CUIBaseWindow* pWindow = g_pWindowMgr->GetWindow(dwUIID);
            if (pWindow != NULL)
            {
                pWindow->SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
                pWindow->SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
            }
        }
    }
    break;
    case UI_MESSAGE_YNRETURN:
        if (m_WorkMessage.m_iParam2 == 1)
        {
            if (m_LetterListBox.HaveCheckedLine() == TRUE)
            {
                static std::deque<LETTERLIST_TEXT*> letterlist;
                letterlist.clear();
                if (m_LetterListBox.GetCheckedLines(&letterlist) == 0) break;
                for (std::deque<LETTERLIST_TEXT*>::iterator iter = letterlist.begin(); iter != letterlist.end(); ++iter)
                {
                    SocketClient->ToGameServer()->SendLetterDeleteRequest((*iter)->m_dwLetterID);
                }
                //m_LetterListBox.Scrolling(0);
            }
        }
        break;
    default:
        break;
    }
    return FALSE;
}

void CUILetterBoxTabWindow::DoActionSub(BOOL bMessageOnly)
{
    m_LetterListBox.DoAction(bMessageOnly);

    m_WriteButton.DoAction(bMessageOnly);
    m_ReadButton.DoAction(bMessageOnly);
    m_ReplyButton.DoAction(bMessageOnly);
    m_DeleteButton.DoAction(bMessageOnly);
    //	m_DeliveryButton.DoAction(bMessageOnly);
}

void CUILetterBoxTabWindow::DoMouseActionSub()
{
    if (MouseLButton)
    {
        if (CheckMouseIn(RPos_x(0), RPos_y(0), 10, 19) == TRUE)
        {
            if (g_dwTopWindow != 0);
            else if (m_bCheckAllState == FALSE)
            {
                PlayBuffer(SOUND_CLICK01);
                CheckAll(TRUE);
            }
            else
            {
                PlayBuffer(SOUND_CLICK01);
                CheckAll(FALSE);
            }
            MouseLButton = FALSE;
        }
        else if (CheckMouseIn(RPos_x(0) + 10, RPos_y(0), m_LetterListBox.GetColumnWidth(0) - 10, 19) == TRUE)
        {
            if (g_pLetterList->GetCurrentSortType() != 0)
            {
                PlayBuffer(SOUND_CLICK01);
                g_pLetterList->Sort(0);
                RefreshLetterList();
            }
            MouseLButton = FALSE;
        }
        else if (CheckMouseIn(RPos_x(0) + m_LetterListBox.GetColumnPos_x(1), RPos_y(0), m_LetterListBox.GetColumnWidth(1), 19) == TRUE)
        {
            if (g_pLetterList->GetCurrentSortType() != 1)
            {
                PlayBuffer(SOUND_CLICK01);
                g_pLetterList->Sort(1);
                RefreshLetterList();
            }
            MouseLButton = FALSE;
        }
        else if (CheckMouseIn(RPos_x(0) + m_LetterListBox.GetColumnPos_x(2), RPos_y(0), m_LetterListBox.GetColumnWidth(2), 19) == TRUE)
        {
            if (g_pLetterList->GetCurrentSortType() != 2)
            {
                PlayBuffer(SOUND_CLICK01);
                g_pLetterList->Sort(2);
                RefreshLetterList();
            }
            MouseLButton = FALSE;
        }
        else if (CheckMouseIn(RPos_x(0) + m_LetterListBox.GetColumnPos_x(3), RPos_y(0), m_LetterListBox.GetColumnWidth(3), 19) == TRUE)
        {
            if (g_pLetterList->GetCurrentSortType() != 3)
            {
                PlayBuffer(SOUND_CLICK01);
                g_pLetterList->Sort(3);
                RefreshLetterList();
            }
            MouseLButton = FALSE;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIFriendWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    SetTitle(pszTitle);
    SetParentUIID(dwParentID);

    SetPosition(50, 50);
    //SetSize(213, 170);
    SetSize(250, 170);
    SetLimitSize(250, 150);

    m_FriendListWnd.Init(GlobalText[1032], GetUIID());
    m_ChatRoomListWnd.Init(GlobalText[1033], GetUIID());
    m_LetterBoxWnd.Init(GlobalText[1034], GetUIID());

    g_pWindowMgr->AddWindowFinder(&m_FriendListWnd);
    g_pWindowMgr->AddWindowFinder(&m_ChatRoomListWnd);
    g_pWindowMgr->AddWindowFinder(&m_LetterBoxWnd);

    m_FriendListWnd.SetArrangeType(0, 0, 21);
    m_FriendListWnd.SetResizeType(3, 0, -19);

    m_ChatRoomListWnd.SetArrangeType(0, 0, 21);
    m_ChatRoomListWnd.SetResizeType(3, 0, -19);

    m_LetterBoxWnd.SetArrangeType(0, 0, 21);
    m_LetterBoxWnd.SetResizeType(3, 0, -19);
}

void CUIFriendWindow::Reset()
{
    SetTabIndex(0);
}

void CUIFriendWindow::Close()
{
    if (g_pWindowMgr->GetWindow(GetUIID())->GetState() == UISTATE_NORMAL)
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
}

void CUIFriendWindow::Refresh()
{
    m_FriendListWnd.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_ChatRoomListWnd.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_LetterBoxWnd.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

    m_FriendListWnd.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_ChatRoomListWnd.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
    m_LetterBoxWnd.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);

    m_FriendListWnd.Refresh();
    m_ChatRoomListWnd.Refresh();
    m_LetterBoxWnd.Refresh();

    g_dwKeyFocusUIID = m_FriendListWnd.GetKeyMoveListUIID();
}

void RenderTabLine(int iPos_x, int iPos_y, int iTabWidth, int iTabHeight, int iTabNum, int iSelectNum)
{
    for (int i = 0; i < iTabNum; ++i)
    {
        SetLineColor(2);
        auto fRPos_x = float(iPos_x + i * iTabWidth);
        if (i == iSelectNum)
        {
            RenderColor((float)fRPos_x, (float)iPos_y, (float)iTabWidth, (float)1);
            RenderColor((float)fRPos_x - 1, (float)iPos_y, (float)1, (float)iTabHeight);
            RenderColor((float)fRPos_x + iTabWidth - 1, (float)iPos_y, (float)1, (float)iTabHeight);
            SetLineColor(5);
            RenderColor((float)fRPos_x, (float)iPos_y + 1, (float)iTabWidth - 1, (float)iTabHeight - 1);
        }
        else
        {
            RenderColor((float)fRPos_x, (float)iPos_y + 1, (float)iTabWidth, (float)1);
            RenderColor((float)fRPos_x + iTabWidth - 1, (float)iPos_y + 1, (float)1, (float)iTabHeight - 1);
            RenderColor((float)fRPos_x, (float)iPos_y + iTabHeight - 1, (float)iTabWidth, (float)1);
            SetLineColor(6);
            RenderColor((float)fRPos_x, (float)iPos_y + 2, (float)iTabWidth - 1, (float)iTabHeight - 3);
        }
    }
}

void CUIFriendWindow::RenderSub()
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_FriendListWnd.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_ChatRoomListWnd.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_LetterBoxWnd.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_FriendListWnd.SetState(UISTATE_MOVE);
        m_ChatRoomListWnd.SetState(UISTATE_MOVE);
        m_LetterBoxWnd.SetState(UISTATE_MOVE);

        if (GetState() == UISTATE_RESIZE)
        {
            m_FriendListWnd.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
            m_ChatRoomListWnd.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
            m_LetterBoxWnd.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
            m_FriendListWnd.SetState(UISTATE_RESIZE);
            m_ChatRoomListWnd.SetState(UISTATE_RESIZE);
            m_LetterBoxWnd.SetState(UISTATE_RESIZE);
        }
    }

    switch (m_iTabIndex)
    {
    case 0:
        m_FriendListWnd.Render();
        break;
    case 1:
        m_LetterBoxWnd.Render();
        break;
    case 2:
        m_ChatRoomListWnd.Render();
        break;
    default:
        break;
    }

    EnableAlphaTest();

    SetLineColor(7);
    RenderColor((float)RPos_x(0), (float)RPos_y(0), (float)RWidth(), (float)20);
    RenderTabLine(RPos_x(0), RPos_y(2), 53, 19, 3, m_iTabIndex);
    SetLineColor(2);
    RenderColor(float(RPos_x(53 * 3)), float(RPos_y(2) + 19 - 1), float(RWidth() - 53 * 3), (float)1);
    EndRenderColor();

    SIZE TextSize;
    int TextLen;
    g_pRenderText->SetBgColor(0);

    if (m_FriendListWnd.GetTitle() != NULL)
    {
        if (m_iTabIndex == 0 || m_iTabMouseOverIndex == 0)
        {
            g_pRenderText->SetTextColor(255, 255, 255, 255);
        }
        else
        {
            g_pRenderText->SetTextColor(230, 220, 200, 255);
        }

        TextLen = lstrlen(m_FriendListWnd.GetTitle());

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_FriendListWnd.GetTitle(), TextLen, &TextSize);
        g_pRenderText->RenderText(RPos_x(0) + (52 - (float)TextSize.cx / g_fScreenRate_x + 0.5f) / 2,
            RPos_y(0) + (24 - (float)TextSize.cy / g_fScreenRate_y + 0.5f) / 2, m_FriendListWnd.GetTitle());
    }
    if (m_LetterBoxWnd.GetTitle() != NULL)
    {
        if (m_iTabIndex == 1 || m_iTabMouseOverIndex == 1)
        {
            g_pRenderText->SetTextColor(255, 255, 255, 255);
        }
        else
        {
            g_pRenderText->SetTextColor(230, 220, 200, 255);
        }
        TextLen = lstrlen(m_LetterBoxWnd.GetTitle());

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_LetterBoxWnd.GetTitle(), TextLen, &TextSize);
        g_pRenderText->RenderText(RPos_x(54) + (52 - (float)TextSize.cx / g_fScreenRate_x + 0.5f) / 2, RPos_y(0) + (24 - (float)TextSize.cy / g_fScreenRate_y + 0.5f) / 2, m_LetterBoxWnd.GetTitle());
    }
    if (m_ChatRoomListWnd.GetTitle() != NULL)
    {
        if (m_iTabIndex == 2 || m_iTabMouseOverIndex == 2)
        {
            g_pRenderText->SetTextColor(255, 255, 255, 255);
        }
        else
        {
            g_pRenderText->SetTextColor(230, 220, 200, 255);
        }
        TextLen = lstrlen(m_ChatRoomListWnd.GetTitle());

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_ChatRoomListWnd.GetTitle(), TextLen, &TextSize);
        g_pRenderText->RenderText(RPos_x(107) + (52 - (float)TextSize.cx / g_fScreenRate_x + 0.5f) / 2, RPos_y(0) + (24 - (float)TextSize.cy / g_fScreenRate_y + 0.5f) / 2, m_ChatRoomListWnd.GetTitle());
    }

    g_pRenderText->SetTextColor(230, 220, 200, 255);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), GlobalText[1035], GlobalText.GetStringSize(1035), &TextSize);
    g_pRenderText->RenderText(RPos_x(0) + RWidth() - (float)TextSize.cx / g_fScreenRate_x - 2, RPos_y(0) + (24 - (float)TextSize.cy / g_fScreenRate_y + 0.5f) / 2, GlobalText[1035]);

    float fCheckBoxPos_x = RPos_x(0) + RWidth() - (float)TextSize.cx / g_fScreenRate_x - 2 - 14;
    float fCheckBoxPos_y = RPos_y(0) + (24 - (float)TextSize.cy / g_fScreenRate_y + 0.5f) / 2;

    RenderCheckBox(fCheckBoxPos_x - 1, fCheckBoxPos_y - 1, g_pWindowMgr->GetChatReject());
}

BOOL CUIFriendWindow::HandleMessage()
{
    switch (m_WorkMessage.m_iMessage)
    {
    case UI_MESSAGE_SELECTED:
        switch (m_iTabIndex)
        {
        case 0:
            g_dwKeyFocusUIID = m_FriendListWnd.GetKeyMoveListUIID();
            break;
        case 1:
            g_dwKeyFocusUIID = m_LetterBoxWnd.GetKeyMoveListUIID();
            break;
        case 2:
            g_dwKeyFocusUIID = m_ChatRoomListWnd.GetKeyMoveListUIID();
            break;
        default:
            break;
        }
        break;
    case UI_MESSAGE_YNRETURN:
        if (m_WorkMessage.m_iParam2 == 1)
        {
            if (g_pWindowMgr->GetChatReject() == FALSE)
            {
                SocketClient->ToGameServer()->SendSetFriendOnlineState(0);
                g_pWindowMgr->SetChatReject(TRUE);
                g_pFriendMenu->CloseAllChatWindow();
            }
        }
        break;
    default:
        break;
    }

    return FALSE;
}

void CUIFriendWindow::DoActionSub(BOOL bMessageOnly)
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE);
    else
    {
        switch (m_iTabIndex)
        {
        case 0:
            m_FriendListWnd.DoAction(bMessageOnly);
            break;
        case 1:
            m_LetterBoxWnd.DoAction(bMessageOnly);
            break;
        case 2:
            m_ChatRoomListWnd.DoAction(bMessageOnly);
            break;
        default:
            break;
        }
    }
}

void CUIFriendWindow::DoMouseActionSub()
{
    if (GetState() == UISTATE_RESIZE) return;

    BOOL bChangeTab = FALSE;
    if (CheckMouseIn(RPos_x(0), RPos_y(2), 53, 19) == TRUE)
    {
        m_iTabMouseOverIndex = 0;
        if (MouseLButton)
        {
            if (m_iTabIndex != 0)
            {
                bChangeTab = TRUE;
                PlayBuffer(SOUND_CLICK01);
            }
            m_iTabIndex = 0;
            g_dwKeyFocusUIID = m_FriendListWnd.GetKeyMoveListUIID();
            MouseLButton = FALSE;
            Refresh();
        }
    }
    else if (CheckMouseIn(RPos_x(53), RPos_y(2), 53, 19) == TRUE)
    {
        m_iTabMouseOverIndex = 1;
        if (MouseLButton)
        {
            if (m_iTabIndex != 1)
            {
                bChangeTab = TRUE;
                PlayBuffer(SOUND_CLICK01);
            }
            m_iTabIndex = 1;
            g_dwKeyFocusUIID = m_LetterBoxWnd.GetKeyMoveListUIID();
            MouseLButton = FALSE;
            Refresh();
        }
    }
    else if (CheckMouseIn(RPos_x(106), RPos_y(2), 53, 19) == TRUE)
    {
        m_iTabMouseOverIndex = 2;
        if (MouseLButton)
        {
            if (m_iTabIndex != 2)
            {
                bChangeTab = TRUE;
                PlayBuffer(SOUND_CLICK01);
            }
            m_iTabIndex = 2;
            g_dwKeyFocusUIID = m_ChatRoomListWnd.GetKeyMoveListUIID();
            MouseLButton = FALSE;
            Refresh();
        }
    }
    else
    {
        m_iTabMouseOverIndex = m_iTabIndex;
        SIZE TextSize;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), GlobalText[1035], GlobalText.GetStringSize(1035), &TextSize);

        if (CheckMouseIn(RPos_x(0) + RWidth() - TextSize.cx - 2 - 14,
            RPos_y(4), TextSize.cx + 2 + 14, 20) == TRUE)
        {
            if (MouseLButtonPop)
            {
                PlayBuffer(SOUND_CLICK01);
                if (g_pWindowMgr->GetChatReject() == TRUE)
                {
                    SocketClient->ToGameServer()->SendSetFriendOnlineState(1);
                    g_pWindowMgr->SetChatReject(FALSE);
                }
                else
                {
                    g_pWindowMgr->AddWindow(UIWNDTYPE_QUESTION, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1036], GetUIID());
                }
                MouseLButtonPop = FALSE;
            }
        }
    }

    if (bChangeTab == TRUE)
    {
        m_FriendListWnd.Refresh();
        m_ChatRoomListWnd.Refresh();
        m_LetterBoxWnd.Refresh();
    }
}

void CUITextInputWindow::InitControls()
{
    m_TextInputBox.Init(g_hWnd, 80, 14, 10);
    m_TextInputBox.SetParentUIID(m_dwUIID);
    m_TextInputBox.SetFont(g_hFont);
    m_TextInputBox.SetOption(UIOPTION_PAINTBACK);
    m_TextInputBox.SetBackColor(0, 0, 0, 0);
    m_TextInputBox.SetParentUIID(GetUIID());
    m_TextInputBox.SetArrangeType(0, 30, 14);
    m_TextInputBox.SetState(UISTATE_NORMAL);

    m_AddButton.Init(1, GlobalText[228]);
    m_AddButton.SetParentUIID(GetUIID());
    m_AddButton.SetArrangeType(0, 18, 40);
    m_AddButton.SetSize(50, 20);

    m_CancelButton.Init(2, GlobalText[229]);
    m_CancelButton.SetParentUIID(GetUIID());
    m_CancelButton.SetArrangeType(0, 73, 40);
    m_CancelButton.SetSize(50, 20);
    Refresh();
}


void CUITextInputWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    SetTitle(pszTitle);
    SetParentUIID(0);
    m_dwReturnWindowUIID = dwParentID;

    SetPosition(50, 50);
    //SetSize(213, 170);
    SetSize(150, 100);
    SetLimitSize(150, 100);
    SetOption(UIWINDOWSTYLE_FIXED);
}

void CUITextInputWindow::Refresh()
{
    m_AddButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_CancelButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_TextInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

    m_TextInputBox.GiveFocus();
    m_bHaveTextBox = TRUE;
}

void CUITextInputWindow::RenderSub()
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_AddButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_CancelButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_TextInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    }

    m_AddButton.Render();
    m_CancelButton.Render();
    EnableAlphaTest();
    m_TextInputBox.Render();
    DisableAlphaBlend();
}

void CUITextInputWindow::ReturnText()
{
    wchar_t* pszReturnText = new wchar_t[MAX_TEXT_LENGTH + 1];
    m_TextInputBox.GetText(pszReturnText);
    m_TextInputBox.SetText(NULL);
    if (pszReturnText[0] == '\0') return;

    g_pWindowMgr->SendUIMessageToWindow(m_dwReturnWindowUIID, UI_MESSAGE_TXTRETURN, GetUIID(), (DWORD)pszReturnText);
    g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
}

BOOL CUITextInputWindow::HandleMessage()
{
    switch (m_WorkMessage.m_iMessage)
    {
    case UI_MESSAGE_SELECTED:
        m_TextInputBox.GiveFocus();
        break;
    case UI_MESSAGE_TEXTINPUT:
    {
        ReturnText();
    }
    break;
    case UI_MESSAGE_BTNLCLICK:
    {
        switch (m_WorkMessage.m_iParam1)
        {
        case 1:
            ReturnText();
            break;
        case 2:
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
            break;
        default:
            break;
        }
    }
    break;
    default:
        break;
    }
    return FALSE;
}

void CUITextInputWindow::DoActionSub(BOOL bMessageOnly)
{
    m_AddButton.DoAction(bMessageOnly);
    m_CancelButton.DoAction(bMessageOnly);
    m_TextInputBox.DoAction(bMessageOnly);
}

void CUITextInputWindow::DoMouseActionSub()
{
    //	if (g_dwMouseUseUIID == GetUIID() && MouseLButton == true)
    //	{
    //		m_TextInputBox.GiveFocus();
    //	}
}

void CUIQuestionWindow::Init(const wchar_t* pszTitle, DWORD dwParentID)
{
    if (m_iDialogType == 0) SetTitle(GlobalText[991]);
    else if (m_iDialogType == 1) SetTitle(GlobalText[228]);
    SetParentUIID(0);
    m_dwReturnWindowUIID = dwParentID;
    memset(m_szCaption, 0, sizeof(m_szCaption));
    memset(m_szSaveID, 0, sizeof(m_szSaveID));
    CutText3(pszTitle, m_szCaption[0], 125, 2, 256);

    SetPosition(50, 50);
    //SetSize(213, 170);
    SetSize(150, 100);
    SetLimitSize(150, 100);
    SetOption(UIWINDOWSTYLE_FIXED);

    if (m_iDialogType == 0)
    {
        m_AddButton.Init(1, GlobalText[1037]);
        m_AddButton.SetParentUIID(GetUIID());
        m_AddButton.SetArrangeType(0, 18, 40);
        m_AddButton.SetSize(50, 20);

        m_CancelButton.Init(2, GlobalText[1038]);
        m_CancelButton.SetParentUIID(GetUIID());
        m_CancelButton.SetArrangeType(0, 73, 40);
        m_CancelButton.SetSize(50, 20);
    }
    else if (m_iDialogType == 1)
    {
        m_AddButton.Init(1, GlobalText[228]);
        m_AddButton.SetParentUIID(GetUIID());
        m_AddButton.SetArrangeType(0, 45, 40);
        m_AddButton.SetSize(50, 20);
    }
}
extern int KeyState[256];

void CUIQuestionWindow::Refresh()
{
    m_AddButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    m_CancelButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);

    KeyState[VK_RETURN] = true;
}

void CUIQuestionWindow::RenderSub()
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_AddButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_CancelButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    }

    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0);

    SIZE TextSize;
    g_pRenderText->RenderText(RPos_x(5), RPos_y(8), m_szCaption[0], 0, 0, RT3_SORT_LEFT, &TextSize);
    if (m_szCaption[1][0] != '\0')
        g_pRenderText->RenderText(RPos_x(5), RPos_y(8 + TextSize.cy), m_szCaption[1]);

    m_AddButton.Render();
    if (m_iDialogType == 0) m_CancelButton.Render();
}

BOOL CUIQuestionWindow::HandleMessage()
{
    switch (m_WorkMessage.m_iMessage)
    {
    case UI_MESSAGE_SELECTED:
        break;
    case UI_MESSAGE_BTNLCLICK:
    {
        switch (m_WorkMessage.m_iParam1)
        {
        case 1:
            if (m_dwReturnWindowUIID == -1)
            {
                SocketClient->ToGameServer()->SendFriendAddResponse(0x01, m_szSaveID);
            }
            else if (m_dwReturnWindowUIID != 0)
            {
                g_pWindowMgr->SendUIMessageToWindow(m_dwReturnWindowUIID, UI_MESSAGE_YNRETURN, GetUIID(), 1);
            }
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
            break;
        case 2:
            if (m_iDialogType != 0) break;
            if (m_dwReturnWindowUIID == -1)
            {
                SocketClient->ToGameServer()->SendFriendAddResponse(0x00, m_szSaveID);
            }
            else if (m_dwReturnWindowUIID != 0)
            {
                g_pWindowMgr->SendUIMessageToWindow(m_dwReturnWindowUIID, UI_MESSAGE_YNRETURN, GetUIID(), 0);
            }
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, GetUIID(), 0);
            break;
        default:
            break;
        }
    }
    break;
    default:
        break;
    }
    return FALSE;
}

void CUIQuestionWindow::DoActionSub(BOOL bMessageOnly)
{
    if (g_pWindowMgr->GetTopWindowUIID() == GetUIID() && PressKey(VK_RETURN))
    {
        SendUIMessage(UI_MESSAGE_BTNLCLICK, 1, 0);
    }

    m_AddButton.DoAction(bMessageOnly);
    m_CancelButton.DoAction(bMessageOnly);
}

void CUIQuestionWindow::SaveID(const wchar_t* pszText)
{
    if (pszText[0] != '\0')
    {
        wcsncpy(m_szSaveID, pszText, MAX_ID_SIZE);
        m_szSaveID[MAX_ID_SIZE] = '\0';
    }
    else
        m_szSaveID[0] = '\0';
}

void CUIFriendMenu::Reset()
{
    if (m_WindowList.size() != 0)
        m_WindowListSelectIter = m_WindowList.end();

    m_WindowList.clear();
    m_NewChatWindowList.clear();
    RemoveAllRequestWindow();
}

void CUIFriendMenu::Init()
{
    memset(&m_WorkMessage, 0, sizeof(UI_MESSAGE));
    m_iMouseClickPos_x = 0;
    m_iMouseClickPos_y = 0;
    m_iMinWidth = 100;
    m_iMinHeight = 100;
    m_iMaxWidth = 0;
    m_iMaxHeight = 0;
    SetOption(UIWINDOWSTYLE_NULL);
    m_bHaveTextBox = FALSE;
    m_iControlButtonClick = 0;

    m_iFriendMenuPos_y = 459 - 24;
    m_iFriendMenuHeight = 18;
    SetPosition(582, m_iFriendMenuPos_y);
    SetSize(52, 0);	//m_iFriendMenuHeight);
    m_fLineHeight = 0;
    m_WindowListSelectIter = m_WindowList.end();
    SetState(UISTATE_HIDE);
    m_fMenuAlpha = 0;
    m_fMenuAlphaAdd = 0;
    m_bNewMailAlert = FALSE;
    m_iBlinkTemp = 0;
    m_iLetterBlink = 0;
    m_bHotKey = FALSE;
}

void CUIFriendMenu::AddWindow(DWORD dwUIID, CUIBaseWindow* pWindow)
{
    if (pWindow == NULL)		return;

    m_WindowList.push_back(dwUIID);
}

void CUIFriendMenu::RemoveWindow(DWORD dwUIID)
{
    BOOL bFind = FALSE;
    for (m_WindowListIter = m_WindowList.begin(); m_WindowListIter != m_WindowList.end(); ++m_WindowListIter)
    {
        if (*m_WindowListIter == dwUIID)
        {
            bFind = TRUE;
            break;
        }
    }
    if (bFind == FALSE) return;
    m_WindowList.erase(m_WindowListIter);
    m_WindowListSelectIter = m_WindowList.end();
}

BOOL CUIFriendMenu::HandleMessage()
{
    return FALSE;
}

void CUIFriendMenu::DoActionSub(BOOL bMessageOnly)
{
    if (m_bHotKey == TRUE && PressKey(VK_RETURN))
    {
        if (m_WindowListSelectIter != m_WindowList.end() && g_pWindowMgr->GetWindow(*m_WindowListSelectIter) != NULL)
        {
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, *m_WindowListSelectIter, 0);
        }
        HideMenu();
        m_bHotKey = FALSE;
    }
}

void CUIFriendMenu::DoMouseActionSub()
{
    int iLineHeight = (m_fLineHeight + 4) * m_WindowList.size();

    if (GetState() == UISTATE_NORMAL)
    {
        if (CheckMouseIn(m_iPos_x, m_iFriendMenuPos_y - iLineHeight, m_iWidth, iLineHeight) == TRUE)
        {
            m_bHotKey = FALSE;
            g_pWindowMgr->SetWindowsEnable(m_dwUIID);

            int iSelectLine = m_WindowList.size() - (MouseY - m_iFriendMenuPos_y + iLineHeight) / (m_fLineHeight + 4);
            m_WindowListIter = m_WindowList.begin();
            for (int i = 0; i < iSelectLine; ++i)
            {
                ++m_WindowListIter;
                if (m_WindowListIter == m_WindowList.end()) break;
            }
            m_WindowListSelectIter = m_WindowListIter;
            if (MouseLButtonPop && GetState() == UISTATE_NORMAL)
            {
                SetFocus(g_hWnd);
                PlayBuffer(SOUND_CLICK01);
                MouseLButtonPop = FALSE;
                if (g_pWindowMgr->GetWindow(*m_WindowListSelectIter) == NULL);
                else if (g_pWindowMgr->GetWindow(*m_WindowListSelectIter)->GetState() == UISTATE_HIDE ||
                    g_pWindowMgr->GetTopNotMainWindowUIID() != *m_WindowListSelectIter)
                {
                    g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, *m_WindowListSelectIter, 0);
                    MouseLButton = false;
                }
                else
                {
                    g_pWindowMgr->SendUIMessage(UI_MESSAGE_HIDE, *m_WindowListSelectIter, 0);
                }
                HideMenu();
            }
            MouseOnWindow = true;
        }
        else if (m_bHotKey == FALSE)
            m_WindowListSelectIter = m_WindowList.end();
    }
}

int CUIFriendMenu::GetBlinkTemp()
{
    return m_iBlinkTemp;
}

void CUIFriendMenu::IncreaseBlinkTemp()
{
    m_iBlinkTemp++;

    if (m_iBlinkTemp > 23)
    {
        m_iBlinkTemp = 0;
    }
}

int CUIFriendMenu::GetLetterBlink()
{
    return m_iLetterBlink;
}

void CUIFriendMenu::IncreaseLetterBlink()
{
    m_iLetterBlink++;

    if (m_iLetterBlink > 5)
    {
        m_iLetterBlink = 0;
        m_bNewMailAlert = FALSE;
    }
}

void CUIFriendMenu::RenderFriendButton()
{
    BOOL bIsAlertTime = (m_iBlinkTemp % 24 < 12);
    if (g_pWindowMgr->GetFriendMainWindow() != NULL)
    {
        RenderBitmap(BITMAP_INTERFACE_EX + 18, m_iPos_x, m_iFriendMenuPos_y, m_iWidth, m_iFriendMenuHeight,
            0.f, 0.f, m_iWidth / 64.f, m_iFriendMenuHeight / 32.f);

        if (g_pFriendMenu->IsNewChatAlert() && bIsAlertTime)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 17, m_iPos_x + 7, m_iFriendMenuPos_y + 3, 25.f, 14.f,
                0.f, 14.f / 32.f, 25.f / 32.f, 14.f / 32.f);
        }
        if (g_pFriendMenu->IsNewMailAlert())
        {
            if (g_pLetterList->CheckNoReadLetter() == FALSE)
                g_pFriendMenu->SetNewMailAlert(FALSE);

            if (bIsAlertTime)
            {
                RenderBitmap(BITMAP_INTERFACE_EX + 16, m_iPos_x + 36, m_iFriendMenuPos_y + 7, 15.f, 9.f,
                    0.f, 0.f, 15.f / 16.f, 9.f / 16.f);
                if (m_iBlinkTemp % 24 == 11) ++m_iLetterBlink;
                if (m_iLetterBlink > 5)
                {
                    m_iLetterBlink = 0;
                    g_pFriendMenu->SetNewMailAlert(FALSE);
                }
            }
        }
        else if (g_pLetterList->CheckNoReadLetter())
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 16, m_iPos_x + 36, m_iFriendMenuPos_y + 7, 15.f, 9.f,
                0.f, 0.f, 15.f / 16.f, 9.f / 16.f);
        }
    }
    else
    {
        if (g_pFriendMenu->IsNewChatAlert() && bIsAlertTime)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 17, m_iPos_x + 4, m_iFriendMenuPos_y + 2, 25.f, 14.f,
                0.f, 0.f / 32.f, 25.f / 32.f, 14.f / 32.f);
        }
        if (g_pFriendMenu->IsNewMailAlert())
        {
            if (bIsAlertTime)
            {
                RenderBitmap(BITMAP_INTERFACE_EX + 16, m_iPos_x + 33, m_iFriendMenuPos_y + 5, 15.f, 9.f,
                    0.f, 0.f, 15.f / 16.f, 9.f / 16.f);
                if (m_iBlinkTemp % 24 == 11) ++m_iLetterBlink;
                if (m_iLetterBlink > 5)
                {
                    m_iLetterBlink = 0;
                    g_pFriendMenu->SetNewMailAlert(FALSE);
                }
            }
        }
        else if (g_pLetterList->CheckNoReadLetter())
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 16, m_iPos_x + 33, m_iFriendMenuPos_y + 5, 15.f, 9.f,
                0.f, 0.f, 15.f / 16.f, 9.f / 16.f);
        }
    }
    ++m_iBlinkTemp;
}

void CUIFriendMenu::RenderSub()
{
    if (m_fLineHeight == 0)
    {
        SIZE TextSize;
        GetTextExtentPoint32(g_pRenderText->GetFontDC(), L"0", 1, &TextSize);

        m_fLineHeight = TextSize.cy / g_fScreenRate_y;
    }
    m_fMenuAlpha += m_fMenuAlphaAdd;
    if (m_fMenuAlpha < 0.0f)
    {
        m_fMenuAlphaAdd = 0;
        m_fMenuAlpha = 0;
    }
    else if (m_fMenuAlpha > 1.0f)
    {
        m_fMenuAlphaAdd = 0;
        m_fMenuAlpha = 1.0f;
    }

    EnableAlphaTest();

    if (m_WindowList.empty() == false && GetState() == UISTATE_NORMAL)
    {
        int iSize = 0;
        for (m_WindowListIter = m_WindowList.begin(); m_WindowListIter != m_WindowList.end(); ++m_WindowListIter, ++iSize)
        {
            assert(g_pWindowMgr->GetWindow(*m_WindowListIter) != NULL);
            if (g_pWindowMgr->GetWindow(*m_WindowListIter)->GetState() == UISTATE_READY)
            {
                --iSize;
                continue;
            }
        }
        if (iSize != 0)
        {
            int iLineHeight = (m_fLineHeight + 4) * iSize;

            SetLineColor(3, m_fMenuAlpha);
            RenderColor(m_iPos_x, m_iFriendMenuPos_y - iLineHeight, m_iWidth, iLineHeight);
            SetLineColor(15, m_fMenuAlpha);
            RenderColor(m_iPos_x, m_iFriendMenuPos_y - iLineHeight, m_iWidth, 1);
            RenderColor(m_iPos_x, m_iFriendMenuPos_y - iLineHeight, 1, iLineHeight);
            RenderColor(m_iPos_x + m_iWidth - 1, m_iFriendMenuPos_y - iLineHeight, 1, iLineHeight);
            RenderColor(m_iPos_x, m_iFriendMenuPos_y, m_iWidth, 1);
            EndRenderColor();

            RenderWindowList();
        }
    }
    DisableAlphaBlend();
}

void CUIFriendMenu::RenderWindowList()
{
    int i = 1;
    for (m_WindowListIter = m_WindowList.begin(); m_WindowListIter != m_WindowList.end(); ++m_WindowListIter, ++i)
    {
        BOOL bAlert = FALSE;
        if (g_pWindowMgr->GetWindow(*m_WindowListIter)->GetState() == UISTATE_READY)
        {
            --i;
            continue;
        }
        BYTE bAlpha = m_fMenuAlpha * 255;

        for (std::deque<DWORD>::iterator iter = m_NewChatWindowList.begin(); iter != m_NewChatWindowList.end(); ++iter)
        {
            if (*iter == *m_WindowListIter)
            {
                bAlert = TRUE;
            }
        }

        if (m_WindowListSelectIter == m_WindowListIter || bAlert == TRUE)
        {
            EnableAlphaTest();
            if (m_WindowListSelectIter == m_WindowListIter)
                SetLineColor(16, m_fMenuAlpha);
            else
                SetLineColor(17, m_fMenuAlpha);
            RenderColor(m_iPos_x + 1, m_iFriendMenuPos_y - (m_fLineHeight + 4) * i + 1, m_iWidth - 2, m_fLineHeight + 3);
            EndRenderColor();
            g_pRenderText->SetTextColor(0, 0, 0, bAlpha);
        }
        else
        {
            g_pRenderText->SetTextColor(230, 220, 200, bAlpha);
        }

        wchar_t szText[64];
        g_pRenderText->SetBgColor(0);
        const wchar_t* pszChatTitleOriginal = g_pWindowMgr->GetWindow(*m_WindowListIter)->GetTitle();

        wchar_t temp[MAX_TEXT_LENGTH + 1] = { 0 };
        wcsncpy(temp, pszChatTitleOriginal, MAX_TEXT_LENGTH + 1);
        wchar_t* pszChatTitle = _wcstok(temp, L",");

        if (wcslen(pszChatTitle) > GlobalText.GetStringSize(994))
        {
            if (wcsncmp(pszChatTitle, GlobalText[995], GlobalText.GetStringSize(995)) == 0)
            {
                CutText3(pszChatTitle + GlobalText.GetStringSize(995) + GlobalText.GetStringSize(994), szText, m_iWidth - 8, 1, 64);
            }
            else
            {
                CutText3(pszChatTitle + GlobalText.GetStringSize(994), szText, m_iWidth - 8, 1, 64);
            }
        }
        else
        {
            wcscpy(szText, GlobalText[995]);
        }
        g_pRenderText->RenderText(m_iPos_x + 2, m_iFriendMenuPos_y - (m_fLineHeight + 4) * i + 3, szText);

        auto* pWindow = (CUIChatWindow*)g_pWindowMgr->GetWindow(*m_WindowListIter);
        if (pWindow != NULL && pWindow->GetUserCount() > 2)
        {
            glColor3f(255, 0, 0);
            RenderBitmap(BITMAP_INTERFACE_EX + 15, (float)m_iPos_x + m_iWidth - 7, (float)m_iFriendMenuPos_y - (m_fLineHeight + 4) * i + 5, (float)4, (float)6, 0.f, 0.f, 4.f / 8.f, 6.f / 8.f);
            glColor3f(255, 255, 255);
        }
    }
}

void CUIFriendMenu::ShowMenu(BOOL bHotKey)
{
    if (m_WindowList.empty() == TRUE) return;
    m_bHotKey = bHotKey;

    if (GetState() == UISTATE_HIDE)
    {
        SetState(UISTATE_NORMAL);
        m_iHeight += (m_fLineHeight + 4) * m_WindowList.size();
        m_iPos_y -= (m_fLineHeight + 4) * m_WindowList.size();
        m_fMenuAlphaAdd = 0.25f;

        if (bHotKey == TRUE)
        {
            m_WindowListSelectIter = m_WindowList.begin();
            if (m_WindowList.size() > 1)
            {
                while (*m_WindowListSelectIter == g_pWindowMgr->GetTopWindowUIID())
                {
                    ++m_WindowListSelectIter;
                    if (m_WindowListSelectIter == m_WindowList.end()) break;
                };
            }
        }
    }
    else if (bHotKey == TRUE)
    {
        if (m_WindowListSelectIter == m_WindowList.end()) m_WindowListSelectIter = m_WindowList.begin();
        else
        {
            ++m_WindowListSelectIter;
            if (m_WindowListSelectIter == m_WindowList.end()) m_WindowListSelectIter = m_WindowList.begin();
        }
    }
}

void CUIFriendMenu::HideMenu()
{
    if (GetState() == UISTATE_NORMAL)
    {
        SetState(UISTATE_HIDE);
        m_iHeight = 0;
        m_iPos_y = m_iFriendMenuPos_y;
        m_fMenuAlphaAdd = -0.25f;
        m_bHotKey = FALSE;
    }
}

void CUIFriendMenu::SetNewChatAlert(DWORD dwAlertWindowID)
{
    BOOL bFind = FALSE;
    for (m_WindowListIter = m_NewChatWindowList.begin(); m_WindowListIter != m_NewChatWindowList.end(); ++m_WindowListIter)
    {
        if (*m_WindowListIter == dwAlertWindowID)
        {
            bFind = TRUE;
            break;
        }
    }
    PlayBuffer(SOUND_FRIEND_CHAT_ALERT);

    if (bFind == FALSE)
        m_NewChatWindowList.push_back(dwAlertWindowID);
}

void CUIFriendMenu::SetNewChatAlertOff(DWORD dwAlertWindowID)
{
    if (m_NewChatWindowList.empty() == TRUE) return;

    BOOL bFind = FALSE;
    for (m_WindowListIter = m_NewChatWindowList.begin(); m_WindowListIter != m_NewChatWindowList.end(); ++m_WindowListIter)
    {
        if (*m_WindowListIter == dwAlertWindowID)
        {
            bFind = TRUE;
            break;
        }
    }
    if (bFind == TRUE)
        m_NewChatWindowList.erase(m_WindowListIter);
}

BOOL CUIFriendMenu::IsNewChatAlert()
{
    if (m_NewChatWindowList.empty() == FALSE)
        return TRUE;
    else
        return FALSE;
}

void CUIFriendMenu::SetNewMailAlert(BOOL bAlert)
{
    m_bNewMailAlert = bAlert;
}

DWORD CUIFriendMenu::CheckChatRoomDuplication(const wchar_t* pszTargetName)
{
    for (m_WindowListIter = m_WindowList.begin(); m_WindowListIter != m_WindowList.end(); ++m_WindowListIter)
    {
        int iResult;
        const wchar_t* pName = ((CUIChatWindow*)g_pWindowMgr->GetWindow(*m_WindowListIter))->GetChatFriend(&iResult);
        if (iResult == 2 || iResult == 0)
        {
            continue;
        }
        else if (pName == NULL || pName[0] == '\0')
        {
            return MCI_SEQ_MAPPER;
        }
        else if (wcsncmp(pName, pszTargetName, MAX_ID_SIZE) == 0)
        {
            return *m_WindowListIter;
        }
    }
    return 0;
}

void CUIFriendMenu::SendChatRoomConnectCheck()
{
    for (m_WindowListIter = m_WindowList.begin(); m_WindowListIter != m_WindowList.end(); ++m_WindowListIter)
    {
        auto* pChatWindow = (CUIChatWindow*)g_pWindowMgr->GetWindow(*m_WindowListIter);
        if (pChatWindow != nullptr)
        {
            Connection* pSocket = pChatWindow->GetCurrentSocket();
            if (pSocket != nullptr
                && pSocket->ToChatServer() != nullptr)
            {
                pSocket->ToChatServer()->SendKeepAlive();
            }
        }
    }
}

void CUIFriendMenu::UpdateAllChatWindowInviteList()
{
    CUIChatWindow* pChatWindow = NULL;
    for (m_WindowListIter = m_WindowList.begin(); m_WindowListIter != m_WindowList.end(); ++m_WindowListIter)
    {
        pChatWindow = (CUIChatWindow*)g_pWindowMgr->GetWindow(*m_WindowListIter);
        if (pChatWindow != NULL)
        {
            if (pChatWindow->GetShowType() == 2)
                pChatWindow->UpdateInvitePalList();
        }
    }
}

void CUIFriendMenu::AddRequestWindow(const wchar_t* szTargetName)
{
    if (szTargetName == NULL) return;
    if (wcslen(szTargetName) > MAX_ID_SIZE) return;
    wchar_t* pszName = new wchar_t[MAX_ID_SIZE + 1];
    wcsncpy(pszName, szTargetName, MAX_ID_SIZE);
    pszName[MAX_ID_SIZE] = '\0';
    m_RequestChatWindowList.push_back(pszName);
}

BOOL CUIFriendMenu::IsRequestWindow(const wchar_t* szTargetName)
{
    for (m_RequestChatWindowListIter = m_RequestChatWindowList.begin(); m_RequestChatWindowListIter != m_RequestChatWindowList.end(); ++m_RequestChatWindowListIter)
    {
        if (wcsncmp(*m_RequestChatWindowListIter, szTargetName, MAX_ID_SIZE) == 0) return TRUE;
    }
    return FALSE;
}

void CUIFriendMenu::RemoveRequestWindow(const wchar_t* szTargetName)
{
    BOOL bFind = FALSE;
    for (m_RequestChatWindowListIter = m_RequestChatWindowList.begin(); m_RequestChatWindowListIter != m_RequestChatWindowList.end(); ++m_RequestChatWindowListIter)
    {
        if (wcsncmp(*m_RequestChatWindowListIter, szTargetName, MAX_ID_SIZE) == 0)
        {
            bFind = TRUE;
            break;
        }
    }
    if (bFind == TRUE)
    {
        if (*m_RequestChatWindowListIter != NULL)
        {
            delete[] * m_RequestChatWindowListIter;
            *m_RequestChatWindowListIter = NULL;
        }
        m_RequestChatWindowList.erase(m_RequestChatWindowListIter);
    }
}

void CUIFriendMenu::RemoveAllRequestWindow()
{
    for (m_RequestChatWindowListIter = m_RequestChatWindowList.begin(); m_RequestChatWindowListIter != m_RequestChatWindowList.end(); ++m_RequestChatWindowListIter)
    {
        if (*m_RequestChatWindowListIter != NULL)
        {
            delete[] * m_RequestChatWindowListIter;
            *m_RequestChatWindowListIter = NULL;
        }
    }
    m_RequestChatWindowList.clear();
}

void CUIFriendMenu::CloseAllChatWindow()
{
    for (m_WindowListIter = m_WindowList.begin(); m_WindowListIter != m_WindowList.end(); ++m_WindowListIter)
    {
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, *m_WindowListIter, 0);
    }
    m_NewChatWindowList.clear();
    RemoveAllRequestWindow();
}

void CUIFriendMenu::LockAllChatWindow()
{
    CUIChatWindow* pWindow = NULL;
    for (m_WindowListIter = m_WindowList.begin(); m_WindowListIter != m_WindowList.end(); ++m_WindowListIter)
    {
        pWindow = (CUIChatWindow*)g_pWindowMgr->GetWindow(*m_WindowListIter);
        if (pWindow != NULL)
        {
            pWindow->AddChatText(255, GlobalText[402], 1, 0);
            pWindow->Lock(TRUE);
        }
    }
}