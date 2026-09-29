#include "stdafx.h"

#include "UI/Events/EventTimerView.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace mu::ui::window;

namespace
{
Rml::Context* TimerContext()
{
    Rml::Context* context = RmlUiRuntime::Instance().GetBackgroundContext();
    return context != nullptr ? context : RmlUiRuntime::Instance().GetContext();
}

template <typename T>
void SyncField(RmlModelBinder<EventTimerRmlModel>& binder, T EventTimerRmlModel::* field, const char* name, T value)
{
    EventTimerRmlModel& model = binder.GetModel();
    if (model.*field == value)
        return;
    model.*field = std::move(value);
    binder.MarkDirty(name);
}

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
} // namespace

mu::ui::window::EventTimerView::EventTimerView(const char* modelName, const char* documentPath)
    : m_ModelName(modelName), m_DocumentPath(documentPath)
{
}

void mu::ui::window::EventTimerView::Build()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(TimerContext(), m_ModelName,
                                                 [](Rml::DataModelConstructor& c, EventTimerRmlModel& model)
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
                                                     c.Bind("kills_color", &model.killsColor);
                                                     c.Bind("time_left_text", &model.timeLeftText);
                                                     c.Bind("time_left_text_px", &model.timeLeftTextPx);
                                                     c.Bind("time_left_color", &model.timeLeftColor);
                                                     c.Bind("time_text", &model.timeText);
                                                     c.Bind("time_text_px", &model.timeTextPx);
                                                     c.Bind("time_color", &model.timeColor);
                                                 });
    if (modelCreated)
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(TimerContext(), m_DocumentPath);
}

void mu::ui::window::EventTimerView::ReloadTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = TimerContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    Build();
}

void mu::ui::window::EventTimerView::Sync(bool visible, const POINT& pos, const Line& first, const Line& second,
                                          const Line& time, float boxLeft, float boxWidth)
{
    Build();
    if (!m_pRmlDoc)
        return;

    UI::RmlBridge::SyncDocumentVisibilityBehind(m_pRmlDoc, visible);
    if (!visible)
        return;

    // CManager scopes LayoutMode::Hud around the window: W/640 x H/480, no offset.
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    SyncField(m_RmlBinder, &EventTimerRmlModel::scaleX, "scale_x", transform.scaleX);
    SyncField(m_RmlBinder, &EventTimerRmlModel::scaleY, "scale_y", transform.scaleY);
    SyncField(m_RmlBinder, &EventTimerRmlModel::inverseScaleX, "inverse_scale_x", 1.0f / transform.scaleX);
    SyncField(m_RmlBinder, &EventTimerRmlModel::inverseScaleY, "inverse_scale_y", 1.0f / transform.scaleY);
    SyncField(m_RmlBinder, &EventTimerRmlModel::panelX, "panel_x", static_cast<float>(pos.x));
    SyncField(m_RmlBinder, &EventTimerRmlModel::panelY, "panel_y", static_cast<float>(pos.y));

    SyncField(m_RmlBinder, &EventTimerRmlModel::boxLeft, "box_left", boxLeft);
    SyncField(m_RmlBinder, &EventTimerRmlModel::boxWidth, "box_width", boxWidth);
    auto syncLine = [&](const Line& line, UI::Scaling::FontRole role, HFONT font,
                        Rml::String EventTimerRmlModel::* text, const char* textName, float EventTimerRmlModel::* px,
                        const char* pxName, Rml::String EventTimerRmlModel::* color, const char* colorName)
    {
        SyncField(m_RmlBinder, text, textName, StringUtils::WideToNarrow(line.text.c_str()));
        SyncField(m_RmlBinder, px, pxName, TextPxInBox(role, font, transform, line.text, boxWidth));
        SyncField(m_RmlBinder, color, colorName, UI::RmlBridge::RgbaToCss(line.color));
    };
    syncLine(first, UI::Scaling::FontRole::Normal, g_hFont, &EventTimerRmlModel::killsText, "kills_text",
             &EventTimerRmlModel::killsTextPx, "kills_text_px", &EventTimerRmlModel::killsColor, "kills_color");
    syncLine(second, UI::Scaling::FontRole::Normal, g_hFont, &EventTimerRmlModel::timeLeftText, "time_left_text",
             &EventTimerRmlModel::timeLeftTextPx, "time_left_text_px", &EventTimerRmlModel::timeLeftColor,
             "time_left_color");
    syncLine(time, UI::Scaling::FontRole::Big, g_hFontBig, &EventTimerRmlModel::timeText, "time_text",
             &EventTimerRmlModel::timeTextPx, "time_text_px", &EventTimerRmlModel::timeColor, "time_color");
}
