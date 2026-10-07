
#include "stdafx.h"
#include "UI/Combat/DuelWatchMainFrameWindow.h"
#include "UI/Core/WindowSystem.h"
#include "GameLogic/Combat/DuelMgr.h"
#include "I18N/All.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <cstdio>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The original's exit button (newui_exit_00, a CButton at (640 - 36, 480 - 29), 36 x 29).
constexpr float kExitX = REFERENCE_WIDTH - 36.f;
constexpr float kExitY = REFERENCE_HEIGHT - 29.f;
constexpr float kExitWidth = 36.f;
constexpr float kExitHeight = 29.f;

// The bar textures, relative to the themed document.
constexpr const char* kHpGauge = "../../../menu_pk_hp03(bar2).jpg";
constexpr const char* kSdGauge = "../../../menu_pk_sd03(bar2).jpg";
constexpr const char* kHpGaugeFx = "../../../menu_pk_hp06(bar).jpg";
constexpr const char* kSdGaugeFx = "../../../menu_pk_sd05(bar).jpg";

DuelWatchGaugeEntry Gauge(const char* src, float sourceWidth, float sourceHeight, float left, float top, float width,
                          float height, bool mirrored)
{
    char rect[64];
    std::snprintf(rect, sizeof(rect), "0 0 %g %g", sourceWidth, sourceHeight);
    return {src, rect, left, top, width, height, mirrored};
}

// The left fighter's health bar: right-aligned on 236 units from x 60, the texture's first
// 235 * rate texels (6 rows) mirrored onto it.
DuelWatchGaugeEntry HeroHp(const char* src, float rate)
{
    return Gauge(src, 235.f * rate, 6.f, 60 + 236.f * (1.f - rate), 440.f, 236.f * rate, 7.f, true);
}

// The right fighter's health bar: from x 344, the texture's first 236 * rate texels, unscaled.
DuelWatchGaugeEntry EnemyHp(const char* src, float rate)
{
    return Gauge(src, 236.f * rate, 7.f, 580.f - 236.f, 440.f, 236.f * rate, 7.f, false);
}

// The shield bars: right-aligned on 154 units from x 142, or from x 344; unscaled texels.
DuelWatchGaugeEntry HeroSd(const char* src, float rate)
{
    return Gauge(src, 154.f * rate, 4.f, 142 + 154.f * (1.f - rate), 450.f, 154.f * rate, 4.f, false);
}

DuelWatchGaugeEntry EnemySd(const char* src, float rate)
{
    return Gauge(src, 154.f * rate, 4.f, 344.f, 450.f, 154.f * rate, 4.f, false);
}

// One bar's step of the original's catch-up: `shown` moves towards `rate` by `step`; while it
// differs, the effect texture shows the larger of both under the bar at the smaller.
void StepBar(std::vector<DuelWatchGaugeEntry>& gauges, float& shown, float rate, float step,
             DuelWatchGaugeEntry (*piece)(const char*, float), const char* bar, const char* fx)
{
    if (shown > rate + step)
    {
        shown -= step;
        gauges.push_back(piece(fx, shown));
        gauges.push_back(piece(bar, rate));
    }
    else if (shown < rate - step)
    {
        shown += step;
        gauges.push_back(piece(fx, rate));
        gauges.push_back(piece(bar, shown));
    }
    else
    {
        shown = rate;
        gauges.push_back(piece(bar, rate));
    }
}
} // namespace

CDuelWatchMainFrameWindow::CDuelWatchMainFrameWindow()
{
    m_pNewUIMng = NULL;

    m_bHasHPReceived = FALSE;
    m_fPrevHPRate1 = 0;
    m_fPrevHPRate2 = 0;
    m_fPrevSDRate1 = 0;
    m_fPrevSDRate2 = 0;
    m_fLastHPRate1 = 0;
    m_fLastHPRate2 = 0;
    m_fLastSDRate1 = 0;
    m_fLastSDRate2 = 0;
    m_fReceivedHPRate1 = 0;
    m_fReceivedHPRate2 = 0;
    m_fReceivedSDRate1 = 0;
    m_fReceivedSDRate2 = 0;
}

