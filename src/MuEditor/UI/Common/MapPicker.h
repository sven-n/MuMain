#pragma once

#ifdef _EDITOR

#include "Data/GameData/MapData/MapRef.h"

#include <string>
#include <vector>

// The editor's one way to pick maps (monsters design, D67): data names maps by
// the numbers the server sends, and the picker lists them with the levels of
// an event under one name, each with its server number and the world folder it
// loads. An event's levels are either maps of their own (Blood Castle 1-8 are
// 11-17 and 52) or stages of one map, which OpenMU tells the client (Devil
// Square 1-4 are stages of 9, 5-7 of 32).
namespace MuEditor::MapPicker
{
// A map or a stage of a row and its label in the row ("5").
struct Member
{
    Data::Maps::MapRef ref;
    std::string label;
};

// A row of the picker: one map, or the levels of an event under its name.
struct Row
{
    // The map that names the row.
    int nameMap = 0;
    std::vector<Member> members;
};

// The rows, by the first member's number.
const std::vector<Row>& GetRows();

// The stages the server sends with a map (Devil Square's); empty for others.
std::vector<int> StagesOf(int map);

// The name of a map or stage in the UI language: "Devil Square 2", "Devil
// Square 5-7" (32 without a stage), "Lorencia".
std::string NameOf(const Data::Maps::MapRef& ref);

// "Devil Square 5-7, Kalima 7"; a whole event by its name; `whenEmpty` for no
// map.
std::string Describe(const std::vector<Data::Maps::MapRef>& maps, const char* whenEmpty);

// "server 9 stages 1-4, 32 stages 5-7; World10": the server numbers, with
// their stages, and the world folders.
std::string InfoOf(const Row& row);

// Whether the list names the map or stage of a member: a map without a stage
// names all of its stages.
bool IsChosen(const std::vector<Data::Maps::MapRef>& maps, const Member& member);

// Adds or removes maps and stages. The list stays sorted without repeats, and
// a map whose every stage is chosen is named without a stage.
void SetMaps(std::vector<Data::Maps::MapRef>& maps, const std::vector<Member>& members, bool on);

// A button that says which maps the list names and opens the picker; true
// when the list changed. `id` is unique in the window.
bool Render(const char* id, std::vector<Data::Maps::MapRef>& maps, const char* whenEmpty);
} // namespace MuEditor::MapPicker

#endif // _EDITOR
