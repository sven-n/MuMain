#include "stdafx.h"
#include "UI/HUD/MatchStatus.h"

#include "Core/Utilities/StringUtils.h"
#include "I18N/All.h"
#include "Network/Server/WSclient.h"
#include "UI/HUD/HudStatus.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlSyncField.h"

#include <cwchar>
#include <string>

namespace
{
Rml::String Narrow(const wchar_t* text)
{
    return StringUtils::WideToNarrow(text);
}

// A team name the packet fills to its full width, unterminated.
Rml::String TeamName(const wchar_t (&name)[MAX_USERNAME_SIZE])
{
    return Narrow(std::wstring(name, wcsnlen(name, MAX_USERNAME_SIZE)).c_str());
}
} // namespace

namespace UI::Hud
{
void MatchStatus::BindModel(Rml::DataModelConstructor& c, MatchStatusRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("big_text_px", &model.bigTextPx);
    c.Bind("countdown", &model.countdown);
    c.Bind("countdown_soon", &model.countdownSoon);
    c.Bind("world_centre", &model.worldCentre);
    c.Bind("result_shown", &model.resultShown);
    c.Bind("title", &model.title);
    c.Bind("vs", &model.vs);
    c.Bind("team1", &model.team1);
    c.Bind("team2", &model.team2);
    c.Bind("score1", &model.score1);
    c.Bind("score2", &model.score2);
    c.Bind("outcome1", &model.outcome1);
    c.Bind("outcome2", &model.outcome2);
    c.Bind("tie_text", &model.tieText);
    c.Bind("tie", &model.tie);
    c.Bind("team1_won", &model.team1Won);
    c.BindEventCallback("match_result_close",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { m_CloseRequested = true; });
}

void MatchStatus::Sync(bool visible)
{
    // The original's OK forgot the result and the time.
    if (m_CloseRequested)
    {
        m_CloseRequested = false;
        g_wtMatchResult.Clear();
        g_wtMatchTimeLeft.m_Time = 0;
    }

    if (!visible)
    {
        Hide();
        return;
    }

    m_View.Ensure();
    if (m_View.Document() == nullptr)
        return;

    auto& binder = m_View.Binder();
    UI::RmlBridge::SyncNativeTextSize(binder);
    SyncField(binder, &MatchStatusRmlModel::bigTextPx, "big_text_px",
              UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Big));

    // The time, only while ten minutes or less remain.
    Rml::String countdown;
    bool soon = false;
    const int time = g_wtMatchTimeLeft.m_Time;
    const int minutes = time / 60;
    const int seconds = time % 60;
    if (time != 0 && minutes <= 10)
    {
        wchar_t text[64] = {};
        if (g_wtMatchTimeLeft.m_Type == 3)
        {
            soon = true;
            mu_swprintf(text, I18N::Game::ItWillStartAfterDSeconds, seconds);
        }
        else
        {
            soon = time < 60;
            mu_swprintf(text, seconds < 10 ? I18N::Game::RemainingHoursD0D : I18N::Game::RemainingSecondsDD, minutes,
                        seconds);
        }
        countdown = Narrow(text);
    }
    SyncField(binder, &MatchStatusRmlModel::countdown, "countdown", countdown);
    SyncField(binder, &MatchStatusRmlModel::countdownSoon, "countdown_soon", soon);
    if (!countdown.empty())
        SyncField(binder, &MatchStatusRmlModel::worldCentre, "world_centre", UncoveredWorldCentreOnHudBoard());

    // The result, until OK.
    const bool resultShown = g_wtMatchResult.m_MatchTeamName1[0] != L'\0';
    SyncField(binder, &MatchStatusRmlModel::resultShown, "result_shown", resultShown);
    if (resultShown)
    {
        const int score1 = g_wtMatchResult.m_Score1;
        const int score2 = g_wtMatchResult.m_Score2;
        SyncField(binder, &MatchStatusRmlModel::title, "title", Narrow(I18N::Game::TournamentResult));
        SyncField(binder, &MatchStatusRmlModel::vs, "vs", Narrow(I18N::Game::VS));
        SyncField(binder, &MatchStatusRmlModel::team1, "team1", TeamName(g_wtMatchResult.m_MatchTeamName1));
        SyncField(binder, &MatchStatusRmlModel::team2, "team2", TeamName(g_wtMatchResult.m_MatchTeamName2));
        SyncField(binder, &MatchStatusRmlModel::score1, "score1", Rml::String(std::to_string(score1)));
        SyncField(binder, &MatchStatusRmlModel::score2, "score2", Rml::String(std::to_string(score2)));
        SyncField(binder, &MatchStatusRmlModel::tie, "tie", score1 == score2);
        SyncField(binder, &MatchStatusRmlModel::team1Won, "team1_won", score1 > score2);
        SyncField(binder, &MatchStatusRmlModel::tieText, "tie_text", Narrow(I18N::Game::Tie));
        SyncField(binder, &MatchStatusRmlModel::outcome1, "outcome1",
                  Narrow(score1 > score2 ? I18N::Game::Win : I18N::Game::Lose));
        SyncField(binder, &MatchStatusRmlModel::outcome2, "outcome2",
                  Narrow(score1 > score2 ? I18N::Game::Lose : I18N::Game::Win));
    }

    UI::RmlBridge::SyncDocumentVisibility(m_View.Document(), resultShown || !countdown.empty());
}

void MatchStatus::Hide()
{
    UI::RmlBridge::SyncDocumentVisibility(m_View.Document(), false);
}

void MatchStatus::Release()
{
    m_View.Release();
}
} // namespace UI::Hud
