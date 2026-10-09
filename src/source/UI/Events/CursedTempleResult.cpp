
#include "stdafx.h"
#include "UI/Events/CursedTempleResult.h"
#include "UI/Events/EventPreview.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Widgets/UIBaseDef.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "I18N/All.h"

#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Character/CharacterManager.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlPointer.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// RenderText() shrinks a text wider than its box to fit it: the size it drew `text` at.
float TextPxInBox(const UI::Scaling::Transform& transform, const wchar_t* text, float boxWidth)
{
    g_pRenderText->SetFont(g_hFont);
    const int width = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
    return UI::Scaling::NativeTextPixelSizeInBox(UI::Scaling::FontRole::Normal, transform, static_cast<float>(width),
                                                 boxWidth);
}
} // namespace

bool mu::ui::window::CCursedTempleResult::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_CURSEDTEMPLE_RESULT, this);

    SetPos(x, y);

    BuildRmlUi();

    Show(false);

    return true;
}

mu::ui::window::CCursedTempleResult::CCursedTempleResult() : m_pNewUIMng(NULL), m_ResultEffectAlph(0.f), m_WinState(0)
{
    Initialize();
}

mu::ui::window::CCursedTempleResult::~CCursedTempleResult()
{
    Destroy();
}

void mu::ui::window::CCursedTempleResult::Initialize() {}

void mu::ui::window::CCursedTempleResult::Destroy()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CCursedTempleResult::ResetGameResultInfo()
{
    m_WinState = 0;

    m_ResultEffectAlph = 0.f;

    m_MyTeam = SEASON3A::eTeam_Count;

    if (m_AlliedTeamGameResult.size() != 0)
        m_AlliedTeamGameResult.clear();

    if (m_IllusionTeamGameResult.size() != 0)
        m_IllusionTeamGameResult.clear();
}

void mu::ui::window::CCursedTempleResult::UpdateResult()
{
    if (m_WinState == 0)
        return;

    m_ResultEffectAlph += 0.015f;
    if (1.0f < m_ResultEffectAlph)
    {
        m_ResultEffectAlph = 1.0f;
    }
}

void mu::ui::window::CCursedTempleResult::OpenningProcess()
{
}

void mu::ui::window::CCursedTempleResult::ClosingProcess()
{
    if (UI::EventPreview::IsShowing(UI::EventPreview::Event::TempleResult))
    {
        UI::EventPreview::Stop();
        return;
    }
    SocketClient->ToGameServer()->SendIllusionTempleRewardRequest();
    ResetGameResultInfo();
}

bool mu::ui::window::CCursedTempleResult::UpdateMouseEvent()
{
    // The Close button is RmlUi's (see Update()); the window keeps the pointer.
    return !UI::RmlBridge::IsPointerOver(m_RmlView.Document());
}

bool mu::ui::window::CCursedTempleResult::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CURSEDTEMPLE_RESULT) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CURSEDTEMPLE_RESULT);
            return false;
        }
    }

    return true;
}

bool mu::ui::window::CCursedTempleResult::Update()
{
    UpdateResult();
    SyncRmlModel();

    // A Close click RmlUi reported (the original's button handling in UpdateMouseEvent()).
    const bool close = m_PendingClose;
    m_PendingClose = false;
    if (close && IsVisible())
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CURSEDTEMPLE_RESULT);

    return true;
}

bool mu::ui::window::CCursedTempleResult::Render()
{
    // Nothing native left: the frame, the banner, the texts and the button are RmlUi. Kept
    // because CObject requires the override.
    return true;
}

void mu::ui::window::CCursedTempleResult::SetResult(const UI::CursedTemple::MatchResult& result)
{
    const int alliedPoint = result.alliedPoints;
    const int illusionPoint = result.illusionPoints;

    if (m_MyTeam == SEASON3A::eTeam_Allied)
    {
        if (illusionPoint < alliedPoint)
            m_WinState = 1;
        else
            m_WinState = 2;

        if (2 > alliedPoint) m_WinState = 2;
    }
    else
    {
        if (alliedPoint < illusionPoint)
            m_WinState = 1;
        else
            m_WinState = 2;

        if (2 > illusionPoint) m_WinState = 2;
    }

    for (const UI::CursedTemple::PlayerResult& player : result.players)
    {
        CursedTempleGameResult row{};
        std::copy_n(player.name.begin(),
                    std::min(player.name.size(), std::size(row.s_characterId) - 1), row.s_characterId);
        row.s_mapnumber = player.mapNumber;
        row.s_team = player.team;
        row.s_class = player.playerClass;
        row.s_addexp = player.addedExperience;

        if (row.s_team == SEASON3A::eTeam_Allied)
        {
            row.s_point = alliedPoint;
            m_AlliedTeamGameResult.push_back(row);
        }
        else
        {
            row.s_point = illusionPoint;
            m_IllusionTeamGameResult.push_back(row);
        }
    }
}

