#pragma once

#include "UI/Events/EventTimerRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

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
// every panel (layer depth 1.2 / 1.3), so the document is in the background context, behind its other documents, like
// the duel and siege boards. Each window owns one, with its own document and data model; the window keeps its time, its
// counts and when it is shown.
class EventTimerView
{
public:
    EventTimerView(const char* modelName, const char* documentPath);

    void Build();
    void ReloadTheme();

    struct Line
    {
        std::wstring text;       // empty: not drawn
        unsigned long color = 0; // the native text colour (RGBA())
    };

    // Per frame, inside the window's CManager transform scope. Every line is centred on the box
    // (reference px from the frame's left) and shrunk to its width.
    void Sync(bool visible, const POINT& pos, const Line& first, const Line& second, const Line& time,
              float boxLeft = 0.f, float boxWidth = 124.f);

private:
    const char* m_ModelName;
    const char* m_DocumentPath;
    RmlModelBinder<EventTimerRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
};
} // namespace mu::ui::window
