// NewUISiegeWarCommander.cpp: implementation of the CNewUISiegeWarCommander class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUISiegeWarCommander.h"
#include "UIControls.h"
#include "zzzTexture.h"

#include "ZzzCharacter.h"

using namespace SEASON3B;

SEASON3B::CNewUISiegeWarCommander::CNewUISiegeWarCommander()
{
    memset(&m_BtnCommandGroupPos, 0, sizeof(POINT));
    memset(&m_BtnCommandPos, 0, sizeof(POINT));

    m_iCurSelectBtnGroup = -1;
    m_iCurSelectBtnCommand = -1;
    m_bMouseInMiniMap = false;

    m_vGuildMemberLocationBuffer.reserve(1600);
}

SEASON3B::CNewUISiegeWarCommander::~CNewUISiegeWarCommander()
{
}

bool SEASON3B::CNewUISiegeWarCommander::OnCreate(int x, int y)
{
    InitCmdGroupBtn();
    InitCmdBtn();
    return true;
}

void SEASON3B::CNewUISiegeWarCommander::OnRelease()
{
    ClearGuildMemberLocation();
}

bool SEASON3B::CNewUISiegeWarCommander::OnUpdate()
{
    return true;
}

bool SEASON3B::CNewUISiegeWarCommander::OnRender()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, m_fMiniMapAlpha);
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    RenderCharPosInMiniMap();
    RenderGuildMemberPosInMiniMap();
    DisableAlphaBlend();
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, m_fMiniMapAlpha);

    if (m_iCurSelectBtnGroup != -1 && m_iCurSelectBtnCommand != -1 && m_bMouseInMiniMap == true)
    {
        RenderCmdIconAtMouse();
    }

    RenderCmdIconInMiniMap();
    RenderCmdGroupBtn();

    if (m_iCurSelectBtnGroup != -1 && m_iCurSelectBtnCommand == -1)
    {
        RenderCmdBtn();
    }

    DisableAlphaBlend();

    return true;
}

bool SEASON3B::CNewUISiegeWarCommander::OnUpdateMouseEvent()
{
    if (OnBtnProcess())
        return false;

    if (CheckMouseIn(m_MiniMapPos.x, m_MiniMapPos.y, 128, 128))
    {
        if (SEASON3B::IsPress(VK_LBUTTON) && m_iCurSelectBtnCommand != -1)
        {
            GuildCommander SelectCmd;
            memset(&SelectCmd, 0, sizeof(GuildCommander));

            SelectCmd.byTeam = m_iCurSelectBtnGroup;
            SelectCmd.byCmd = m_iCurSelectBtnCommand;
            SelectCmd.byX = (MouseX + m_MiniMapScaleOffset.x - m_MiniMapPos.x) * m_iMiniMapScale;
            SelectCmd.byY = 256 - (MouseY + m_MiniMapScaleOffset.y - m_MiniMapPos.y) * m_iMiniMapScale;
            SelectCmd.byLifeTime = 100;

            SocketClient->ToGameServer()->SendCastleGuildCommand(SelectCmd.byTeam, SelectCmd.byX, SelectCmd.byY, SelectCmd.byCmd);

            m_iCurSelectBtnCommand = -1;

            return false;
        }
        m_bMouseInMiniMap = true;
    }
    else
    {
        m_bMouseInMiniMap = false;
    }

    return true;
}

bool SEASON3B::CNewUISiegeWarCommander::OnUpdateKeyEvent()
{
    return true;
}

bool SEASON3B::CNewUISiegeWarCommander::OnBtnProcess()
{
    for (int i = 0; i < MAX_COMMANDGROUP; i++)
    {
        if (m_BtnCommandGroup[i].UpdateMouseEvent())
        {
            if (m_iCurSelectBtnGroup != -1)
            {
                SetBtnState(m_iCurSelectBtnGroup, false);
            }

            if (m_iCurSelectBtnGroup == i)
            {
                m_iCurSelectBtnGroup = -1;
                SetBtnState(i, false);
            }
            else
            {
                m_iCurSelectBtnGroup = i;
                SetBtnState(i, true);
            }

            m_iCurSelectBtnCommand = -1;

            return true;
        }
    }

    if (m_iCurSelectBtnGroup != -1 && m_iCurSelectBtnCommand == -1)
    {
        for (int j = 0; j < MINIMAP_CMD_MAX; j++)
        {
            if (m_BtnCommand[j].UpdateMouseEvent())
            {
                m_iCurSelectBtnCommand = j;

                return true;
            }
        }
    }

    return false;
}

