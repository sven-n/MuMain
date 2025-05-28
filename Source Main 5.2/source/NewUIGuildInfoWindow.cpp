//////////////////////////////////////////////////////////////////////
// NewUIGuildInfoWindow.cpp: implementation of the CNewUIGuildInfoWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIGuildInfoWindow.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "NewUICustomMessageBox.h"
#include "DSPlaySound.h"
#include "UIGuildInfo.h"
#include "UIControls.h"
#include "UIPopup.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzInfomation.h"

#include "CharacterManager.h"

int	DeleteIndex = 0;
int AppointStatus = 0;
wchar_t DeleteID[100];

extern CUIPopup* g_pUIPopup;
extern MARK_t GuildMark[MAX_MARKS];

using namespace SEASON3B;

void RenderText(wchar_t* text, int x, int y, int sx, int sy, DWORD color, DWORD backcolor, int sort)
{
    g_pRenderText->SetFont(g_hFont);

    DWORD backuptextcolor = g_pRenderText->GetTextColor();
    DWORD backuptextbackcolor = g_pRenderText->GetBgColor();

    g_pRenderText->SetTextColor(color);
    g_pRenderText->SetBgColor(backcolor);
    g_pRenderText->RenderText(x, y, text, sx, sy, sort);

    g_pRenderText->SetTextColor(backuptextcolor);
    g_pRenderText->SetBgColor(backuptextbackcolor);
}

int SEASON3B::CNewUIGuildInfoWindow::GetGuildMemberIndex(wchar_t* szName)
{
    for (int i = 0; i < g_nGuildMemberCount; ++i)
    {
        if (GuildList[i].Name && !wcscmp(GuildList[i].Name, szName))
            return i;
    }

    return -1;
}

SEASON3B::CNewUIGuildInfoWindow::CNewUIGuildInfoWindow() : m_Button(NULL)
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_nCurrentTab = 1;
    m_EventState = EVENT_NONE;
    m_Loc = 0;
    m_BackUp = 0;
    m_CurrentListPos = 0;
    m_Loc_Bk = 0;
    m_Tot_Notice = 0;
    m_dwPopupID = 0;

    m_bRequestUnionList = false;
}

SEASON3B::CNewUIGuildInfoWindow::~CNewUIGuildInfoWindow()
{
    Release();
}

bool SEASON3B::CNewUIGuildInfoWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_GUILDINFO, this);

    SetPos(x, y);

    LoadImages();
    m_Button = new CNewUIButton[BUTTON_END];
    for (int i = 0; i < BUTTON_END; ++i)
    {
        m_Button[i].ChangeButtonImgState(true, IMAGE_GUILDINFO_BUTTON, true);
        m_Button[i].ChangeButtonInfo(0, 0, 64, 29);
    }
    m_Button[BUTTON_GET_POSITION].SetPos(m_Pos.x + 3, m_Pos.y + 360);
    m_Button[BUTTON_FREE_POSITION].SetPos(m_Pos.x + 64, m_Pos.y + 360);
    m_Button[BUTTON_GET_OUT].SetPos(m_Pos.x + 125, m_Pos.y + 360);
    m_Button[BUTTON_GET_POSITION].ChangeText(GlobalText[1307]);
    m_Button[BUTTON_FREE_POSITION].ChangeText(GlobalText[1308]);
    m_Button[BUTTON_GET_OUT].ChangeText(GlobalText[1309]);

    m_BtnExit.ChangeButtonImgState(true, IMAGE_GUILDINFO_EXIT_BTN, false);
    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_BtnExit.ChangeToolTipText(GlobalText[1002], true);

    Show(false);

    return true;
}

void SEASON3B::CNewUIGuildInfoWindow::OpenningProcess()
{
    m_nCurrentTab = 1;

    SocketClient->ToGameServer()->SendGuildListRequest();
}

void SEASON3B::CNewUIGuildInfoWindow::ClosingProcess()
{
    m_bRequestUnionList = false;
}

void SEASON3B::CNewUIGuildInfoWindow::Release()
{
    UnloadImages();

    SAFE_DELETE_ARRAY(m_Button);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIGuildInfoWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool SEASON3B::CNewUIGuildInfoWindow::UpdateMouseEvent()
{
    bool ret = true;

    if (SEASON3B::IsPress(VK_LBUTTON))
    {
        ret = Check_Mouse(MouseX, MouseY);
        if (ret == false)
        {
            PlayBuffer(SOUND_CLICK01);
        }
    }

    if (m_EventState == EVENT_SCROLL_BTN_DOWN)
    {
        if (SEASON3B::IsRepeat(VK_LBUTTON))
        {
            return false;
        }
        if (SEASON3B::IsRelease(VK_LBUTTON))
        {
            m_EventState = EVENT_NONE;
            return true;
        }
    }

    Check_Btn();

    if (m_nCurrentTab == 0)
    {
        m_GuildNotice.DoAction();
    }
    else if (m_nCurrentTab == 1)
    {
        m_GuildMember.DoAction();
    }
    else if (m_nCurrentTab == 2)
    {
        m_UnionListBox.DoAction();
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, GUILDINFO_WIDTH, GUILDINFO_HEIGHT))
    {
        return false;
    }

    return ret;
}

