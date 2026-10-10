#include "stdafx.h"

#include "UI/Events/EventEntryView.h"

#include "I18N/All.h"
#include "Core/Utilities/StringUtils.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace mu::ui::window;

mu::ui::window::EventEntryView::EventEntryView(const char* modelName, const char* documentPath)
    : m_View(modelName, [this](Rml::DataModelConstructor& c, EventEntryRmlModel& model) { BindModel(c, model); },
             {{documentPath}}, {.stacking = UI::RmlBridge::ThemedStacking::Front})
{
}

void mu::ui::window::EventEntryView::BindModel(Rml::DataModelConstructor& c, EventEntryRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("title_text", &model.titleText);
    c.Bind("exit_tooltip", &model.exitTooltip);

    auto line = c.RegisterStruct<EventEntryLineEntry>();
    line.RegisterMember("text", &EventEntryLineEntry::text);
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
    EventEntryRmlModel& model = m_View.GetModel();
    model.titleText = StringUtils::WideToNarrow(title != nullptr ? title : L"");
    model.lines.clear();
    for (const std::wstring& text : lines)
        model.lines.push_back({StringUtils::WideToNarrow(text.c_str())});
    model.buttons.clear();
    for (const Button& button : buttons)
        model.buttons.push_back({StringUtils::WideToNarrow(button.label.c_str()), button.enabled});
    m_View.MarkDirty("title_text");
    m_View.MarkDirty("lines");
    m_View.MarkDirty("buttons");
}

bool mu::ui::window::EventEntryView::IsPointerOver() const
{
    return UI::RmlBridge::IsPointerOver(m_View.Document());
}

void mu::ui::window::EventEntryView::Sync(bool visible)
{
    Build();
    if (!m_View.Document())
        return;

    // Layer depth 4.0/4.1: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_View.Document(), visible);
    if (!visible)
        return;

    SyncTextSizes();
}

void mu::ui::window::EventEntryView::SyncTextSizes()
{
    SyncField(m_View.Binder(), &EventEntryRmlModel::textPx, "text_px",
              UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Normal));
    SyncField(m_View.Binder(), &EventEntryRmlModel::boldTextPx, "bold_text_px",
              UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold));
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
