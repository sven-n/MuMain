#pragma once

#include "UI/Events/EventTimerRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <string>

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The RmlUi side of the event time HUDs that share the original's layout (CBloodCastle,
// CChaosCastleTime, CEmpireGuardianTimer): newui_Figure_blood (124 x 81), an optional first line
// at y 13, a second at y 38 and the time in the big font at y 50, each in its own colour. The original drew them under
// every panel (layer depth 1.2 / 1.3), and so does the document, like the duel and siege boards. Each window owns one, with its own document and data model; the window keeps its time, its
// counts and when it is shown.
class EventTimerView
{
public:
    // The document the workspace places (CObject::GetPlacedDocument()).
    Rml::ElementDocument* Document() const { return m_View.Document(); }
    EventTimerView(const char* modelName, const char* documentPath);

    void Build();
    // Unloads the document; from the window's own Release().
    void Release() { m_View.Release(); }

    struct Line
    {
        std::wstring text; // empty: not drawn
        // How pressing the line is, which every theme colours: "plain", "standby" before the
        // event starts, "normal", then "closing", "imminent" and "expiring" as the clock runs
        // down. The original set a colour per state directly.
        const char* state = "normal";
    };

    // Per frame, inside the window's CManager transform scope. Every line is centred on the box
    // (reference px from the frame's left) and shrunk to its width.
    void Sync(bool visible, const Line& first, const Line& second, const Line& time,
              float boxLeft = 0.f, float boxWidth = 124.f);

private:
    UI::RmlBridge::ThemedView<EventTimerRmlModel> m_View;
};
} // namespace mu::ui::window
