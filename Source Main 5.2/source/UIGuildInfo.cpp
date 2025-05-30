//////////////////////////////////////////////////////////////////////////
//  UIGuildInfo.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInterface.h"
#include "ZzzTexture.h"
#include "UIManager.h"
#include "UIGuildInfo.h"
#include "CSParts.h"
#include "NewUICommonMessageBox.h"
#include "CharacterManager.h"
#include "DSPlaySound.h"
#include "ZzzInventory.h"



extern int			DeleteGuildIndex;
extern DWORD		g_dwActiveUIID;

wchar_t	s_szTargetID[MAX_ID_SIZE + 1];
extern int s_nTargetFireMemberIndex = 0;

char Guild_Skill_Button = 0;

void RenderGoldRect(float fPos_x, float fPos_y, float fWidth, float fHeight, int iFillType = 0);

int DoBreakUpGuildAction(POPUP_RESULT Result)
{
    if (Result == POPUP_RESULT_YES)
    {
        DeleteGuildIndex = 0;
        ErrorMessage = MESSAGE_DELETE_GUILD;
        ClearInput(FALSE);
        InputEnable = FALSE;
        InputNumber = 1;
        InputTextMax[0] = g_iLengthAuthorityCode;
        InputTextHide[0] = 1;
    }

    return 1;
}

int DoDisbandAction(POPUP_RESULT Result)
{
    if (Result == POPUP_RESULT_YES)
    {
        SocketClient->ToGameServer()->SendGuildRoleAssignRequest(G_PERSON, s_szTargetID, 0x03);
        SocketClient->ToGameServer()->SendGuildListRequest();
    }
    return 1;
}

int DoFireAction(POPUP_RESULT Result)
{
    if (Result == POPUP_RESULT_YES)
    {
        DeleteGuildIndex = s_nTargetFireMemberIndex;
        ErrorMessage = MESSAGE_DELETE_GUILD;
    }
    return 1;
}

POINT			s_ptAppointWindow;
enum eAppointType { APPOINT_SUBGUILDMASTER = 64, APPOINT_BATTLEMASTSER = 32 };
eAppointType	s_eAppointType;
CUIButton		s_PopupAppointOkButton;
CUIButton		s_PopupAppointCancelButton;
GUILD_STATUS	s_eAppointStatus;

void DoAppointAction()
{
    if (CheckMouseIn(s_ptAppointWindow.x, s_ptAppointWindow.y, 213, 2 * 5 + 6 * 40))
        MouseOnWindow = TRUE;

    if (MouseLButtonPush)
    {
        if (CheckMouseIn(s_ptAppointWindow.x + 20, s_ptAppointWindow.y + 19, 170, 20.f))
            s_eAppointType = APPOINT_SUBGUILDMASTER;
        if (CheckMouseIn(s_ptAppointWindow.x + 20, s_ptAppointWindow.y + 19 + 25 - 3, 170, 20.f))
            s_eAppointType = APPOINT_BATTLEMASTSER;
    }

    if (s_PopupAppointOkButton.DoMouseAction())
    {
        SocketClient->ToGameServer()->SendGuildRoleAssignRequest(s_eAppointStatus, s_szTargetID, s_eAppointStatus == G_PERSON ? 0x01 : 0x02);

        SocketClient->ToGameServer()->SendGuildListRequest();

        g_pUIPopup->CancelPopup();
    }
    if (s_PopupAppointCancelButton.DoMouseAction())
        g_pUIPopup->CancelPopup();
}

