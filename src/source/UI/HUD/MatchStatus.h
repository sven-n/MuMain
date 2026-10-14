#pragma once

#include "UI/HUD/MatchStatusRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace UI::Hud
{
// A guild war's or battle soccer's time and result, which the original drew over the world
// (RenderTournamentInterface()): the countdown above the HUD and the result panel with its OK
// button, which forgets both. The server's g_wtMatchTimeLeft and g_wtMatchResult fill it.
class MatchStatus
{
public:
    // Once a frame while the main scene draws its labels.
    void Sync(bool visible);
    void Hide();
    void Release();

private:
    void BindModel(Rml::DataModelConstructor& c, MatchStatusRmlModel& model);

    UI::RmlBridge::ThemedView<MatchStatusRmlModel> m_View{
        "match_status", [this](Rml::DataModelConstructor& c, MatchStatusRmlModel& model) { BindModel(c, model); },
        {{"Data/Interface/RmlUi/match_status.rml"}}};
    bool m_CloseRequested = false;
};
} // namespace UI::Hud
