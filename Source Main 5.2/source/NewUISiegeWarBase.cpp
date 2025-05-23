// NewUISiegeWarBase.cpp: implementation of the CNewUISiegeWarBase class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

using namespace SEASON3B;

#include "NewUISiegeWarBase.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "UIGuildInfo.h"
#include "CharacterManager.h"
#include "SkillManager.h"

SEASON3B::CNewUISiegeWarBase::CNewUISiegeWarBase()
{
    m_iMiniMapScale = 1;
    m_fMiniMapAlpha = 1.f;

    m_bSecond = true;
    m_fTime = 0.f;
    m_iHour = 0;
    m_iMinute = 0;
    m_bRenderSkillUI = false;
    m_bRenderToolTip = false;

    memset(&m_MiniMapFramePos, 0, sizeof(POINT));
    memset(&m_MiniMapPos, 0, sizeof(POINT));
    memset(&m_TimeUIPos, 0, sizeof(POINT));
    memset(&m_SkillFramePos, 0, sizeof(POINT));
    memset(&m_BtnSkillScrollUpPos, 0, sizeof(POINT));
    memset(&m_BtnSkillScrollDnPos, 0, sizeof(POINT));
    memset(&m_SkillIconPos, 0, sizeof(POINT));
    memset(&m_UseSkillDestKillPos, 0, sizeof(POINT));
    memset(&m_CurKillCountPos, 0, sizeof(POINT));
    memset(&m_BtnAlphaPos, 0, sizeof(POINT));
    memset(&m_SkillTooltipPos, 0, sizeof(POINT));

    memset(&m_HeroPosInWorld, 0, sizeof(POINT));
    memset(&m_HeroPosInMiniMap, 0, sizeof(POINT));
    memset(&m_MiniMapScaleOffset, 0, sizeof(POINT));
}

SEASON3B::CNewUISiegeWarBase::~CNewUISiegeWarBase()
{
}

bool SEASON3B::CNewUISiegeWarBase::Create(int x, int y)
{
    SetPos(x, y);

    if (!OnCreate(x, y))
        return false;

    wchar_t szText[256] = { NULL, };
    swprintf(szText, L"%d", (int)(m_fMiniMapAlpha * 100.5f));
    m_BtnAlpha.ChangeText(szText);
    m_BtnAlpha.ChangeButtonImgState(true, IMAGE_BTN_ALPHA, true);
    m_BtnAlpha.ChangeButtonInfo(m_BtnAlphaPos.x, m_BtnAlphaPos.y, BTN_ALPHA_WIDTH, BTN_ALPHA_HEIGHT);

    if (battleCastle::IsBattleCastleStart() == true)
    {
        // 		if( InitBattleSkill() == true )
        // 		{
        // 			m_bRenderSkillUI = true;
        // 		}
        // 		else
        // 		{
        // 			m_bRenderSkillUI = false;
        // 		}

        InitBattleSkill();
    }

    return true;
}

void SEASON3B::CNewUISiegeWarBase::Release()
{
    ReleaseBattleSkill();

    OnRelease();
}

bool SEASON3B::CNewUISiegeWarBase::Update()
{
    UpdateBuffState();
    UpdateHeroPos();

    OnUpdate();

    return true;
}

