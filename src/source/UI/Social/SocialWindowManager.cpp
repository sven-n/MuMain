///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Social/SocialWindowManager.h"
#include "UI/Social/FriendWindowViews.h"
#include "Core/Time/FrameTimerScheduler.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/AI/GOBoid.h"
#include "UI/Core/UIManager.h"
#include "Character/CSParts.h"
#include "GameLogic/Skills/SummonSystem.h"
#include "World/MapInfra/MapManager.h"
#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Scaling/UITransform.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Camera/CameraProjection.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include "I18N/All.h"
#include "Render/Text/CUIRenderText.h"
#include "Render/Text/TextWrap.h"
#include "Core/Input/ImeInput.h"

using mu::ui::window::CheckMouseIn;   // WindowCommon.h


extern int	 g_iChatInputType;
extern DWORD g_dwActiveUIID;
extern DWORD g_dwMouseUseUIID;
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

    Core::Time::FrameTimerScheduler::Instance().SetRepeating(
        CHATCONNECT_TIMER, 15 * 1000, [] { g_pFriendMenu->SendChatRoomConnectCheck(); });

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
    SetChatReject(FALSE);
    if (GetFriendMainWindow() != NULL)
    {
        GetFriendMainWindow()->Reset();
    }
    if (g_iChatInputType == 0)
    {
        // 2 means logging out; not required for OpenMU servers.
        SocketClient->ToGameServer()->SendSetFriendOnlineState(2);
    }
}

DWORD CUIWindowMgr::AddWindow(int iWindowType, int iPos_x, int iPos_y, const wchar_t* pszTitle, DWORD dwParentID, int iOption)
{
    if (g_iChatInputType == 0/* || g_dwTopWindow != 0*/) return 0;
    CUIBaseWindow* pbw = NULL;

    switch (iWindowType)
    {
    case UIWNDTYPE_CHAT:
    case UIWNDTYPE_CHAT_READY:
        pbw = new CUIChatWindow;
        if (m_dwMainWindowUIID != 0)
        {
            auto* pMainWnd = static_cast<CUIFriendWindow*>(GetWindow(m_dwMainWindowUIID));
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
                static_cast<CUIFriendWindow*>(pbw)->SetTabIndex(1);
            }
            else
            {
                static_cast<CUIFriendWindow*>(pbw)->SetTabIndex(m_iLastFriendWindowTabIndex);
            }
            g_pFriendMenu->SetNewMailAlert(FALSE);
            if (IsServerEnable() == FALSE)
            {
                SocketClient->ToGameServer()->SendFriendListRequest();
            }
        }
        else return 0;
        break;
    case UIWNDTYPE_READLETTER:
        pbw = new CUILetterReadWindow;
        if (m_dwMainWindowUIID != 0)
        {
            auto* pMainWnd = static_cast<CUIFriendWindow*>(GetWindow(m_dwMainWindowUIID));
            if (pMainWnd != NULL)
                pMainWnd->AddWindow(pbw->GetUIID(), pszTitle);
        }
        break;
    case UIWNDTYPE_WRITELETTER:
        if (g_dwTopWindow != 0) return 0;
        pbw = new CUILetterWriteWindow;
        if (m_dwMainWindowUIID != 0)
        {
            auto* pMainWnd = static_cast<CUIFriendWindow*>(GetWindow(m_dwMainWindowUIID));
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

    if (iWindowType != UIWNDTYPE_FRIENDMAIN && !(iOption & UIADDWND_FORCEPOSITION))
    {
        const auto bounds = UI::Scaling::FloatingWorkspaceBounds(WindowWidth, WindowHeight);
        for (m_WindowMapIter = m_WindowMap.begin(); m_WindowMapIter != m_WindowMap.end(); ++m_WindowMapIter)
        {
            if (m_WindowMapIter->second->GetPosition_x() == iPos_x &&
                m_WindowMapIter->second->GetPosition_y() == iPos_y)
            {
                if (iPos_x + pbw->GetWidth() + 20 <= bounds.width)
                    iPos_x += 20;
                if (iPos_y + pbw->GetHeight() + 20 <= bounds.height)
                    iPos_y += 20;
                if (iPos_x + pbw->GetWidth() + 20 > bounds.width && iPos_y + pbw->GetHeight() + 20 > bounds.height)
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

        // The default place (0, 332) and the cascade put a window's lower part under the bottom
        // HUD, where its buttons cannot be reached (an original defect): keep it above the HUD.
        const int contentHeight =
            static_cast<int>(UI::Scaling::FloatingWorkspaceContentHeight(WindowWidth, WindowHeight));
        if (iPos_y + pbw->GetHeight() > contentHeight)
            iPos_y = std::max(contentHeight - pbw->GetHeight(), 0);
    }
    if (iWindowType != UIWNDTYPE_FRIENDMAIN)
        pbw->SetPosition(iPos_x, iPos_y);

    DWORD dwUIID = pbw->GetUIID();

    m_WindowMap.insert(std::pair<DWORD, CUIBaseWindow*>(dwUIID, pbw));
    m_WindowArrangeList.push_back(dwUIID);
    if (iWindowType == UIWNDTYPE_CHAT || iWindowType == UIWNDTYPE_CHAT_READY) g_pFriendMenu->AddWindow(dwUIID, pbw);

    pbw->Refresh();
    if (iWindowType == UIWNDTYPE_CHAT)
    {
        static_cast<CUIChatWindow*>(pbw)->FocusReset();
    }

    return dwUIID;
}

void CUIWindowMgr::RemoveWindow(DWORD dwUIID)
{
    if (m_dwMainWindowUIID == dwUIID)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_FRIEND);

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
            m_iLastFriendWindowTabIndex = static_cast<CUIFriendWindow*>(pWindow)->GetTabIndex();
        }
        m_dwMainWindowUIID = 0;
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
        auto* pMainWnd = static_cast<CUIFriendWindow*>(GetWindow(m_dwMainWindowUIID));
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
            CUIBaseWindow* window = m_WindowMapIter->second;
            if (window->GetState() != UISTATE_HIDE && window->GetState() != UISTATE_READY)
            {
                // Every window of this family draws itself through its own document now; the
                // manager is left with the native 3D each one renders over it.
                window->RenderRmlOverlay();
            }
        }
    }
    m_bRenderFrame = TRUE;
}

