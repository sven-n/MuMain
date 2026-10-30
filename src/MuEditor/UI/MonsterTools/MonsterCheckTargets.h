#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/ToolGroups.h"

#include <optional>
#include <string_view>

// Where Data Checks sends a problem: the monster tool that edits the field
// it names, so a click on the problem opens that tool for the monster.
namespace MuEditor::Monsters
{
enum class MonsterTool
{
    Browser,
    Translations,
    Attacks,
};

// The field without the monster the monster list's problems name first:
// "name.de" of "monster 2 name.de", "kind" of "monster 2 kind". Any other
// field ("monsters[3].number") as it is.
std::string_view GetMonsterField(std::string_view field);

// A name ("name", "name.de") goes to Monster Translations, which has the names
// in all languages; the attacks ("attacks", "attacks.skills") go to Monster
// Attacks; the kind, the number, an unknown key and a problem without a field
// go to the Monster Browser. Takes the field with or without the monster in
// front.
MonsterTool GetToolForField(std::string_view field);

// The map variant a field of the attacks names: 0 for "monster 291
// mapVariants[0].attacks.otherwise"; nullopt for the attacks of all maps
// ("monster 291 attacks") and any other field.
std::optional<int> GetMapVariantOfField(std::string_view field);

// The window of a monster tool: its "###" id and its flag among the editor's
// open tool windows.
const ToolWindowEntry& GetToolWindow(MonsterTool tool);
} // namespace MuEditor::Monsters

#endif // _EDITOR
