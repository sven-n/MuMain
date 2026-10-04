///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core/Input/KeyState.h"
#include "Core/Time/FrameTimerScheduler.h"
#include "GameLogic/Items/CComGem.h"
#include "UI/Social/SocialWindowCore.h"
#include "UI/Social/SocialWindowManager.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

#include "Core/Utilities/ReadScript.h"
#include "GameLogic/Events/Cinematic/CMVP1stDirection.h"
#include "UI/Core/UIManager.h"
#include "GameLogic/Items/InventoryUtils.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "Render/Text/CUIRenderText.h"
#include "Render/Text/TextWrap.h"
#include <vector>

extern BYTE m_CrywolfState;

extern int g_iChatInputType;

extern BOOL g_bUseWindowMode;

BOOL g_bUseChatListBox = TRUE;

extern int DeleteGuildIndex;
extern bool ChatWindowEnable;
void SetLineColor(int iType, float fAlphaRate = 1.0f);

DWORD g_dwLastUIID = 0;
DWORD CreateUIID()
{
    return ++g_dwLastUIID;
}
DWORD g_dwActiveUIID = 0;
DWORD g_dwMouseUseUIID = 0;

DWORD g_dwTopWindow = 0;
DWORD g_dwKeyFocusUIID = 0;
DWORD g_dwCurrentPressedButtonID = 0;

// mu::ui::window::CheckMouseIn (WindowCommon.h) is this function's left-top branch and is what
// the rest of the client uses. Only this toolkit ever asks for left-down origins, so the
// coordinate-type form stays private to it.
const int COORDINATE_TYPE_LEFT_TOP = 1;
const int COORDINATE_TYPE_LEFT_DOWN = 2;

static BOOL CheckMouseIn(int iPos_x, int iPos_y, int iWidth, int iHeight)
{
    return MouseX >= iPos_x && MouseX < iPos_x + iWidth && MouseY >= iPos_y && MouseY < iPos_y + iHeight;
}

void CUIMessage::SendUIMessage(int iMessage, LONG_PTR iParam1, LONG_PTR iParam2)
{
    static UI_MESSAGE tempmsg;
    tempmsg.m_iMessage = iMessage;
    tempmsg.m_iParam1 = iParam1;
    tempmsg.m_iParam2 = iParam2;

    m_MessageList.push_back(tempmsg);
}

void CUIMessage::GetUIMessage()
{
    if (m_MessageList.empty()) return;

    auto msgiter = m_MessageList.begin();
    m_WorkMessage.m_iMessage = msgiter->m_iMessage;
    m_WorkMessage.m_iParam1 = msgiter->m_iParam1;
    m_WorkMessage.m_iParam2 = msgiter->m_iParam2;

    m_MessageList.pop_front();
}

CUIControl::CUIControl()
{
    m_dwUIID = CreateUIID();
    m_dwParentUIID = 0;
    SetState(0);
    m_iOptions = 0;
    SetPosition(0, 0);
    SetSize(100, 100);
}

void CUIControl::SetState(int iState)
{
    m_iState = iState;
}

int CUIControl::GetState()
{
    return m_iState;
}

void CUIControl::SetPosition(int iPos_x, int iPos_y)
{
    m_iPos_x = iPos_x;
    m_iPos_y = iPos_y;
}

void CUIControl::SetSize(int iWidth, int iHeight)
{
    m_iWidth = iWidth;
    m_iHeight = iHeight;
}

void CUIControl::SendUIMessageDirect(int iMessage, int iParam1, int iParam2)
{
    SendUIMessage(iMessage, iParam1, iParam2);
    DoAction(TRUE);
}

BOOL CUIControl::DoAction(BOOL bMessageOnly)
{
    while (m_MessageList.empty() == FALSE)
    {
        GetUIMessage();
        HandleMessage();
    }

    DoActionSub(bMessageOnly);

    if (bMessageOnly == TRUE) return 0;

    if (::CheckMouseIn(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight))
    {
        if (g_dwMouseUseUIID == 0)
        {
            if (g_dwActiveUIID == 0 || g_dwActiveUIID == GetUIID())
            {
                g_dwMouseUseUIID = GetUIID();
            }
            else
            {
                return FALSE;
            }
        }
        if (GetState() == UISTATE_NORMAL)
        {
            if (g_dwMouseUseUIID != GetUIID() && g_dwActiveUIID != GetUIID())
                return FALSE;

            if (MouseLButton == true)
            {
                CUIControl* pRootWindow = nullptr;
                if (GetParentUIID() == 0)
                {
                    pRootWindow = this;
                }
                else
                {
                    pRootWindow = g_pWindowMgr->GetWindow(GetParentUIID());
                }

                while (pRootWindow != nullptr && pRootWindow->GetParentUIID() != 0)
                {
                    pRootWindow = g_pWindowMgr->GetWindow(pRootWindow->GetParentUIID());
                }
                if (pRootWindow != nullptr && g_pWindowMgr != nullptr)
                {
                    if (g_pWindowMgr->IsWindow(pRootWindow->GetUIID()) == TRUE)
                        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, pRootWindow->GetUIID(), 0);
                }
            }
        }
    }
    return DoMouseAction();
}