CDuelWatchMainFrameWindow::~CDuelWatchMainFrameWindow()
{
    Release();
}

bool CDuelWatchMainFrameWindow::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_DUELWATCH_MAINFRAME, this);

    BuildRmlUi();

    Show(false);

    return true;
}

void CDuelWatchMainFrameWindow::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

bool CDuelWatchMainFrameWindow::UpdateMouseEvent()
{
    // The exit button's click is RmlUi's (duel_watch_exit); the pointer on it goes to nothing
    // behind the frame.
    if (CheckMouseIn(static_cast<int>(kExitX), static_cast<int>(kExitY), static_cast<int>(kExitWidth),
                     static_cast<int>(kExitHeight)))
        return false;
    return true;
}

bool CDuelWatchMainFrameWindow::UpdateKeyEvent()
{
    return true;
}

bool CDuelWatchMainFrameWindow::Update()
{
    SyncView();

    if (m_PendingExit)
    {
        m_PendingExit = false;
        if (IsVisible() && g_DuelMgr.GetCurrentChannel() >= 0)
        {
            SocketClient->ToGameServer()->SendDuelChannelQuitRequest();
        }
    }
    return true;
}

bool CDuelWatchMainFrameWindow::Render()
{
    // Nothing native left: the frame, the names, the score marks, the gauges and the exit button
    // are RmlUi (SyncView()). Kept because CObject requires the override.
    return true;
}

bool CDuelWatchMainFrameWindow::IsVisible() const
{
    return CObject::IsVisible();
}

void CDuelWatchMainFrameWindow::OpeningProcess()
{
    m_bHasHPReceived = FALSE;
    m_PendingExit = false;
}

void CDuelWatchMainFrameWindow::ClosingProcess()
{
    m_bHasHPReceived = FALSE;
}

float CDuelWatchMainFrameWindow::GetLayerDepth()
{
    return 5.0f;
}

void CDuelWatchMainFrameWindow::BindRmlModel(Rml::DataModelConstructor& c, DuelWatchFrameRmlModel& model)
{
    c.Bind("scale_x", &model.scaleX);
    c.Bind("scale_y", &model.scaleY);
    c.Bind("inverse_scale_x", &model.inverseScaleX);
    c.Bind("inverse_scale_y", &model.inverseScaleY);
    c.Bind("watching", &model.watching);
    c.Bind("exit_hint", &model.exitHint);
    auto name = c.RegisterStruct<DuelWatchNameEntry>();
    name.RegisterMember("text", &DuelWatchNameEntry::text);
    name.RegisterMember("text_px", &DuelWatchNameEntry::textPx);
    c.Bind("hero_name", &model.heroName);
    c.Bind("enemy_name", &model.enemyName);
    c.RegisterArray<std::vector<float>>();
    c.Bind("score_marks", &model.scoreMarks);
    auto gauge = c.RegisterStruct<DuelWatchGaugeEntry>();
    gauge.RegisterMember("src", &DuelWatchGaugeEntry::src);
    gauge.RegisterMember("rect", &DuelWatchGaugeEntry::rect);
    gauge.RegisterMember("left", &DuelWatchGaugeEntry::left);
    gauge.RegisterMember("top", &DuelWatchGaugeEntry::top);
    gauge.RegisterMember("width", &DuelWatchGaugeEntry::width);
    gauge.RegisterMember("height", &DuelWatchGaugeEntry::height);
    gauge.RegisterMember("mirrored", &DuelWatchGaugeEntry::mirrored);
    c.RegisterArray<std::vector<DuelWatchGaugeEntry>>();
    c.Bind("gauges", &model.gauges);
    c.BindEventCallback("duel_watch_exit", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingExit = true; });
}

void CDuelWatchMainFrameWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

std::vector<DuelWatchGaugeEntry> CDuelWatchMainFrameWindow::StepGauges()
{
    // The original's Render(): the rates it last drew catch up with the received ones each frame,
    // faster the larger the change since the last packet (a 1/236 or 1/154 of the bar per step
    // and 5 more per whole bar changed).
    if (m_bHasHPReceived == FALSE || g_DuelMgr.GetFighterRegenerated())
    {
        m_bHasHPReceived = TRUE;
        g_DuelMgr.SetFighterRegenerated(FALSE);
        m_fPrevHPRate1 = m_fLastHPRate1 = m_fReceivedHPRate1 = g_DuelMgr.GetHP(DUEL_HERO);
        m_fPrevHPRate2 = m_fLastHPRate2 = m_fReceivedHPRate2 = g_DuelMgr.GetHP(DUEL_ENEMY);
        m_fPrevSDRate1 = m_fLastSDRate1 = m_fReceivedSDRate1 = g_DuelMgr.GetSD(DUEL_HERO);
        m_fPrevSDRate2 = m_fLastSDRate2 = m_fReceivedSDRate2 = g_DuelMgr.GetSD(DUEL_ENEMY);
    }

    if (m_fLastHPRate1 != g_DuelMgr.GetHP(DUEL_HERO) || m_fLastHPRate2 != g_DuelMgr.GetHP(DUEL_ENEMY) ||
        m_fLastSDRate1 != g_DuelMgr.GetSD(DUEL_HERO) || m_fLastSDRate2 != g_DuelMgr.GetSD(DUEL_ENEMY))
    {
        m_fLastHPRate1 = g_DuelMgr.GetHP(DUEL_HERO);
        m_fLastHPRate2 = g_DuelMgr.GetHP(DUEL_ENEMY);
        m_fLastSDRate1 = g_DuelMgr.GetSD(DUEL_HERO);
        m_fLastSDRate2 = g_DuelMgr.GetSD(DUEL_ENEMY);
        m_fReceivedHPRate1 = m_fPrevHPRate1;
        m_fReceivedHPRate2 = m_fPrevHPRate2;
        m_fReceivedSDRate1 = m_fPrevSDRate1;
        m_fReceivedSDRate2 = m_fPrevSDRate2;
    }

    auto step = [](float received, float rate, float length)
    { return 1.0f / length * static_cast<float>(int(absf(received - rate) * 5.0f) + 2); };

    std::vector<DuelWatchGaugeEntry> gauges;
    StepBar(gauges, m_fPrevHPRate1, g_DuelMgr.GetHP(DUEL_HERO),
            step(m_fReceivedHPRate1, g_DuelMgr.GetHP(DUEL_HERO), 236.f), HeroHp, kHpGauge, kHpGaugeFx);
    StepBar(gauges, m_fPrevHPRate2, g_DuelMgr.GetHP(DUEL_ENEMY),
            step(m_fReceivedHPRate2, g_DuelMgr.GetHP(DUEL_ENEMY), 236.f), EnemyHp, kHpGauge, kHpGaugeFx);
    StepBar(gauges, m_fPrevSDRate1, g_DuelMgr.GetSD(DUEL_HERO),
            step(m_fReceivedSDRate1, g_DuelMgr.GetSD(DUEL_HERO), 154.f), HeroSd, kSdGauge, kSdGaugeFx);
    // The original measured this step against the left fighter's shield (an obvious slip).
    StepBar(gauges, m_fPrevSDRate2, g_DuelMgr.GetSD(DUEL_ENEMY),
            step(m_fReceivedSDRate2, g_DuelMgr.GetSD(DUEL_ENEMY), 154.f), EnemySd, kSdGauge, kSdGaugeFx);

    // An empty bar draws nothing.
    gauges.erase(
        std::remove_if(gauges.begin(), gauges.end(), [](const DuelWatchGaugeEntry& g) { return g.width <= 0.f; }),
        gauges.end());
    return gauges;
}

