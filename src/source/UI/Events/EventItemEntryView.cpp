#include "stdafx.h"

#include "UI/Events/EventItemEntryView.h"

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

mu::ui::window::EventItemEntryView::EventItemEntryView(const char* modelName, const char* documentPath,
                                                       const char* bgModelName, const char* bgDocumentPath)
    : m_ModelName(modelName), m_DocumentPath(documentPath), m_BgModelName(bgModelName), m_BgDocumentPath(bgDocumentPath)
{
}

void mu::ui::window::EventItemEntryView::Build()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), m_ModelName,
        [this](Rml::DataModelConstructor& c, EventItemEntryRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("bold_text_px", &model.boldTextPx);
            c.Bind("input_value", &model.inputValue);

            auto text = c.RegisterStruct<EventItemEntryTextEntry>();
            text.RegisterMember("text", &EventItemEntryTextEntry::text);
            text.RegisterMember("text_px", &EventItemEntryTextEntry::textPx);
            c.RegisterArray<std::vector<EventItemEntryTextEntry>>();
            c.Bind("texts", &model.texts);

            auto button = c.RegisterStruct<EventItemEntryButtonEntry>();
            button.RegisterMember("label", &EventItemEntryButtonEntry::label);
            button.RegisterMember("locked", &EventItemEntryButtonEntry::locked);
            button.RegisterMember("bold", &EventItemEntryButtonEntry::bold);
            button.RegisterMember("label_line_px", &EventItemEntryButtonEntry::labelLinePx);
            c.RegisterArray<std::vector<EventItemEntryButtonEntry>>();
            c.Bind("buttons", &model.buttons);

            c.BindEventCallback("item_entry_press",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                                {
                                    if (arguments.size() == 1)
                                        m_PressedButton = arguments[0].Get<int>(-1);
                                });
        });
    if (!modelCreated)
        return;
    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), m_DocumentPath);

    if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
    {
        const bool bgModelCreated =
            m_BgRmlBinder.Create(bgContext, m_BgModelName,
                                 [](Rml::DataModelConstructor& c, EventItemEntryBgRmlModel& model)
                                 {
                                     c.Bind("root_x", &model.rootX);
                                     c.Bind("root_y", &model.rootY);
                                     c.Bind("root_scale", &model.rootScale);
                                 });
        if (bgModelCreated)
            m_pRmlBgDoc = UI::RmlBridge::CreateBackgroundDocument(m_BgDocumentPath);
    }
}

void mu::ui::window::EventItemEntryView::ReloadTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;
    if (m_pRmlBgDoc)
    {
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            m_BgRmlBinder.Destroy(bgContext);
            bgContext->UnloadDocument(m_pRmlBgDoc);
        }
        m_pRmlBgDoc = nullptr;
    }

    Build();
}

void mu::ui::window::EventItemEntryView::SetTexts(std::vector<Text> texts)
{
    m_Texts = std::move(texts);
}

void mu::ui::window::EventItemEntryView::SetButtons(const std::vector<Button>& buttons)
{
    m_Buttons = buttons;
}

void mu::ui::window::EventItemEntryView::Sync(bool visible, const POINT& pos)
{
    Build();
    if (!m_pRmlDoc)
        return;

    // The frame: RenderBackgroundLayer() paints whatever is shown in the background context, so
    // this is what hides it with the window.
    if (m_pRmlBgDoc)
    {
        UI::RmlBridge::SyncDocumentVisibility(m_pRmlBgDoc, visible);
        if (visible)
            UI::RmlBridge::SyncRootTransform(m_BgRmlBinder, pos);
    }

    // Texts and buttons: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, visible);
    if (!visible)
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);
    SyncTexts();
    SyncButtons();
}

void mu::ui::window::EventItemEntryView::SyncTexts()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    EventItemEntryRmlModel& model = m_RmlBinder.GetModel();

    // RenderText(x, y, text, width, 0, RT3_SORT_CENTER): shrunk to its box if wider.
    std::vector<EventItemEntryTextEntry> texts;
    for (const Text& text : m_Texts)
    {
        const UI::Scaling::FontRole role = text.bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        g_pRenderText->SetFont(text.bold ? g_hFontBold : g_hFont);
        const int width = g_pRenderText->MeasureText(text.text.c_str(), static_cast<int>(text.text.size())).cx;
        texts.push_back(
            {StringUtils::WideToNarrow(text.text.c_str()),
             UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(width), text.width)});
    }
    if (model.texts == texts)
        return;
    model.texts = std::move(texts);
    m_RmlBinder.MarkDirty("texts");
}

void mu::ui::window::EventItemEntryView::SyncButtons()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    EventItemEntryRmlModel& model = m_RmlBinder.GetModel();

    const float boldTextPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform);
    if (model.boldTextPx != boldTextPx)
    {
        model.boldTextPx = boldTextPx;
        m_RmlBinder.MarkDirty("bold_text_px");
    }

    std::vector<EventItemEntryButtonEntry> entries;
    for (std::size_t i = 0; i < m_Buttons.size(); ++i)
    {
        const Button& button = m_Buttons[i];
        const int lineHeight =
            CUIRenderTextSDLTtf::LineHeight(button.bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal);
        EventItemEntryButtonEntry entry;
        entry.label = StringUtils::WideToNarrow(button.label.c_str());
        entry.locked = button.locked;
        entry.bold = button.bold;
        entry.labelLinePx = static_cast<float>(lineHeight) * transform.scaleY;
        entries.push_back(std::move(entry));
    }
    if (model.buttons == entries)
        return;
    model.buttons = std::move(entries);
    m_RmlBinder.MarkDirty("buttons");
}

int mu::ui::window::EventItemEntryView::TakePressedButton()
{
    const int pressed = m_PressedButton;
    m_PressedButton = -1;
    if (pressed < 0 || pressed >= static_cast<int>(m_Buttons.size()) || m_Buttons[pressed].locked)
        return -1;
    return pressed;
}

Rml::Element* mu::ui::window::EventItemEntryView::GetElementById(const char* id) const
{
    return m_pRmlDoc != nullptr ? m_pRmlDoc->GetElementById(id) : nullptr;
}

const Rml::String& mu::ui::window::EventItemEntryView::InputValue() const
{
    return m_RmlBinder.GetModel().inputValue;
}

void mu::ui::window::EventItemEntryView::SetInputValue(const Rml::String& value)
{
    SyncField(m_RmlBinder, &EventItemEntryRmlModel::inputValue, "input_value", Rml::String(value));
}

void mu::ui::window::EventItemEntryView::RefreshPanelSize(float& width, float& height) const
{
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", width, height);
}