bool SEASON3B::CNewUIGuildInfoWindow::Check_Btn()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (m_BtnExit.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_GUILDINFO);
        m_EventState = EVENT_NONE;
        return false;
    }
    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_GUILDINFO);
        m_EventState = EVENT_NONE;
        return false;
    }

    if (m_EventState == EVENT_SCROLL_BTN_DOWN && SEASON3B::IsRelease(VK_LBUTTON))
    {
        m_EventState = EVENT_NONE;
        return false;
    }

    if (m_nCurrentTab == 0)
    {
        if (m_Button[BUTTON_GUILD_OUT].UpdateMouseEvent())
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                if (!wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[Hero->GuildMarkIndex].UnionName))
                {
                    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGuildOutPerson));
                }
                else
                {
                    DeleteIndex = GetGuildMemberIndex(Hero->ID);
                    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGuildBreakMsgBoxLayout));
                }
            }
            else
            {
                DeleteIndex = GetGuildMemberIndex(Hero->ID);
                SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGuildBreakPasswordMsgBoxLayout));
            }
        }
    }
    else if (m_nCurrentTab == 1)
    {
        if (m_Button[BUTTON_GET_OUT].UpdateMouseEvent())
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
                {
                    if (pText->m_GuildStatus != G_MASTER)
                    {
                        if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
                        {
                            DeleteIndex = GetGuildMemberIndex(pText->m_szID);
                            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGuildPerson_Get_Out));
                        }
                    }
                }
            }
        }
        else if (m_Button[BUTTON_GET_POSITION].UpdateMouseEvent())
        {
            if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
            {
                if (Hero->GuildStatus == G_MASTER)
                {
                    if (pText->m_GuildStatus != G_MASTER)
                    {
                        if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
                        {
                            AppointStatus = (GUILD_STATUS)pText->m_GuildStatus;
                            DeleteIndex = GetGuildMemberIndex(pText->m_szID);
                            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGuild_ToPerson_PositionLayout));
                        }
                    }
                }
            }
        }
        else if (m_Button[BUTTON_FREE_POSITION].UpdateMouseEvent())
        {
            if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
            {
                if (pText->m_GuildStatus == G_SUB_MASTER || pText->m_GuildStatus == G_BATTLE_MASTER)
                {
                    if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
                    {
                        AppointStatus = (GUILD_STATUS)pText->m_GuildStatus;
                        DeleteIndex = GetGuildMemberIndex(pText->m_szID);
                        CNewUICommonMessageBox* pMsgBox = NULL;
                        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGuildPerson_Cancel_Position_MsgBoxLayout), &pMsgBox);
                        if (pMsgBox != NULL)
                        {
                            wchar_t strText[256];
                            swprintf(strText, GlobalText[1367], pText->m_szID);
                            pMsgBox->AddMsg(strText);
                            pMsgBox->AddMsg(GlobalText[1368]);
                        }
                    }
                }
            }
        }
    }
    else if (m_nCurrentTab == 2)
    {
        if (m_Button[BUTTON_UNION_CREATE].UpdateMouseEvent())
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                if (UNIONGUILD_TEXT* pText = m_UnionListBox.GetSelectedText())
                {
                    if (wcscmp(pText->szName, GuildMark[Hero->GuildMarkIndex].GuildName))
                    {
                        wcscpy(DeleteID, pText->szName);
                        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CUnionGuild_Break_MsgBoxLayout));
                    }
                }
            }
        }
        else if (m_Button[BUTTON_UNION_OUT].UpdateMouseEvent())
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                if (!wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[Hero->GuildMarkIndex].UnionName))
                {
                    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CUnionGuild_Out_MsgBoxLayout));
                }
                else
                {
                    SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(0x01, 0x02, Hero->Key);
                }
            }
        }
    }
    return true;
}

bool SEASON3B::CNewUIGuildInfoWindow::Check_Mouse(int mx, int my)
{
    if (mx > m_Pos.x && mx < (m_Pos.x + GUILDINFO_WIDTH) && my > m_Pos.y && my < (m_Pos.y + GUILDINFO_HEIGHT))
    {
        for (int i = 0; i < 3; i++)
        {
            int Tab_Pos = i * 56;
            if (mx > (m_Pos.x + 12 + Tab_Pos) && mx < (m_Pos.x + 12 + Tab_Pos + 56) && my > m_Pos.y && my < (m_Pos.y + 90))
            {
                if (m_nCurrentTab != i)
                {
                    m_BackUp = 0;
                    m_Loc = 0;
                    m_CurrentListPos = 0;
                }
                m_nCurrentTab = i;
                switch (m_nCurrentTab)
                {
                case 0:
                    break;
                case 1:
                {
                    SocketClient->ToGameServer()->SendGuildListRequest();
                }
                break;
                case 2:
                {
                    if (m_bRequestUnionList == false
                        && GuildMark[Hero->GuildMarkIndex].UnionName[0] != NULL)
                    {
                        SocketClient->ToGameServer()->SendRequestAllianceList();
                        m_bRequestUnionList = true;
                    }
                }
                break;
                }
                return false;
            }
        }
    }
    if (m_nCurrentTab == 0)
    {
        if (SEASON3B::CheckMouseIn(m_Pos.x + 163, m_Pos.y + 262 + m_Loc, 18, 33 + m_Loc) == true && m_EventState == EVENT_NONE)
        {
            m_EventState = EVENT_SCROLL_BTN_DOWN;
            if (m_BackUp == 0)
            {
                m_BackUp = (262 + (MouseY - 262));
            }
            return false;
        }
    }
    else if (m_nCurrentTab == 1)
    {
        if ((MouseX > m_Pos.x + 166 && MouseX < (m_Pos.x + 181) && MouseY > m_Pos.y + 125 + m_Loc && MouseY < (m_Pos.y + 175 + m_Loc)) && m_EventState == EVENT_NONE)
        {
            m_EventState = EVENT_SCROLL_BTN_DOWN;
            if (m_BackUp == 0)
                m_BackUp = (125 + (MouseY - 125));
            return false;
        }
    }
    return true;
}

