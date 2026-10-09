#include "stdafx.h"

#include "UI/Dialogs/MessageBoxView.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

using namespace mu::ui::window;

namespace
{
bool SameLine(const MessageBoxViewLineEntry& a, const MessageBoxViewLineEntry& b)
{
    return a.text == b.text && a.left == b.left && a.top == b.top && a.bold == b.bold && a.color == b.color &&
           a.textPx == b.textPx;
}

bool SameButton(const MessageBoxViewButtonEntry& a, const MessageBoxViewButtonEntry& b)
{
    return a.label == b.label && a.index == b.index && a.left == b.left && a.top == b.top && a.width == b.width &&
           a.height == b.height && a.enabled == b.enabled && a.okArt == b.okArt;
}
} // namespace

mu::ui::window::MessageBoxView::MessageBoxView()
    // One document and model name: a second box while one is open gets no view (CreateDataModel()
    // refuses the name), which no caller does.
    : m_View("message_box_view",
             [this](Rml::DataModelConstructor& c, MessageBoxViewRmlModel& model) { BindModel(c, model); },
             {{"Data/Interface/RmlUi/message_box_view.rml"}}, {.stacking = UI::RmlBridge::ThemedStacking::Front})
{
}

mu::ui::window::MessageBoxView::~MessageBoxView()
{
    Destroy();
}

void mu::ui::window::MessageBoxView::BindModel(Rml::DataModelConstructor& c, MessageBoxViewRmlModel& model)
{
    c.Bind("root_x", &model.rootX);
    c.Bind("root_y", &model.rootY);
    c.Bind("root_scale", &model.rootScale);
    c.Bind("kind", &model.kind);
    c.Bind("text_px", &model.textPx);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("back_height", &model.backHeight);
    c.RegisterArray<std::vector<float>>();
    if (auto strip = c.RegisterStruct<MessageBoxViewStripEntry>())
    {
        strip.RegisterMember("kind", &MessageBoxViewStripEntry::kind);
    }
    c.RegisterArray<std::vector<MessageBoxViewStripEntry>>();
    c.Bind("strips", &model.strips);
    c.Bind("separators", &model.separators);
    c.Bind("progress_shown", &model.progressShown);
    c.Bind("progress_top", &model.progressTop);
    c.Bind("progress_width", &model.progressWidth);

    auto line = c.RegisterStruct<MessageBoxViewLineEntry>();
    line.RegisterMember("text", &MessageBoxViewLineEntry::text);
    line.RegisterMember("left", &MessageBoxViewLineEntry::left);
    line.RegisterMember("top", &MessageBoxViewLineEntry::top);
    line.RegisterMember("bold", &MessageBoxViewLineEntry::bold);
    line.RegisterMember("color", &MessageBoxViewLineEntry::color);
    line.RegisterMember("text_px", &MessageBoxViewLineEntry::textPx);
    c.RegisterArray<std::vector<MessageBoxViewLineEntry>>();
    c.Bind("lines", &model.lines);

    auto button = c.RegisterStruct<MessageBoxViewButtonEntry>();
    button.RegisterMember("label", &MessageBoxViewButtonEntry::label);
    button.RegisterMember("index", &MessageBoxViewButtonEntry::index);
    button.RegisterMember("left", &MessageBoxViewButtonEntry::left);
    button.RegisterMember("top", &MessageBoxViewButtonEntry::top);
    button.RegisterMember("width", &MessageBoxViewButtonEntry::width);
    button.RegisterMember("height", &MessageBoxViewButtonEntry::height);
    button.RegisterMember("enabled", &MessageBoxViewButtonEntry::enabled);
    button.RegisterMember("ok_art", &MessageBoxViewButtonEntry::okArt);
    c.RegisterArray<std::vector<MessageBoxViewButtonEntry>>();
    c.Bind("buttons", &model.buttons);
    c.Bind("placed_buttons", &model.placedButtons);

    BindList(c, model);

    c.BindEventCallback("message_box_button",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                m_PressedButton = arguments[0].Get<int>(-1);
                        });
}

void mu::ui::window::MessageBoxView::Create(int middleCount, float backHeight, const char* kind)
{
    if (m_View.Document() != nullptr || !RmlUiRuntime::Instance().IsCreated())
        return;

    m_View.GetModel().kind = kind;
    SetFrame(middleCount, backHeight);
    m_View.Ensure();
}

void mu::ui::window::MessageBoxView::SetFrame(int middleCount, float backHeight, int middlesAboveDivider)
{
    // The strips in order: middleCount plain ones, with the divider after the first
    // middlesAboveDivider of them (or last, when that count reaches the end). Each strip's height
    // and the stacking are the theme's -- these say only what is there and in what order.
    std::vector<MessageBoxViewStripEntry> strips;
    strips.reserve(static_cast<size_t>(middleCount) + 1);
    for (int i = 0; i < middleCount; ++i)
    {
        if (i == middlesAboveDivider)
            strips.push_back({"divider"});
        strips.push_back({"middle"});
    }
    if (middlesAboveDivider >= middleCount)
        strips.push_back({"divider"});

    MessageBoxViewRmlModel& model = m_View.GetModel();
    const bool sameStrips =
        model.strips.size() == strips.size() &&
        std::equal(model.strips.begin(), model.strips.end(), strips.begin(),
                   [](const MessageBoxViewStripEntry& a, const MessageBoxViewStripEntry& b)
                   { return a.kind == b.kind; });
    if (model.middleCount == middleCount && model.backHeight == backHeight && sameStrips)
        return;
    model.middleCount = middleCount;
    model.backHeight = backHeight;
    model.strips = std::move(strips);
    m_View.MarkDirty("back_height");
    m_View.MarkDirty("strips");
}

