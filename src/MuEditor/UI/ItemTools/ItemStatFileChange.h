#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// What a change of the item stat file reaches: the items that take another
// stat profile with it and the profiles whose terms change (rule D26: Item
// Formulas lists them before it applies the change).
namespace MuEditor::Items
{
struct StatProfileChange
{
    int itemType = 0;
    // The profile the item names before and after ("" for the default).
    std::string from;
    std::string to;
};

// The items of `slots` that take another profile when the stat file goes from
// `before` to `after`, in item type order: an item that follows the profile
// rules (its profile is the one they give, or it names none) takes what the
// new rules give, where that differs; a profile picked for an item, also
// "default", stays. With `renamedFrom`, every item that names it (also a pick)
// goes to `renamedTo`.
std::vector<StatProfileChange> FindStatProfileChanges(std::span<const Data::Items::ItemDefinition> slots,
                                                      const Data::Items::ItemStatProfiles& before,
                                                      const Data::Items::ItemStatProfiles& after,
                                                      std::string_view renamedFrom = {},
                                                      std::string_view renamedTo = {});

// The items of `slots` with the changes applied, for ItemEdits::SetStatFile.
std::vector<Data::Items::ItemDefinition> ApplyStatProfileChanges(std::span<const Data::Items::ItemDefinition> slots,
                                                                 std::span<const StatProfileChange> changes);

// Renames the profile `from` and every profile rule that names it.
void RenameStatProfile(Data::Items::ItemStatFile& file, std::string_view from, const std::string& to);

// The item types of `slots` whose descriptions or name lines name the table, as a placeholder ({table:<name>}) or in a
// line's "values" ("table:<name>"), in item type order.
std::vector<int> FindTableDescriptionUsers(std::span<const Data::Items::ItemDefinition> slots, std::string_view table);

// Has every item of `slots` whose description or name lines name the table `from` name `to` instead, in every
// language and in the lines' "values": an item of `changed` is changed there, another one is added to it as changed.
void RenameTableInDescriptions(std::span<const Data::Items::ItemDefinition> slots,
                               std::vector<Data::Items::ItemDefinition>& changed, std::string_view from,
                               std::string_view to);

// The names of the profiles of `after` ("" for the default) whose terms give
// other values than under `before`: other level bonuses for the levels 0 to
// 16, other excellent or ancient terms, other requirement terms, or other drop
// level bonuses for all of them. A profile `before` does not have counts.
std::vector<std::string> FindProfilesWithChangedTerms(const Data::Items::ItemStatProfiles& before,
                                                      const Data::Items::ItemStatProfiles& after);

// The item types a change of the stat file reaches: the ones `changes` gives
// another profile and the ones whose profile is in `changedProfiles`, in item
// type order.
std::vector<int> FindItemsReached(std::span<const Data::Items::ItemDefinition> slots,
                                  std::span<const StatProfileChange> changes,
                                  std::span<const std::string> changedProfiles);
} // namespace MuEditor::Items

#endif // _EDITOR
