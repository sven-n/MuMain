
#include "stdafx.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/Combat/SiegeWarfare.h"
#include "UI/Events/EventPreview.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Combat/SiegeWarCommander.h"
#include "UI/Combat/SiegeWarSoldier.h"
#include "UI/Combat/SiegeWarObserver.h"
#include "Engine/Object/ZzzInventory.h"
#include "Guild/GuildTypes.h"
#include "World/MapInfra/MapManager.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

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

    Show(true);

    SetPos(x, y);

    return true;
}

void mu::ui::window::CSiegeWarfare::Release()
{

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

    m_RmlView.Release();
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

    if ((gMapManager.InBattleCastle() || UI::EventPreview::IsShowing(UI::EventPreview::Event::Siege)) &&
        battleCastle::IsBattleCastleStart() == true)
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

void mu::ui::window::CSiegeWarfare::BindRmlModel(Rml::DataModelConstructor& c, SiegeWarfareRmlModel& model)
{
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("big_text_px", &model.bigTextPx);
    c.Bind("frame_x", &model.frameX);
    c.Bind("frame_y", &model.frameY);
    c.Bind("alpha", &model.alpha);
    c.Bind("map_rect", &model.mapRect);
    c.Bind("alpha_label", &model.alphaLabel);
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
    c.Bind("skill_affordable", &model.skillAffordable);
    c.Bind("kills_needed", &model.killsNeeded);
    c.Bind("kills", &model.kills);

    auto button = c.RegisterStruct<SiegeWarButtonEntry>();
    button.RegisterMember("left", &SiegeWarButtonEntry::left);
    button.RegisterMember("top", &SiegeWarButtonEntry::top);
    button.RegisterMember("selected", &SiegeWarButtonEntry::selected);

    // The HUD's buttons; a click on one never reaches the world.
    c.BindEventCallback("siege_alpha_click", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { if (m_pSiegeWarUI) m_pSiegeWarUI->ToggleAlpha(); });
    c.BindEventCallback("siege_scale_click", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { if (m_pSiegeWarUI) m_pSiegeWarUI->ToggleMiniMapScale(); });
    c.BindEventCallback("siege_scroll_up_click", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { if (m_pSiegeWarUI) m_pSiegeWarUI->ScrollSkillUp(); });
    c.BindEventCallback("siege_scroll_down_click", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { if (m_pSiegeWarUI) m_pSiegeWarUI->ScrollSkillDown(); });
    c.BindEventCallback("siege_team_click", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                        {
                            if (m_pSiegeWarUI && args.size() == 1)
                                m_pSiegeWarUI->OnTeamClick(args[0].Get<int>(-1));
                        });
    c.BindEventCallback("siege_order_click", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                        {
                            if (m_pSiegeWarUI && args.size() == 1)
                                m_pSiegeWarUI->OnOrderClick(args[0].Get<int>(-1));
                        });
    button.RegisterMember("label", &SiegeWarButtonEntry::label);
    c.RegisterArray<std::vector<SiegeWarButtonEntry>>();
    c.Bind("teams", &model.teams);
    c.Bind("orders", &model.orders);
    c.Bind("cursor_visible", &model.cursorVisible);
    c.Bind("cursor_left", &model.cursorLeft);
    c.Bind("cursor_top", &model.cursorTop);
    c.Bind("cursor_command", &model.cursorCommand);
    c.Bind("cursor_team", &model.cursorTeam);
}

void mu::ui::window::CSiegeWarfare::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CSiegeWarfare::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // The original drew the HUD only on the siege map (Render()).
    const bool shown = IsVisible() && m_pSiegeWarUI != NULL &&
                       (gMapManager.InBattleCastle() || UI::EventPreview::IsShowing(UI::EventPreview::Event::Siege));
    UI::RmlBridge::SyncDocumentVisibilityBehind(m_RmlView.Document(), shown);
    if (!shown)
        return;

    SiegeWarfareRmlModel& next = m_NextRmlModel;
    m_pSiegeWarUI->SetDocument(m_RmlView.Document());
    next.boldTextPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold);
    next.bigTextPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Big);
    m_pSiegeWarUI->FillRmlModel(next);
    ApplyRmlModel(next);
}

void mu::ui::window::CSiegeWarfare::ApplyRmlModel(const SiegeWarfareRmlModel& next)
{
    SiegeWarfareRmlModel& model = m_RmlView.GetModel();
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::boldTextPx, "bold_text_px", next.boldTextPx);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::bigTextPx, "big_text_px", next.bigTextPx);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::frameX, "frame_x", next.frameX);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::frameY, "frame_y", next.frameY);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::alpha, "alpha", next.alpha);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::mapRect, "map_rect", next.mapRect);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::alphaLabel, "alpha_label", next.alphaLabel);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::timeVisible, "time_visible", next.timeVisible);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::timeText, "time_text", next.timeText);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::dots, "dots", next.dots);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::heroLeft, "hero_left", next.heroLeft);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::heroTop, "hero_top", next.heroTop);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::commands, "commands", next.commands);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::skillVisible, "skill_visible", next.skillVisible);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::skillRect, "skill_rect", next.skillRect);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::skillAffordable, "skill_affordable", next.skillAffordable);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::killsNeeded, "kills_needed", next.killsNeeded);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::kills, "kills", next.kills);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::teams, "teams", next.teams);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::orders, "orders", next.orders);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::cursorVisible, "cursor_visible", next.cursorVisible);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::cursorLeft, "cursor_left", next.cursorLeft);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::cursorTop, "cursor_top", next.cursorTop);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::cursorCommand, "cursor_command", next.cursorCommand);
    SyncField(m_RmlView.Binder(), &SiegeWarfareRmlModel::cursorTeam, "cursor_team", next.cursorTeam);
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

void mu::ui::window::CSiegeWarfare::CreatePreviewMiniMapUI(SIEGEWAR_TYPE type)
{
    InitMiniMapUI();
    switch (type)
    {
    case SIEGEWAR_TYPE_COMMANDER: m_pSiegeWarUI = new CSiegeWarCommander; break;
    case SIEGEWAR_TYPE_SOLDIER: m_pSiegeWarUI = new CSiegeWarSoldier; break;
    default: m_pSiegeWarUI = new CSiegeWarObserver; type = SIEGEWAR_TYPE_OBSERVER; break;
    }
    m_iCurSiegeWarType = type;
    m_bCreated = type != SIEGEWAR_TYPE_OBSERVER;
    m_pSiegeWarUI->Create(m_Pos.x, m_Pos.y);
    Show(true);
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