bool SEASON3B::CNewUIGuildInfoWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GUILDINFO) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_GUILDINFO);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}
bool SEASON3B::CNewUIGuildInfoWindow::Update()
{
    return true;
}

bool SEASON3B::CNewUIGuildInfoWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    RenderFrame();

    if (Hero->GuildStatus == G_NONE)
    {
        RenderNoneGuild();
    }
    else
    {
        RenderTabButton();

        switch (m_nCurrentTab)
        {
        case 0:
            Render_Guild_History();
            break;
        case 1:
            Render_Guild_Enum();
            RenderScrollBar();
            break;
        case 2:
            Render_Guild_Info();
            break;
        }
        Render_Text();

        EnableAlphaTest();
        m_BtnExit.Render();
    }

    DisableAlphaBlend();
    return true;
}

void SEASON3B::CNewUIGuildInfoWindow::RenderFrame()
{
    RenderImage(IMAGE_GUILDINFO_BACK, m_Pos.x, m_Pos.y, GUILDINFO_WIDTH, GUILDINFO_HEIGHT);
    RenderImage(IMAGE_GUILDINFO_TOP, m_Pos.x, m_Pos.y, GUILDINFO_WIDTH, 64.f);
    RenderImage(IMAGE_GUILDINFO_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_GUILDINFO_RIGHT, m_Pos.x + GUILDINFO_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_GUILDINFO_BOTTOM, m_Pos.x, m_Pos.y + GUILDINFO_HEIGHT - 45, GUILDINFO_WIDTH, 45.f);
}

void SEASON3B::CNewUIGuildInfoWindow::RenderNoneGuild()
{
    POINT ptOrigin = { m_Pos.x + 15, m_Pos.y + 38 };
    ptOrigin.x += 10;
    ptOrigin.y += 8;

    wchar_t Text[128];
    memset(&Text, 0, sizeof(wchar_t) * 128);
    swprintf(Text, GlobalText[180]);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 15, Text, 190, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[185]);
    ptOrigin.y += 15;
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[186]);
    ptOrigin.y += 15;
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[187]);

    m_BtnExit.Render();
}

void SEASON3B::CNewUIGuildInfoWindow::RenderTabButton()
{
    RenderImage(IMAGE_GUILDINFO_TAB_LIST, m_Pos.x + 12, m_Pos.y + 68, 166.f, 22.f);
    int Tab_Pos = m_nCurrentTab * 55;
    RenderImage(IMAGE_GUILDINFO_TAB_POINT, m_Pos.x + 12 + Tab_Pos, m_Pos.y + 68, 56.f, 22.f);
}

