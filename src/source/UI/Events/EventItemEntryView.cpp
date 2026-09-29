#include "stdafx.h"

#include "UI/Events/EventItemEntryView.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

using namespace mu::ui::window;

namespace
{
constexpr int kButtonHeight = 23;
} // namespace

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
            c.Bind("label_top", &model.labelTop);
            c.Bind("label_line_px", &model.labelLinePx);

            auto text = c.RegisterStruct<EventItemEntryTextEntry>();
            text.RegisterMember("text", &EventItemEntryTextEntry::text);
            text.RegisterMember("left", &EventItemEntryTextEntry::left);
            text.RegisterMember("top", &EventItemEntryTextEntry::top);
            text.RegisterMember("width", &EventItemEntryTextEntry::width);
            text.RegisterMember("text_px", &EventItemEntryTextEntry::textPx);
            text.RegisterMember("bold", &EventItemEntryTextEntry::bold);
            text.RegisterMember("color", &EventItemEntryTextEntry::color);
            c.RegisterArray<std::vector<EventItemEntryTextEntry>>();
            c.Bind("texts", &model.texts);

            auto button = c.RegisterStruct<EventItemEntryButtonEntry>();
            button.RegisterMember("label", &EventItemEntryButtonEntry::label);
            button.RegisterMember("index", &EventItemEntryButtonEntry::index);
            button.RegisterMember("locked", &EventItemEntryButtonEntry::locked);
            button.RegisterMember("left", &EventItemEntryButtonEntry::left);
            button.RegisterMember("top", &EventItemEntryButtonEntry::top);
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

    SetButtons(m_Buttons);
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
    if (!m_pRmlDoc)
        return;

    std::vector<EventItemEntryButtonEntry> entries;
    for (std::size_t i = 0; i < buttons.size(); ++i)
    {
        EventItemEntryButtonEntry entry;
        entry.label = StringUtils::WideToNarrow(buttons[i].label.c_str());
        entry.index = static_cast<int>(i);
        entry.locked = buttons[i].locked;
        entry.left = buttons[i].left;
        entry.top = buttons[i].top;
        entries.push_back(std::move(entry));
    }
    EventItemEntryRmlModel& model = m_RmlBinder.GetModel();
    const bool same =
        model.buttons.size() == entries.size() &&
        std::equal(model.buttons.begin(), model.buttons.end(), entries.begin(),
                   [](const EventItemEntryButtonEntry& a, const EventItemEntryButtonEntry& b)
                   { return a.label == b.label && a.locked == b.locked && a.left == b.left && a.top == b.top; });
    if (same)
        return;
    model.buttons = std::move(entries);
    m_RmlBinder.MarkDirty("buttons");
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
}

void mu::ui::window::EventItemEntryView::SyncTexts()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    EventItemEntryRmlModel& model = m_RmlBinder.GetModel();

    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    // CButton::Render()'s whole-unit centring.
    const int labelTopUnits = kButtonHeight / 2 - lineHeight / 2;
    const float labelTop = static_cast<float>(labelTopUnits);
    const float labelLinePx = static_cast<float>(lineHeight) * transform.scaleY;
    if (model.labelTop != labelTop || model.labelLinePx != labelLinePx)
    {
        model.labelTop = labelTop;
        model.labelLinePx = labelLinePx;
        m_RmlBinder.MarkDirty("label_top");
        m_RmlBinder.MarkDirty("label_line_px");
    }

    // RenderText(x, y, text, width, 0, RT3_SORT_CENTER): shrunk to its box if wider.
    std::vector<EventItemEntryTextEntry> texts;
    for (const Text& text : m_Texts)
    {
        const UI::Scaling::FontRole role = text.bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        g_pRenderText->SetFont(text.bold ? g_hFontBold : g_hFont);
        const int width = g_pRenderText->MeasureText(text.text.c_str(), static_cast<int>(text.text.size())).cx;
        texts.push_back({StringUtils::WideToNarrow(text.text.c_str()), text.left, text.top, text.width,
                         UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(width), text.width),
                         text.bold, UI::RmlBridge::RgbaToCss(text.color)});
    }
    const bool same = model.texts.size() == texts.size() &&
                      std::equal(model.texts.begin(), model.texts.end(), texts.begin(),
                                 [](const EventItemEntryTextEntry& a, const EventItemEntryTextEntry& b)
                                 {
                                     return a.text == b.text && a.left == b.left && a.top == b.top &&
                                            a.width == b.width && a.textPx == b.textPx && a.bold == b.bold &&
                                            a.color == b.color;
                                 });
    if (same)
        return;
    model.texts = std::move(texts);
    m_RmlBinder.MarkDirty("texts");
}

int mu::ui::window::EventItemEntryView::TakePressedButton()
{
    const int pressed = m_PressedButton;
    m_PressedButton = -1;
    if (pressed < 0 || pressed >= static_cast<int>(m_Buttons.size()) || m_Buttons[pressed].locked)
        return -1;
    return pressed;
}