void RenderAppoint()
{
    s_ptAppointWindow = g_pUIManager->RenderWindowBase(150);

    EnableAlphaTest();

    RenderGoldRect(s_ptAppointWindow.x + 20, s_ptAppointWindow.y + 19, 170, 20.f, (s_eAppointType == APPOINT_SUBGUILDMASTER ? 1 : 0));
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(s_ptAppointWindow.x + 20, s_ptAppointWindow.y + 24, GlobalText[1311], 170, 0, RT3_SORT_CENTER);

    RenderGoldRect(s_ptAppointWindow.x + 20, s_ptAppointWindow.y + 19 + 25 - 5, 170, 20.f, (s_eAppointType == APPOINT_BATTLEMASTSER ? 1 : 0));
    g_pRenderText->RenderText(s_ptAppointWindow.x + 20, s_ptAppointWindow.y + 24 + 25 - 3, GlobalText[1312], 170, 0, RT3_SORT_CENTER);

    wchar_t Text[64];
    swprintf(Text, GlobalText[1314], s_szTargetID, (s_eAppointType == APPOINT_SUBGUILDMASTER ? GlobalText[1301] : GlobalText[1302]));

    g_pRenderText->RenderText(s_ptAppointWindow.x + 20, s_ptAppointWindow.y + 24 + 25 * 2 + 1, Text, 170, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(s_ptAppointWindow.x + 20, s_ptAppointWindow.y + 24 + 25 * 2 + 1 + 18, GlobalText[1315], 170, 0, RT3_SORT_CENTER);

    s_PopupAppointOkButton.SetPosition(s_ptAppointWindow.x + 15 + 40, s_ptAppointWindow.y + 117);
    s_PopupAppointOkButton.Render();
    s_PopupAppointCancelButton.SetPosition(s_ptAppointWindow.x + 15 + 95, s_ptAppointWindow.y + 117);
    s_PopupAppointCancelButton.Render();
}

CUIGuildInfo::CUIGuildInfo()
{
    m_bOpened = false;
    m_bRequestUnionList = FALSE;
    m_nCurrentTab = 1;
    SetPosition(640 - 190, 0);
    SetSize(190, 380);

    ZeroMemory(m_szRivalGuildName, sizeof(char) * (MAX_GUILDNAME + 1));

    m_dwPopupID = 0;

    int nButtonID = 1;

    m_BreakUpGuildButton.Init(nButtonID++, L"");
    m_BreakUpGuildButton.SetParentUIID(GetUIID());
    m_BreakUpGuildButton.SetSize(50, 18);
    m_BreakUpGuildButton.SetPosition(GetPosition_x() + 70, GetPosition_y() + m_iHeight - m_BreakUpGuildButton.GetHeight());

    s_ptAppointWindow.x = s_ptAppointWindow.y = 0;
    m_AppointButton.Init(nButtonID++, GlobalText[1307]);
    m_AppointButton.SetParentUIID(GetUIID());
    m_AppointButton.SetSize(50, 18);
    m_AppointButton.SetPosition(GetPosition_x() + 15 + 3, GetPosition_y() + 365);

    m_DisbandButton.Init(nButtonID++, GlobalText[1308]);
    m_DisbandButton.SetParentUIID(GetUIID());
    m_DisbandButton.SetSize(50, 18);
    m_DisbandButton.SetPosition(GetPosition_x() + 15 + 55, GetPosition_y() + 365);

    m_FireButton.Init(nButtonID++, GlobalText[1309]);
    m_FireButton.SetParentUIID(GetUIID());
    m_FireButton.SetSize(50, 18);
    m_FireButton.SetPosition(GetPosition_x() + 15 + 107, GetPosition_y() + 365);

    s_eAppointType = APPOINT_SUBGUILDMASTER;

    s_PopupAppointOkButton.Init(nButtonID++, GlobalText[228]);
    s_PopupAppointOkButton.SetParentUIID(GetUIID());
    s_PopupAppointOkButton.SetSize(50, 18);

    s_PopupAppointCancelButton.Init(nButtonID++, GlobalText[229]);
    s_PopupAppointCancelButton.SetParentUIID(GetUIID());
    s_PopupAppointCancelButton.SetSize(50, 18);

    m_BreakUnionButton.Init(nButtonID++, L"");
    m_BreakUnionButton.SetParentUIID(GetUIID());
    m_BreakUnionButton.SetSize(50, 18);
    m_BreakUnionButton.SetPosition(GetPosition_x() + 15 + 55, GetPosition_y() + 350);

    m_BanUnionButton.Init(nButtonID++, GlobalText[1422]);
    m_BanUnionButton.SetParentUIID(GetUIID());
    m_BanUnionButton.SetSize(50, 18);
    m_BanUnionButton.SetPosition(GetPosition_x() + 15 + 55, GetPosition_y() + 220);
}

CUIGuildInfo::~CUIGuildInfo()
{
}

int CUIGuildInfo::GetGuildMemberIndex(wchar_t* szName)
{
    for (int i = 0; i < g_nGuildMemberCount; ++i)
    {
        if (GuildList[i].Name && !wcscmp(GuildList[i].Name, szName))
            return i;
    }

    return -1;
}

const wchar_t* CUIGuildInfo::GetGuildMasterName()
{
    for (int i = 0; i < g_nGuildMemberCount; ++i)
    {
        if (GuildList[i].Name && GuildList[i].GuildStatus == G_MASTER)
            return GuildList[i].Name;
    }

    return GlobalText[1361];
}

const wchar_t* CUIGuildInfo::GetSubGuildMasterName()
{
    for (int i = 0; i < g_nGuildMemberCount; ++i)
    {
        if (GuildList[i].Name && GuildList[i].GuildStatus == G_SUB_MASTER)
            return GuildList[i].Name;
    }

    return GlobalText[1361];
}

const wchar_t* CUIGuildInfo::GetBattleMasterName()
{
    for (int i = 0; i < g_nGuildMemberCount; ++i)
    {
        if (GuildList[i].Name && GuildList[i].GuildStatus == G_BATTLE_MASTER)
            return GuildList[i].Name;
    }

    return GlobalText[1361];
}

void CUIGuildInfo::CloseMyPopup()
{
    DWORD dwCurrPopupID = g_pUIPopup->GetPopupID();
    if (dwCurrPopupID == m_dwPopupID)
        g_pUIPopup->CancelPopup();
}

void CUIGuildInfo::DoGuildInfoTabMouseAction()
{
    g_dwActiveUIID = m_GuildNoticeListBox.GetUIID();
    m_GuildNoticeListBox.DoAction();
    g_dwActiveUIID = 0;

    if (m_BreakUpGuildButton.DoMouseAction())
    {
        if (IsGuildMaster())
        {
            if (!wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[Hero->GuildMarkIndex].UnionName))
            {
                wchar_t szText[50];
                wcscpy(szText, GlobalText[1270]);
                m_dwPopupID = g_pUIPopup->SetPopup(szText, 1, 50, POPUP_OK, NULL);
            }
            else
            {
                wchar_t szText[4][100];

                wcscpy(szText[0], GlobalText[1363]);
                wcscpy(szText[1], GlobalText[1364]);
                wcscpy(szText[2], GlobalText[1365]);
                wcscpy(szText[3], GlobalText[1366]);
                m_dwPopupID = g_pUIPopup->SetPopup(&szText[0][0], 4, 100, POPUP_YESNO, ::DoBreakUpGuildAction);
            }
        }
        else
        {
            DeleteGuildIndex = GetGuildMemberIndex(Hero->ID);
            ErrorMessage = MESSAGE_DELETE_GUILD;
            ClearInput(FALSE);
            InputEnable = FALSE;
            InputNumber = 1;
            InputTextMax[0] = g_iLengthAuthorityCode;
            InputTextHide[0] = 1;
        }
    }
}

