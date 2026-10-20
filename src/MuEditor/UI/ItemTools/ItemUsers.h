#pragma once

#ifdef _EDITOR

#include <string_view>
#include <vector>

// The items that use a shared definition (D26), for the "used by" lists of
// the item tools and the warning before a shared definition changes. Each
// list is in item type order; model entries count also where the number has
// no item.
namespace MuEditor::Items
{
std::vector<int> FindSharedModelUsers(std::string_view name);
std::vector<int> FindRenderStyleUsers(std::string_view name);
std::vector<int> FindItemEffectUsers(std::string_view name);
// The items with the color in any of their glow colors (glow, shine,
// ancient), also as the default they leave out.
std::vector<int> FindGlowColorUsers(std::string_view name);
// The items whose stat profile it is; "" and "default" both name the
// default profile.
std::vector<int> FindStatProfileUsers(std::string_view name);
} // namespace MuEditor::Items

#endif // _EDITOR