// Runs from the post-RmlUi seam (UI::RmlBridge::OverlayRender), so a portrait stands on its own
// panel instead of under every panel in the frame.
//
// Only the window in focus draws one. That seam sits above the whole main context rather than at
// any one window's depth, so a portrait drawn for a window that is not in front would stand over
// the very windows covering it. The front window has nothing of this family above it, which is
// the one case where "above everything" and "at this window's depth" agree. The others show their
// empty well until they are brought forward -- GetTopWindowUIID()'s own notion of front, the same
// one the original title bar used to decide which window looked active.
void CUIWindowMgr::RenderOverlay3D()
{
    CUIBaseWindow* focused = nullptr;
    for (const DWORD uiid : m_WindowArrangeList)
    {
        const auto found = m_WindowMap.find(uiid);
        if (found == m_WindowMap.end())
            continue;
        CUIBaseWindow* window = found->second;
        // Not GetTopWindowUIID() itself: that answers with the arrange list's back even when it
        // is hidden, which would suppress the portrait of the window actually in front.
        if (window->GetState() != UISTATE_HIDE && window->GetState() != UISTATE_READY)
            focused = window;
    }
    if (focused != nullptr)
        focused->RenderAboveRmlUi();
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
            const auto bounds = UI::Scaling::FloatingWorkspaceBounds(WindowWidth, WindowHeight);
            const int contentHeight =
                static_cast<int>(UI::Scaling::FloatingWorkspaceContentHeight(WindowWidth, WindowHeight));
            OpenMainWnd(bounds.width - 250, contentHeight - 170);
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

CUIBaseWindow* CUIWindowMgr::GetWindow(DWORD dwUIID)
{
    m_WindowMapIter = m_WindowMap.find(dwUIID);
    if (m_WindowMapIter == m_WindowMap.end())
        return nullptr;
    return m_WindowMapIter->second;
}

BOOL CUIWindowMgr::IsWindow(DWORD dwUIID)
{
    m_WindowMapIter = m_WindowMap.find(dwUIID);
    return m_WindowMapIter != m_WindowMap.end();
}

void CUIWindowMgr::SendUIMessageToWindow(DWORD dwUIID, int iMessage, LONG_PTR iParam1, LONG_PTR iParam2)
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
                if (GetWindow(GetTopWindowUIID()) != NULL && GetWindow(GetTopWindowUIID())->HaveTextBox() == TRUE)
                    SaveIMEStatus();

                SetFocus(g_hWnd);
                    }
            if (GetWindow(m_WorkMessage.m_iParam1)->GetState() == UISTATE_HIDE)
                ShowHideWindow(m_WorkMessage.m_iParam1, TRUE);

            m_WindowArrangeListIter = m_WindowArrangeList.end();
            --m_WindowArrangeListIter;

            const bool inputOwnsSelection = RmlFieldHasFocus(m_WorkMessage.m_iParam1);
            if ((int)(*m_WindowArrangeListIter) != m_WorkMessage.m_iParam1
                || (GetFocus() == g_hWnd && !inputOwnsSelection))
            {
                m_WindowArrangeList.remove(m_WorkMessage.m_iParam1);
                m_WindowArrangeList.push_back(m_WorkMessage.m_iParam1);
                if (!inputOwnsSelection)
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
        if (g_pSystemLogBox->CheckChatRedundancy(I18N::Game::YouCannotUseTheMyFriend, 2) == FALSE)
            g_pSystemLogBox->AddText(I18N::Game::YouCannotUseTheMyFriend, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return;
    }
    int iLevel = CharacterAttribute->Level;

    if (iLevel < 6)
    {
        if (g_pSystemLogBox->CheckChatRedundancy(I18N::Game::YouMustBeAtLeastLevel6ToUseTheMyFriendFunction) == FALSE)
            g_pSystemLogBox->AddText(I18N::Game::YouMustBeAtLeastLevel6ToUseTheMyFriendFunction, mu::ui::window::TYPE_SYSTEM_MESSAGE);
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
        AddWindow(UIWNDTYPE_FRIENDMAIN, iPos_x, iPos_y, I18N::Game::Lookup(m_iFriendMainWindowTitleNumber));
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, m_dwMainWindowUIID, 0);
        RefreshMainWndChatRoomList();
        PlayBuffer(SOUND_CLICK01);
        PlayBuffer(SOUND_INTERFACE01);
    }
    else
    {
        AddWindow(UIWNDTYPE_FRIENDMAIN, m_iMainWindowPos_x, m_iMainWindowPos_y, I18N::Game::Lookup(m_iFriendMainWindowTitleNumber));
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, m_dwMainWindowUIID, 0);
        CUIBaseWindow* pWindow = GetWindow(m_dwMainWindowUIID);
        if (pWindow != NULL)
        {
            static_cast<CUIFriendWindow*>(pWindow)->RestoreSemanticLayout(
                m_iMainWindowPos_x, m_iMainWindowPos_y, m_iMainWindowWidth, m_iMainWindowHeight);
            pWindow->SetBackPosition(m_bIsMainWindowMaximize, m_iMainWindowBackPos_y, m_iMainWindowBackHeight);
            static_cast<CUIFriendWindow*>(pWindow)->RestoreSemanticMaximized();
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
    static_cast<CUIFriendWindow*>(pWindow)->ResetWindow();
    for (m_WindowMapIter = m_WindowMap.begin(); m_WindowMapIter != m_WindowMap.end(); ++m_WindowMapIter)
    {
        if (m_dwMainWindowUIID != m_WindowMapIter->first && m_WindowMapIter->second->GetState() != UISTATE_READY)
            static_cast<CUIFriendWindow*>(pWindow)->AddWindow(m_WindowMapIter->first,
                                                              m_WindowMapIter->second->GetTitle());
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
                GetFriendMainWindow()->SetTitle(I18N::Game::Lookup(m_iFriendMainWindowTitleNumber));
        }
    }
    else
    {
        if (m_iFriendMainWindowTitleNumber != 1066)
        {
            m_iFriendMainWindowTitleNumber = 1066;
            if (GetFriendMainWindow() != NULL)
                GetFriendMainWindow()->SetTitle(I18N::Game::Lookup(m_iFriendMainWindowTitleNumber));
        }
    }
}
void SetLineColor(int iType, float fAlphaRate = 1.0f)
{
    const BYTE windowAlpha = static_cast<BYTE>(255.f * fAlphaRate);
    switch (iType)
    {
    case 0: SetRenderColor(146, 134, 121, windowAlpha); break;
    case 1: SetRenderColor(37, 37, 37, windowAlpha); break;
    case 2: SetRenderColor(106, 97, 88, windowAlpha); break;
    case 3: SetRenderColor(0, 0, 0, static_cast<BYTE>(179.f * fAlphaRate)); break;
    case 4: SetRenderColor(173, 167, 150, windowAlpha); break;
    case 5: SetRenderColor(53, 49, 48, windowAlpha); break;
    case 6: SetRenderColor(26, 22, 21, windowAlpha); break;
    case 7: SetRenderColor(0, 0, 0, static_cast<BYTE>(255.f * fAlphaRate)); break;
    case 8: SetRenderColor(153, 156, 166, windowAlpha); break;
    case 9: SetRenderColor(136, 138, 147, windowAlpha); break;
    case 10: SetRenderColor(83, 85, 93, windowAlpha); break;
    case 11: SetRenderColor(102, 104, 112, windowAlpha); break;
    case 12: SetRenderColor(0, 0, 8, windowAlpha); break;
    case 13: SetRenderColor(0, 0, 0, windowAlpha); break;
    case 14: SetRenderColor(185, 185, 185, windowAlpha); break;
    case 15: SetRenderColor(194, 194, 194, windowAlpha); break;
    case 16: SetRenderColor(194, 194, 194, windowAlpha); break;
    case 17: SetRenderColor(209, 188, 134, windowAlpha); break;
    case 18: SetRenderColor(205, 209, 133, windowAlpha); break;
    default: break;
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

void CUIBaseWindow::SetReturnText(const wchar_t* text)
{
    m_returnText = text ? text : L"";
}

std::wstring CUIBaseWindow::TakeReturnText()
{
    std::wstring text;
    text.swap(m_returnText);
    return text;
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
    (void)iSelect;
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
        g_pRenderText->SetFont(g_hFont);
    }
    if (CheckOption(UIWINDOWSTYLE_RESIZEABLE))
    {
        RenderBitmap(BITMAP_INTERFACE_EX + 11, (float)m_iPos_x + m_iWidth - 10, (float)m_iPos_y + m_iHeight - 10, (float)9, (float)9,
            0.f, 0.f, 9.f / 16.f, 9.f / 16.f);
    }
    if (g_pWindowMgr->GetTopWindowUIID() != GetUIID() || !g_pUIManager->IsOpen(MUTEX_FRIEND))
    {
        DisableAlphaBlend();
    }

    RenderOver();
}


void CUIBaseWindow::RenderRmlOverlay()
{
    EnableAlphaTest();
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
            const auto bounds = UI::Scaling::FloatingWorkspaceBounds(WindowWidth, WindowHeight);
            if (g_dwMouseUseUIID == 0)
                g_dwMouseUseUIID = GetUIID();
            MouseOnWindow = true;

            if (m_iPos_x + MouseX - m_iMouseClickPos_x < 0)
                m_iPos_x = 0;
            else if (m_iPos_x + m_iWidth + MouseX - m_iMouseClickPos_x > bounds.width)
                m_iPos_x = bounds.width - m_iWidth;
            else
                m_iPos_x += MouseX - m_iMouseClickPos_x;

            if (m_iPos_y + MouseY - m_iMouseClickPos_y < 0)
            {
                m_iPos_y = 0;
            }
            else if (m_iPos_y + m_iHeight + MouseY - m_iMouseClickPos_y > bounds.height)
            {
                m_iPos_y = bounds.height - m_iHeight;
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
            const auto bounds = UI::Scaling::FloatingWorkspaceBounds(WindowWidth, WindowHeight);
            if (m_iResizeDir == 135)
            {
                if (g_dwMouseUseUIID == 0)
                    g_dwMouseUseUIID = GetUIID();
                MouseOnWindow = true;

                if (m_iPos_x + m_iWidth + MouseX - m_iMouseClickPos_x > bounds.width)
                    m_iWidth = bounds.width - m_iPos_x;
                else
                    m_iWidth += MouseX - m_iMouseClickPos_x;

                if (m_iWidth < m_iMinWidth)
                {
                    m_iWidth = m_iMinWidth;
                }
                else if (m_iMaxWidth > 0 && m_iWidth > m_iMaxWidth)
                {
                    m_iWidth = m_iMaxWidth;
                }
                else
                    m_iMouseClickPos_x = MouseX;

                if (m_iPos_y + m_iHeight + MouseY - m_iMouseClickPos_y > bounds.height)
                    m_iHeight = bounds.height - m_iPos_y;
                else
                    m_iHeight += MouseY - m_iMouseClickPos_y;

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
        m_iHeight = static_cast<int>(UI::Scaling::FloatingWorkspaceContentHeight(WindowWidth, WindowHeight));
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

void ReceiveChatRoomConnectResult(DWORD dwWindowUIID, const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_CHAT_JOIN_RESULT)ReceiveBuffer;
    switch (Data->Result)
    {
    case 0x00:
        g_pWindowMgr->Dialogs().Notice(I18N::Game::ChatRoomIsFull);
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
    wchar_t szName[MAX_USERNAME_SIZE + 1] = { 0 };
    CMultiLanguage::ConvertFromUtf8(szName, Data->Name, MAX_USERNAME_SIZE);
    szName[MAX_USERNAME_SIZE] = '\0';
    wchar_t szText[MAX_TEXT_LENGTH + 1] = { 0 };
    CMultiLanguage::ConvertFromUtf8(szText, Data->Name, MAX_USERNAME_SIZE);
    szText[MAX_USERNAME_SIZE] = '\0';
    switch (Data->Type)
    {
    case 0x00:
        if (pChatWindow->AddChatPal(szName, Data->Index, 0) >= 3)
        {
            wcscat(szText, I18N::Game::HasEntered);
            pChatWindow->AddChatText(255, szText, 1, 0);
        }
        break;
    case 0x01:
        if (pChatWindow->GetUserCount() >= 3)
        {
            wcscat(szText, I18N::Game::HasLeft);
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
    // The only handler here that used the window without checking it, and it looked it up once
    // per name rather than once.
    auto* pChatWindow = (CUIChatWindow*)g_pWindowMgr->GetWindow(dwWindowUIID);
    if (pChatWindow == NULL) return;

    auto Header = (LPFS_CHAT_USERLIST_HEADER)ReceiveBuffer;
    int iMoveOffset = sizeof(FS_CHAT_USERLIST_HEADER);
    wchar_t szName[MAX_USERNAME_SIZE + 1] = { 0 };
    for (int i = 0; i < Header->Count; ++i)
    {
        auto Data = (LPFS_CHAT_USERLIST_DATA)(ReceiveBuffer + iMoveOffset);
        CMultiLanguage::ConvertFromUtf8(szName, Data->Name, MAX_USERNAME_SIZE);
        szName[MAX_USERNAME_SIZE] = '\0';
        pChatWindow->AddChatPal(szName, Data->Index, 0);
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
        g_pSystemLogBox->AddText(I18N::Game::NewMessageHasArrived, mu::ui::window::TYPE_SYSTEM_MESSAGE);
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
    g_pSystemLogBox->AddText(message, mu::ui::window::TYPE_SYSTEM_MESSAGE);
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

void CUIChatWindow::HandlePacketS(int32_t handle, const BYTE* ReceiveBuffer, int32_t Size)
{
    // find() was dereferenced unguarded. The Connection registers this callback from its own
    // constructor, before ConnectToChatServer() below records the handle, so a packet arriving in
    // that window read end() -- and so did one arriving for a room already closed.
    const auto found = ConnectionHandleToWindowUuid.find(handle);
    if (found == ConnectionHandleToWindowUuid.end() || found->second == 0)
        return;
    TranslateChattingProtocol(found->second, ReceiveBuffer, Size);
}

void CUIChatWindow::ConnectToChatServer(const wchar_t* pszIP, DWORD dwRoomNumber, DWORD dwTicket)
{
    m_dwRoomNumber = dwRoomNumber;

    _connection = new Connection(MU_C16(pszIP), 55980, true, &HandlePacketS);

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

        // Before the connection goes: this entry used to outlive the window, so a late packet on
        // the handle still resolved to the UIID of a window that no longer exists.
        ConnectionHandleToWindowUuid.erase(_connection->GetHandle());

        delete _connection;
        _connection = nullptr;
    }
}

void CFriendList::AddFriend(const wchar_t* pszID, BYTE Number, BYTE Server)
{
    static GUILDLIST_TEXT FriendData;
    wcsncpy(FriendData.m_szID, pszID, MAX_USERNAME_SIZE);
    FriendData.m_szID[MAX_USERNAME_SIZE] = '\0';
    FriendData.m_Number = Number;
    FriendData.m_Server = Server;

    m_FriendList.insert(m_FriendList.end(), FriendData);
}

void CFriendList::RemoveFriend(const wchar_t* pszID)
{
    for (m_FriendListIter = m_FriendList.begin(); m_FriendListIter != m_FriendList.end(); ++m_FriendListIter)
    {
        if (wcsncmp(m_FriendListIter->m_szID, pszID, MAX_USERNAME_SIZE) == 0)
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
        if (pszID != NULL && wcsncmp(m_FriendListIter->m_szID, pszID, MAX_USERNAME_SIZE) == 0) iResult = i;
    }
    return iResult;
}

void CFriendList::UpdateFriendState(const wchar_t* pszID, BYTE Number, BYTE Server)
{
    for (m_FriendListIter = m_FriendList.begin(); m_FriendListIter != m_FriendList.end(); ++m_FriendListIter)
    {
        if (wcsncmp(m_FriendListIter->m_szID, pszID, MAX_USERNAME_SIZE) == 0)
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
    int iLength = std::min<int>(wcslen(pszText1), wcslen(pszText2));
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
    wcsncpy(text.m_szID, pszID, MAX_USERNAME_SIZE);
    text.m_szID[MAX_USERNAME_SIZE] = '\0';
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
        m_LetterCache.erase(m_LetterCacheIter);
    }
}

void CLetterList::ClearLetterTextCache()
{
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
        g_pRenderText->SetFont(g_hFont);
        m_fLineHeight = g_pRenderText->MeasureText(L"0", 1).cy;
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
        wchar_t* context = nullptr;
        wchar_t* pszChatTitle = wcstok_s(temp, L",", &context);

        if (wcslen(pszChatTitle) > wcslen(I18N::Game::Talking))
        {
            if (wcsncmp(pszChatTitle, I18N::Game::Offline, wcslen(I18N::Game::Offline)) == 0)
            {
                CutText3(pszChatTitle + wcslen(I18N::Game::Offline) + wcslen(I18N::Game::Talking), szText, m_iWidth - 8, 1, 64);
            }
            else
            {
                CutText3(pszChatTitle + wcslen(I18N::Game::Talking), szText, m_iWidth - 8, 1, 64);
            }
        }
        else
        {
            wcscpy(szText, I18N::Game::Offline);
        }
        g_pRenderText->RenderText(m_iPos_x + 2, m_iFriendMenuPos_y - (m_fLineHeight + 4) * i + 3, szText);

        auto* pWindow = (CUIChatWindow*)g_pWindowMgr->GetWindow(*m_WindowListIter);
        if (pWindow != NULL && pWindow->GetUserCount() > 2)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 15, (float)m_iPos_x + m_iWidth - 7, (float)m_iFriendMenuPos_y - (m_fLineHeight + 4) * i + 5, (float)4, (float)6, 0.f, 0.f, 4.f / 8.f, 6.f / 8.f);
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
        else if (wcsncmp(pName, pszTargetName, MAX_USERNAME_SIZE) == 0)
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
    if (wcslen(szTargetName) > MAX_USERNAME_SIZE) return;
    wchar_t* pszName = new wchar_t[MAX_USERNAME_SIZE + 1];
    wcsncpy(pszName, szTargetName, MAX_USERNAME_SIZE);
    pszName[MAX_USERNAME_SIZE] = '\0';
    m_RequestChatWindowList.push_back(pszName);
}

BOOL CUIFriendMenu::IsRequestWindow(const wchar_t* szTargetName)
{
    for (m_RequestChatWindowListIter = m_RequestChatWindowList.begin(); m_RequestChatWindowListIter != m_RequestChatWindowList.end(); ++m_RequestChatWindowListIter)
    {
        if (wcsncmp(*m_RequestChatWindowListIter, szTargetName, MAX_USERNAME_SIZE) == 0) return TRUE;
    }
    return FALSE;
}

void CUIFriendMenu::RemoveRequestWindow(const wchar_t* szTargetName)
{
    BOOL bFind = FALSE;
    for (m_RequestChatWindowListIter = m_RequestChatWindowList.begin(); m_RequestChatWindowListIter != m_RequestChatWindowList.end(); ++m_RequestChatWindowListIter)
    {
        if (wcsncmp(*m_RequestChatWindowListIter, szTargetName, MAX_USERNAME_SIZE) == 0)
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
            pWindow->AddChatText(255, I18N::Game::YouAreDisconnectedFromTheServer, 1, 0);
            pWindow->Lock(TRUE);
        }
    }
}