void SEASON3B::CNewUIGuildInfoWindow::Render_Text()
{
    wchar_t Text[300];
    POINT ptOrigin;
    swprintf(Text, GlobalText[180]);
    RenderText(Text, m_Pos.x, m_Pos.y + 12, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);
    ptOrigin.x = m_Pos.x + 35;
    ptOrigin.y = m_Pos.y + 48;

    g_pRenderText->SetTextColor(200, 255, 100, 255);
    swprintf(Text, L"%s ( Score:%d )", GuildMark[Hero->GuildMarkIndex].GuildName, GuildTotalScore);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, Text, 120, 0, RT3_SORT_CENTER);

    glColor4f(0.6f, 0.6f, 0.6f, 1.f);

    if (m_nCurrentTab == 0)
    {
        glColor4f(1.f, 1.f, 1.f, 1.f);
    }
    swprintf(Text, GlobalText[180]);
    RenderText(Text, m_Pos.x + 13 + (0 * 55), m_Pos.y + 76, 55, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);
    glColor4f(0.6f, 0.6f, 0.6f, 1.f);

    if (m_nCurrentTab == 1)
    {
        glColor4f(1.f, 1.f, 1.f, 1.f);
    }
    swprintf(Text, GlobalText[1330]);
    RenderText(Text, m_Pos.x + 13 + (1 * 55), m_Pos.y + 76, 55, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);
    glColor4f(0.6f, 0.6f, 0.6f, 1.f);

    if (m_nCurrentTab == 2)
    {
        glColor4f(1.f, 1.f, 1.f, 1.f);
    }
    swprintf(Text, GlobalText[1352]);
    RenderText(Text, m_Pos.x + 13 + (2 * 55), m_Pos.y + 76, 55, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);
    glColor4f(0.6f, 0.6f, 0.6f, 1.f);

    if (m_nCurrentTab == 0)
    {
        m_Button[BUTTON_GUILD_OUT].SetPos(m_Pos.x + 100, m_Pos.y + 350);
        if (Hero->GuildStatus == G_MASTER)
        {
            m_Button[BUTTON_GUILD_OUT].ChangeText(GlobalText[188]);
        }
        else
        {
            m_Button[BUTTON_GUILD_OUT].ChangeText(GlobalText[189]);
        }

        swprintf(Text, GlobalText[1323]);
        RenderText(Text, m_Pos.x + 22, m_Pos.y + 249, 40, 0, _ARGB(255, 255, 185, 1), 0x00000000, RT3_SORT_CENTER);

        m_GuildNotice.SetSize(160, 80);
        m_GuildNotice.SetPosition(m_Pos.x + 15, m_Pos.y + 264 + m_GuildNotice.GetHeight());
        m_GuildNotice.Render();

        int Nm_Loc = m_Pos.y + 169;
        swprintf(Text, L"%s :", GlobalText[1332]);
        RenderText(Text, m_Pos.x + 22, Nm_Loc, 40, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_LEFT);
        Nm_Loc += 13;

        swprintf(Text, GlobalText[1256], GuildTotalScore);
        RenderText(Text, m_Pos.x + 22, Nm_Loc, 80, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_LEFT);

        Nm_Loc += 13;
        if (Hero->GuildStatus == G_MASTER)
        {
            int Class = gCharacterManager.GetBaseClass(CharacterAttribute->Class);
            if (Class == CLASS_DARK_LORD)
            {
                int nCount = CharacterAttribute->Level / 10 + CharacterAttribute->Charisma / 10;
                if (nCount > 80)
                    nCount = 80;

                swprintf(Text, GlobalText[1362], g_nGuildMemberCount, nCount);
            }
            else
            {
                swprintf(Text, GlobalText[1362], g_nGuildMemberCount, CharacterAttribute->Level / 10);
            }
        }
        else
        {
            swprintf(Text, GlobalText[1310], g_nGuildMemberCount);
        }
        RenderText(Text, m_Pos.x + 22, Nm_Loc, 80, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_LEFT);

        Nm_Loc += 13;
        swprintf(Text, L"%s : %s", GlobalText[1321], m_RivalGuildName[0] ? m_RivalGuildName : GlobalText[1361]);
        RenderText(Text, m_Pos.x + 22, Nm_Loc, 0, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_LEFT);
    }
    else if (m_nCurrentTab == 1)
    {
        glColor4f(1.f, 1.f, 1.f, 1.f);
        RenderText((wchar_t*)GlobalText[1389], m_Pos.x + 24, m_Pos.y + 112, 40, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_LEFT);
        RenderText((wchar_t*)GlobalText[1307], m_Pos.x + 89, m_Pos.y + 112, 40, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_LEFT);
        RenderText((wchar_t*)GlobalText[1022], m_Pos.x + 126, m_Pos.y + 112, 40, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_LEFT);

        m_GuildMember.SetSize(160, 420);
        m_GuildMember.SetPosition(m_Pos.x + 13, m_Pos.y + 123 + m_GuildMember.GetHeight());
        m_GuildMember.Render();
    }
    else
        if (m_nCurrentTab == 2)
        {
            //BUTTON_UNION_CREATE
            glColor4f(1.f, 1.f, 1.f, 1.f);
            m_Button[BUTTON_UNION_CREATE].SetPos(m_Pos.x + 30, m_Pos.y + 230);
            m_Button[BUTTON_UNION_OUT].SetPos(m_Pos.x + 100, m_Pos.y + 230);
            m_Button[BUTTON_UNION_CREATE].ChangeText(GlobalText[1422]);
            m_Button[BUTTON_UNION_OUT].ChangeText(GlobalText[1324]);
            if (GuildMark[Hero->GuildMarkIndex].UnionName[0] != NULL)
            {
                RenderText((wchar_t*)GlobalText[182], m_Pos.x + 34, m_Pos.y + 115, 40, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_LEFT);
                RenderText((wchar_t*)GlobalText[1330], m_Pos.x + 140, m_Pos.y + 115, 40, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_LEFT);
            }
        }
}

