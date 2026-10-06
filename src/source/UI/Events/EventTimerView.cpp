#include "stdafx.h"

#include "UI/Events/EventTimerView.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace mu::ui::window;

namespace
{
// RenderText(x, y, text, boxWidth, 0, RT3_SORT_CENTER) in `font`: the size it drew `text` at.
float TextPxInBox(UI::Scaling::FontRole role, HFONT font, const UI::Scaling::Transform& transform,
                  const std::wstring& text, float boxWidth)
{
    if (text.empty())
        return 0.f;
    g_pRenderText->SetFont(font);
    const int width = g_pRenderText->MeasureText(text.c_str(), static_cast<int>(text.size())).cx;
    return UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(width), boxWidth);
}

void BindTimerModel(Rml::DataModelConstructor& c, EventTimerRmlModel& model)
{
    c.Bind("scale_x", &model.scaleX);
    c.Bind("scale_y", &model.scaleY);
    c.Bind("inverse_scale_x", &model.inverseScaleX);
    c.Bind("inverse_scale_y", &model.inverseScaleY);
    c.Bind("panel_x", &model.panelX);
    c.Bind("panel_y", &model.panelY);
    c.Bind("box_left", &model.boxLeft);
    c.Bind("box_width", &model.boxWidth);
    c.Bind("kills_text", &model.killsText);
    c.Bind("kills_text_px", &model.killsTextPx);
    c.Bind("kills_state", &model.killsState);
    c.Bind("time_left_text", &model.timeLeftText);
    c.Bind("time_left_text_px", &model.timeLeftTextPx);
    c.Bind("time_left_state", &model.timeLeftState);
    c.Bind("time_text", &model.timeText);
    c.Bind("time_text_px", &model.timeTextPx);
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

void mu::ui::window::EventTimerView::Sync(bool visible, const POINT& pos, const Line& first, const Line& second,
                                          const Line& time, float boxLeft, float boxWidth)
{
    Build();
    if (!m_View.Document())
        return;

    UI::RmlBridge::SyncDocumentVisibilityBehind(m_View.Document(), visible);
    if (!visible)
        return;

    // CManager scopes LayoutMode::HudFrame around the window: the bottom HUD's uniform scale, no offset.
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    SyncField(m_View.Binder(), &EventTimerRmlModel::scaleX, "scale_x", transform.scaleX);
    SyncField(m_View.Binder(), &EventTimerRmlModel::scaleY, "scale_y", transform.scaleY);
    SyncField(m_View.Binder(), &EventTimerRmlModel::inverseScaleX, "inverse_scale_x", 1.0f / transform.scaleX);
    SyncField(m_View.Binder(), &EventTimerRmlModel::inverseScaleY, "inverse_scale_y", 1.0f / transform.scaleY);
    SyncField(m_View.Binder(), &EventTimerRmlModel::panelX, "panel_x", static_cast<float>(pos.x));
    SyncField(m_View.Binder(), &EventTimerRmlModel::panelY, "panel_y", static_cast<float>(pos.y));

    SyncField(m_View.Binder(), &EventTimerRmlModel::boxLeft, "box_left", boxLeft);
    SyncField(m_View.Binder(), &EventTimerRmlModel::boxWidth, "box_width", boxWidth);
    auto syncLine = [&](const Line& line, UI::Scaling::FontRole role, HFONT font,
                        Rml::String EventTimerRmlModel::* text, const char* textName, float EventTimerRmlModel::* px,
                        const char* pxName, Rml::String EventTimerRmlModel::* state, const char* stateName)
    {
        SyncField(m_View.Binder(), text, textName, StringUtils::WideToNarrow(line.text.c_str()));
        SyncField(m_View.Binder(), px, pxName, TextPxInBox(role, font, transform, line.text, boxWidth));
        SyncField(m_View.Binder(), state, stateName, Rml::String(line.state));
    };
    syncLine(first, UI::Scaling::FontRole::Normal, g_hFont, &EventTimerRmlModel::killsText, "kills_text",
             &EventTimerRmlModel::killsTextPx, "kills_text_px", &EventTimerRmlModel::killsState, "kills_state");
    syncLine(second, UI::Scaling::FontRole::Normal, g_hFont, &EventTimerRmlModel::timeLeftText, "time_left_text",
             &EventTimerRmlModel::timeLeftTextPx, "time_left_text_px", &EventTimerRmlModel::timeLeftState,
             "time_left_state");
    syncLine(time, UI::Scaling::FontRole::Big, g_hFontBig, &EventTimerRmlModel::timeText, "time_text",
             &EventTimerRmlModel::timeTextPx, "time_text_px", &EventTimerRmlModel::timeState, "time_state");
}