void CUIGuildInfo::RenderGuildInfoTab()
{
    POINT ptOrigin = { GetPosition_x() + 15 + 54, GetPosition_y() + 75 + 29 };

    RenderGoldRect(ptOrigin.x, ptOrigin.y, 53, 53);
    ptOrigin.x += 3;	ptOrigin.y += 3;
    CreateGuildMark(Hero->GuildMarkIndex);
    RenderBitmap(BITMAP_GUILD, ptOrigin.x, ptOrigin.y, 48, 48);

    wchar_t szTemp[64];

    ptOrigin.x = GetPosition_x() + 15 + 10;	ptOrigin.y = GetPosition_y() + 170;
    RenderGoldRect(ptOrigin.x, ptOrigin.y, 140, 62);
    ptOrigin.x += 5;
    ptOrigin.y += 8;
    swprintf(szTemp, L"%s :", GlobalText[1332]);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, szTemp, 140 * g_fScreenRate_x, 0, RT3_SORT_LEFT);
    ptOrigin.y += 13;
    swprintf(szTemp, GlobalText[1256], GuildTotalScore);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, szTemp, 140 * g_fScreenRate_x, 0, RT3_SORT_LEFT);

    ptOrigin.y += 13;
    if (IsGuildMaster())
    {
        int Class = gCharacterManager.GetBaseClass(CharacterAttribute->Class);
        if (Class == CLASS_DARK_LORD)
        {
            int nCount = CharacterAttribute->Level / 10 + CharacterAttribute->Charisma / 10;
            if (nCount > 80)	nCount = 80;
            swprintf(szTemp, GlobalText[1362], g_nGuildMemberCount, nCount);
        }
        else
            swprintf(szTemp, GlobalText[1362], g_nGuildMemberCount, CharacterAttribute->Level / 10);
    }
    else
    {
        swprintf(szTemp, GlobalText[1310], g_nGuildMemberCount);
    }
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, szTemp, 140 * g_fScreenRate_x, 0, RT3_SORT_LEFT);

    ptOrigin.y += 13;
    swprintf(szTemp, L"%s : %s", GlobalText[1321], m_szRivalGuildName[0] ? m_szRivalGuildName : GlobalText[1361]);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, szTemp, 140 * g_fScreenRate_x, 0, RT3_SORT_LEFT);

    ptOrigin.x -= 5;
    ptOrigin.y += 33;
    g_pRenderText->RenderText(ptOrigin.x + 5, ptOrigin.y, GlobalText[1323], 136 * g_fScreenRate_x, 0, RT3_SORT_LEFT);

    ptOrigin.y += 10;
    m_GuildNoticeListBox.SetSize(136, 80);
    m_GuildNoticeListBox.SetPosition(ptOrigin.x + 3, ptOrigin.y + m_GuildNoticeListBox.GetHeight());
    m_GuildNoticeListBox.Render();
    RenderGoldRect(ptOrigin.x, ptOrigin.y - 14, 140, 95);

    m_BreakUpGuildButton.SetCaption(IsGuildMaster() ? GlobalText[188] : GlobalText[189]);
    m_BreakUpGuildButton.Render();
}