void SEASON3B::CNewUIGuildInfoWindow::Render_Guild_History()
{
    for (int x = m_Pos.x + 73; x < m_Pos.x + 73 + 42; x++)
    {
        RenderImage(IMAGE_GUILDINFO_TOP_PIXEL, x, m_Pos.y + 104, 1, 14);
        RenderImage(IMAGE_GUILDINFO_BOTTOM_PIXEL, x, m_Pos.y + 104 + 34, 1, 14);
    }
    for (int y = m_Pos.y + 104; y < m_Pos.y + 104 + 42; y++)
    {
        RenderImage(IMAGE_GUILDINFO_LEFT_PIXEL, m_Pos.x + 70, y, 14, 1);
        RenderImage(IMAGE_GUILDINFO_RIGHT_PIXEL, m_Pos.x + 105, y, 14, 1);
    }

    RenderImage(IMAGE_GUILDINFO_TOP_LEFT, m_Pos.x + 70, m_Pos.y + 104, 14, 14);
    RenderImage(IMAGE_GUILDINFO_TOP_RIGHT, m_Pos.x + 105, m_Pos.y + 104, 14, 14);
    RenderImage(IMAGE_GUILDINFO_BOTTOM_LEFT, m_Pos.x + 70, m_Pos.y + 138, 14, 14);
    RenderImage(IMAGE_GUILDINFO_BOTTOM_RIGHT, m_Pos.x + 105, m_Pos.y + 138, 14, 14);

    CreateGuildMark(Hero->GuildMarkIndex);
    RenderBitmap(BITMAP_GUILD, m_Pos.x + 74, m_Pos.y + 106, 39, 39);

    for (int x = m_Pos.x + 12; x < m_Pos.x + 12 + 166; x++)
    {
        RenderImage(IMAGE_GUILDINFO_TOP_PIXEL, x, m_Pos.y + 159, 1, 14);
        RenderImage(IMAGE_GUILDINFO_BOTTOM_PIXEL, x, m_Pos.y + 159 + 56, 1, 14);
    }

    for (int y = m_Pos.y + 159; y < m_Pos.y + 159 + 65; y++)
    {
        RenderImage(IMAGE_GUILDINFO_LEFT_PIXEL, m_Pos.x + 10, y, 14, 1);
        RenderImage(IMAGE_GUILDINFO_RIGHT_PIXEL, m_Pos.x + 167, y, 14, 1);
    }

    RenderImage(IMAGE_GUILDINFO_TOP_LEFT, m_Pos.x + 10, m_Pos.y + 159, 14, 14);
    RenderImage(IMAGE_GUILDINFO_TOP_RIGHT, m_Pos.x + 167, m_Pos.y + 159, 14, 14);
    RenderImage(IMAGE_GUILDINFO_BOTTOM_LEFT, m_Pos.x + 10, m_Pos.y + 215, 14, 14);
    RenderImage(IMAGE_GUILDINFO_BOTTOM_RIGHT, m_Pos.x + 167, m_Pos.y + 215, 14, 14);

    glColor4ub(0, 0, 0, 255);
    RenderColor(m_Pos.x + 11, m_Pos.y + 260, 165, 84);
    EndRenderColor();

    for (int x = m_Pos.x + 12; x < m_Pos.x + 12 + 165; x++)
    {
        if (x > 73)
            RenderImage(IMAGE_GUILDINFO_TOP_PIXEL, x, m_Pos.y + 260, 1, 14);
        RenderImage(IMAGE_GUILDINFO_BOTTOM_PIXEL, x, m_Pos.y + 260 + 74, 1, 14);
    }

    for (int y = m_Pos.y + 260; y < m_Pos.y + 260 + 82; y++)
    {
        RenderImage(IMAGE_GUILDINFO_LEFT_PIXEL, m_Pos.x + 10, y, 14, 1);
        RenderImage(IMAGE_GUILDINFO_RIGHT_PIXEL, m_Pos.x + 167, y, 14, 1);
    }
    RenderImage(IMAGE_GUILDINFO_TOP_RIGHT, m_Pos.x + 167, m_Pos.y + 260, 14, 14);
    RenderImage(IMAGE_GUILDINFO_BOTTOM_LEFT, m_Pos.x + 10, m_Pos.y + 334, 14, 14);
    RenderImage(IMAGE_GUILDINFO_BOTTOM_RIGHT, m_Pos.x + 167, m_Pos.y + 334, 14, 14);
    RenderImage(IMAGE_GUILDINFO_TAB_HEAD, m_Pos.x + 11, m_Pos.y + 237, 63.f, 25.f);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderImage(IMAGE_GUILDINFO_SCROLL_TOP, m_Pos.x + 167,
        262, 7, 3);

    for (int i = 0; i < 7; i++)
    {
        RenderImage(IMAGE_GUILDINFO_SCROLL_MIDDLE, m_Pos.x + 167,
            261 + (float)(i * 10 + 4), 7, 15);
    }

    RenderImage(IMAGE_GUILDINFO_SCROLL_BOTTOM, m_Pos.x + 167, 338, 7, 3);

    int Line = 0;
    if (m_EventState == EVENT_SCROLL_BTN_DOWN && m_BackUp > 0)
    {
        m_Loc = (MouseY - m_BackUp);
        if (m_Loc < 0)
            m_Loc = 0;
        else
            if (m_Loc > 49)
                m_Loc = 49;

        if (m_Loc != m_Loc_Bk)
        {
            int Loc_Scroll = ((float)(m_Tot_Notice - m_GuildNotice.GetBoxSize()) / (float)49) * (float)m_Loc;
            Line = Loc_Scroll - m_CurrentListPos;
            m_CurrentListPos += Line;
            m_Loc_Bk = m_Loc;
        }
    }

    if (m_EventState == EVENT_SCROLL_BTN_DOWN)
        glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
    else
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderImage(IMAGE_GUILDINFO_SCROLLBAR_ON, m_Pos.x + 163, 262 + m_Loc, 15, 30);

    m_GuildNotice.Scrolling(Line);

    m_Button[BUTTON_GUILD_OUT].Render();
}