void mu::ui::window::CCursedTempleResult::BindRmlModel(Rml::DataModelConstructor& c, CursedTempleResultRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("line_height_px", &model.lineHeightPx);
    auto lineType = c.RegisterStruct<CursedTempleResultLine>();
    lineType.RegisterMember("text", &CursedTempleResultLine::text);
    lineType.RegisterMember("text_px", &CursedTempleResultLine::textPx);
    c.Bind("hero_list_label", &model.heroListLabel);
    c.Bind("column_header", &model.columnHeader);
    c.Bind("camp_label", &model.campLabel);
    c.Bind("character_label", &model.characterLabel);
    c.Bind("class_label", &model.classLabel);
    c.Bind("exp_label", &model.expLabel);
    c.Bind("point_label", &model.pointLabel);
    c.Bind("reward_hint", &model.rewardHint);
    auto resultRow = c.RegisterStruct<CursedTempleResultRow>();
    resultRow.RegisterMember("team", &CursedTempleResultRow::team);
    resultRow.RegisterMember("name", &CursedTempleResultRow::name);
    resultRow.RegisterMember("class_name", &CursedTempleResultRow::className);
    resultRow.RegisterMember("added_exp", &CursedTempleResultRow::addedExp);
    resultRow.RegisterMember("point", &CursedTempleResultRow::point);
    resultRow.RegisterMember("hero", &CursedTempleResultRow::hero);
    c.RegisterArray<std::vector<CursedTempleResultRow>>();
    c.Bind("allied_rows", &model.alliedRows);
    c.Bind("illusion_rows", &model.illusionRows);
    c.Bind("close_text", &model.closeText);
    c.Bind("label_line_px", &model.labelLinePx);
    c.Bind("banner", &model.banner);
    c.Bind("banner_alpha", &model.bannerAlpha);
    c.BindEventCallback("result_close", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingClose = true; });
}

void mu::ui::window::CCursedTempleResult::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CCursedTempleResult::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // Layer depth 10.2: over the HUD and the panels.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());
    SyncTexts();
}

void mu::ui::window::CCursedTempleResult::SyncTexts()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    CursedTempleResultRmlModel updated = m_RmlView.GetModel();
    const float textPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Normal);
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    updated.lineHeightPx = static_cast<float>(lineHeight) * transform.scaleY;
    updated.labelLinePx = updated.lineHeightPx;
    updated.closeText = StringUtils::WideToNarrow(I18N::Game::Close388);

    // The original's RenderResultPanel(): the banner centred over the window (which sat centred on
    // the reference width), 110 units above it, fading in. It drew under the alpha test
    // (EnableAlphaTest(), GL_GREATER 0.25), so the banner stayed invisible until the fade passed a
    // quarter.
    constexpr float kAlphaTestReference = 0.25f;
    updated.banner = m_WinState;
    const float fade = std::clamp(m_ResultEffectAlph, 0.f, 1.f);
    updated.bannerAlpha = fade > kAlphaTestReference ? fade : 0.f;

    // The original's RenderText(): the centred lines shrunk to the window's width, then one row
    // per player, its cells left-aligned at fixed offsets (long names run into the next cell, as
    // they did), the hero's row on a text box. The theme places all of it.
    auto line = [&](const wchar_t* text)
    {
        return CursedTempleResultLine{StringUtils::WideToNarrow(text),
                                      TextPxInBox(transform, text, CURSEDTEMPLE_RESULT_WINDOW_WIDTH)};
    };
    auto addRows = [&](const CT_GameResult_list& results, std::vector<CursedTempleResultRow>& rows)
    {
        for (const CursedTempleGameResult& info : results)
        {
            wchar_t Text[200] = {};
            CursedTempleResultRow row;
            row.team = StringUtils::WideToNarrow(SEASON3A::eTeam_Allied == info.s_team ? I18N::Game::MUAlliance
                                                                                      : I18N::Game::IllusionSorcery);
            row.name = StringUtils::WideToNarrow(info.s_characterId);
            row.className = StringUtils::WideToNarrow(gCharacterManager.GetCharacterClassText(info.s_class));
            mu_swprintf(Text, L"%d", info.s_addexp);
            row.addedExp = StringUtils::WideToNarrow(Text);
            mu_swprintf(Text, L"%d", info.s_point);
            row.point = StringUtils::WideToNarrow(Text);
            row.hero = wcscmp(info.s_characterId, Hero->ID) == 0;
            rows.push_back(std::move(row));
        }
    };

    wchar_t Text[200] = {};
    updated.heroListLabel = line(I18N::Game::HeroList);
    mu_swprintf(Text, L"  %ls           %ls        %ls     %ls    %ls", I18N::Game::Camp, I18N::Game::Character,
                I18N::Game::Class, I18N::Game::EXP, I18N::Game::Point);
    updated.columnHeader = line(Text);
    updated.campLabel = StringUtils::WideToNarrow(I18N::Game::Camp);
    updated.characterLabel = StringUtils::WideToNarrow(I18N::Game::Character);
    updated.classLabel = StringUtils::WideToNarrow(I18N::Game::Class);
    updated.expLabel = StringUtils::WideToNarrow(I18N::Game::EXP);
    updated.pointLabel = StringUtils::WideToNarrow(I18N::Game::Point);
    updated.alliedRows.clear();
    updated.illusionRows.clear();
    addRows(m_AlliedTeamGameResult, updated.alliedRows);
    addRows(m_IllusionTeamGameResult, updated.illusionRows);
    updated.rewardHint = line(I18N::Game::YouMayBeCompensatedByClickingOnTheCloseButton);

    CursedTempleResultRmlModel& model = m_RmlView.GetModel();
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::lineHeightPx, "line_height_px", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::labelLinePx, "label_line_px", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::closeText, "close_text", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::banner, "banner", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::bannerAlpha, "banner_alpha", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::heroListLabel, "hero_list_label", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::columnHeader, "column_header", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::campLabel, "camp_label", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::characterLabel, "character_label", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::classLabel, "class_label", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::expLabel, "exp_label", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::pointLabel, "point_label", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::rewardHint, "reward_hint", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::alliedRows, "allied_rows", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleResultRmlModel::illusionRows, "illusion_rows", updated);
}
