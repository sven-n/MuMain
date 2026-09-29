#include "stdafx.h"

#include "UI/Dialogs/MessageBoxView.h"

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
bool SameLine(const MessageBoxViewLineEntry& a, const MessageBoxViewLineEntry& b)
{
    return a.text == b.text && a.left == b.left && a.top == b.top && a.bold == b.bold && a.color == b.color &&
           a.textPx == b.textPx;
}

bool SameListRow(const MessageBoxViewListRowEntry& a, const MessageBoxViewListRowEntry& b)
{
    return a.text == b.text && a.top == b.top && a.selected == b.selected;
}

bool SameButton(const MessageBoxViewButtonEntry& a, const MessageBoxViewButtonEntry& b)
{
    return a.label == b.label && a.index == b.index && a.left == b.left && a.top == b.top && a.width == b.width &&
           a.height == b.height && a.labelLeft == b.labelLeft && a.labelTop == b.labelTop && a.enabled == b.enabled &&
           a.okArt == b.okArt;
}
} // namespace

mu::ui::window::MessageBoxView::~MessageBoxView()
{
    Destroy();
}

void mu::ui::window::MessageBoxView::Create(int middleCount, float backHeight)
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    // One document and model name: a second box of this kind while one is open gets no view
    // (CreateDataModel() refuses the name), which no caller does.
    m_ModelName = "message_box_view";
    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), m_ModelName,
        [this](Rml::DataModelConstructor& c, MessageBoxViewRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
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
            button.RegisterMember("label_left", &MessageBoxViewButtonEntry::labelLeft);
            button.RegisterMember("label_top", &MessageBoxViewButtonEntry::labelTop);
            button.RegisterMember("enabled", &MessageBoxViewButtonEntry::enabled);
            button.RegisterMember("ok_art", &MessageBoxViewButtonEntry::okArt);
            c.RegisterArray<std::vector<MessageBoxViewButtonEntry>>();
            c.Bind("buttons", &model.buttons);

            c.Bind("list_shown", &model.listShown);
            c.Bind("list_left", &model.listLeft);
            c.Bind("list_top", &model.listTop);
            c.Bind("list_width", &model.listWidth);
            c.Bind("list_height", &model.listHeight);
            auto row = c.RegisterStruct<MessageBoxViewListRowEntry>();
            row.RegisterMember("text", &MessageBoxViewListRowEntry::text);
            row.RegisterMember("top", &MessageBoxViewListRowEntry::top);
            row.RegisterMember("selected", &MessageBoxViewListRowEntry::selected);
            c.RegisterArray<std::vector<MessageBoxViewListRowEntry>>();
            c.Bind("list_rows", &model.listRows);
            c.Bind("list_up_pressed", &model.listUpPressed);
            c.Bind("list_down_pressed", &model.listDownPressed);
            c.Bind("list_track_top", &model.listTrackTop);
            c.Bind("list_track_height", &model.listTrackHeight);
            c.Bind("list_thumb_top", &model.listThumbTop);
            c.Bind("list_thumb_height", &model.listThumbHeight);
            c.Bind("list_thumb_bottom_top", &model.listThumbBottomTop);

            c.BindEventCallback("message_box_button",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                                {
                                    if (arguments.size() == 1)
                                        m_PressedButton = arguments[0].Get<int>(-1);
                                });
        });
    if (!modelCreated)
    {
        m_ModelName.clear();
        return;
    }

    SetFrame(middleCount, backHeight);

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/message_box_view.rml");
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

    MessageBoxViewRmlModel& model = m_RmlBinder.GetModel();
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
    m_RmlBinder.MarkDirty("back_height");
    m_RmlBinder.MarkDirty("strips");
}

void mu::ui::window::MessageBoxView::SetSeparators(const std::vector<float>& tops)
{
    MessageBoxViewRmlModel& model = m_RmlBinder.GetModel();
    if (model.separators == tops)
        return;
    model.separators = tops;
    m_RmlBinder.MarkDirty("separators");
}

void mu::ui::window::MessageBoxView::SetProgress(float top, float fraction)
{
    MessageBoxViewRmlModel& model = m_RmlBinder.GetModel();
    const bool shown = top >= 0.f;
    // RenderProgress(): the fill 150 units wide at the full fraction, not clamped.
    const float width = 150.f * fraction;
    if (model.progressShown != shown)
    {
        model.progressShown = shown;
        m_RmlBinder.MarkDirty("progress_shown");
    }
    if (model.progressTop != top)
    {
        model.progressTop = top;
        m_RmlBinder.MarkDirty("progress_top");
    }
    if (model.progressWidth != width)
    {
        model.progressWidth = width;
        m_RmlBinder.MarkDirty("progress_width");
    }
}