void CUIGuildInfo::DoGuildMemberTabMouseAction()
{
    g_dwActiveUIID = m_GuildMemberListBox.GetUIID();
    m_GuildMemberListBox.DoAction();
    g_dwActiveUIID = 0;

    if (PressKey(VK_PRIOR))
        m_GuildMemberListBox.Scrolling(-1 * m_GuildMemberListBox.GetBoxSize());
    if (PressKey(VK_NEXT))
        m_GuildMemberListBox.Scrolling(m_GuildMemberListBox.GetBoxSize());

    if (IsGuildMaster())
    {
        if (m_AppointButton.DoMouseAction())
        {
            if (GUILDLIST_TEXT* pText = m_GuildMemberListBox.GetSelectedText())
            {
                if (pText->m_GuildStatus != G_MASTER)
                {
                    if (GUILDLIST_TEXT* pText = m_GuildMemberListBox.GetSelectedText())
                    {
                        s_eAppointStatus = (GUILD_STATUS)pText->m_GuildStatus;
                        wcscpy(s_szTargetID, pText->m_szID);
                        m_dwPopupID = g_pUIPopup->SetPopup(NULL, 0, 0, POPUP_CUSTOM, NULL);
                        g_pUIPopup->SetPopupExtraFunc(::DoAppointAction, ::RenderAppoint);
                    }
                }
            }
        }

        if (m_DisbandButton.DoMouseAction())
        {
            if (GUILDLIST_TEXT* pText = m_GuildMemberListBox.GetSelectedText())
            {
                if (pText->m_GuildStatus == G_SUB_MASTER || pText->m_GuildStatus == G_BATTLE_MASTER)
                {
                    wchar_t szText[2][64];
                    GUILDLIST_TEXT* pText = m_GuildMemberListBox.GetSelectedText();
                    swprintf(szText[0], GlobalText[1367], pText->m_szID);
                    wcscpy(szText[1], GlobalText[1368]);

                    if (GUILDLIST_TEXT* pText = m_GuildMemberListBox.GetSelectedText())
                        wcscpy(s_szTargetID, pText->m_szID);

                    m_dwPopupID = g_pUIPopup->SetPopup(&szText[0][0], 2, 64, POPUP_YESNO, ::DoDisbandAction);
                }
            }
        }

        if (m_FireButton.DoMouseAction())
        {
            if (GUILDLIST_TEXT* pText = m_GuildMemberListBox.GetSelectedText())
            {
                if (pText->m_GuildStatus != G_MASTER)
                {
                    GUILDLIST_TEXT* pText = m_GuildMemberListBox.GetSelectedText();

                    if (GUILDLIST_TEXT* pText = m_GuildMemberListBox.GetSelectedText())
                    {
                        s_nTargetFireMemberIndex = GetGuildMemberIndex(pText->m_szID);
                    }

                    wchar_t szText[2][64];
                    swprintf(szText[0], GlobalText[1367], pText->m_szID);
                    wcscpy(szText[1], GlobalText[1369]);
                    m_dwPopupID = g_pUIPopup->SetPopup(&szText[0][0], 2, 64, POPUP_YESNO, ::DoFireAction);
                }
            }
        }
    }
}

