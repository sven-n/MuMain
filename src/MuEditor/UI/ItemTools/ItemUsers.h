#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <map>
#include <string>
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
// Whether `name` is the item's stat profile; "" and "default" both name the
// default profile.
bool UsesStatProfile(const Data::Items::ItemDefinition& item, std::string_view name);
// The items whose stat profile it is (UsesStatProfile).
std::vector<int> FindStatProfileUsers(std::string_view name);
// The profile rules of the item stat file that give it (their indexes); ""
// and "default" both name the default profile.
std::vector<int> FindStatProfileRules(std::string_view name);

// How many items and profile rules name each stat profile ("" and "default"
// together), and which items' descriptions name each level bonus table, for
// the lists that show the count beside each name. Counted again when the item
// data changed.
class ItemStatProfileUseCounts
{
public:
    int Items(std::string_view name);
    int Rules(std::string_view name);
    // The item types whose descriptions name the table, in item type order.
    const std::vector<int>& DescriptionUsers(std::string_view table);

private:
    using Counts = std::map<std::string, int, std::less<>>;

    void Update();

    Counts m_items;
    Counts m_rules;
    std::map<std::string, std::vector<int>, std::less<>> m_descriptionUsers;
    int m_version = -1;
};
} // namespace MuEditor::Items

#endif // _EDITOR