void SEASON3B::CNewUIGuildInfoWindow::RenderScrollBar()
{
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderImage(IMAGE_GUILDINFO_SCROLL_TOP, m_Pos.x + 170, 125, 7, 3);

    for (int i = 0; i < 22; i++)
        RenderImage(IMAGE_GUILDINFO_SCROLL_MIDDLE, m_Pos.x + 170, 124 + (float)(i * 10 + 4), 7, 15);

    RenderImage(IMAGE_GUILDINFO_SCROLL_BOTTOM, m_Pos.x + 170, 352, 7, 3);

    int Line = 0;

    if (m_EventState == EVENT_SCROLL_BTN_DOWN && m_BackUp > 0)
    {
        m_Loc = (MouseY - m_BackUp);
        if (m_Loc < 0)
            m_Loc = 0;
        else
            if (m_Loc > 200)
                m_Loc = 200;

        if (m_Loc != m_Loc_Bk)
        {
            int Loc_Scroll = ((float)(g_nGuildMemberCount - m_GuildMember.GetBoxSize()) / (float)200) * (float)m_Loc;
            Line = Loc_Scroll - m_CurrentListPos;
            m_CurrentListPos += Line;
            m_Loc_Bk = m_Loc;
        }
    }

    if (m_EventState == EVENT_SCROLL_BTN_DOWN)
        glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
    else
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderImage(IMAGE_GUILDINFO_SCROLLBAR_ON, m_Pos.x + 166, 125 + m_Loc, 15, 30);

    m_GuildMember.Scrolling(Line);
}

void SEASON3B::CNewUIGuildInfoWindow::Render_Guild_Info()
{
    POINT ptOrigin = { m_Pos.x + 15, m_Pos.y + 98 };

    if (GuildMark[Hero->GuildMarkIndex].UnionName[0] == NULL)
    {
        ptOrigin.x += 10;
        ptOrigin.y += 8;

        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->SetBgColor(0);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1257]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1258]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1259]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1260]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1261]);

        ptOrigin.y += 25;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1262]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1263]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1264]);
        ptOrigin.y += 20;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1265]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1266]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1267]);

        g_pRenderText->SetTextColor(0xFFFFFFFF);
    }
    else
    {
        glColor4ub(21, 21, 21, 150);
        RenderColor(m_Pos.x + 12, m_Pos.y + 12 + 113, 165, 85);

        glColor4ub(59, 59, 59, 150);
        RenderColor(m_Pos.x + 12, m_Pos.y + 12 + 98, 165, 15);
        EndRenderColor();

        for (int x = m_Pos.x + 12; x < m_Pos.x + 12 + 166; x++)
        {
            RenderImage(IMAGE_GUILDINFO_TOP_PIXEL, x, m_Pos.y + 109, 1, 14);
            RenderImage(IMAGE_GUILDINFO_BOTTOM_PIXEL, x, m_Pos.y + 109 + 92, 1, 14);
        }
        for (int y = m_Pos.y + 109; y < m_Pos.y + 109 + 100; y++)
        {
            RenderImage(IMAGE_GUILDINFO_LEFT_PIXEL, m_Pos.x + 10, y, 14, 1);
            RenderImage(IMAGE_GUILDINFO_RIGHT_PIXEL, m_Pos.x + 167, y, 14, 1);
        }

        RenderImage(IMAGE_GUILDINFO_TOP_LEFT, m_Pos.x + 10, m_Pos.y + 109, 14, 14);
        RenderImage(IMAGE_GUILDINFO_TOP_RIGHT, m_Pos.x + 167, m_Pos.y + 109, 14, 14);
        RenderImage(IMAGE_GUILDINFO_BOTTOM_LEFT, m_Pos.x + 10, m_Pos.y + 201, 14, 14);
        RenderImage(IMAGE_GUILDINFO_BOTTOM_RIGHT, m_Pos.x + 167, m_Pos.y + 201, 14, 14);

        EndRenderColor();

        m_UnionListBox.SetSize(160, 80);
        m_UnionListBox.SetPosition(m_Pos.x + 15, m_Pos.y + 210);
        m_UnionListBox.Render();

        EnableAlphaTest();

        m_Button[BUTTON_UNION_CREATE].Render();
        m_Button[BUTTON_UNION_OUT].Render();
    }
}

