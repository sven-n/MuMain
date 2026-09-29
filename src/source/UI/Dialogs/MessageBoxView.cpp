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
    return a.text == b.text && a.left == b.left && a.top == b.top && a.bold == b.bold && a.color == b.color;
}

bool SameButton(const MessageBoxViewButtonEntry& a, const MessageBoxViewButtonEntry& b)
{
    return a.label == b.label && a.index == b.index && a.left == b.left && a.top == b.top && a.width == b.width &&
           a.height == b.height && a.labelLeft == b.labelLeft && a.labelTop == b.labelTop && a.enabled == b.enabled;
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
            c.Bind("middle_count", &model.middleCount);
            c.Bind("back_height", &model.backHeight);
            c.RegisterArray<std::vector<int>>();
            c.Bind("middles", &model.middles);

            auto line = c.RegisterStruct<MessageBoxViewLineEntry>();
            line.RegisterMember("text", &MessageBoxViewLineEntry::text);
            line.RegisterMember("left", &MessageBoxViewLineEntry::left);
            line.RegisterMember("top", &MessageBoxViewLineEntry::top);
            line.RegisterMember("bold", &MessageBoxViewLineEntry::bold);
            line.RegisterMember("color", &MessageBoxViewLineEntry::color);
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
            c.RegisterArray<std::vector<MessageBoxViewButtonEntry>>();
            c.Bind("buttons", &model.buttons);

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

    MessageBoxViewRmlModel& model = m_RmlBinder.GetModel();
    model.middleCount = middleCount;
    model.backHeight = backHeight;
    for (int i = 0; i < middleCount; ++i)
        model.middles.push_back(i);

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/message_box_view.rml");
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
                               UI::RmlBridge::RgbaToCss(line.color)});
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
                                 static_cast<float>(labelTop), button.enabled});
    }
    if (model.buttons.size() != buttonEntries.size() ||
        !std::equal(model.buttons.begin(), model.buttons.end(), buttonEntries.begin(), SameButton))
    {
        model.buttons = std::move(buttonEntries);
        m_RmlBinder.MarkDirty("buttons");
    }
}

int mu::ui::window::MessageBoxView::TakePressedButton()
{
    const int pressed = m_PressedButton;
    m_PressedButton = -1;
    return pressed;
}
