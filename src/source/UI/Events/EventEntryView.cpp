#include "stdafx.h"

#include "UI/Events/EventEntryView.h"

#include "I18N/All.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace mu::ui::window;

namespace
{
constexpr int kTitleBoxWidth = 72;
constexpr int kLineBoxWidth = 190;

} // namespace

mu::ui::window::EventEntryView::EventEntryView(const char* modelName, const char* documentPath)
    : m_View(modelName, [this](Rml::DataModelConstructor& c, EventEntryRmlModel& model) { BindModel(c, model); },
             {{documentPath}}, {.stacking = UI::RmlBridge::ThemedStacking::Front})
{
}

void mu::ui::window::EventEntryView::BindModel(Rml::DataModelConstructor& c, EventEntryRmlModel& model)
{
    c.Bind("root_x", &model.rootX);
    c.Bind("root_y", &model.rootY);
    c.Bind("root_scale", &model.rootScale);
    c.Bind("text_px", &model.textPx);
    c.Bind("title_text_px", &model.titleTextPx);
    c.Bind("title_line_px", &model.titleLinePx);
    c.Bind("button_label_line_px", &model.buttonLabelLinePx);
    c.Bind("button_label_text_px", &model.buttonLabelTextPx);
    c.Bind("title_text", &model.titleText);
    c.Bind("exit_tooltip", &model.exitTooltip);

    auto line = c.RegisterStruct<EventEntryLineEntry>();
    line.RegisterMember("text", &EventEntryLineEntry::text);
    line.RegisterMember("text_px", &EventEntryLineEntry::textPx);
    c.RegisterArray<std::vector<EventEntryLineEntry>>();
    c.Bind("lines", &model.lines);

    auto button = c.RegisterStruct<EventEntryButtonEntry>();
    button.RegisterMember("label", &EventEntryButtonEntry::label);
    button.RegisterMember("enabled", &EventEntryButtonEntry::enabled);
    c.RegisterArray<std::vector<EventEntryButtonEntry>>();
    c.Bind("buttons", &model.buttons);

    c.BindEventCallback("entry_press",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                m_PressedButton = arguments[0].Get<int>(-1);
                        });
    c.BindEventCallback("entry_exit", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_ExitPressed = true; });
    model.exitTooltip = StringUtils::WideToNarrow(I18N::Game::Close388);
}

void mu::ui::window::EventEntryView::Build()
{
    m_View.Ensure();
}

void mu::ui::window::EventEntryView::SetContent(const wchar_t* title, const std::vector<std::wstring>& lines,
                                                const std::vector<Button>& buttons)
{
    Build();
    m_Title = title != nullptr ? title : L"";
    m_LineTexts = lines;

    EventEntryRmlModel& model = m_View.GetModel();
    model.titleText = StringUtils::WideToNarrow(m_Title.c_str());
    model.lines.clear();
    for (const std::wstring& text : lines)
        model.lines.push_back({StringUtils::WideToNarrow(text.c_str()), 0.f});
    model.buttons.clear();
    for (const Button& button : buttons)
        model.buttons.push_back({StringUtils::WideToNarrow(button.label.c_str()), button.enabled});
    m_View.MarkDirty("title_text");
    m_View.MarkDirty("lines");
    m_View.MarkDirty("buttons");
}

bool mu::ui::window::EventEntryView::PanelSize(float& width, float& height) const
{
    return UI::RmlBridge::RefreshLogicalPanelSize(m_View.Document(), "panel", width, height);
}

void mu::ui::window::EventEntryView::Sync(bool visible, const POINT& pos)
{
    Build();
    if (!m_View.Document())
        return;

    // Layer depth 4.0/4.1: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_View.Document(), visible);
    if (!visible)
        return;

    UI::RmlBridge::SyncRootTransform(m_View.Binder(), pos);
    UI::RmlBridge::SyncNativeTextSize(m_View.Binder());
    SyncTextSizes();
}

void mu::ui::window::EventEntryView::SyncTextSizes()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    EventEntryRmlModel& model = m_View.GetModel();

    // RenderText(x + 60, y + 12, title, 72, 0, RT3_SORT_CENTER), bold: shrunk to fit its box, its
    // top staying at y + 12.
    g_pRenderText->SetFont(g_hFontBold);
    const int titleWidth = g_pRenderText->MeasureText(m_Title.c_str(), static_cast<int>(m_Title.size())).cx;
    const float boldPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform);
    const float titlePx = UI::Scaling::NativeTextPixelSizeInBox(
        UI::Scaling::FontRole::Bold, transform, static_cast<float>(titleWidth), static_cast<float>(kTitleBoxWidth));
    SyncField(m_View.Binder(), &EventEntryRmlModel::titleTextPx, "title_text_px", titlePx);
    SyncField(m_View.Binder(), &EventEntryRmlModel::titleLinePx, "title_line_px",
              static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Bold)) * transform.scaleY *
                  (titlePx / boldPx));

    // The description lines: the normal font, each shrunk to the 190-unit box if wider.
    g_pRenderText->SetFont(g_hFont);
    bool linesChanged = false;
    for (std::size_t i = 0; i < model.lines.size() && i < m_LineTexts.size(); ++i)
    {
        const std::wstring& text = m_LineTexts[i];
        const int width = g_pRenderText->MeasureText(text.c_str(), static_cast<int>(text.size())).cx;
        const float px = UI::Scaling::NativeTextPixelSizeInBox(
            UI::Scaling::FontRole::Normal, transform, static_cast<float>(width), static_cast<float>(kLineBoxWidth));
        if (model.lines[i].textPx != px)
        {
            model.lines[i].textPx = px;
            linesChanged = true;
        }
    }
    if (linesChanged)
        m_View.MarkDirty("lines");

    // CButton::Render(): the bold label, centred on its button -- the same for every button, so
    // it is the model's, not each entry's.
    const int boldHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Bold);
    SyncField(m_View.Binder(), &EventEntryRmlModel::buttonLabelLinePx, "button_label_line_px",
              static_cast<float>(boldHeight) * transform.scaleY);
    SyncField(m_View.Binder(), &EventEntryRmlModel::buttonLabelTextPx, "button_label_text_px", boldPx);
}

int mu::ui::window::EventEntryView::TakePressedButton()
{
    const int pressed = m_PressedButton;
    m_PressedButton = -1;
    const EventEntryRmlModel& model = m_View.GetModel();
    if (pressed < 0 || pressed >= static_cast<int>(model.buttons.size()) || !model.buttons[pressed].enabled)
        return -1;
    return pressed;
}

bool mu::ui::window::EventEntryView::TakeExitPressed()
{
    const bool pressed = m_ExitPressed;
    m_ExitPressed = false;
    return pressed;
}