void SEASON3B::CNewUIGuildInfoWindow::Render_Guild_Enum()
{
    glColor4ub(21, 21, 21, 150);
    RenderColor(m_Pos.x + 12, m_Pos.y + 12 + 113, 165, 232);
    glColor4ub(59, 59, 59, 150);
    RenderColor(m_Pos.x + 12, m_Pos.y + 12 + 93, 165, 20);
    EndRenderColor();

    for (int x = m_Pos.x + 12; x < m_Pos.x + 177; x++)
    {
        RenderImage(IMAGE_GUILDINFO_TOP_PIXEL, x, m_Pos.y + 93 + 12, 1, 14);
        RenderImage(IMAGE_GUILDINFO_BOTTOM_PIXEL, x, m_Pos.y + 3 + 344, 1, 14);
    }
    for (int y = m_Pos.y + 12 + 93; y < m_Pos.y + 12 + 344; y++)
    {
        RenderImage(IMAGE_GUILDINFO_LEFT_PIXEL, m_Pos.x + 8, y, 14, 1);
        RenderImage(IMAGE_GUILDINFO_RIGHT_PIXEL, m_Pos.x + 168, y, 14, 1);
    }

    RenderImage(IMAGE_GUILDINFO_TOP_LEFT, m_Pos.x + 8, m_Pos.y + 105, 14, 14);
    RenderImage(IMAGE_GUILDINFO_TOP_RIGHT, m_Pos.x + 168, m_Pos.y + 105, 14, 14);
    RenderImage(IMAGE_GUILDINFO_BOTTOM_LEFT, m_Pos.x + 8, m_Pos.y + 347, 14, 14);
    RenderImage(IMAGE_GUILDINFO_BOTTOM_RIGHT, m_Pos.x + 168, m_Pos.y + 347, 14, 14);

    if (Hero->GuildStatus == G_MASTER)
    {
        m_Button[BUTTON_GET_POSITION].Render();
        m_Button[BUTTON_FREE_POSITION].Render();
        m_Button[BUTTON_GET_OUT].Render();
    }
}

float SEASON3B::CNewUIGuildInfoWindow::GetLayerDepth()
{
    return 4.5f;
}

void SEASON3B::CNewUIGuildInfoWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_GUILDINFO_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_GUILDINFO_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_GUILDINFO_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_GUILDINFO_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_GUILDINFO_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_guild_tab01.tga", IMAGE_GUILDINFO_TAB_LIST, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_guild_tab02.tga", IMAGE_GUILDINFO_TAB_POINT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_guild_tab03.tga", IMAGE_GUILDINFO_TAB_HEAD, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_item_table03(Up).tga", IMAGE_GUILDINFO_TOP_PIXEL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_table03(Dw).tga", IMAGE_GUILDINFO_BOTTOM_PIXEL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_table03(L).tga", IMAGE_GUILDINFO_LEFT_PIXEL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_table03(R).tga", IMAGE_GUILDINFO_RIGHT_PIXEL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_GUILDINFO_EXIT_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_small.tga", IMAGE_GUILDINFO_BUTTON, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_scrollbar_up.tga", IMAGE_GUILDINFO_SCROLL_TOP);
    LoadBitmap(L"Interface\\newui_scrollbar_m.tga", IMAGE_GUILDINFO_SCROLL_MIDDLE);
    LoadBitmap(L"Interface\\newui_scrollbar_down.tga", IMAGE_GUILDINFO_SCROLL_BOTTOM);
    LoadBitmap(L"Interface\\newui_scroll_on.tga", IMAGE_GUILDINFO_SCROLLBAR_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scroll_off.tga", IMAGE_GUILDINFO_SCROLLBAR_OFF, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scrollbar_stretch.jpg", IMAGE_GUILDINFO_DRAG_BTN, GL_LINEAR);
}

void SEASON3B::CNewUIGuildInfoWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_GUILDINFO_BOTTOM);
    DeleteBitmap(IMAGE_GUILDINFO_RIGHT);
    DeleteBitmap(IMAGE_GUILDINFO_LEFT);
    DeleteBitmap(IMAGE_GUILDINFO_TOP);
    DeleteBitmap(IMAGE_GUILDINFO_BACK);

    DeleteBitmap(IMAGE_GUILDINFO_TAB_LIST);
    DeleteBitmap(IMAGE_GUILDINFO_TAB_POINT);
    DeleteBitmap(IMAGE_GUILDINFO_TAB_HEAD);
    DeleteBitmap(IMAGE_GUILDINFO_BUTTON);

    DeleteBitmap(IMAGE_GUILDINFO_TOP_PIXEL);
    DeleteBitmap(IMAGE_GUILDINFO_BOTTOM_PIXEL);
    DeleteBitmap(IMAGE_GUILDINFO_RIGHT_PIXEL);

    DeleteBitmap(IMAGE_GUILDINFO_LEFT_PIXEL);
    DeleteBitmap(IMAGE_GUILDINFO_EXIT_BTN);

    DeleteBitmap(IMAGE_GUILDINFO_SCROLL_TOP);
    DeleteBitmap(IMAGE_GUILDINFO_SCROLL_MIDDLE);
    DeleteBitmap(IMAGE_GUILDINFO_SCROLL_BOTTOM);
    DeleteBitmap(IMAGE_GUILDINFO_SCROLLBAR_ON);
    DeleteBitmap(IMAGE_GUILDINFO_SCROLLBAR_OFF);
    DeleteBitmap(IMAGE_GUILDINFO_DRAG_BTN);
}