void CUIGuildInfo::RenderGuildMemberTab()
{
    if (g_nGuildMemberCount > 0)
        m_GuildMemberListBox.Render();

    if (IsGuildMaster())
    {
        m_AppointButton.Render();
        m_DisbandButton.Render();
        m_FireButton.Render();
    }
}

int DoBanUnionGuildAction(POPUP_RESULT Result)
{
    if (Result == POPUP_RESULT_YES)
    {
        SocketClient->ToGameServer()->SendRemoveAllianceGuildRequest(s_szTargetID);
    }
    return 1;
}

void CUIGuildInfo::DoGuildUnionMouseAction()
{
    g_dwActiveUIID = m_UnionListBox.GetUIID();
    m_UnionListBox.DoAction();
    g_dwActiveUIID = 0;

    if (Hero->GuildStatus == G_MASTER)
    {
        if (GuildMark[Hero->GuildMarkIndex].UnionName[0])
        {
            if (m_BreakUnionButton.DoMouseAction())
            {
                if (!wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[Hero->GuildMarkIndex].UnionName))
                {
                    wchar_t szText[50];
                    wcscpy(szText, GlobalText[1271]);
                    m_dwPopupID = g_pUIPopup->SetPopup(szText, 1, 50, POPUP_OK, NULL);
                }
                else
                {
                    SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(0x01, 0x02, Hero->Key);
                }
            }

            if (m_BanUnionButton.DoMouseAction())
            {
                if (UNIONGUILD_TEXT* pText = m_UnionListBox.GetSelectedText())
                {
                    if (wcscmp(pText->szName, GuildMark[Hero->GuildMarkIndex].GuildName))
                    {
                        wchar_t szText[2][64];

                        swprintf(szText[0], GlobalText[1423], pText->szName);
                        wcscpy(szText[1], GlobalText[1369]);
                        wcscpy(s_szTargetID, pText->szName);

                        m_dwPopupID = g_pUIPopup->SetPopup(&szText[0][0], 2, 64, POPUP_YESNO, ::DoBanUnionGuildAction);
                    }
                }
            }
        }
    }
}

void CUIGuildInfo::RenderGuildUnionTab()
{
    POINT ptOrigin = { GetPosition_x() + 15, GetPosition_y() + 98 };

    if (GuildMark[Hero->GuildMarkIndex].UnionName[0] == NULL)
    {
        ptOrigin.x += 10;
        ptOrigin.y += 8;

        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1257]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1258]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1259]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1260]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1261]);

        g_pRenderText->SetTextColor(0xFFFFFFFF);
        g_pRenderText->SetBgColor(0xFF0000CC);

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
    else if (GuildMark[Hero->GuildMarkIndex].UnionName[0])
    {
        ptOrigin.y += 12;
        m_UnionListBox.SetSize(160, 80);
        m_UnionListBox.SetPosition(ptOrigin.x, ptOrigin.y + m_GuildNoticeListBox.GetHeight());
        m_UnionListBox.Render();

        if (Hero->GuildStatus == G_MASTER)
        {
            if (!wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[Hero->GuildMarkIndex].UnionName))
            {
                m_BreakUnionButton.SetCaption(GlobalText[1324]);
                m_BanUnionButton.SetState(UISTATE_NORMAL);
                m_BanUnionButton.Render();
            }
            else
            {
                m_BreakUnionButton.SetCaption(GlobalText[1325]);
                m_BanUnionButton.SetState(UISTATE_DISABLE);
            }
            m_BreakUnionButton.Render();
        }
    }
}