void mu::ui::window::MessageBoxView::Destroy()
{
    if (m_ModelName.empty())
        return;
    if (RmlUiRuntime::Instance().IsCreated())
    {
        Rml::Context* context = RmlUiRuntime::Instance().GetContext();
        if (m_pRmlDoc)
            context->UnloadDocument(m_pRmlDoc);
        m_RmlBinder.Destroy(context);
    }
    m_pRmlDoc = nullptr;
    m_ModelName.clear();
}

void mu::ui::window::MessageBoxView::Sync(const POINT& pos, const std::vector<Line>& lines,
                                          const std::vector<Button>& buttons)
{
    if (!m_pRmlDoc)
        return;

    // Message boxes draw over every window: in front of the other documents.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, true);
    UI::RmlBridge::SyncRootTransform(m_RmlBinder, pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);

    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    MessageBoxViewRmlModel& model = m_RmlBinder.GetModel();
    const float boldPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform);
    if (model.boldTextPx != boldPx)
    {
        model.boldTextPx = boldPx;
        m_RmlBinder.MarkDirty("bold_text_px");
    }

    std::vector<MessageBoxViewLineEntry> lineEntries;
    for (const Line& line : lines)
        lineEntries.push_back({StringUtils::WideToNarrow(line.text.c_str()), line.left, line.top, line.bold,
                               UI::RmlBridge::RgbaToCss(line.color), line.textPx});
    if (model.lines.size() != lineEntries.size() ||
        !std::equal(model.lines.begin(), model.lines.end(), lineEntries.begin(), SameLine))
    {
        model.lines = std::move(lineEntries);
        m_RmlBinder.MarkDirty("lines");
    }

    // CMessageBoxButton::Render(): the label in the normal font at the box's whole-unit centre.
    g_pRenderText->SetFont(g_hFont);
    std::vector<MessageBoxViewButtonEntry> buttonEntries;
    for (std::size_t i = 0; i < buttons.size(); ++i)
    {
        const Button& button = buttons[i];
        const SIZE size = g_pRenderText->MeasureText(button.label.c_str(), static_cast<int>(button.label.size()));
        const int labelLeft = static_cast<int>(button.width / 2) - static_cast<int>(size.cx / 2);
        const int labelTop = static_cast<int>(button.height / 2) - static_cast<int>(size.cy / 2);
        buttonEntries.push_back({StringUtils::WideToNarrow(button.label.c_str()), static_cast<int>(i), button.left,
                                 button.top, button.width, button.height, static_cast<float>(labelLeft),
                                 static_cast<float>(labelTop), button.enabled, button.okArt});
    }
    if (model.buttons.size() != buttonEntries.size() ||
        !std::equal(model.buttons.begin(), model.buttons.end(), buttonEntries.begin(), SameButton))
    {
        model.buttons = std::move(buttonEntries);
        m_RmlBinder.MarkDirty("buttons");
    }
}

void mu::ui::window::MessageBoxView::SyncList(const List* list)
{
    if (!m_pRmlDoc)
        return;

    MessageBoxViewRmlModel& model = m_RmlBinder.GetModel();
    auto sync = [this](auto& field, const auto& value, const char* name)
    {
        if (field != value)
        {
            field = value;
            m_RmlBinder.MarkDirty(name);
        }
    };

    sync(model.listShown, list != nullptr, "list_shown");
    if (!list)
        return;

    sync(model.listLeft, list->left, "list_left");
    sync(model.listTop, list->top, "list_top");
    sync(model.listWidth, list->width, "list_width");
    sync(model.listHeight, list->height, "list_height");
    sync(model.listUpPressed, list->upPressed, "list_up_pressed");
    sync(model.listDownPressed, list->downPressed, "list_down_pressed");
    sync(model.listTrackTop, list->trackTop, "list_track_top");
    sync(model.listTrackHeight, list->trackHeight, "list_track_height");
    sync(model.listThumbTop, list->thumbTop, "list_thumb_top");
    sync(model.listThumbHeight, list->thumbHeight, "list_thumb_height");
    sync(model.listThumbBottomTop, list->thumbBottomTop, "list_thumb_bottom_top");

    std::vector<MessageBoxViewListRowEntry> rows;
    for (const List::Row& row : list->rows)
        rows.push_back({StringUtils::WideToNarrow(row.text.c_str()), row.top, row.selected});
    if (model.listRows.size() != rows.size() ||
        !std::equal(model.listRows.begin(), model.listRows.end(), rows.begin(), SameListRow))
    {
        model.listRows = std::move(rows);
        m_RmlBinder.MarkDirty("list_rows");
    }
}

int mu::ui::window::MessageBoxView::TakePressedButton()
{
    const int pressed = m_PressedButton;
    m_PressedButton = -1;
    return pressed;
}