void SEASON3B::CNewUIGuildInfoWindow::AddGuildNotice(wchar_t* szText)
{
    wchar_t szTemp[5][MAX_TEXT_LENGTH + 1] = { {0}, {0}, {0}, {0}, {0} };
    CutText3(szText, szTemp[0], 110, 5, MAX_TEXT_LENGTH + 1);

    for (int i = 0; i < 5; ++i)
    {
        if (szTemp[i][0])
        {
            m_GuildNotice.AddText(szTemp[i]);
            m_Tot_Notice++;
        }
    }
    m_GuildNotice.Scrolling(m_GuildNotice.GetLineNum() - m_GuildNotice.GetBoxSize());
}

void SEASON3B::CNewUIGuildInfoWindow::AddGuildMember(GUILD_LIST_t* pInfo)
{
    m_GuildMember.AddText(pInfo->Name, pInfo->Number, pInfo->Server, pInfo->GuildStatus);
    m_GuildMember.Scrolling(-m_GuildMember.GetBoxSize());
}

void SEASON3B::CNewUIGuildInfoWindow::GuildClear()
{
    m_GuildMember.Clear();
}

void SEASON3B::CNewUIGuildInfoWindow::UnionGuildClear()
{
    m_UnionListBox.Clear();
}

void SEASON3B::CNewUIGuildInfoWindow::NoticeClear()
{
    m_GuildNotice.Clear();
}

void SEASON3B::CNewUIGuildInfoWindow::SetRivalGuildName(wchar_t* szName)
{
    memcpy(m_RivalGuildName, szName, sizeof(char) * MAX_GUILDNAME);
    m_RivalGuildName[MAX_GUILDNAME] = NULL;
}

void SEASON3B::CNewUIGuildInfoWindow::AddUnionList(BYTE* pGuildMark, wchar_t* szGuildName, int nMemberCount)
{
    m_UnionListBox.AddText(pGuildMark, szGuildName, nMemberCount);
    m_bRequestUnionList = false;
}

int SEASON3B::CNewUIGuildInfoWindow::GetUnionCount()
{
    return m_UnionListBox.GetTextCount();
}

void SEASON3B::CNewUIGuildInfoWindow::ReceiveGuildRelationShip(BYTE byRelationShipType, BYTE byRequestType,
    BYTE  byTargetUserIndexH, BYTE byTargetUserIndexL)
{
    if (!g_MessageBox->IsEmpty())
    {
        SocketClient->ToGameServer()->SendGuildRelationshipChangeResponse(
            byRelationShipType,
            byRequestType,
            0x00,
            MAKEWORD(byTargetUserIndexH, byTargetUserIndexL));
    }
    else
    {
        m_MessageInfo.s_byRelationShipType = byRelationShipType;
        m_MessageInfo.s_byRelationShipRequestType = byRequestType;
        m_MessageInfo.s_byTargetUserIndexH = byTargetUserIndexH;
        m_MessageInfo.s_byTargetUserIndexL = byTargetUserIndexL;

        int nCharKey = MAKEWORD(m_MessageInfo.s_byTargetUserIndexL, m_MessageInfo.s_byTargetUserIndexH);
        int nIndex = FindCharacterIndex(nCharKey);
        if (nIndex < 0 || nIndex >= MAX_CHARACTERS_CLIENT)
            return;
        CHARACTER* pPlayer = &CharactersClient[nIndex];

        wchar_t szText[3][64];
        ZeroMemory(szText, sizeof(szText));

        if (m_MessageInfo.s_byRelationShipType == 0x01)			// Union
        {
            if (m_MessageInfo.s_byRelationShipRequestType == 0x01)	// Join
            {
                swprintf(szText[0], GlobalText[1280], pPlayer->ID);
                swprintf(szText[1], GlobalText[1281]);
                swprintf(szText[2], GlobalText[1283]);
            }
            else										// Break Off
            {
                swprintf(szText[0], GlobalText[1280], pPlayer->ID);
                swprintf(szText[1], GlobalText[1282]);
                swprintf(szText[2], GlobalText[1283]);
            }
        }
        else if (m_MessageInfo.s_byRelationShipType == 0x02)		// Rival
        {
            if (m_MessageInfo.s_byRelationShipRequestType == 0x01)	// Join
            {
                swprintf(szText[0], GlobalText[1284], pPlayer->ID);
                swprintf(szText[1], GlobalText[1286]);
                swprintf(szText[2], GlobalText[1283]);
            }
            else
            {
                swprintf(szText[0], GlobalText[1284], pPlayer->ID);
                swprintf(szText[1], GlobalText[1285]);
                swprintf(szText[2], GlobalText[1283]);
            }
        }

        SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGuildRelationShipMsgBoxLayout), &pMsgBox);
        if (pMsgBox)
        {
            pMsgBox->AddMsg(szText[0]);
            pMsgBox->AddMsg(szText[1]);
            pMsgBox->AddMsg(szText[2]);
        }
    }
}