void CUIGuildInfo::SetRivalGuildName(wchar_t* szName)
{
    memcpy(m_szRivalGuildName, szName, sizeof(char) * MAX_GUILDNAME);
    m_szRivalGuildName[MAX_GUILDNAME] = NULL;
}

void CUIGuildInfo::AddGuildNotice(wchar_t* szText)
{
    wchar_t szTemp[5][MAX_TEXT_LENGTH + 1] = { {0}, {0}, {0}, {0}, {0} };
    CutText3(szText, szTemp[0], 110, 5, MAX_TEXT_LENGTH + 1);

    for (int i = 0; i < 5; ++i)
    {
        if (szTemp[i][0])
            m_GuildNoticeListBox.AddText(szTemp[i]);
    }
    m_GuildNoticeListBox.Scrolling(m_GuildNoticeListBox.GetLineNum() - m_GuildNoticeListBox.GetBoxSize());
}

void CUIGuildInfo::ClearGuildLog()
{
    m_GuildNoticeListBox.Clear();
}

void CUIGuildInfo::AddMemberList(GUILD_LIST_t* pInfo)
{
    m_GuildMemberListBox.AddText(pInfo->Name, pInfo->Number, pInfo->Server, pInfo->GuildStatus);
}

void CUIGuildInfo::ClearMemberList()
{
    m_GuildMemberListBox.Clear();
}

void CUIGuildInfo::AddUnionList(BYTE* pGuildMark, wchar_t* szGuildName, int nMemberCount)
{
    m_UnionListBox.AddText(pGuildMark, szGuildName, nMemberCount);
}

int CUIGuildInfo::GetUnionCount()
{
    return m_UnionListBox.GetTextCount();
}

void CUIGuildInfo::ClearUnionList()
{
    m_UnionListBox.Clear();
}

BOOL CUIGuildInfo::DoMouseAction()
{
    if (CheckMouseIn(GetPosition_x(), GetPosition_y(), GetWidth(), 256 + 177))
        MouseOnWindow = TRUE;

    if (MouseLButtonPush && CheckMouseIn(GetPosition_x() + 25, GetPosition_y() + 395, 24, 24))
    {
        MouseLButtonPush = FALSE;
        MouseUpdateTime = 0;
        MouseUpdateTimeMax = 6;
        PlayBuffer(SOUND_CLICK01);
        Close();
    }

    if (Hero->GuildStatus == G_NONE) return FALSE;

    if (MouseLButton)
    {
        if (m_nCurrentTab != 0 && CheckMouseIn(GetPosition_x() + 15, GetPosition_y() + 75, 52, 16))
        {
            CloseMyPopup();
            m_nCurrentTab = 0;
        }
        else if (m_nCurrentTab != 1 && CheckMouseIn(GetPosition_x() + 15 + 52, GetPosition_y() + 75, 52, 16))
        {
            CloseMyPopup();
            m_nCurrentTab = 1;
        }
        else if (m_nCurrentTab != 2 && CheckMouseIn(GetPosition_x() + 15 + 52 * 2, GetPosition_y() + 75, 52, 16))
        {
            if (!m_bRequestUnionList && GuildMark[Hero->GuildMarkIndex].UnionName[0])
            {
                SocketClient->ToGameServer()->SendRequestAllianceList();
                m_bRequestUnionList = TRUE;
            }
            CloseMyPopup();
            m_nCurrentTab = 2;
        }
    }

    switch (m_nCurrentTab)
    {
    case 0:
        DoGuildInfoTabMouseAction();
        break;
    case 1:
        DoGuildMemberTabMouseAction();
        break;
    case 2:
        DoGuildUnionMouseAction();
        break;
    default:
        break;
    };
    return FALSE;
}