void CDuelWatchMainFrameWindow::SyncView()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    // CManager scopes LayoutMode::Hud around the window: W/640 x H/480, no offset.
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    SyncField(m_RmlView.Binder(), &DuelWatchFrameRmlModel::scaleX, "scale_x", transform.scaleX);
    SyncField(m_RmlView.Binder(), &DuelWatchFrameRmlModel::scaleY, "scale_y", transform.scaleY);
    SyncField(m_RmlView.Binder(), &DuelWatchFrameRmlModel::inverseScaleX, "inverse_scale_x", 1.0f / transform.scaleX);
    SyncField(m_RmlView.Binder(), &DuelWatchFrameRmlModel::inverseScaleY, "inverse_scale_y", 1.0f / transform.scaleY);

    SyncField(m_RmlView.Binder(), &DuelWatchFrameRmlModel::exitHint, "exit_hint",
              StringUtils::WideToNarrow(I18N::Game::DuelFinished));

    const bool watching = g_DuelMgr.GetCurrentChannel() != -1;
    SyncField(m_RmlView.Binder(), &DuelWatchFrameRmlModel::watching, "watching", watching);
    if (!watching)
        return;

    // The fighters' names, bold, each shrunk to its own 55-unit box; the themes place them.
    g_pRenderText->SetFont(g_hFontBold);
    const auto name = [&](int player)
    {
        const wchar_t* text = g_DuelMgr.GetDuelPlayerID(player);
        const int width = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        return DuelWatchNameEntry{StringUtils::WideToNarrow(text),
                                  UI::Scaling::NativeTextPixelSizeInBox(UI::Scaling::FontRole::Bold, transform,
                                                                        static_cast<float>(width), 55.f)};
    };
    SyncField(m_RmlView.Binder(), &DuelWatchFrameRmlModel::heroName, "hero_name", name(DUEL_HERO));
    SyncField(m_RmlView.Binder(), &DuelWatchFrameRmlModel::enemyName, "enemy_name", name(DUEL_ENEMY));
    DuelWatchFrameRmlModel& model = m_RmlView.GetModel();

    // A mark per point: the left fighter's from x 57 rightwards, the right one's from x 566 leftwards.
    std::vector<float> scoreMarks;
    scoreMarks.reserve(
        static_cast<size_t>(std::max(0, g_DuelMgr.GetScore(DUEL_HERO) + g_DuelMgr.GetScore(DUEL_ENEMY))));
    for (int i = 0; i < g_DuelMgr.GetScore(DUEL_HERO); ++i)
        scoreMarks.push_back(57.f + 17.f * static_cast<float>(i));
    for (int i = 0; i < g_DuelMgr.GetScore(DUEL_ENEMY); ++i)
        scoreMarks.push_back(REFERENCE_WIDTH - 74.f - 17.f * static_cast<float>(i));
    if (model.scoreMarks != scoreMarks)
    {
        model.scoreMarks = std::move(scoreMarks);
        m_RmlView.MarkDirty("score_marks");
    }

    std::vector<DuelWatchGaugeEntry> gauges = StepGauges();
    const bool sameGauges = model.gauges.size() == gauges.size() &&
                            std::equal(model.gauges.begin(), model.gauges.end(), gauges.begin(),
                                       [](const DuelWatchGaugeEntry& a, const DuelWatchGaugeEntry& b)
                                       {
                                           return a.src == b.src && a.rect == b.rect && a.left == b.left &&
                                                  a.top == b.top && a.width == b.width && a.height == b.height &&
                                                  a.mirrored == b.mirrored;
                                       });
    if (!sameGauges)
    {
        model.gauges = std::move(gauges);
        m_RmlView.MarkDirty("gauges");
    }
}
