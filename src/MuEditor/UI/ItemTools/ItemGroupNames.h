#pragma once

#ifdef _EDITOR

#include <string>

namespace MuEditor::Items
{
// "0 Sword": the group's number and the name its item file has
// (Group00_Sword.json), for the group lists of the item tools.
std::string GetItemGroupLabel(int group);
} // namespace MuEditor::Items

#endif // _EDITOR