void CUIGuildInfo::Render()
{
    glColor4f(1.f, 1.f, 1.f, 1.f);

    POINT ptOrigin = { GetPosition_x(), GetPosition_y() };

    DisableAlphaBlend();
    RenderBitmap(BITMAP_INVENTORY, ptOrigin.x, ptOrigin.y, 190, 256, 0.f, 0.f, 190.f / 256.f, 256.f / 256.f);
    RenderBitmap(BITMAP_INVENTORY + 1, ptOrigin.x, ptOrigin.y + 256, 190, 177, 0.f, 0.f, 190.f / 256.f, 177.f / 256.f);

    EnableAlphaTest();

    ptOrigin.x = GetPosition_x() + 35;
    ptOrigin.y = GetPosition_y() + 12;
    wchar_t szTemp[100];
    wcscpy(szTemp, GlobalText[180]);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, szTemp, 120, 0, RT3_SORT_CENTER);

    if (Hero->GuildStatus != G_NONE)
    {
        ptOrigin.x = GetPosition_x() + 35;
        ptOrigin.y = GetPosition_y() + 53;
        g_pRenderText->SetTextColor(200, 255, 100, 255);
        swprintf(szTemp, L"%s ( Score:%d )", GuildMark[Hero->GuildMarkIndex].GuildName, GuildTotalScore);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, szTemp, 120, 0, RT3_SORT_CENTER);
    }

    ptOrigin.x = GetPosition_x() + 20;
    ptOrigin.y = GetPosition_y() + 50;
    if (Hero->GuildStatus == G_NONE)
    {
        g_pRenderText->SetFont(g_hFont);
        g_pRenderText->SetTextColor(230, 230, 230, 255);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[185]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[186]);
        ptOrigin.y += 15;
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[187]);
    }

    glColor4f(1.f, 1.f, 1.f, 1.f);
    ptOrigin.x = GetPosition_x() + 25;
    ptOrigin.y = GetPosition_y() + 395;
    RenderBitmap(BITMAP_INVENTORY_BUTTON, ptOrigin.x, ptOrigin.y, 24, 24, 0.f, 0.f, 24.f / 32.f, 24.f / 32.f);
    if (CheckMouseIn(ptOrigin.x, ptOrigin.y, 24, 24))
    {
        g_pRenderText->SetFont(g_hFont);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->SetBgColor(0, 0, 0, 255);
        RenderTipText(ptOrigin.x, ptOrigin.y - 13, GlobalText[220]);
    }

    if (Hero->GuildStatus == G_NONE)
        return;

    ptOrigin.x = GetPosition_x() + 15;
    ptOrigin.y = GetPosition_y() + 91;
    glColor4ub(146, 144, 141, 255);
    if (m_nCurrentTab != 0) RenderColor(ptOrigin.x, ptOrigin.y, 54, 1);
    ptOrigin.x += 54;
    if (m_nCurrentTab != 1) RenderColor(ptOrigin.x, ptOrigin.y, 54, 1);
    ptOrigin.x += 54;
    if (m_nCurrentTab != 2) RenderColor(ptOrigin.x, ptOrigin.y, 54, 1);
    EndRenderColor();

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    ptOrigin.x = GetPosition_x() + 15;
    ptOrigin.y = GetPosition_y() + 75;
    if (m_nCurrentTab == 0) glColor4f(1.f, 1.f, 1.f, 1.f);
    else glColor4f(0.6f, 0.6f, 0.6f, 1.f);
    RenderBitmap(BITMAP_INTERFACE_EX + 9, ptOrigin.x, ptOrigin.y - (m_nCurrentTab == 0 ? 2 : 0),
        (float)52, (float)16 + (m_nCurrentTab == 0 ? 2 : 0), 0.f, 0.f, 48.f / 64.f, 15.f / 16.f);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 4 - (m_nCurrentTab == 0 ? 1 : 0), GlobalText[946], 52, 0, RT3_SORT_CENTER);

    ptOrigin.x += 54;
    if (m_nCurrentTab == 1) glColor4f(1.f, 1.f, 1.f, 1.f);
    else glColor4f(0.6f, 0.6f, 0.6f, 1.f);
    RenderBitmap(BITMAP_INTERFACE_EX + 9, ptOrigin.x, ptOrigin.y - (m_nCurrentTab == 1 ? 2 : 0),
        (float)52, (float)16 + (m_nCurrentTab == 1 ? 2 : 0), 0.f, 0.f, 48.f / 64.f, 15.f / 16.f);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 4 - (m_nCurrentTab == 1 ? 1 : 0), GlobalText[1330], 52, 0, RT3_SORT_CENTER);

    ptOrigin.x += 54;
    if (m_nCurrentTab == 2) glColor4f(1.f, 1.f, 1.f, 1.f);
    else glColor4f(0.6f, 0.6f, 0.6f, 1.f);
    RenderBitmap(BITMAP_INTERFACE_EX + 9, ptOrigin.x, ptOrigin.y - (m_nCurrentTab == 2 ? 2 : 0),
        (float)52, (float)16 + (m_nCurrentTab == 2 ? 2 : 0), 0.f, 0.f, 48.f / 64.f, 15.f / 16.f);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 4 - (m_nCurrentTab == 2 ? 1 : 0), GlobalText[1352], 52, 0, RT3_SORT_CENTER);

    glColor4f(1.f, 1.f, 1.f, 1.f);

    switch (m_nCurrentTab)
    {
    case 0:
        RenderGuildInfoTab();
        break;
    case 1:
        RenderGuildMemberTab();
        break;
    case 2:
        RenderGuildUnionTab();
        break;
    default:
        break;
    };
}

