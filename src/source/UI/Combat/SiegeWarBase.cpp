
#include "stdafx.h"

using namespace SEASON3B;
using namespace mu::ui::window;

#include "UI/Combat/SiegeWarBase.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Guild/GuildTypes.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Skills/SkillManager.h"
#include "UI/HUD/Skills/SkillTooltip.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlPointer.h"
#include <RmlUi/Core/ElementDocument.h>
#include "Core/Utilities/StringUtils.h"

namespace
{
BYTE ToColorByte(float value)
{
    return static_cast<BYTE>(std::clamp(value, 0.f, 1.f) * 255.f);
}

} // namespace

mu::ui::window::CSiegeWarBase::CSiegeWarBase()
{
    m_iMiniMapScale = 1;
    m_fMiniMapAlpha = 1.f;

    m_bSecond = true;
    m_fTime = 0.f;
    m_iHour = 0;
    m_iMinute = 0;
    m_bRenderSkillUI = false;
    m_bRenderToolTip = false;

    memset(&m_HeroPosInWorld, 0, sizeof(POINT));
    memset(&m_HeroPosInMiniMap, 0, sizeof(POINT));
    memset(&m_MiniMapScaleOffset, 0, sizeof(POINT));
}

mu::ui::window::CSiegeWarBase::~CSiegeWarBase()
{
}