bool SEASON3B::CNewUISiegeWarBase::Render()
{
    wchar_t szText[256] = { 0, };

    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, m_fMiniMapAlpha);
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    RenderBitmap(IMAGE_MINIMAP, (float)(m_MiniMapPos.x), (float)(m_MiniMapPos.y), 128.f, 128.f, m_fMiniMapTexU, m_fMiniMapTexV, 0.5f * m_iMiniMapScale, 0.5f * m_iMiniMapScale);

    RenderImage(IMAGE_MINIMAP_FRAME, m_MiniMapFramePos.x, m_MiniMapFramePos.y, MINIMAP_FRAME_WIDTH, MINIMAP_FRAME_HEIGHT);
    RenderImage(IMAGE_TIME_FRAME, m_TimeUIPos.x, m_TimeUIPos.y, TIME_FRAME_WIDTH, TIME_FRAME_HEIGHT);

    glColor4f(1.f, 1.f, 1.f, m_fMiniMapAlpha);
    if (battleCastle::IsBattleCastleStart())
    {
        g_pRenderText->SetFont(g_hFontBig);

        if ((WorldTime - m_fTime) > 500)
        {
            m_fTime = WorldTime;
            m_bSecond = true;
        }

        if (m_bSecond)
        {
            if (m_iMinute < 10)
            {
                swprintf(szText, L"%d:0%d", m_iHour, m_iMinute);
            }
            else
            {
                swprintf(szText, L"%d:%d", m_iHour, m_iMinute);
            }
        }
        else
        {
            if (m_iMinute < 10)
            {
                swprintf(szText, L"%d 0%d", m_iHour, m_iMinute);
            }
            else
            {
                swprintf(szText, L"%d %d", m_iHour, m_iMinute);
            }
        }
        g_pRenderText->RenderText(m_TimeUIPos.x, m_TimeUIPos.y + 10, szText, 134, 0, RT3_SORT_CENTER);
    }

    OnRender();

    glColor4f(1.f, 1.f, 0.f, m_fMiniMapAlpha);
    RenderColor((float)(m_HeroPosInMiniMap.x), (float)(m_HeroPosInMiniMap.y), 3, 3);

    DisableAlphaBlend();

    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, m_fMiniMapAlpha);

    if (m_bRenderSkillUI == true)
    {
        RenderImage(IMAGE_BATTLESKILL_FRAME, m_SkillFramePos.x, m_SkillFramePos.y,
            BATTLESKILL_FRAME_WIDTH, BATTLESKILL_FRAME_HEIGHT);

        RenderSkillIcon();

        m_BtnSkillScroll[0].ChangeAlpha(m_fMiniMapAlpha);
        m_BtnSkillScroll[1].ChangeAlpha(m_fMiniMapAlpha);
        m_BtnSkillScroll[0].Render();
        m_BtnSkillScroll[1].Render();
    }

    m_BtnAlpha.SetFont(g_hFontBold);
    m_BtnAlpha.ChangeAlpha(m_fMiniMapAlpha);
    m_BtnAlpha.Render();

    DisableAlphaBlend();

    return true;
}

bool SEASON3B::CNewUISiegeWarBase::InitBattleSkill()
{
    ReleaseBattleSkill();

    if (!(Hero->EtcPart == PARTS_ATTACK_TEAM_MARK
        || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK2
        || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK3
        || Hero->EtcPart == PARTS_DEFENSE_TEAM_MARK)
        || Hero->GuildStatus == G_PERSON)
    {
        return false;
    }

    m_BtnSkillScroll[0].ChangeButtonImgState(true, IMAGE_SKILL_BTN_SCROLL_UP, true);
    m_BtnSkillScroll[0].ChangeButtonInfo(m_BtnSkillScrollUpPos.x, m_BtnSkillScrollUpPos.y, SKILL_BTN_SCROLL_WIDTH, SKILL_BTN_SCROLL_HEIGHT);
    m_BtnSkillScroll[1].ChangeButtonImgState(true, IMAGE_SKILL_BTN_SCROLL_DN, true);
    m_BtnSkillScroll[1].ChangeButtonInfo(m_BtnSkillScrollDnPos.x, m_BtnSkillScrollDnPos.y, SKILL_BTN_SCROLL_WIDTH, SKILL_BTN_SCROLL_HEIGHT);

    switch (Hero->GuildStatus)
    {
    case G_MASTER:
    {
        m_listBattleSkill.push_back(AT_SKILL_INVISIBLE);
        m_listBattleSkill.push_back(AT_SKILL_REMOVAL_INVISIBLE);
        if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
        {
            m_listBattleSkill.push_back(AT_SKILL_REMOVAL_BUFF);
        }
        m_bRenderSkillUI = true;
    }break;
    case G_SUB_MASTER:
    {
        m_listBattleSkill.push_back(AT_SKILL_INVISIBLE);
        m_listBattleSkill.push_back(AT_SKILL_REMOVAL_INVISIBLE);
        m_bRenderSkillUI = true;
    }break;
    case G_BATTLE_MASTER:
    {
        m_listBattleSkill.push_back(AT_SKILL_STUN);
        m_listBattleSkill.push_back(AT_SKILL_REMOVAL_STUN);
        m_listBattleSkill.push_back(AT_SKILL_MANA);
        m_bRenderSkillUI = true;
    }break;
    }

    m_iterCurBattleSkill = m_listBattleSkill.begin();

    Hero->GuildSkill = (*m_iterCurBattleSkill);

    return true;
}

