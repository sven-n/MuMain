
#include "stdafx.h"
#include "UI/Combat/SiegeWarCommander.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/Social/SocialWindowBase.h"
#include "Render/Textures/ZzzTexture.h"

#include "Engine/Object/ZzzCharacter.h"

using namespace SEASON3B;
using namespace mu::ui::window;

mu::ui::window::CSiegeWarCommander::CSiegeWarCommander()
{

    m_iCurSelectBtnGroup = -1;
    m_iCurSelectBtnCommand = -1;
    m_bMouseInMiniMap = false;

    m_vGuildMemberLocationBuffer.reserve(1600);
}

mu::ui::window::CSiegeWarCommander::~CSiegeWarCommander() {}

bool mu::ui::window::CSiegeWarCommander::OnCreate()
{
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
        // The pointer from the frame's top-left (#siege_hud), whose reference px the document
        // places in.
        Rml::Vector2f pointer;
        UI::RmlBridge::PointerIn(Element("siege_hud"), pointer);
        model.cursorLeft = pointer.x;
        model.cursorTop = pointer.y;
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
    // Where on the drawn map the pointer is, in its 128 reference px; a chosen command lands there.
    Rml::Vector2f local;
    m_bMouseInMiniMap = UI::RmlBridge::PointerIn(Element("map"), local) && local.x >= 0.f && local.y >= 0.f &&
                        local.x < 128.f && local.y < 128.f;
    if (m_bMouseInMiniMap && mu::ui::window::IsPress(VK_LBUTTON) && m_iCurSelectBtnCommand != -1)
    {
        GuildCommander SelectCmd;
        memset(&SelectCmd, 0, sizeof(GuildCommander));

        SelectCmd.byTeam = m_iCurSelectBtnGroup;
        SelectCmd.byCmd = m_iCurSelectBtnCommand;
        SelectCmd.byX = static_cast<BYTE>((static_cast<int>(local.x) + m_MiniMapScaleOffset.x) * m_iMiniMapScale);
        SelectCmd.byY = static_cast<BYTE>(256 - (static_cast<int>(local.y) + m_MiniMapScaleOffset.y) * m_iMiniMapScale);
        SelectCmd.byLifeTime = 100;

        SocketClient->ToGameServer()->SendCastleGuildCommand(SelectCmd.byTeam, SelectCmd.byX, SelectCmd.byY,
                                                             SelectCmd.byCmd);

        m_iCurSelectBtnCommand = -1;

        return false;
    }

    return true;
}

bool mu::ui::window::CSiegeWarCommander::OnUpdateKeyEvent()
{
    return true;
}

// The team and command buttons are the document's (OnTeamClick(), OnOrderClick()).
bool mu::ui::window::CSiegeWarCommander::OnBtnProcess()
{
    return false;
}

// A team's button chooses that team, or lets go of it; its command buttons then show.
void mu::ui::window::CSiegeWarCommander::OnTeamClick(int team)
{
    if (team < 0 || team >= MAX_COMMANDGROUP)
        return;
    m_iCurSelectBtnGroup = m_iCurSelectBtnGroup == team ? -1 : team;
    m_iCurSelectBtnCommand = -1;
}

// A command for the chosen team, placed with the next click on the map.
void mu::ui::window::CSiegeWarCommander::OnOrderClick(int order)
{
    if (m_iCurSelectBtnGroup != -1 && m_iCurSelectBtnCommand == -1 && order >= 0 && order < MINIMAP_CMD_MAX)
        m_iCurSelectBtnCommand = order;
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
        if (pos.x < kMiniMapPos.x || pos.x > kMiniMapPos.x + 128 || pos.y < kMiniMapPos.y ||
            pos.y > kMiniMapPos.y + 128)
            continue;
        model.dots.push_back({static_cast<float>(pos.x), static_cast<float>(pos.y)});
    }
}

void mu::ui::window::CSiegeWarCommander::FillTeamButtons(SiegeWarfareRmlModel& model)
{
    for (int i = 0; i < MAX_COMMANDGROUP; i++)
        model.teams.push_back({i == m_iCurSelectBtnGroup, std::to_string(i + 1)});
}

// The three command buttons, which the theme places beside the chosen team.
void mu::ui::window::CSiegeWarCommander::FillCommandButtons(SiegeWarfareRmlModel& model)
{
    model.ordersVisible = true;
    model.chosenTeam = m_iCurSelectBtnGroup;
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
