#include "stdafx.h"

#include "UI/Events/EventTimerView.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace mu::ui::window;

namespace
{
void BindTimerModel(Rml::DataModelConstructor& c, EventTimerRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("big_text_px", &model.bigTextPx);
    c.Bind("kills_text", &model.killsText);
    c.Bind("kills_state", &model.killsState);
    c.Bind("time_left_text", &model.timeLeftText);
    c.Bind("time_left_state", &model.timeLeftState);
    c.Bind("time_text", &model.timeText);
    c.Bind("time_state", &model.timeState);
}
} // namespace

mu::ui::window::EventTimerView::EventTimerView(const char* modelName, const char* documentPath)
    : m_View(modelName, BindTimerModel, {{documentPath}}, {.stacking = UI::RmlBridge::ThemedStacking::Back})
{
}

void mu::ui::window::EventTimerView::Build()
{
    m_View.Ensure();
}

void mu::ui::window::EventTimerView::Sync(bool visible, const Line& first, const Line& second, const Line& time)
{
    Build();
    if (!m_View.Document())
        return;

    UI::RmlBridge::SyncDocumentVisibilityBehind(m_View.Document(), visible);
    if (!visible)
        return;


    SyncField(m_View.Binder(), &EventTimerRmlModel::textPx, "text_px",
              UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Normal));
    SyncField(m_View.Binder(), &EventTimerRmlModel::bigTextPx, "big_text_px",
              UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Big));
    auto syncLine = [&](const Line& line, Rml::String EventTimerRmlModel::* text, const char* textName,
                        Rml::String EventTimerRmlModel::* state, const char* stateName)
    {
        SyncField(m_View.Binder(), text, textName, StringUtils::WideToNarrow(line.text.c_str()));
        SyncField(m_View.Binder(), state, stateName, Rml::String(line.state));
    };
    syncLine(first, &EventTimerRmlModel::killsText, "kills_text", &EventTimerRmlModel::killsState, "kills_state");
    syncLine(second, &EventTimerRmlModel::timeLeftText, "time_left_text", &EventTimerRmlModel::timeLeftState,
             "time_left_state");
    syncLine(time, &EventTimerRmlModel::timeText, "time_text", &EventTimerRmlModel::timeState, "time_state");
}
