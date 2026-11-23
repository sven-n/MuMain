#include "stdafx.h"
#include "UI/Diagnostics/PingOverlay.h"

#include "UI/Placement/WindowPlacement.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/RmlBridge/RmlWorkspaceParticipant.h"

#include <memory>

namespace UI::Diagnostics::PingOverlay
{
namespace
{
struct PingModel
{
    bool measuring = false;
    bool unanswered = false;
    int milliseconds = 0;
};

using View = UI::RmlBridge::ThemedView<PingModel>;
std::unique_ptr<View> g_view;
bool g_shown = false;
// The text changed last frame: the panel has its new width now, and its corner is on the right.
bool g_resized = false;

void RegisterModel(Rml::DataModelConstructor& constructor, PingModel& model)
{
    constructor.Bind("measuring", &model.measuring);
    constructor.Bind("unanswered", &model.unanswered);
    constructor.Bind("ms", &model.milliseconds);
}

bool EnsureView()
{
    if (!g_view)
    {
        UI::RmlBridge::ThemedViewOptions options;
        options.afterBuild = [] { UI::Placement::Invalidate(); };
        g_view = std::make_unique<View>(
            "ping", RegisterModel, std::vector<UI::RmlBridge::ThemedDocumentSpec>{{"Data/Interface/RmlUi/ping.rml"}},
            std::move(options));
    }
    return g_view->Ensure();
}
} // namespace

void Initialize()
{
    UI::RmlBridge::RegisterWorkspaceDocument("ping", [] { return g_view ? g_view->Document() : nullptr; }, "panel");
}

void Update(const Network::RoundTripMeter::Reading& reading)
{
    if (!reading.shown && !g_shown)
        return;
    if (!EnsureView())
        return;

    const bool changed = reading.shown != g_shown || g_resized;
    g_resized = false;

    auto& model = g_view->GetModel();
    const PingModel next{!reading.milliseconds && !reading.unanswered, reading.unanswered,
                         reading.milliseconds.value_or(0)};
    if (next.measuring != model.measuring || next.unanswered != model.unanswered ||
        next.milliseconds != model.milliseconds)
    {
        model = next;
        g_view->MarkDirty("measuring");
        g_view->MarkDirty("unanswered");
        g_view->MarkDirty("ms");
        g_resized = true;
    }

    g_shown = reading.shown;
    UI::RmlBridge::SyncDocumentVisibilityInFront(g_view->Document(), g_shown);
    if (changed)
        UI::Placement::Invalidate();
    // Also works in the scenes where CSystem::Update does not run.
    if (g_shown || changed)
        UI::Placement::Update();
}

void Release()
{
    UI::Placement::UnregisterParticipant("ping");
    g_view.reset();
    g_shown = false;
    g_resized = false;
}
} // namespace UI::Diagnostics::PingOverlay
