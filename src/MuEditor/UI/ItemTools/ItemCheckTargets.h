#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/MuEditorToolWindows.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <string_view>

// Where the item checks send a problem: the item tool that edits the field it
// names, so a click on the problem opens that tool for the item.
namespace MuEditor::Items
{
enum class ItemTool
{
    Basics,
    Requirements,
    Rules,
    Categories,
    Model,
    Translations,
    Browser,
};

// The window of an item tool: its "###" id (MuEditor::WindowIds) and its flag
// among the editor's open tool windows.
struct ItemToolWindowRef
{
    const char* windowId;
    bool MuEditor::ToolWindows::* open;
};

// The tool of a field as the item files and the model files name it
// (docs/item-data.md): "glow.color" and "renderStyle" go to the model tool,
// "requirementsByLevel.level[3]" to the requirements, "name.de" to the
// translations, "number" and a problem of the whole item (no field) to the
// browser, and every other field of an item to the basics.
ItemTool GetToolForField(std::string_view field);

// GetToolForField, except that a problem of a name in another language than
// English goes to the translations: the item checks name the language in the
// message ("the de name is longer than ..."), the field is only "name".
ItemTool GetToolForIssue(const Data::Items::ItemDataIssue& issue);

ItemToolWindowRef GetToolWindow(ItemTool tool);
} // namespace MuEditor::Items

#endif // _EDITOR