void SEASON3B::CNewUISiegeWarCommander::OnSetPos(int x, int y)
{
    m_BtnCommandGroupPos.x = x;
    m_BtnCommandGroupPos.y = y + 5;
}

void SEASON3B::CNewUISiegeWarCommander::InitCmdGroupBtn()
{
    int iVal = 0;
    wchar_t sztext[255] = { 0, };

    for (int i = 0; i < MAX_COMMANDGROUP; i++)
    {
        iVal = i * MINIMAP_BTN_GROUP_HEIGHT;
        m_BtnCommandGroup[i].ChangeButtonImgState(true, IMAGE_MINIMAP_BTN_GROUP, true);
        m_BtnCommandGroup[i].ChangeButtonInfo(m_BtnCommandGroupPos.x, m_BtnCommandGroupPos.y + iVal, MINIMAP_BTN_GROUP_WIDTH, MINIMAP_BTN_GROUP_HEIGHT);
        swprintf(sztext, L"%d", i + 1);
        m_BtnCommandGroup[i].ChangeText(sztext);
    }
}

void SEASON3B::CNewUISiegeWarCommander::InitCmdBtn()
{
    for (int i = 0; i < MINIMAP_CMD_MAX; i++)
    {
        m_BtnCommand[i].ChangeButtonImgState(true, IMAGE_MINIMAP_BTN_COMMAND, true);
    }
}

void SEASON3B::CNewUISiegeWarCommander::RenderCharPosInMiniMap()
{
    float fPosX, fPosY;

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c != NULL && c->Object.Live && c != Hero && (c->Object.Kind == KIND_PLAYER || c->Object.Kind == KIND_MONSTER || c->Object.Kind == KIND_NPC))
        {
            OBJECT* o = &c->Object;

            if (g_isCharacterBuff(o, static_cast<eBuffState>(m_dwBuffState)))
            {
                continue;
            }
            else
            {
                if (o->Kind == KIND_NPC || o->Kind == KIND_MONSTER && o->Type == MODEL_LIFE_STONE)
                {
                    glColor4f(1.f, 0.f, 1.f, m_fMiniMapAlpha);
                }
                else
                {
                    glColor4f(0.8f, 0.f, 0.f, m_fMiniMapAlpha);
                }
            }

            fPosX = ((c->PositionX)) / m_iMiniMapScale - m_MiniMapScaleOffset.x + m_MiniMapPos.x;
            fPosY = (256 - (c->PositionY)) / m_iMiniMapScale - m_MiniMapScaleOffset.y + m_MiniMapPos.y;
            RenderColor(fPosX, fPosY, 3, 3);
        }
    }
}

void SEASON3B::CNewUISiegeWarCommander::RenderGuildMemberPosInMiniMap()
{
    std::vector<VisibleUnitLocation>::iterator   UnitIterator;
    POINT Pos;
    memset(&Pos, 0, sizeof(POINT));

    for (UnitIterator = m_vGuildMemberLocationBuffer.begin(); UnitIterator != m_vGuildMemberLocationBuffer.end(); ++UnitIterator)
    {
        switch (UnitIterator->bIndex)
        {
        case 0:
            glColor4f(0.f, 1.f, 0.f, m_fMiniMapAlpha);
            break;

        case 1:
            glColor4f(0.f, 1.f, 1.f, m_fMiniMapAlpha);
            break;

        case 2:
            glColor4f(1.f, 1.f, 0.f, m_fMiniMapAlpha);
            break;
        }
        Pos.x = (UnitIterator->x) / m_iMiniMapScale - m_MiniMapScaleOffset.x + m_MiniMapPos.x;
        Pos.y = (256 - UnitIterator->y) / m_iMiniMapScale - m_MiniMapScaleOffset.y + m_MiniMapPos.y;

        if (Pos.x<m_MiniMapPos.x || Pos.x>m_MiniMapPos.x + 128 || Pos.y<m_MiniMapPos.y || Pos.y>m_MiniMapPos.y + 128)
        {
            continue;
        }

        RenderColor(Pos.x, Pos.y, 3, 3);
    }
}

