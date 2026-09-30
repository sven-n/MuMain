
#include "stdafx.h"
#include "UI/Combat/SiegeWarfare.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Combat/SiegeWarCommander.h"
#include "UI/Combat/SiegeWarSoldier.h"
#include "UI/Combat/SiegeWarObserver.h"
#include "Engine/Object/ZzzInventory.h"
#include "Guild/UIGuildInfo.h"
#include "World/MapInfra/MapManager.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The original drew the HUD under nearly every other window (layer depth 1.6), and a docked
// panel's frame is painted in the background context before the native windows: only a document
// in that same context, behind the others, stays under them (as the duel and battle-soccer boards
// do). The durability warnings, the logs and every native window then draw over the HUD.
Rml::Context* HudContext()
{
    Rml::Context* context = RmlUiRuntime::Instance().GetBackgroundContext();
    return context != nullptr ? context : RmlUiRuntime::Instance().GetContext();
}
} // namespace

mu::ui::window::CSiegeWarfare::CSiegeWarfare()
{
    m_pNewUIMng = NULL;
    m_pSiegeWarUI = NULL;
    m_iCurSiegeWarType = SIEGEWAR_TYPE_NONE;
    m_byGuildStatus = G_NONE;
    m_sGuildMarkIndex = -1;

    m_iHour = 0;
    m_iMinute = 0;
    m_iSecond = 0;
    m_dwSyncTime = 0;

    m_bCreated = true;

    memset(&m_Pos, 0, sizeof(POINT));
}

mu::ui::window::CSiegeWarfare::~CSiegeWarfare()
{
    Release();
}

bool mu::ui::window::CSiegeWarfare::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SIEGEWARFARE, this);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(true);

    SetPos(x, y);

    return true;
}

void mu::ui::window::CSiegeWarfare::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pSiegeWarUI)
    {
        m_pSiegeWarUI->Release();
        SAFE_DELETE(m_pSiegeWarUI);
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CSiegeWarfare::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CSiegeWarfare::UpdateMouseEvent()
{
    if (m_pSiegeWarUI)
    {
        if (!m_pSiegeWarUI->UpdateMouseEvent())
            return false;
    }

    return true;
}

bool mu::ui::window::CSiegeWarfare::UpdateKeyEvent()
{
    if (m_pSiegeWarUI)
    {
        if (!m_pSiegeWarUI->UpdateKeyEvent())
            return false;
    }

    return true;
}

bool mu::ui::window::CSiegeWarfare::Update()
{
    if (IsVisible() == false || m_pSiegeWarUI == NULL)
    {
        SyncRmlModel();
        return true;
    }

    if (gMapManager.InBattleCastle() && battleCastle::IsBattleCastleStart() == true)
    {
        m_iSecond = m_iSecond - (GetTickCount() - m_dwSyncTime);
        if (m_iSecond <= 0)
        {
            if (m_iMinute <= 0)
            {
                if (m_iHour <= 0)
                {
                    m_iSecond = 0;
                    m_iMinute = 0;
                    m_iHour = 0;
                }
                else
                {
                    --m_iHour;
                    m_iMinute = m_iMinute + 60;
                }
            }
            else
            {
                --m_iMinute;
                m_iSecond = m_iSecond + 60000;
            }
        }

        m_dwSyncTime = GetTickCount();

        m_pSiegeWarUI->SetTime(m_iHour, m_iMinute);
    }

    m_pSiegeWarUI->Update();
    SyncRmlModel();

    return true;
}

bool mu::ui::window::CSiegeWarfare::Render()
{
    // Nothing native left: siege_warfare.rml draws the HUD (SyncRmlModel() from Update()). Kept
    // because CObject requires the override.
    return true;
}

void mu::ui::window::CSiegeWarfare::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(HudContext(), "siege_warfare",
                                                 [](Rml::DataModelConstructor& c, SiegeWarfareRmlModel& model)
                                                 {
                                                     c.Bind("scale_x", &model.scaleX);
                                                     c.Bind("scale_y", &model.scaleY);
                                                     c.Bind("inverse_scale_x", &model.inverseScaleX);
                                                     c.Bind("inverse_scale_y", &model.inverseScaleY);
                                                     c.Bind("bold_text_px", &model.boldTextPx);
                                                     c.Bind("big_text_px", &model.bigTextPx);
                                                     c.Bind("frame_x", &model.frameX);
                                                     c.Bind("frame_y", &model.frameY);
                                                     c.Bind("alpha", &model.alpha);
                                                     c.Bind("map_rect", &model.mapRect);
                                                     c.Bind("alpha_label", &model.alphaLabel);
                                                     c.Bind("alpha_frame", &model.alphaFrame);
                                                     c.Bind("time_visible", &model.timeVisible);
                                                     c.Bind("time_text", &model.timeText);

                                                     auto dot = c.RegisterStruct<SiegeWarDotEntry>();
                                                     dot.RegisterMember("left", &SiegeWarDotEntry::left);
                                                     dot.RegisterMember("top", &SiegeWarDotEntry::top);
                                                     c.RegisterArray<std::vector<SiegeWarDotEntry>>();
                                                     c.Bind("dots", &model.dots);
                                                     c.Bind("hero_left", &model.heroLeft);
                                                     c.Bind("hero_top", &model.heroTop);

                                                     auto command = c.RegisterStruct<SiegeWarCommandEntry>();
                                                     command.RegisterMember("left", &SiegeWarCommandEntry::left);
                                                     command.RegisterMember("top", &SiegeWarCommandEntry::top);
                                                     command.RegisterMember("command", &SiegeWarCommandEntry::command);
                                                     command.RegisterMember("team", &SiegeWarCommandEntry::team);
                                                     command.RegisterMember("color", &SiegeWarCommandEntry::color);
                                                     c.RegisterArray<std::vector<SiegeWarCommandEntry>>();
                                                     c.Bind("commands", &model.commands);

                                                     c.Bind("skill_visible", &model.skillVisible);
                                                     c.Bind("skill_rect", &model.skillRect);
                                                     c.Bind("skill_color", &model.skillColor);
                                                     c.Bind("kills_needed", &model.killsNeeded);
                                                     c.Bind("kills", &model.kills);
                                                     c.Bind("scroll_up_frame", &model.scrollUpFrame);
                                                     c.Bind("scroll_down_frame", &model.scrollDownFrame);

                                                     auto button = c.RegisterStruct<SiegeWarButtonEntry>();
                                                     button.RegisterMember("left", &SiegeWarButtonEntry::left);
                                                     button.RegisterMember("top", &SiegeWarButtonEntry::top);
                                                     button.RegisterMember("frame", &SiegeWarButtonEntry::frame);
                                                     button.RegisterMember("label", &SiegeWarButtonEntry::label);
                                                     c.RegisterArray<std::vector<SiegeWarButtonEntry>>();
                                                     c.Bind("teams", &model.teams);
                                                     c.Bind("orders", &model.orders);
                                                     c.Bind("cursor_visible", &model.cursorVisible);
                                                     c.Bind("cursor_left", &model.cursorLeft);
                                                     c.Bind("cursor_top", &model.cursorTop);
                                                     c.Bind("cursor_command", &model.cursorCommand);
                                                     c.Bind("cursor_team", &model.cursorTeam);
                                                 });

    if (modelCreated)
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(HudContext(), "Data/Interface/RmlUi/siege_warfare.rml");
}

