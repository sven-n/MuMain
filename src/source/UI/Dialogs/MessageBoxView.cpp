#include "stdafx.h"

#include "UI/Dialogs/MessageBoxView.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

using namespace mu::ui::window;


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
    c.Bind("kind", &model.kind);
    c.Bind("text_px", &model.textPx);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("line_height", &model.lineHeight);
    c.Bind("bold_line_height", &model.boldLineHeight);
    if (auto strip = c.RegisterStruct<MessageBoxViewStripEntry>())
    {
        strip.RegisterMember("kind", &MessageBoxViewStripEntry::kind);
    }
    c.RegisterArray<std::vector<MessageBoxViewStripEntry>>();
    c.Bind("strips", &model.strips);
    c.Bind("progress_shown", &model.progressShown);
    c.Bind("progress_width", &model.progressWidth);

    auto cell = c.RegisterStruct<MessageBoxViewCellEntry>();
    cell.RegisterMember("text", &MessageBoxViewCellEntry::text);
    cell.RegisterMember("bold", &MessageBoxViewCellEntry::bold);
    cell.RegisterMember("color", &MessageBoxViewCellEntry::color);
    cell.RegisterMember("text_px", &MessageBoxViewCellEntry::textPx);
    c.RegisterArray<std::vector<MessageBoxViewCellEntry>>();
    auto line = c.RegisterStruct<MessageBoxViewLineEntry>();
    line.RegisterMember("role", &MessageBoxViewLineEntry::role);
    line.RegisterMember("bold", &MessageBoxViewLineEntry::bold);
    line.RegisterMember("cells", &MessageBoxViewLineEntry::cells);
    c.RegisterArray<std::vector<MessageBoxViewLineEntry>>();
    c.Bind("lines", &model.lines);

    auto button = c.RegisterStruct<MessageBoxViewButtonEntry>();
    button.RegisterMember("label", &MessageBoxViewButtonEntry::label);
    button.RegisterMember("index", &MessageBoxViewButtonEntry::index);
    button.RegisterMember("enabled", &MessageBoxViewButtonEntry::enabled);
    button.RegisterMember("ok_art", &MessageBoxViewButtonEntry::okArt);
    c.RegisterArray<std::vector<MessageBoxViewButtonEntry>>();
    c.Bind("buttons", &model.buttons);

    BindList(c, model);

    c.BindEventCallback("message_box_button",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                m_PressedButton = arguments[0].Get<int>(-1);
                        });
}

void mu::ui::window::MessageBoxView::Create(int middleCount, const char* kind)
{
    if (m_View.Document() != nullptr || !RmlUiRuntime::Instance().IsCreated())
        return;

    m_View.GetModel().kind = kind;
    SetFrame(middleCount);
    m_View.Ensure();
}

void mu::ui::window::MessageBoxView::SetFrame(int middleCount, int middlesAboveDivider)
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
    if (model.middleCount == middleCount && sameStrips)
        return;
    model.middleCount = middleCount;
    model.strips = std::move(strips);
    m_View.MarkDirty("strips");
}

void mu::ui::window::MessageBoxView::SetProgress(float fraction)
{
    // RenderProgress(): the fill 150 units wide at the full fraction, not clamped.
    SyncField(m_View.Binder(), &MessageBoxViewRmlModel::progressShown, "progress_shown", true);
    SyncField(m_View.Binder(), &MessageBoxViewRmlModel::progressWidth, "progress_width", 150.f * fraction);
}

Rml::Element* mu::ui::window::MessageBoxView::Panel() const
{
    return m_View.Document() != nullptr ? m_View.Document()->GetElementById("panel") : nullptr;
}

void mu::ui::window::MessageBoxView::Destroy()
{
    m_PressedButton = -1;
    m_PressedListRow = -1;
    m_View.Release();
}

void mu::ui::window::MessageBoxView::Sync(const std::vector<Line>& lines, const std::vector<Button>& buttons)
{
    if (!m_View.Document())
        return;

    // Message boxes draw over every window: in front of the other documents.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_View.Document(), true);
    UI::RmlBridge::SyncNativeTextSize(m_View.Binder());

    MessageBoxViewRmlModel& model = m_View.GetModel();
    const float boldPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold);
    if (model.boldTextPx != boldPx)
    {
        model.boldTextPx = boldPx;
        m_View.MarkDirty("bold_text_px");
    }

    SyncField(m_View.Binder(), &MessageBoxViewRmlModel::lineHeight, "line_height",
              static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal)));
    SyncField(m_View.Binder(), &MessageBoxViewRmlModel::boldLineHeight, "bold_line_height",
              static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Bold)));

    std::vector<MessageBoxViewLineEntry> lineEntries;
    for (const Line& line : lines)
    {
        MessageBoxViewLineEntry entry{line.role, !line.cells.empty() && line.cells.front().bold, {}};
        for (const Cell& cell : line.cells)
            entry.cells.push_back({StringUtils::WideToNarrow(cell.text.c_str()), cell.bold,
                                   UI::RmlBridge::RgbaToCss(cell.color), cell.textPx});
        lineEntries.push_back(std::move(entry));
    }
    SyncField(m_View.Binder(), &MessageBoxViewRmlModel::lines, "lines", std::move(lineEntries));

    // CMessageBoxButton::Render(): the label in the normal font, centred on its button.
    std::vector<MessageBoxViewButtonEntry> buttonEntries;
    for (std::size_t i = 0; i < buttons.size(); ++i)
        buttonEntries.push_back({StringUtils::WideToNarrow(buttons[i].label.c_str()), static_cast<int>(i),
                                 buttons[i].enabled, buttons[i].okArt});
    SyncField(m_View.Binder(), &MessageBoxViewRmlModel::buttons, "buttons", std::move(buttonEntries));
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