void SEASON3B::CNewUISiegeWarCommander::RenderCmdIconAtMouse()
{
    int iWidth, iHeight;
    wchar_t szText[256] = { 0, };

    switch (m_iCurSelectBtnCommand)
    {
    case 0: iWidth = COMMAND_ATTACK_WIDTH; iHeight = COMMAND_ATTACK_HEIGHT; break;
    case 1: iWidth = COMMAND_DEFENCE_WIDTH; iHeight = COMMAND_DEFENCE_HEIGHT; break;
    case 2: iWidth = COMMAND_WAIT_WIDTH; iHeight = COMMAND_WAIT_HEIGHT; break;
    }

    swprintf(szText, L"%d", m_iCurSelectBtnGroup + 1);
    g_pRenderText->RenderText(MouseX - 13, MouseY - 6, szText);
    RenderImage(IMAGE_COMMAND_ATTACK + m_iCurSelectBtnCommand, MouseX - 8, MouseY - 8, iWidth, iHeight);
}

void SEASON3B::CNewUISiegeWarCommander::RenderCmdGroupBtn()
{
    for (int i = 0; i < MAX_COMMANDGROUP; i++)
    {
        m_BtnCommandGroup[i].SetFont(g_hFontBold);
        m_BtnCommandGroup[i].ChangeAlpha(m_fMiniMapAlpha);
        m_BtnCommandGroup[i].Render();
    }
}

void SEASON3B::CNewUISiegeWarCommander::RenderCmdBtn()
{
    if (m_iCurSelectBtnGroup < 5)
    {
        for (int i = 0; i < MINIMAP_CMD_MAX; i++)
        {
            m_BtnCommand[i].ChangeButtonInfo(m_BtnCommandGroupPos.x + MINIMAP_BTN_GROUP_WIDTH, m_BtnCommandGroupPos.y + (m_iCurSelectBtnGroup * MINIMAP_BTN_GROUP_HEIGHT) + (i * MINIMAP_BTN_GROUP_HEIGHT), MINIMAP_BTN_COMMAND_WIDTH, MINIMAP_BTN_COMMAND_HEIGHT);
            m_BtnCommand[i].ChangeAlpha(m_fMiniMapAlpha);
            m_BtnCommand[i].Render();
        }
        RenderImage(IMAGE_COMMAND_ATTACK, m_BtnCommandGroupPos.x + MINIMAP_BTN_GROUP_WIDTH + 8, m_BtnCommandGroupPos.y + (m_iCurSelectBtnGroup * MINIMAP_BTN_GROUP_HEIGHT) + 5, COMMAND_ATTACK_WIDTH, COMMAND_ATTACK_HEIGHT);
        RenderImage(IMAGE_COMMAND_DEFENCE, m_BtnCommandGroupPos.x + MINIMAP_BTN_GROUP_WIDTH + 8, m_BtnCommandGroupPos.y + (m_iCurSelectBtnGroup * MINIMAP_BTN_GROUP_HEIGHT) + MINIMAP_BTN_GROUP_HEIGHT + 3, COMMAND_DEFENCE_WIDTH, COMMAND_DEFENCE_HEIGHT);
        RenderImage(IMAGE_COMMAND_WAIT, m_BtnCommandGroupPos.x + MINIMAP_BTN_GROUP_WIDTH + 10, m_BtnCommandGroupPos.y + (m_iCurSelectBtnGroup * MINIMAP_BTN_GROUP_HEIGHT) + (2 * MINIMAP_BTN_GROUP_HEIGHT) + 5, COMMAND_WAIT_WIDTH, COMMAND_WAIT_HEIGHT);
    }
    else
    {
        for (int i = 0; i < MINIMAP_CMD_MAX; i++)
        {
            m_BtnCommand[i].ChangeButtonInfo(m_BtnCommandGroupPos.x + MINIMAP_BTN_GROUP_WIDTH, m_BtnCommandGroupPos.y + (4 * MINIMAP_BTN_GROUP_HEIGHT) + (i * MINIMAP_BTN_GROUP_HEIGHT), MINIMAP_BTN_COMMAND_WIDTH, MINIMAP_BTN_COMMAND_HEIGHT);
            m_BtnCommand[i].ChangeAlpha(m_fMiniMapAlpha);
            m_BtnCommand[i].Render();
        }
        RenderImage(IMAGE_COMMAND_ATTACK, m_BtnCommandGroupPos.x + MINIMAP_BTN_GROUP_WIDTH + 8, m_BtnCommandGroupPos.y + (4 * MINIMAP_BTN_GROUP_HEIGHT) + 5, COMMAND_ATTACK_WIDTH, COMMAND_ATTACK_HEIGHT);
        RenderImage(IMAGE_COMMAND_DEFENCE, m_BtnCommandGroupPos.x + MINIMAP_BTN_GROUP_WIDTH + 8, m_BtnCommandGroupPos.y + (4 * MINIMAP_BTN_GROUP_HEIGHT) + MINIMAP_BTN_GROUP_HEIGHT + 3, COMMAND_DEFENCE_WIDTH, COMMAND_DEFENCE_HEIGHT);
        RenderImage(IMAGE_COMMAND_WAIT, m_BtnCommandGroupPos.x + MINIMAP_BTN_GROUP_WIDTH + 10, m_BtnCommandGroupPos.y + (4 * MINIMAP_BTN_GROUP_HEIGHT) + (2 * MINIMAP_BTN_GROUP_HEIGHT) + 5, COMMAND_WAIT_WIDTH, COMMAND_WAIT_HEIGHT);
    }
}