void mu::ui::window::CSiegeWarfare::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = HudContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void mu::ui::window::CSiegeWarfare::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // The original drew the HUD only on the siege map (Render()).
    const bool shown = IsVisible() && m_pSiegeWarUI != NULL && gMapManager.InBattleCastle();
    UI::RmlBridge::SyncDocumentVisibilityBehind(m_pRmlDoc, shown);
    if (!shown)
        return;

    // CManager scopes LayoutMode::Hud around this window: W/640 x H/480, no offset.
    SiegeWarfareRmlModel& next = m_NextRmlModel;
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    next.scaleX = transform.scaleX;
    next.scaleY = transform.scaleY;
    next.inverseScaleX = 1.0f / transform.scaleX;
    next.inverseScaleY = 1.0f / transform.scaleY;
    next.boldTextPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform);
    next.bigTextPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Big, transform);
    m_pSiegeWarUI->FillRmlModel(next);
    ApplyRmlModel(next);
}

void mu::ui::window::CSiegeWarfare::ApplyRmlModel(const SiegeWarfareRmlModel& next)
{
    SiegeWarfareRmlModel& model = m_RmlBinder.GetModel();
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::scaleX, "scale_x", next.scaleX);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::scaleY, "scale_y", next.scaleY);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::inverseScaleX, "inverse_scale_x", next.inverseScaleX);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::inverseScaleY, "inverse_scale_y", next.inverseScaleY);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::boldTextPx, "bold_text_px", next.boldTextPx);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::bigTextPx, "big_text_px", next.bigTextPx);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::frameX, "frame_x", next.frameX);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::frameY, "frame_y", next.frameY);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::alpha, "alpha", next.alpha);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::mapRect, "map_rect", next.mapRect);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::alphaLabel, "alpha_label", next.alphaLabel);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::alphaFrame, "alpha_frame", next.alphaFrame);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::timeVisible, "time_visible", next.timeVisible);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::timeText, "time_text", next.timeText);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::dots, "dots", next.dots);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::heroLeft, "hero_left", next.heroLeft);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::heroTop, "hero_top", next.heroTop);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::commands, "commands", next.commands);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::skillVisible, "skill_visible", next.skillVisible);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::skillRect, "skill_rect", next.skillRect);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::skillColor, "skill_color", next.skillColor);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::killsNeeded, "kills_needed", next.killsNeeded);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::kills, "kills", next.kills);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::scrollUpFrame, "scroll_up_frame", next.scrollUpFrame);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::scrollDownFrame, "scroll_down_frame", next.scrollDownFrame);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::teams, "teams", next.teams);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::orders, "orders", next.orders);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::cursorVisible, "cursor_visible", next.cursorVisible);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::cursorLeft, "cursor_left", next.cursorLeft);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::cursorTop, "cursor_top", next.cursorTop);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::cursorCommand, "cursor_command", next.cursorCommand);
    SyncField(m_RmlBinder, &SiegeWarfareRmlModel::cursorTeam, "cursor_team", next.cursorTeam);
}

