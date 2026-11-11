#include "stdafx.h"
#include "UI/Social/SocialWindowBase.h"

BOOL g_bUseChatListBox = TRUE;

DWORD g_dwActiveUIID = 0;
DWORD g_dwMouseUseUIID = 0;

DWORD g_dwTopWindow = 0;
DWORD g_dwKeyFocusUIID = 0;

namespace
{
DWORD g_dwLastUIID = 0;
} // namespace

DWORD CreateUIID()
{
    return ++g_dwLastUIID;
}

void CUIMessage::SendUIMessage(int iMessage, LONG_PTR iParam1, LONG_PTR iParam2)
{
    m_MessageList.push_back({iMessage, iParam1, iParam2});
}

void CUIMessage::GetUIMessage()
{
    if (m_MessageList.empty())
        return;

    m_WorkMessage = m_MessageList.front();
    m_MessageList.pop_front();
}

void CUIBaseWindow::SendUIMessageDirect(int iMessage, int iParam1, int iParam2)
{
    SendUIMessage(iMessage, iParam1, iParam2);
    DoAction(TRUE);
}

BOOL CUIBaseWindow::DoAction(BOOL bMessageOnly)
{
    (void)bMessageOnly;
    while (!m_MessageList.empty())
    {
        GetUIMessage();
        HandleMessage();
    }
    return TRUE;
}