void SEASON3B::CNewUISiegeWarBase::ReleaseBattleSkill()
{
    m_listBattleSkill.clear();

    m_bRenderSkillUI = false;
}

void SEASON3B::CNewUISiegeWarBase::SetSkillScrollUp()
{
    if (m_listBattleSkill.begin() == m_iterCurBattleSkill)
        return;

    m_iterCurBattleSkill--;

    Hero->GuildSkill = (*m_iterCurBattleSkill);
}

void SEASON3B::CNewUISiegeWarBase::SetSkillScrollDn()
{
    if (m_listBattleSkill.end() == ++m_iterCurBattleSkill)
    {
        m_iterCurBattleSkill--;
        return;
    }

    Hero->GuildSkill = (*m_iterCurBattleSkill);
}

bool SEASON3B::CNewUISiegeWarBase::UpdateMouseEvent()
{
    if (!OnUpdateMouseEvent())
        return false;

    if (BtnProcess())
        return false;

    if (CheckMouseIn(m_MiniMapFramePos.x, m_MiniMapFramePos.y, MINIMAP_FRAME_WIDTH, MINIMAP_FRAME_HEIGHT)
        || CheckMouseIn(m_TimeUIPos.x, m_TimeUIPos.y, TIME_FRAME_WIDTH, TIME_FRAME_HEIGHT))
        return false;

    if (m_bRenderSkillUI == true)
    {
        if (CheckMouseIn(m_SkillIconPos.x, m_SkillIconPos.y, SKILL_ICON_WIDTH, SKILL_ICON_HEIGHT))
        {
            m_bRenderToolTip = true;
            return false;
        }
        else
        {
            m_bRenderToolTip = false;
        }

        if (CheckMouseIn(m_SkillFramePos.x, m_SkillFramePos.y, BATTLESKILL_FRAME_WIDTH, BATTLESKILL_FRAME_HEIGHT))
            return false;
    }

    return true;
}

bool SEASON3B::CNewUISiegeWarBase::UpdateKeyEvent()
{
    if (!OnUpdateKeyEvent())
        return false;

    return true;
}

bool SEASON3B::CNewUISiegeWarBase::BtnProcess()
{
    POINT ptScaleBtn = { m_MiniMapFramePos.x + 134, m_MiniMapFramePos.y + 7 };

    if (m_BtnAlpha.UpdateMouseEvent())
    {
        if (m_fMiniMapAlpha <= 0.5f)
        {
            m_fMiniMapAlpha = 1.f;
        }
        else
        {
            m_fMiniMapAlpha = m_fMiniMapAlpha - 0.1f;
        }

        wchar_t szText[256] = { NULL, };
        swprintf(szText, L"%d", (int)(m_fMiniMapAlpha * 100.5f));
        m_BtnAlpha.ChangeText(szText);
        m_BtnAlpha.ChangeAlpha(m_fMiniMapAlpha);

        return true;
    }

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptScaleBtn.x, ptScaleBtn.y, 13, 12))
    {
        if (m_iMiniMapScale == 1)
            m_iMiniMapScale = 2;
        else
            m_iMiniMapScale = 1;

        return true;
    }

    if (m_bRenderSkillUI == true)
    {
        if (m_BtnSkillScroll[0].UpdateMouseEvent())
        {
            SetSkillScrollUp();
            return true;
        }
        if (m_BtnSkillScroll[1].UpdateMouseEvent())
        {
            SetSkillScrollDn();
            return true;
        }

        if (MouseWheel > 0)
        {
            SetSkillScrollUp();
            MouseWheel = 0;
            return true;
        }
        if (MouseWheel < 0)
        {
            SetSkillScrollDn();
            MouseWheel = 0;
            return true;
        }
    }

    return false;
}