void SEASON3B::CNewUISiegeWarCommander::SetBtnState(int iBtnType, bool bStateDown)
{
    if (bStateDown)
    {
        m_BtnCommandGroup[iBtnType].UnRegisterButtonState();
        m_BtnCommandGroup[iBtnType].RegisterButtonState(BUTTON_STATE_UP, IMAGE_MINIMAP_BTN_GROUP, 2);
        m_BtnCommandGroup[iBtnType].RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MINIMAP_BTN_GROUP, 2);
        m_BtnCommandGroup[iBtnType].RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MINIMAP_BTN_GROUP, 2);
        m_BtnCommandGroup[iBtnType].ChangeImgIndex(IMAGE_MINIMAP_BTN_GROUP, 2);
    }
    else
    {
        m_BtnCommandGroup[iBtnType].UnRegisterButtonState();
        m_BtnCommandGroup[iBtnType].RegisterButtonState(BUTTON_STATE_UP, IMAGE_MINIMAP_BTN_GROUP, 0);
        m_BtnCommandGroup[iBtnType].RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MINIMAP_BTN_GROUP, 1);
        m_BtnCommandGroup[iBtnType].RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MINIMAP_BTN_GROUP, 2);
        m_BtnCommandGroup[iBtnType].ChangeImgIndex(IMAGE_MINIMAP_BTN_GROUP, 0);
    }
}

void SEASON3B::CNewUISiegeWarCommander::ClearGuildMemberLocation(void)
{
    m_vGuildMemberLocationBuffer.clear();
}

void SEASON3B::CNewUISiegeWarCommander::SetGuildMemberLocation(BYTE type, int x, int y)
{
    VisibleUnitLocation vLocation = { type, (BYTE)x, (BYTE)y };

    m_vGuildMemberLocationBuffer.push_back(vLocation);
}

void SEASON3B::CNewUISiegeWarCommander::OnLoadImages()
{
    LoadBitmap(L"Interface\\newui_SW_Minimap_Bt_group.tga", IMAGE_MINIMAP_BTN_GROUP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_SW_Minimap_Bt_Command.tga", IMAGE_MINIMAP_BTN_COMMAND, GL_LINEAR);
}

void SEASON3B::CNewUISiegeWarCommander::OnUnloadImages()
{
    DeleteBitmap(IMAGE_MINIMAP_BTN_GROUP);
    DeleteBitmap(IMAGE_MINIMAP_BTN_COMMAND);
}