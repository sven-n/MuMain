
#include "stdafx.h"
#include "UI/Events/CursedTempleResult.h"
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
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

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
    UI::RmlBridge::UnregisterForThemeReload(this);

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
    SocketClient->ToGameServer()->SendIllusionTempleRewardRequest();
    ResetGameResultInfo();
}

bool mu::ui::window::CCursedTempleResult::UpdateMouseEvent()
{
    // The Close button is RmlUi's (see Update()); the window keeps the pointer.
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, CURSEDTEMPLE_RESULT_WINDOW_WIDTH, CURSEDTEMPLE_RESULT_WINDOW_HEIGHT).Contains(MouseX, MouseY))
    {
        return false;
    }

    return true;
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

void mu::ui::window::CCursedTempleResult::ReceiveCursedTempleGameResult(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_CURSED_TEMPLE_RESULT)ReceiveBuffer;

    int	alliedPoint = data->btAlliedPoint;
    int	illusionPoint = data->btIllusionPoint;
    int userCount = data->btUserCount;

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

    int Offset = sizeof(PMSG_CURSED_TEMPLE_RESULT);

    for (int i = 0; i < userCount; i++)
    {
        auto data2 = (LPPMSG_CURSED_TEMPLE_USER_ADD_EXP)(ReceiveBuffer + Offset);

        CursedTempleGameResult TempData{};
        CMultiLanguage::ConvertFromUtf8(TempData.s_characterId, data2->GameId, MAX_USERNAME_SIZE);

        TempData.s_mapnumber = (short)data2->byMapNumber;

        TempData.s_team = static_cast<SEASON3A::eCursedTempleTeam>(data2->btTeam);
        TempData.s_class = gCharacterManager.ChangeServerClassTypeToClientClassType(data2->btClass);
        TempData.s_addexp = data2->nAddExp;

        if (TempData.s_team == SEASON3A::eTeam_Allied)
        {
            TempData.s_point = alliedPoint;
            m_AlliedTeamGameResult.push_back(TempData);
        }
        else
        {
            TempData.s_point = illusionPoint;
            m_IllusionTeamGameResult.push_back(TempData);
        }

        Offset += sizeof(PMSG_CURSED_TEMPLE_USER_ADD_EXP);
    }
}

void mu::ui::window::CCursedTempleResult::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "cursed_temple_result",
        [this](Rml::DataModelConstructor& c, CursedTempleResultRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("line_height_px", &model.lineHeightPx);
            auto lineType = c.RegisterStruct<CursedTempleResultLine>();
            lineType.RegisterMember("text", &CursedTempleResultLine::text);
            lineType.RegisterMember("text_px", &CursedTempleResultLine::textPx);
            c.Bind("hero_list_label", &model.heroListLabel);
            c.Bind("column_header", &model.columnHeader);
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
            c.Bind("label_top", &model.labelTop);
            c.Bind("label_line_px", &model.labelLinePx);
            c.Bind("banner", &model.banner);
            c.Bind("banner_left", &model.bannerLeft);
            c.Bind("banner_alpha", &model.bannerAlpha);
            c.BindEventCallback("result_close", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                                { m_PendingClose = true; });
        });

    if (!modelCreated)
        return;

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/cursed_temple_result.rml");
}

void mu::ui::window::CCursedTempleResult::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void mu::ui::window::CCursedTempleResult::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // Layer depth 10.2: over the HUD and the panels.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);
    SyncTexts();
}

void mu::ui::window::CCursedTempleResult::SyncTexts()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    CursedTempleResultRmlModel updated = m_RmlBinder.GetModel();
    const float textPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform);
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    updated.lineHeightPx = static_cast<float>(lineHeight) * transform.scaleY;
    updated.labelTop = static_cast<float>(23 / 2 - lineHeight / 2);
    updated.labelLinePx = updated.lineHeightPx;
    updated.closeText = StringUtils::WideToNarrow(I18N::Game::Close388);

    // The original's RenderResultPanel(): the banner centred on the reference width, 110 units
    // above the window, fading in. It drew under the alpha test (EnableAlphaTest(), GL_GREATER
    // 0.25), so the banner stayed invisible until the fade passed a quarter.
    constexpr float kAlphaTestReference = 0.25f;
    updated.banner = m_WinState;
    const int bannerX = (REFERENCE_WIDTH - 360) / 2;
    updated.bannerLeft = static_cast<float>(bannerX - m_Pos.x);
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
    updated.alliedRows.clear();
    updated.illusionRows.clear();
    addRows(m_AlliedTeamGameResult, updated.alliedRows);
    addRows(m_IllusionTeamGameResult, updated.illusionRows);
    updated.rewardHint = line(I18N::Game::YouMayBeCompensatedByClickingOnTheCloseButton);

    CursedTempleResultRmlModel& model = m_RmlBinder.GetModel();
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::lineHeightPx, "line_height_px", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::labelTop, "label_top", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::labelLinePx, "label_line_px", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::closeText, "close_text", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::banner, "banner", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::bannerLeft, "banner_left", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::bannerAlpha, "banner_alpha", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::heroListLabel, "hero_list_label", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::columnHeader, "column_header", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::rewardHint, "reward_hint", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::alliedRows, "allied_rows", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleResultRmlModel::illusionRows, "illusion_rows", updated);
}
