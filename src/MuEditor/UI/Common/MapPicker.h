#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>

// The editor's one way to pick maps (monsters design, D67): data names maps by
// the numbers the server sends, and the picker lists them with the rounds of
// an event under one name (Devil Square 1-4 and 5-7, Blood Castle 1-8, ...),
// each with its server number and the world folder it loads.
namespace MuEditor::MapPicker
{
// A map of a row: its server number and, in a group, its round ("5-7").
struct Member
{
    int map = 0;
    std::string round;
};

// A row of the picker: one map, or the rounds of an event under its name.
struct Row
{
    // The first member's map.
    int nameMap = 0;
    std::vector<Member> members;
};

// The rows, by the first member's number.
const std::vector<Row>& GetRows();

// The map's name in the UI language, with its round in a group:
// "Devil Square 5-7".
std::string NameOf(int map);

// "Devil Square 5-7, Kalima 7"; `whenEmpty` for no map.
std::string Describe(const std::vector<int>& maps, const char* whenEmpty);

// "server 9, 32 · World10": the server numbers and the world folders.
std::string InfoOf(const Row& row);

// Adds or removes maps; the list stays sorted, without repeats.
void SetMaps(std::vector<int>& maps, const std::vector<Member>& members, bool on);

// A button that says which maps the list names and opens the picker; true
// when the list changed. `id` is unique in the window.
bool Render(const char* id, std::vector<int>& maps, const char* whenEmpty);
} // namespace MuEditor::MapPicker

#endif // _EDITOR
