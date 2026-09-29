
#include "stdafx.h"
#include "UI/Combat/SiegeWarCommander.h"
#include "UI/Widgets/UIControls.h"
#include "Render/Textures/ZzzTexture.h"

#include "Engine/Object/ZzzCharacter.h"

using namespace SEASON3B;
using namespace mu::ui::window;

mu::ui::window::CSiegeWarCommander::CSiegeWarCommander()
{
    memset(&m_BtnCommandGroupPos, 0, sizeof(POINT));
    memset(&m_BtnCommandPos, 0, sizeof(POINT));

    m_iCurSelectBtnGroup = -1;
    m_iCurSelectBtnCommand = -1;
    m_bMouseInMiniMap = false;

    m_vGuildMemberLocationBuffer.reserve(1600);
}

mu::ui::window::CSiegeWarCommander::~CSiegeWarCommander() {}

bool mu::ui::window::CSiegeWarCommander::OnCreate(int x, int y)
{
    InitCmdGroupBtn();
    return true;
}

void mu::ui::window::CSiegeWarCommander::OnRelease()
{
    ClearGuildMemberLocation();
}

bool mu::ui::window::CSiegeWarCommander::OnUpdate()
{
    return true;
}

// The original's OnRender(): the dots of everyone in view and of the guild's members, the chosen
// command under the pointer, the teams' commands, the team buttons and, for a chosen team, its
// command buttons.
void mu::ui::window::CSiegeWarCommander::OnFillRmlModel(SiegeWarfareRmlModel& model)
{
    FillCharacterDots(model);
    FillGuildMemberDots(model);

    model.cursorVisible = m_iCurSelectBtnGroup != -1 && m_iCurSelectBtnCommand != -1 && m_bMouseInMiniMap;
    if (model.cursorVisible)
    {
        model.cursorLeft = static_cast<float>(MouseX);
        model.cursorTop = static_cast<float>(MouseY);
        model.cursorCommand = m_iCurSelectBtnCommand;
        model.cursorTeam = std::to_string(m_iCurSelectBtnGroup + 1);
    }

    FillCommands(model);
    FillTeamButtons(model);
    if (m_iCurSelectBtnGroup != -1 && m_iCurSelectBtnCommand == -1)
        FillCommandButtons(model);
}

