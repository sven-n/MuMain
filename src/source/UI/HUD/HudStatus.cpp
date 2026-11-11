#include "stdafx.h"
#include "UI/HUD/HudStatus.h"

#include "Core/Utilities/StringUtils.h"
#include "Engine/Object/ZzzInterface.h"
#include "GameLogic/Events/MatchEvent.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/Placement/WindowPlacement.h"
#include "UI/Scaling/UITransform.h"

#include <string>

namespace UI::Hud
{
float UncoveredWorldCentreOnHudBoard()
{
    // The HUD board stands centred on the window's bottom at the HUD's scale.
    const float width = static_cast<float>(WindowWidth);
    const float scale = UI::Scaling::BottomHudScale(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
    const float centrePx = 0.5f * (UI::Placement::UncoveredWorldLeft() + UI::Placement::UncoveredWorldRight());
    return (centrePx - (0.5f * width - 320.f * scale)) / scale;
}

void StatusTexts::BindModel(Rml::DataModelConstructor& c, HudStatusRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("switch_first", &model.switchFirst);
    c.Bind("switch_second", &model.switchSecond);
    c.Bind("macro_visible", &model.macroVisible);
    c.Bind("macro_fraction", &model.macroFraction);
    c.Bind("world_centre", &model.worldCentre);
    c.Bind("countdown", &model.countdown);
}

void StatusTexts::Sync(bool visible, bool worldOverlays)
{
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

    std::wstring switches[2];
    if (worldOverlays)
        CollectCrownSwitchLines(switches);
    SyncField(binder, &HudStatusRmlModel::switchFirst, "switch_first",
              Rml::String(StringUtils::WideToNarrow(switches[0].c_str())));
    SyncField(binder, &HudStatusRmlModel::switchSecond, "switch_second",
              Rml::String(StringUtils::WideToNarrow(switches[1].c_str())));

    float macroFraction = 0.f;
    const bool macroVisible = MacroCooldownFraction(macroFraction);
    SyncField(binder, &HudStatusRmlModel::macroVisible, "macro_visible", macroVisible);
    SyncField(binder, &HudStatusRmlModel::macroFraction, "macro_fraction", macroVisible ? macroFraction : 0.f);
    if (macroVisible)
        SyncField(binder, &HudStatusRmlModel::worldCentre, "world_centre", UncoveredWorldCentreOnHudBoard());

    SyncField(binder, &HudStatusRmlModel::countdown, "countdown",
              Rml::String(StringUtils::WideToNarrow(matchEvent::CountdownText().c_str())));

    const HudStatusRmlModel& model = binder.GetModel();
    const bool anything = !model.switchFirst.empty() || !model.switchSecond.empty() || model.macroVisible ||
                          !model.countdown.empty();
    UI::RmlBridge::SyncDocumentVisibility(m_View.Document(), anything);
}

void StatusTexts::Hide()
{
    UI::RmlBridge::SyncDocumentVisibility(m_View.Document(), false);
}

void StatusTexts::Release()
{
    m_View.Release();
}
} // namespace UI::Hud