void SEASON3B::CNewUISiegeWarBase::UpdateBuffState()
{
    DWORD m_dwBuffState = -1;

    OBJECT* o = &Hero->Object;

    if (g_isCharacterBuff(o, eBuff_CastleRegimentAttack1))
    {
        m_dwBuffState = eBuff_CastleRegimentAttack1;
    }
    else if (g_isCharacterBuff(o, eBuff_CastleRegimentAttack2))
    {
        m_dwBuffState = eBuff_CastleRegimentAttack2;
    }
    else if (g_isCharacterBuff(o, eBuff_CastleRegimentAttack3))
    {
        m_dwBuffState = eBuff_CastleRegimentAttack3;
    }
    else if (g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
    {
        m_dwBuffState = eBuff_CastleRegimentDefense;
    }
}

void SEASON3B::CNewUISiegeWarBase::UpdateHeroPos()
{
    m_HeroPosInWorld.x = (Hero->PositionX) / m_iMiniMapScale;
    m_HeroPosInWorld.y = (256 - (Hero->PositionY)) / m_iMiniMapScale;

    m_MiniMapScaleOffset.x = max((m_HeroPosInWorld.x - (64 * m_iMiniMapScale)), 0);
    m_MiniMapScaleOffset.y = min(max((m_HeroPosInWorld.y - (64 * m_iMiniMapScale)), 0), 128);

    m_HeroPosInMiniMap.x = m_HeroPosInWorld.x - m_MiniMapScaleOffset.x + m_MiniMapPos.x;
    m_HeroPosInMiniMap.y = m_HeroPosInWorld.y - m_MiniMapScaleOffset.y + m_MiniMapPos.y;

    m_fMiniMapTexU = (float)(m_MiniMapScaleOffset.x) / (256.f / (float)m_iMiniMapScale);
    m_fMiniMapTexV = (float)(m_MiniMapScaleOffset.y) / (256.f / (float)m_iMiniMapScale);
}

void SEASON3B::CNewUISiegeWarBase::RenderCmdIconInMiniMap()
{
    int iWidth, iHeight;
    wchar_t szText[256] = { 0, };
    POINT Pos;
    memset(&Pos, 0, sizeof(POINT));

    for (int i = 0; i < MAX_COMMANDGROUP; i++)
    {
        int iBWidth;
        switch (m_CmdBuffer[i].byCmd)
        {
        case 0: iWidth = COMMAND_ATTACK_WIDTH; iHeight = COMMAND_ATTACK_HEIGHT; iBWidth = 16; break;
        case 1: iWidth = COMMAND_DEFENCE_WIDTH; iHeight = COMMAND_DEFENCE_HEIGHT; iBWidth = 32; break;
        case 2: iWidth = COMMAND_WAIT_WIDTH; iHeight = COMMAND_WAIT_HEIGHT; iBWidth = 16; break;
        }

        if (m_CmdBuffer[i].byCmd != 3 && m_CmdBuffer[i].byTeam >= 0 && m_CmdBuffer[i].byTeam <= 6)
        {
            Pos.x = (m_CmdBuffer[i].byX) / m_iMiniMapScale - m_MiniMapScaleOffset.x + m_MiniMapPos.x;
            Pos.y = (256 - m_CmdBuffer[i].byY) / m_iMiniMapScale - m_MiniMapScaleOffset.y + m_MiniMapPos.y;

            if (Pos.x<m_MiniMapPos.x || Pos.x>m_MiniMapPos.x + 128 || Pos.y<m_MiniMapPos.y || Pos.y>m_MiniMapPos.y + 128)
                continue;

            if (m_CmdBuffer[i].byLifeTime > 0)
            {
                glColor4f(1.f, 1.f + sinf(m_CmdBuffer[i].byLifeTime * 0.2f), 1.f + sinf(m_CmdBuffer[i].byLifeTime * 0.2f), m_fMiniMapAlpha);
                m_CmdBuffer[i].byLifeTime--;
            }
            else
            {
                glColor4f(1.f, 1.f, 1.f, m_fMiniMapAlpha);
            }
            swprintf(szText, L"%d", m_CmdBuffer[i].byTeam + 1);
            g_pRenderText->RenderText(Pos.x - 12, Pos.y - 5, szText);
            RenderBitmap(IMAGE_COMMAND_ATTACK + m_CmdBuffer[i].byCmd, Pos.x - 7, Pos.y - 7, 11.f, 11.f, 0.f, 0.f, ((float)iWidth - 1.f) / (float)iBWidth, ((float)iHeight - 1.f) / 16.f);
        }
    }
}

void SEASON3B::CNewUISiegeWarBase::RenderSkillIcon()
{
    int iUseSkillDestKill;
    int iSelectSkill;
    int iCurKillCount;

    wchar_t szText[256] = { NULL, };

    iUseSkillDestKill = SkillAttribute[Hero->GuildSkill].KillCount;

    iSelectSkill = (*m_iterCurBattleSkill);
    iCurKillCount = Hero->GuildMasterKillCount;

    if (Hero->GuildMasterKillCount < iUseSkillDestKill)
    {
        glColor4f(1.f, 0.5f, 0.5f, m_fMiniMapAlpha);
    }

    int src_x, src_y;
    src_x = ((iSelectSkill - 57) % 8) * 20.f;
    src_y = ((iSelectSkill - 57) / 8) * 28.f;

    RenderImage(IMAGE_SKILL_ICON, m_SkillIconPos.x + 1, m_SkillIconPos.y, (float)SKILL_ICON_WIDTH, (float)SKILL_ICON_HEIGHT, src_x, src_y);

    glColor4f(1.f, 1.f, 1.f, m_fMiniMapAlpha);

    g_pRenderText->SetFont(g_hFontBig);
    swprintf(szText, L"%d", iUseSkillDestKill);
    g_pRenderText->RenderText(m_UseSkillDestKillPos.x, m_UseSkillDestKillPos.y, szText);
    swprintf(szText, L"%d", iCurKillCount);
    g_pRenderText->RenderText(m_CurKillCountPos.x, m_CurKillCountPos.y, szText);

    if (m_bRenderToolTip == true)
    {
        RenderSkillInfo(m_SkillTooltipPos.x, m_SkillTooltipPos.y, FindHotKey(Hero->GuildSkill), Hero->GuildSkill, STRP_BOTTOMCENTER);
    }
}

//---------------------------------------------------------------------------------------------
// SetPos
void SEASON3B::CNewUISiegeWarBase::SetPos(int x, int y)
{
    m_MiniMapFramePos.x = x;
    m_MiniMapFramePos.y = y;
    m_MiniMapPos.x = m_MiniMapFramePos.x + 25;
    m_MiniMapPos.y = m_MiniMapFramePos.y + 28;
    m_TimeUIPos.x = m_MiniMapFramePos.x + 20;
    m_TimeUIPos.y = m_MiniMapFramePos.y + MINIMAP_FRAME_HEIGHT - 4;
    m_SkillFramePos.x = x + 26;
    m_SkillFramePos.y = y - BATTLESKILL_FRAME_HEIGHT;
    m_BtnSkillScrollUpPos.x = m_SkillFramePos.x + 48;
    m_BtnSkillScrollUpPos.y = m_SkillFramePos.y + 21;
    m_BtnSkillScrollDnPos.x = m_BtnSkillScrollUpPos.x;
    m_BtnSkillScrollDnPos.y = m_BtnSkillScrollUpPos.y + 15;
    m_SkillIconPos.x = m_SkillFramePos.x + 25;
    m_SkillIconPos.y = m_SkillFramePos.y + 21;
    m_UseSkillDestKillPos.x = m_SkillFramePos.x + 78;
    m_UseSkillDestKillPos.y = m_SkillFramePos.y + 28;
    m_CurKillCountPos.x = m_SkillFramePos.x + 97;
    m_CurKillCountPos.y = m_UseSkillDestKillPos.y;
    m_BtnAlphaPos.x = m_MiniMapFramePos.x + 58;
    m_BtnAlphaPos.y = m_MiniMapFramePos.y + 4;

    m_SkillTooltipPos.y = m_SkillFramePos.y + 16;
    m_SkillTooltipPos.x = m_SkillFramePos.x + 30;

    OnSetPos(x, y);
}

void  SEASON3B::CNewUISiegeWarBase::SetTime(int iHour, int iMinute)
{
    m_iHour = iHour;
    m_iMinute = iMinute;
}

void SEASON3B::CNewUISiegeWarBase::SetMapInfo(GuildCommander& data)
{
    m_CmdBuffer[data.byTeam].byCmd = data.byCmd;
    m_CmdBuffer[data.byTeam].byTeam = data.byTeam;
    m_CmdBuffer[data.byTeam].byX = data.byX;
    m_CmdBuffer[data.byTeam].byY = data.byY;
    m_CmdBuffer[data.byTeam].byLifeTime = 100;
}

void SEASON3B::CNewUISiegeWarBase::SetRenderSkillUI(bool bRenderSkillUI)
{
    m_bRenderSkillUI = bRenderSkillUI;
}

void SEASON3B::CNewUISiegeWarBase::LoadImages()
{
    LoadBitmap(L"World31\\Map1.jpg", IMAGE_MINIMAP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_SW_Minimap_Frame.tga", IMAGE_MINIMAP_FRAME, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_SW_Time_Frame.tga", IMAGE_TIME_FRAME, GL_LINEAR);
    LoadBitmap(L"Interface\\i_attack.tga", IMAGE_COMMAND_ATTACK);
    LoadBitmap(L"Interface\\i_defense.tga", IMAGE_COMMAND_DEFENCE);
    LoadBitmap(L"Interface\\i_wait.tga", IMAGE_COMMAND_WAIT);
    LoadBitmap(L"Interface\\newui_SW_BattleSkill_Frame.tga", IMAGE_BATTLESKILL_FRAME, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_skill_scroll_up.jpg", IMAGE_SKILL_BTN_SCROLL_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_skill_scroll_dn.jpg", IMAGE_SKILL_BTN_SCROLL_DN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skill2.jpg", IMAGE_SKILL_ICON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_SW_MiniMap_Bt_clearness.jpg", IMAGE_BTN_ALPHA, GL_LINEAR);

    OnLoadImages();
}

void SEASON3B::CNewUISiegeWarBase::UnLoadImages()
{
    DeleteBitmap(IMAGE_MINIMAP);
    DeleteBitmap(IMAGE_MINIMAP_FRAME);
    DeleteBitmap(IMAGE_TIME_FRAME);
    DeleteBitmap(IMAGE_COMMAND_ATTACK);
    DeleteBitmap(IMAGE_COMMAND_DEFENCE);
    DeleteBitmap(IMAGE_COMMAND_WAIT);
    DeleteBitmap(IMAGE_BATTLESKILL_FRAME);
    DeleteBitmap(IMAGE_SKILL_BTN_SCROLL_UP);
    DeleteBitmap(IMAGE_SKILL_BTN_SCROLL_DN);
    DeleteBitmap(IMAGE_SKILL_ICON);
    DeleteBitmap(IMAGE_BTN_ALPHA);

    OnUnloadImages();
}