void mu::ui::window::MessageBoxView::SetSeparators(const std::vector<float>& tops)
{
    MessageBoxViewRmlModel& model = m_View.GetModel();
    if (model.separators == tops)
        return;
    model.separators = tops;
    m_View.MarkDirty("separators");
}

void mu::ui::window::MessageBoxView::SetProgress(float top, float fraction)
{
    MessageBoxViewRmlModel& model = m_View.GetModel();
    const bool shown = top >= 0.f;
    // RenderProgress(): the fill 150 units wide at the full fraction, not clamped.
    const float width = 150.f * fraction;
    if (model.progressShown != shown)
    {
        model.progressShown = shown;
        m_View.MarkDirty("progress_shown");
    }
    if (model.progressTop != top)
    {
        model.progressTop = top;
        m_View.MarkDirty("progress_top");
    }
    if (model.progressWidth != width)
    {
        model.progressWidth = width;
        m_View.MarkDirty("progress_width");
    }
}

void mu::ui::window::MessageBoxView::Destroy()
{
    m_PressedButton = -1;
    m_PressedListRow = -1;
    m_View.Release();
}

void mu::ui::window::MessageBoxView::Sync(const POINT& pos, const std::vector<Line>& lines,
                                          const std::vector<Button>& buttons)
{
    if (!m_View.Document())
        return;

    // Message boxes draw over every window: in front of the other documents.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_View.Document(), true);
    UI::RmlBridge::SyncRootTransform(m_View.Binder(), pos);
    UI::RmlBridge::SyncNativeTextSize(m_View.Binder());

    MessageBoxViewRmlModel& model = m_View.GetModel();
    const float boldPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold);
    if (model.boldTextPx != boldPx)
    {
        model.boldTextPx = boldPx;
        m_View.MarkDirty("bold_text_px");
    }

    std::vector<MessageBoxViewLineEntry> lineEntries;
    for (const Line& line : lines)
        lineEntries.push_back({StringUtils::WideToNarrow(line.text.c_str()), line.left, line.top, line.bold,
                               UI::RmlBridge::RgbaToCss(line.color), line.textPx});
    if (model.lines.size() != lineEntries.size() ||
        !std::equal(model.lines.begin(), model.lines.end(), lineEntries.begin(), SameLine))
    {
        model.lines = std::move(lineEntries);
        m_View.MarkDirty("lines");
    }

    // CMessageBoxButton::Render(): the label in the normal font, centred on its button.
    std::vector<MessageBoxViewButtonEntry> buttonEntries;
    std::vector<MessageBoxViewButtonEntry> placedEntries;
    for (std::size_t i = 0; i < buttons.size(); ++i)
    {
        const Button& button = buttons[i];
        (button.placed ? placedEntries : buttonEntries)
            .push_back({StringUtils::WideToNarrow(button.label.c_str()), static_cast<int>(i), button.left, button.top,
                        button.width, button.height, button.enabled, button.okArt});
    }
    const auto syncButtons = [this](std::vector<MessageBoxViewButtonEntry>& current,
                                    std::vector<MessageBoxViewButtonEntry>&& next, const char* name)
    {
        if (current.size() == next.size() && std::equal(current.begin(), current.end(), next.begin(), SameButton))
            return;
        current = std::move(next);
        m_View.MarkDirty(name);
    };
    syncButtons(model.buttons, std::move(buttonEntries), "buttons");
    syncButtons(model.placedButtons, std::move(placedEntries), "placed_buttons");
}

void mu::ui::window::MessageBoxView::BindList(Rml::DataModelConstructor& constructor, MessageBoxViewRmlModel& model)
{
    constructor.Bind("list_shown", &model.listShown);
    if (auto row = constructor.RegisterStruct<MessageBoxViewListRowEntry>())
    {
        row.RegisterMember("text", &MessageBoxViewListRowEntry::text);
        row.RegisterMember("index", &MessageBoxViewListRowEntry::index);
        row.RegisterMember("selected", &MessageBoxViewListRowEntry::selected);
    }
    constructor.RegisterArray<std::vector<MessageBoxViewListRowEntry>>();
    constructor.Bind("list_rows", &model.listRows);
    constructor.BindEventCallback("message_box_list_row",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
        {
            if (arguments.size() == 1)
                m_PressedListRow = arguments[0].Get<int>(-1);
        });
}

void mu::ui::window::MessageBoxView::SyncList(const List* list)
{
    if (!m_View.Document())
        return;
    SyncField(m_View.Binder(), &MessageBoxViewRmlModel::listShown, "list_shown", list != nullptr);
    if (list && m_View.GetModel().listRows != *list)
    {
        m_View.GetModel().listRows = *list;
        m_View.MarkDirty("list_rows");
    }
}

int mu::ui::window::MessageBoxView::TakePressedListRow()
{
    const int pressed = m_PressedListRow;
    m_PressedListRow = -1;
    return pressed;
}

int mu::ui::window::MessageBoxView::TakePressedButton()
{
    const int pressed = m_PressedButton;
    m_PressedButton = -1;
    return pressed;
}
