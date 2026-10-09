#include "stdafx.h"

#include "UI/Inventory/TipTextListView.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
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
Rml::String ArgbToCss(unsigned int argb)
{
    return "rgba(" + std::to_string((argb >> 16) & 0xFFu) + ", " + std::to_string((argb >> 8) & 0xFFu) + ", " +
           std::to_string(argb & 0xFFu) + ", " + std::to_string((argb >> 24) & 0xFFu) + ")";
}

bool SameBox(const TipTextListBoxEntry& a, const TipTextListBoxEntry& b)
{
    return a.left == b.left && a.top == b.top && a.width == b.width && a.height == b.height && a.kind == b.kind;
}

bool SameLine(const TipTextListLineEntry& a, const TipTextListLineEntry& b)
{
    return a.text == b.text && a.left == b.left && a.top == b.top && a.width == b.width && a.textPx == b.textPx &&
           a.align == b.align && a.bold == b.bold && a.color == b.color;
}

void BindTipTextListModel(Rml::DataModelConstructor& c, TipTextListRmlModel& model)
{
    auto box = c.RegisterStruct<TipTextListBoxEntry>();
    box.RegisterMember("left", &TipTextListBoxEntry::left);
    box.RegisterMember("top", &TipTextListBoxEntry::top);
    box.RegisterMember("width", &TipTextListBoxEntry::width);
    box.RegisterMember("height", &TipTextListBoxEntry::height);
    box.RegisterMember("kind", &TipTextListBoxEntry::kind);
    c.RegisterArray<std::vector<TipTextListBoxEntry>>();
    c.Bind("boxes", &model.boxes);
    auto line = c.RegisterStruct<TipTextListLineEntry>();
    line.RegisterMember("text", &TipTextListLineEntry::text);
    line.RegisterMember("left", &TipTextListLineEntry::left);
    line.RegisterMember("top", &TipTextListLineEntry::top);
    line.RegisterMember("width", &TipTextListLineEntry::width);
    line.RegisterMember("text_px", &TipTextListLineEntry::textPx);
    line.RegisterMember("align", &TipTextListLineEntry::align);
    line.RegisterMember("bold", &TipTextListLineEntry::bold);
    line.RegisterMember("color", &TipTextListLineEntry::color);
    c.RegisterArray<std::vector<TipTextListLineEntry>>();
    c.Bind("lines", &model.lines);
}
} // namespace

mu::ui::window::TipTextListView::TipTextListView(const char* modelName, const char* documentPath)
    : m_View(modelName, BindTipTextListModel, {{documentPath}}, {.stacking = UI::RmlBridge::ThemedStacking::Front})
{
}

void mu::ui::window::TipTextListView::Build()
{
    m_View.Ensure();
}

void mu::ui::window::TipTextListView::Sync(bool visible, const TipTextListRecord& record)
{
    Build();
    if (!m_View.Document())
        return;

    // Layer depth 6.5 / 6.6: over the HUD and the panels.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_View.Document(), visible);
    if (!visible)
        return;

    std::vector<TipTextListBoxEntry> boxes;
    for (const TipTextListRecord::Box& box : record.boxes)
        boxes.push_back({box.x, box.y, box.width, box.height, static_cast<int>(box.kind)});

    std::vector<TipTextListLineEntry> lines;
    for (const TipTextListRecord::Line& line : record.lines)
    {
        // RenderText()'s text box: the box width by the line's height, behind the text.
        if (line.bgColor >> 24 != 0)
            boxes.push_back({line.x, line.y, line.boxWidth, line.height,
                             static_cast<int>(TipTextListRecord::BoxKind::LineBackdrop)});
        g_pRenderText->SetFont(line.bold ? g_hFontBold : g_hFont);
        const int measured = g_pRenderText->MeasureText(line.text.c_str(), static_cast<int>(line.text.size())).cx;
        const auto role = line.bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        const int align = line.sort == RT3_SORT_CENTER ? 1 : line.sort == RT3_SORT_RIGHT ? 2 : 0;
        lines.push_back({StringUtils::WideToNarrow(line.text.c_str()), line.x, line.y, line.boxWidth,
                         UI::RmlBridge::NativeTextPxInBox(role, static_cast<float>(measured), line.boxWidth), align,
                         line.bold, UI::RmlBridge::RgbaToCss(line.color)});
    }

    TipTextListRmlModel& model = m_View.GetModel();
    if (model.boxes.size() != boxes.size() ||
        !std::equal(model.boxes.begin(), model.boxes.end(), boxes.begin(), SameBox))
    {
        model.boxes = std::move(boxes);
        m_View.MarkDirty("boxes");
    }
    if (model.lines.size() != lines.size() ||
        !std::equal(model.lines.begin(), model.lines.end(), lines.begin(), SameLine))
    {
        model.lines = std::move(lines);
        m_View.MarkDirty("lines");
    }
}
