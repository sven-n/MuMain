#pragma once

#include "UI/HUD/HudStatusRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace UI::Hud
{
// The centre of the world the open docks leave uncovered, in the HUD board's reference px.
float UncoveredWorldCentreOnHudBoard();

// The status texts the original drew over the world above the HUD: the crown switch lines, the
// macro cooldown and an event's entry countdown. One document on the HUD board; the game state
// fills it once a frame.
class StatusTexts
{
public:
    // Once a frame while the main scene draws its labels; `worldOverlays` is false in the top view,
    // where the original left the crown switch lines out.
    void Sync(bool visible, bool worldOverlays);
    void Hide();
    void Release();

private:
    static void BindModel(Rml::DataModelConstructor& c, HudStatusRmlModel& model);

    UI::RmlBridge::ThemedView<HudStatusRmlModel> m_View{"hud_status", BindModel,
                                                        {{"Data/Interface/RmlUi/hud_status.rml"}}};
};
} // namespace UI::Hud