void CUIGuildInfo::Open()
{
    if (m_bOpened)	return;

    m_bOpened = TRUE;
}

bool CUIGuildInfo::IsOpen()
{
    return m_bOpened;
}

void CUIGuildInfo::Close()
{
    if (!m_bOpened)	return;

    m_bOpened = FALSE;

    m_bRequestUnionList = FALSE;
    m_nCurrentTab = 1;

    PlayBuffer(SOUND_INTERFACE01);
    CloseMyPopup();
}

bool CheckUseMasterSkill(CHARACTER* c, int Index)
{
    BYTE GuildStatus = c->GuildStatus;
    BYTE Class = gCharacterManager.GetBaseClass(c->Class);
    bool bUse = false;

    if (SkillAttribute[Index].RequireClass[Class] != 0)
    {
        if ((GuildStatus == G_MASTER && SkillAttribute[Index].RequireDutyClass[0]) ||
            (GuildStatus == G_SUB_MASTER && SkillAttribute[Index].RequireDutyClass[1]) ||
            (GuildStatus == G_BATTLE_MASTER && SkillAttribute[Index].RequireDutyClass[2]))
        {
            bUse = true;
        }
    }

    return bUse;
}

void UseBattleMasterSkill(void)
{
    if (!(Hero->EtcPart == PARTS_ATTACK_TEAM_MARK
        || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK2
        || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK3
        || Hero->EtcPart == PARTS_DEFENSE_TEAM_MARK))
    {
        return;
    }

    if (Hero->GuildStatus == G_PERSON)
    {
        return;
    }

    int MaxKillCount = SkillAttribute[Hero->GuildSkill].KillCount;

    if (Hero->GuildMasterKillCount >= MaxKillCount)
    {
        if (HIBYTE(GetAsyncKeyState(VK_SHIFT)))
        {
            if (Hero->BackupCurrentSkill == 255)
            {
                Hero->BackupCurrentSkill = Hero->CurrentSkill;
            }
            Hero->CurrentSkill = FindHotKey(Hero->GuildSkill);
        }
        else
        {
            if (Hero->BackupCurrentSkill != 255)
            {
                Hero->CurrentSkill = Hero->BackupCurrentSkill;
                Hero->BackupCurrentSkill = 255;
            }
        }
    }
}