bool mu::ui::window::CSiegeWarCommander::OnUpdateMouseEvent()
{
    if (OnBtnProcess())
        return false;

    if (CheckMouseIn(m_MiniMapPos.x, m_MiniMapPos.y, 128, 128))
    {
        if (mu::ui::window::IsPress(VK_LBUTTON) && m_iCurSelectBtnCommand != -1)
        {
            GuildCommander SelectCmd;
            memset(&SelectCmd, 0, sizeof(GuildCommander));

            SelectCmd.byTeam = m_iCurSelectBtnGroup;
            SelectCmd.byCmd = m_iCurSelectBtnCommand;
            SelectCmd.byX = (MouseX + m_MiniMapScaleOffset.x - m_MiniMapPos.x) * m_iMiniMapScale;
            SelectCmd.byY = 256 - (MouseY + m_MiniMapScaleOffset.y - m_MiniMapPos.y) * m_iMiniMapScale;
            SelectCmd.byLifeTime = 100;

            SocketClient->ToGameServer()->SendCastleGuildCommand(SelectCmd.byTeam, SelectCmd.byX, SelectCmd.byY,
                                                                 SelectCmd.byCmd);

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

bool mu::ui::window::CSiegeWarCommander::OnUpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CSiegeWarCommander::OnBtnProcess()
{
    for (int i = 0; i < MAX_COMMANDGROUP; i++)
    {
        if (m_BtnCommandGroup[i].UpdateMouseEvent())
        {
            // A chosen team's button shows its down row (FillTeamButtons()).
            m_iCurSelectBtnGroup = m_iCurSelectBtnGroup == i ? -1 : i;

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

void mu::ui::window::CSiegeWarCommander::OnSetPos(int x, int y)
{
    m_BtnCommandGroupPos.x = x;
    m_BtnCommandGroupPos.y = y + 5;
}

void mu::ui::window::CSiegeWarCommander::InitCmdGroupBtn()
{
    // The buttons only hit-test and keep their up / over / down state here; siege_warfare.rml
    // draws them.
    for (int i = 0; i < MAX_COMMANDGROUP; i++)
    {
        m_BtnCommandGroup[i].ChangeButtonInfo(m_BtnCommandGroupPos.x,
                                              m_BtnCommandGroupPos.y + i * MINIMAP_BTN_GROUP_HEIGHT,
                                              MINIMAP_BTN_GROUP_WIDTH, MINIMAP_BTN_GROUP_HEIGHT);
    }
}

// Everyone in view except those with the siege side's buff, as a dot (the original's
// RenderCharPosInMiniMap(); its per-kind colour branches were empty).
void mu::ui::window::CSiegeWarCommander::FillCharacterDots(SiegeWarfareRmlModel& model)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c != NULL && c->Object.Live && c != Hero &&
            (c->Object.Kind == KIND_PLAYER || c->Object.Kind == KIND_MONSTER || c->Object.Kind == KIND_NPC))
        {
            OBJECT* o = &c->Object;
            if (g_isCharacterBuff(o, static_cast<eBuffState>(m_dwBuffState)))
                continue;

            const POINT pos = MiniMapPoint(c->PositionX, c->PositionY);
            model.dots.push_back({static_cast<float>(pos.x), static_cast<float>(pos.y)});
        }
    }
}

// The guild members the server reports, inside the map only (RenderGuildMemberPosInMiniMap()).
void mu::ui::window::CSiegeWarCommander::FillGuildMemberDots(SiegeWarfareRmlModel& model)
{
    for (const VisibleUnitLocation& unit : m_vGuildMemberLocationBuffer)
    {
        const POINT pos = MiniMapPoint(unit.x, unit.y);
        if (pos.x < m_MiniMapPos.x || pos.x > m_MiniMapPos.x + 128 || pos.y < m_MiniMapPos.y ||
            pos.y > m_MiniMapPos.y + 128)
            continue;
        model.dots.push_back({static_cast<float>(pos.x), static_cast<float>(pos.y)});
    }
}

void mu::ui::window::CSiegeWarCommander::FillTeamButtons(SiegeWarfareRmlModel& model)
{
    for (int i = 0; i < MAX_COMMANDGROUP; i++)
    {
        CButton& button = m_BtnCommandGroup[i];
        model.teams.push_back({static_cast<float>(button.GetPos().x), static_cast<float>(button.GetPos().y),
                               i == m_iCurSelectBtnGroup ? 2 : ButtonFrame(button), std::to_string(i + 1)});
    }
}

// The three command buttons beside the chosen team (teams 6 and 7 share team 5's row, as in the
// original's RenderCmdBtn()); they hit-test where they were last shown.
void mu::ui::window::CSiegeWarCommander::FillCommandButtons(SiegeWarfareRmlModel& model)
{
    const int row = std::min(m_iCurSelectBtnGroup, 4);
    for (int i = 0; i < MINIMAP_CMD_MAX; i++)
    {
        CButton& button = m_BtnCommand[i];
        button.ChangeButtonInfo(m_BtnCommandGroupPos.x + MINIMAP_BTN_GROUP_WIDTH,
                                m_BtnCommandGroupPos.y + (row + i) * MINIMAP_BTN_GROUP_HEIGHT,
                                MINIMAP_BTN_COMMAND_WIDTH, MINIMAP_BTN_COMMAND_HEIGHT);
        model.orders.push_back(
            {static_cast<float>(button.GetPos().x), static_cast<float>(button.GetPos().y), ButtonFrame(button), {}});
    }
}

void mu::ui::window::CSiegeWarCommander::ClearGuildMemberLocation(void)
{
    m_vGuildMemberLocationBuffer.clear();
}

void mu::ui::window::CSiegeWarCommander::SetGuildMemberLocation(BYTE type, int x, int y)
{
    VisibleUnitLocation vLocation = {type, (BYTE)x, (BYTE)y};

    m_vGuildMemberLocationBuffer.push_back(vLocation);
}