bool mu::ui::window::CSiegeWarBase::Create()
{
    if (!OnCreate())
        return false;

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

void mu::ui::window::CSiegeWarBase::Release()
{
    ReleaseBattleSkill();

    OnRelease();
}

bool mu::ui::window::CSiegeWarBase::Update()
{
    UpdateBuffState();
    UpdateHeroPos();

    OnUpdate();

    return true;
}

void mu::ui::window::CSiegeWarBase::FillRmlModel(SiegeWarfareRmlModel& model)
{
    model.alpha = m_fMiniMapAlpha;

    // RenderBitmap(IMAGE_MINIMAP, ..., 128 x 128, u = offset / (256 / scale), width 0.5 * scale):
    // the 256 x 256 map's texels from offset * scale, 128 * scale of them.
    model.mapRect = std::to_string(m_MiniMapScaleOffset.x * m_iMiniMapScale) + " " +
                    std::to_string(m_MiniMapScaleOffset.y * m_iMiniMapScale) + " " +
                    std::to_string(128 * m_iMiniMapScale) + " " + std::to_string(128 * m_iMiniMapScale);

    model.alphaLabel = std::to_string(static_cast<int>(m_fMiniMapAlpha * 100.5f));

    // The remaining time, only while a siege runs; the colon never blinks (m_bSecond stays true
    // in the original too).
    model.timeVisible = battleCastle::IsBattleCastleStart();
    if (model.timeVisible)
    {
        if ((WorldTime - m_fTime) > 500)
        {
            m_fTime = WorldTime;
            m_bSecond = true;
        }
        wchar_t szText[64] = {};
        mu_swprintf(szText, m_iMinute < 10 ? (m_bSecond ? L"%d:0%d" : L"%d 0%d") : (m_bSecond ? L"%d:%d" : L"%d %d"),
                    m_iHour, m_iMinute);
        model.timeText = StringUtils::WideToNarrow(szText);
    }
    else
    {
        model.timeText.clear();
    }

    model.dots.clear();
    model.commands.clear();
    model.teams.clear();
    model.ordersVisible = false;
    model.cursorVisible = false;
    OnFillRmlModel(model);

    model.heroLeft = static_cast<float>(m_HeroPosInMiniMap.x);
    model.heroTop = static_cast<float>(m_HeroPosInMiniMap.y);

    FillSkill(model);
}

bool mu::ui::window::CSiegeWarBase::InitBattleSkill()
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

void mu::ui::window::CSiegeWarBase::ReleaseBattleSkill()
{
    m_listBattleSkill.clear();

    m_bRenderSkillUI = false;
}

void mu::ui::window::CSiegeWarBase::SetSkillScrollUp()
{
    if (m_listBattleSkill.begin() == m_iterCurBattleSkill)
        return;

    m_iterCurBattleSkill--;

    Hero->GuildSkill = (*m_iterCurBattleSkill);
}

void mu::ui::window::CSiegeWarBase::SetSkillScrollDn()
{
    if (m_listBattleSkill.end() == ++m_iterCurBattleSkill)
    {
        m_iterCurBattleSkill--;
        return;
    }

    Hero->GuildSkill = (*m_iterCurBattleSkill);
}

bool mu::ui::window::CSiegeWarBase::UpdateMouseEvent()
{
    if (!OnUpdateMouseEvent())
        return false;

    if (BtnProcess())
        return false;

    // The frames hold the pointer by where they are drawn; the skill's icon shows its tooltip.
    if (UI::RmlBridge::IsPointerWithin(Element("frame_art")) || UI::RmlBridge::IsPointerWithin(Element("time_frame")))
        return false;

    if (m_bRenderSkillUI == true)
    {
        m_bRenderToolTip = UI::RmlBridge::IsPointerWithin(Element("skill_icon"));
        if (m_bRenderToolTip)
            return false;

        if (UI::RmlBridge::IsPointerWithin(Element("skill_frame")))
            return false;
    }

    return true;
}

bool mu::ui::window::CSiegeWarBase::UpdateKeyEvent()
{
    if (!OnUpdateKeyEvent())
        return false;

    return true;
}

// The wheel scrolls the battle skill wherever the pointer is; the buttons are the document's.
bool mu::ui::window::CSiegeWarBase::BtnProcess()
{
    if (m_bRenderSkillUI == true)
    {
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

void mu::ui::window::CSiegeWarBase::ToggleAlpha()
{
    if (m_fMiniMapAlpha <= 0.5f)
        m_fMiniMapAlpha = 1.f;
    else
        m_fMiniMapAlpha = m_fMiniMapAlpha - 0.1f;
}

void mu::ui::window::CSiegeWarBase::ToggleMiniMapScale()
{
    m_iMiniMapScale = m_iMiniMapScale == 1 ? 2 : 1;
}

void mu::ui::window::CSiegeWarBase::ScrollSkillUp()
{
    if (m_bRenderSkillUI)
        SetSkillScrollUp();
}

void mu::ui::window::CSiegeWarBase::ScrollSkillDown()
{
    if (m_bRenderSkillUI)
        SetSkillScrollDn();
}

Rml::Element* mu::ui::window::CSiegeWarBase::Element(const char* id) const
{
    return m_Document != nullptr ? m_Document->GetElementById(id) : nullptr;
}

void mu::ui::window::CSiegeWarBase::UpdateBuffState()
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

void mu::ui::window::CSiegeWarBase::UpdateHeroPos()
{
    m_HeroPosInWorld.x = (Hero->PositionX) / m_iMiniMapScale;
    m_HeroPosInWorld.y = (256 - (Hero->PositionY)) / m_iMiniMapScale;

    m_MiniMapScaleOffset.x = std::max<int>((m_HeroPosInWorld.x - (64 * m_iMiniMapScale)), 0);
    m_MiniMapScaleOffset.y = std::min<int>(std::max<int>((m_HeroPosInWorld.y - (64 * m_iMiniMapScale)), 0), 128);

    m_HeroPosInMiniMap.x = m_HeroPosInWorld.x - m_MiniMapScaleOffset.x + kMiniMapPos.x;
    m_HeroPosInMiniMap.y = m_HeroPosInWorld.y - m_MiniMapScaleOffset.y + kMiniMapPos.y;

    m_fMiniMapTexU = (float)(m_MiniMapScaleOffset.x) / (256.f / (float)m_iMiniMapScale);
    m_fMiniMapTexV = (float)(m_MiniMapScaleOffset.y) / (256.f / (float)m_iMiniMapScale);
}

POINT mu::ui::window::CSiegeWarBase::MiniMapPoint(int x, int y) const
{
    return {x / m_iMiniMapScale - m_MiniMapScaleOffset.x + kMiniMapPos.x,
            (256 - y) / m_iMiniMapScale - m_MiniMapScaleOffset.y + kMiniMapPos.y};
}

void mu::ui::window::CSiegeWarBase::FillCommands(SiegeWarfareRmlModel& model)
{
    for (int i = 0; i < MAX_COMMANDGROUP; i++)
    {
        GuildCommander& command = m_CmdBuffer[i];
        if (command.byCmd > 2 || command.byTeam > 6)
            continue;

        const POINT pos = MiniMapPoint(command.byX, command.byY);
        if (pos.x < kMiniMapPos.x || pos.x > kMiniMapPos.x + 128 || pos.y < kMiniMapPos.y ||
            pos.y > kMiniMapPos.y + 128)
            continue;

        // A new command pulses: green and blue follow 1 + sin(lifetime * 0.2), clamped.
        BYTE pulse = 255;
        if (command.byLifeTime > 0)
        {
            pulse = ToColorByte(1.f + sinf(command.byLifeTime * 0.2f));
            command.byLifeTime--;
        }

        SiegeWarCommandEntry entry;
        entry.left = static_cast<float>(pos.x);
        entry.top = static_cast<float>(pos.y);
        entry.command = command.byCmd;
        entry.team = std::to_string(command.byTeam + 1);
        entry.color = "rgb(255, " + std::to_string(pulse) + ", " + std::to_string(pulse) + ")";
        model.commands.push_back(std::move(entry));
    }
}

void mu::ui::window::CSiegeWarBase::FillSkill(SiegeWarfareRmlModel& model)
{
    model.skillVisible = m_bRenderSkillUI && m_iterCurBattleSkill != m_listBattleSkill.end();
    if (!model.skillVisible)
        return;

    const int killsNeeded = SkillAttribute[Hero->GuildSkill].KillCount;
    const int selectedSkill = *m_iterCurBattleSkill;
    model.skillRect = std::to_string(((selectedSkill - 57) % 8) * 20) + " " +
                      std::to_string(((selectedSkill - 57) / 8) * 28) + " 20 28";
    model.skillAffordable = Hero->GuildMasterKillCount >= killsNeeded;
    model.killsNeeded = std::to_string(killsNeeded);
    model.kills = std::to_string(Hero->GuildMasterKillCount);

    if (m_bRenderToolTip == true)
    {
        UI::Skills::Tooltip::Model tooltipModel;
        if (UI::Skills::Tooltip::BuildModelForSlot(FindHotKey(Hero->GuildSkill), tooltipModel))
        {
            UI::RmlBridge::Tooltip::Config config;
            config.lines = UI::Skills::Tooltip::ToRmlBridgeLines(tooltipModel);
            // The original's (30, 16) in the skill frame, where the frame is drawn.
            Rml::Element* frame = Element("skill");
            Rml::Vector2f origin;
            if (frame != nullptr && UI::RmlBridge::DrawnTopLeft(*frame, origin))
            {
                const float scale = UI::RmlBridge::DrawnScale(*frame);
                config.anchorX = origin.x + 30.f * scale;
                config.anchorY = origin.y + 16.f * scale;
            }
            // STRP_BOTTOMCENTER's old native meaning: grow upward from sy (see RenderTipTextList()).
            config.anchor = UI::RmlBridge::Tooltip::AnchorPoint::AboveLeft;
            config.textAlign = UI::RmlBridge::Tooltip::Config::TextAlign::Center; // RenderTipTextList()'s own default (RT3_SORT_CENTER).
            UI::RmlBridge::Tooltip::Show(config, this);
        }
        else
        {
            UI::RmlBridge::Tooltip::Hide(this);
        }
    }
    else
    {
        UI::RmlBridge::Tooltip::Hide(this);
    }
}

void  mu::ui::window::CSiegeWarBase::SetTime(int iHour, int iMinute)
{
    m_iHour = iHour;
    m_iMinute = iMinute;
}

void mu::ui::window::CSiegeWarBase::SetMapInfo(GuildCommander& data)
{
    m_CmdBuffer[data.byTeam].byCmd = data.byCmd;
    m_CmdBuffer[data.byTeam].byTeam = data.byTeam;
    m_CmdBuffer[data.byTeam].byX = data.byX;
    m_CmdBuffer[data.byTeam].byY = data.byY;
    m_CmdBuffer[data.byTeam].byLifeTime = 100;
}

void mu::ui::window::CSiegeWarBase::SetRenderSkillUI(bool bRenderSkillUI)
{
    m_bRenderSkillUI = bRenderSkillUI;
}