float mu::ui::window::CSiegeWarfare::GetLayerDepth()
{
    return 1.6f;
}

void mu::ui::window::CSiegeWarfare::OpenningProcess()
{
}

void mu::ui::window::CSiegeWarfare::ClosingProcess()
{
}

void mu::ui::window::CSiegeWarfare::SetGuildData(const CHARACTER* pCharacter)
{
    m_sGuildMarkIndex = pCharacter->GuildMarkIndex;
    m_byGuildStatus = pCharacter->GuildStatus;
}

bool mu::ui::window::CSiegeWarfare::CreateMiniMapUI()
{
    if (m_pSiegeWarUI != NULL)
    {
        InitMiniMapUI();
    }

    if (!(Hero->EtcPart == PARTS_ATTACK_TEAM_MARK
        || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK2
        || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK3
        || Hero->EtcPart == PARTS_DEFENSE_TEAM_MARK)
        || Hero->GuildStatus == G_PERSON)
    {
        m_byGuildStatus = G_NONE;
    }

    switch (m_byGuildStatus)
    {
    case G_NONE:
    {
        m_pSiegeWarUI = new CSiegeWarObserver;		// Observer
        m_iCurSiegeWarType = SIEGEWAR_TYPE_OBSERVER;
        m_bCreated = false;
    }break;
    case G_MASTER:
    {
        if (wcscmp(GuildMark[m_sGuildMarkIndex].UnionName, L"") == 0
            || wcscmp(GuildMark[m_sGuildMarkIndex].GuildName, GuildMark[m_sGuildMarkIndex].UnionName) == 0)
        {
            m_pSiegeWarUI = new CSiegeWarCommander;	// Commander
            m_iCurSiegeWarType = SIEGEWAR_TYPE_COMMANDER;
        }
        else
        {
            m_pSiegeWarUI = new CSiegeWarSoldier;	// Soldier
            m_iCurSiegeWarType = SIEGEWAR_TYPE_SOLDIER;
        }
        m_bCreated = true;
    }break;
    default:
    {
        m_pSiegeWarUI = new CSiegeWarSoldier;	// Soldier
        m_iCurSiegeWarType = SIEGEWAR_TYPE_SOLDIER;
        m_bCreated = true;
    }break;
    }

    m_pSiegeWarUI->Create(m_Pos.x, m_Pos.y);
    Show(true);

    return true;
}

void mu::ui::window::CSiegeWarfare::ClearGuildMemberLocation(void)
{
    if (m_iCurSiegeWarType == SIEGEWAR_TYPE_COMMANDER)
    {
        ((CSiegeWarCommander*)m_pSiegeWarUI)->ClearGuildMemberLocation();
    }
}

void mu::ui::window::CSiegeWarfare::SetGuildMemberLocation(BYTE type, int x, int y)
{
    if (m_iCurSiegeWarType == SIEGEWAR_TYPE_COMMANDER)
    {
        ((CSiegeWarCommander*)m_pSiegeWarUI)->SetGuildMemberLocation(type, x, y);
    }
}

void mu::ui::window::CSiegeWarfare::InitMiniMapUI()
{
    if (m_pSiegeWarUI == NULL)
    {
        return;
    }

    m_pSiegeWarUI->Release();

    SAFE_DELETE(m_pSiegeWarUI);

    m_iCurSiegeWarType = SIEGEWAR_TYPE_NONE;
    m_byGuildStatus = G_NONE;
    m_sGuildMarkIndex = -1;
}

void  mu::ui::window::CSiegeWarfare::SetTime(BYTE byHour, BYTE byMinute)
{
    m_iHour = (int)byHour;
    m_iMinute = (int)byMinute;
    m_iSecond = 60000;
    m_dwSyncTime = GetTickCount();
}

void mu::ui::window::CSiegeWarfare::SetMapInfo(GuildCommander& data)
{
    if (m_pSiegeWarUI == NULL)
    {
        return;
    }

    m_pSiegeWarUI->SetMapInfo(data);
}

void mu::ui::window::CSiegeWarfare::InitSkillUI()
{
    if (m_pSiegeWarUI == NULL)
    {
        return;
    }

    m_pSiegeWarUI->InitBattleSkill();
}

void mu::ui::window::CSiegeWarfare::ReleaseSkillUI()
{
    if (m_pSiegeWarUI == NULL)
    {
        return;
    }

    m_pSiegeWarUI->ReleaseBattleSkill();
}