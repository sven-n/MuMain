#include "stdafx.h"

#include "UI/Inventory/ItemHelpView.h"

#include "Core/Utilities/StringUtils.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlSyncField.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace mu::ui::window;

namespace
{
const char* ColorName(int color)
{
    switch (color)
    {
    case TEXT_COLOR_BLUE: return "blue";
    case TEXT_COLOR_RED: return "red";
    case TEXT_COLOR_YELLOW: return "yellow";
    case TEXT_COLOR_GREEN: return "green";
    case TEXT_COLOR_DARKRED: return "hl-darkred";
    case TEXT_COLOR_PURPLE: return "purple";
    case TEXT_COLOR_DARKBLUE: return "hl-darkblue";
    case TEXT_COLOR_DARKYELLOW: return "hl-darkyellow";
    case TEXT_COLOR_GREEN_BLUE: return "hl-greenblue";
    case TEXT_COLOR_GRAY: return "gray";
    case TEXT_COLOR_REDPURPLE: return "redpurple";
    case TEXT_COLOR_VIOLET: return "violet";
    case TEXT_COLOR_ORANGE: return "orange";
    default: return "white";
    }
}

void BindItemHelpModel(Rml::DataModelConstructor& c, ItemHelpRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("line_height", &model.lineHeight);
    c.Bind("bold_line_height", &model.boldLineHeight);

    auto line = c.RegisterStruct<ItemHelpLineEntry>();
    line.RegisterMember("text", &ItemHelpLineEntry::text);
    line.RegisterMember("color", &ItemHelpLineEntry::color);
    line.RegisterMember("bold", &ItemHelpLineEntry::bold);
    line.RegisterMember("spacer", &ItemHelpLineEntry::spacer);
    c.RegisterArray<std::vector<ItemHelpLineEntry>>();
    c.Bind("lines", &model.lines);
    c.Bind("tail_lines", &model.tailLines);

    auto cell = c.RegisterStruct<ItemHelpCellEntry>();
    cell.RegisterMember("text", &ItemHelpCellEntry::text);
    cell.RegisterMember("can_equip", &ItemHelpCellEntry::canEquip);
    c.RegisterArray<std::vector<ItemHelpCellEntry>>();
    auto column = c.RegisterStruct<ItemHelpColumnEntry>();
    column.RegisterMember("heading", &ItemHelpColumnEntry::heading);
    column.RegisterMember("cells", &ItemHelpColumnEntry::cells);
    c.RegisterArray<std::vector<ItemHelpColumnEntry>>();
    c.Bind("columns", &model.columns);
}
} // namespace

mu::ui::window::ItemHelpView::ItemHelpView(const char* modelName, const char* documentPath)
    : m_View(modelName, BindItemHelpModel, {{documentPath}}, {.stacking = UI::RmlBridge::ThemedStacking::Front})
{
}

void mu::ui::window::ItemHelpView::Build()
{
    m_View.Ensure();
}

void mu::ui::window::ItemHelpView::Sync(bool visible, std::vector<ItemHelpLineEntry> lines, Table table,
                                        std::vector<ItemHelpLineEntry> tailLines)
{
    Build();
    if (!m_View.Document())
        return;

    // Layer depth 6.5 / 6.6: over the HUD and the panels.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_View.Document(), visible);
    if (!visible)
        return;

    auto& binder = m_View.Binder();
    SyncField(binder, &ItemHelpRmlModel::textPx, "text_px", UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Normal));
    SyncField(binder, &ItemHelpRmlModel::boldTextPx, "bold_text_px",
              UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold));
    SyncField(binder, &ItemHelpRmlModel::lineHeight, "line_height",
              static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal)));
    SyncField(binder, &ItemHelpRmlModel::boldLineHeight, "bold_line_height",
              static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Bold)));
    SyncField(binder, &ItemHelpRmlModel::lines, "lines", std::move(lines));
    SyncField(binder, &ItemHelpRmlModel::columns, "columns", std::move(table));
    SyncField(binder, &ItemHelpRmlModel::tailLines, "tail_lines", std::move(tailLines));
}

std::vector<ItemHelpLineEntry> mu::ui::window::ItemHelpView::TextListLines(int count)
{
    std::vector<ItemHelpLineEntry> lines;
    for (int i = 0; i < count && TextList[i][0] != L'\0'; ++i)
    {
        ItemHelpLineEntry line;
        if (TextList[i][0] == L'\n')
            line.spacer = "half";
        else if (TextList[i][0] == L' ' && TextList[i][1] == L'\0')
            line.spacer = "full";
        else
        {
            line.text = StringUtils::WideToNarrow(TextList[i]);
            line.color = ColorName(TextListColor[i]);
            line.bold = TextBold[i] != 0;
        }
        lines.push_back(std::move(line));
    }
    return lines;
}
