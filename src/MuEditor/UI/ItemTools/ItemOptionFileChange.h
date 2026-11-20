#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemOptionDatabase.h"

#include <array>
#include <functional>
#include <map>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// What a change of the option files reaches, and the changes Item Options makes from them: the items that link a
// definition, the ones a change gives other lines or values (rule D26: Item Options lists them before it applies the
// change), a rename with the links and rules that name the definition, and the levels of a value list.
namespace MuEditor::Items
{
// The file a new definition of the type goes in: the one most definitions of the type are in (the two ancient types
// count as one, they share a file), else the type's name with a capital letter ("Excellent.json", "Ancient.json" for
// the ancient types), as the files are named by convention.
std::string FindFileOfType(const Data::Items::ItemOptionFiles& files, Data::Items::ItemOptionType type);

// The definition of `files` with the name, or nullptr.
const Data::Items::ItemOptionDefinition* FindDefinitionNamed(const Data::Items::ItemOptionFiles& files,
                                                             std::string_view name);

// The ancient bonus definitions of `files` items pick by their attackType (1 to 4), by name (AncientBonusNames): the
// name where `files` has it, else empty. As in the game (ItemOptionDatabase::FindAncientBonus), only a definition of
// type ancientBonus is picked.
using AncientBonusPicks = std::array<std::string_view, Data::Items::AncientBonusNames.size()>;
AncientBonusPicks FindAncientBonusPicks(const Data::Items::ItemOptionFiles& files);

// Whether the definition of that name is one only the edited copy has (New, Copy): `stored` has none of it.
bool IsOnlyInCopy(const Data::Items::ItemOptionFiles& stored, std::string_view name);

// The picks once the copy `edit` is applied: those of `stored`, and those of the ancient bonus definitions only `edit`
// has (IsOnlyInCopy). A definition `stored` has keeps its stored type until it is applied.
AncientBonusPicks MergeCopyPicks(const Data::Items::ItemOptionFiles& stored, const Data::Items::ItemOptionFiles& edit);

// The ancient bonus definition the item's attackType picks (of FindAncientBonusPicks), or empty: the item uses it
// without a link.
std::string_view FindAncientBonusPick(const Data::Items::ItemDefinition& item, const AncientBonusPicks& picks);

// The number of existing items of `slots` that use each definition: link it by name, or pick it by attackType
// (`picks`).
std::map<std::string, int, std::less<>> CountOptionUsers(std::span<const Data::Items::ItemDefinition> slots,
                                                         const AncientBonusPicks& picks);

// The definitions Item Options lists: those of `stored`, then those only the edited copy `edit` has (New, Copy) until
// they are applied, each of the type `typeFilter` (any when below 0) and with `search` in its name or English display
// name (any when empty).
std::vector<const Data::Items::ItemOptionDefinition*> ListDefinitions(const Data::Items::ItemOptionFiles& stored,
                                                                      const Data::Items::ItemOptionFiles& edit,
                                                                      int typeFilter, std::string_view search);

// Drops the marks of names neither `stored` nor `edit` has a definition of (an undo, a redo or a revert removed it).
void DropMissingMarks(std::set<std::string, std::less<>>& marked, const Data::Items::ItemOptionFiles& stored,
                      const Data::Items::ItemOptionFiles& edit);

// Whether items pick the definition by its name and type, so a rename would lose them.
bool IsPickedByName(const Data::Items::ItemOptionDefinition& definition);

// Whether linking the definition to the item does something: not when the item links a definition of its type already
// (the first counts), for the ancient types (the item sets give them) and for excellent options on wings and horns (the
// excellent byte means something else there). The link picker of Item Basics offers only these.
bool IsUsefulOptionLink(const Data::Items::ItemDefinition& item, const Data::Items::ItemOptionDefinition& definition,
                        const Data::Items::ItemOptionDatabase& options);

// What the edited copy of the option files does when the stored files change (an undo, a redo).
enum class EditedCopyState
{
    // It had no changes and took the stored files again.
    Follows,
    // It keeps its changes, made from the stored files as they are.
    Edited,
    // It keeps its changes, but the stored files are no longer the ones it was taken from (`loaded`): applying it
    // would write back what changed them, so it waits for a revert.
    Stale,
};

// Without changes (`edit` is `loaded`), or when `stored` became the copy's changes (a redo of them), the copy takes
// `stored` again (both are set to it); with other changes, it is stale when `stored` is not `loaded`.
EditedCopyState UpdateEditedCopy(Data::Items::ItemOptionFiles& edit, Data::Items::ItemOptionFiles& loaded,
                                 const Data::Items::ItemOptionFiles& stored);

// The item types of `slots` that link the definition or pick it of `files` by their attackType, in item type order.
std::vector<int> FindOptionUsers(std::span<const Data::Items::ItemDefinition> slots,
                                 const Data::Items::ItemOptionFiles& files, std::string_view name);

// How many rules of `rules` give the definition.
int CountOptionRuleUses(std::span<const Data::Items::OptionRule> rules, std::string_view name);

// The names of the definitions of `after` that give other lines or values than under `before`: a definition `before`
// does not have, one whose type or options changed, or one whose options name a text that changed. A definition only
// `before` has counts too, under its name.
std::vector<std::string> FindChangedDefinitions(const Data::Items::ItemOptionFiles& before,
                                                const Data::Items::ItemOptionFiles& after);

// The item types of `slots` that use a definition of `changed` (FindChangedDefinitions) as FindOptionUsers finds them
// in `before` or `after` (an attackType picks a bonus definition the change adds as well as one it removes), and with
// `renamedFrom` the ones that use it, in item type order.
std::vector<int> FindItemsReached(std::span<const Data::Items::ItemDefinition> slots,
                                  const Data::Items::ItemOptionFiles& before, const Data::Items::ItemOptionFiles& after,
                                  std::span<const std::string> changed, std::string_view renamedFrom = {});

// Renames the definition `from` and its name in every rule.
void RenameOptionDefinition(Data::Items::ItemOptionFiles& files, std::string_view from, const std::string& to);

// The items of `slots` whose list of definitions the change from `before` to `after` changes, with their new list in
// the order `after` takes it in (ItemOptionDatabase::OrderLinks): for ItemEdits::SetOptions. An item whose list is the
// one `before`'s rules give (FollowItemOptions) takes what `after`'s rules give; another keeps its list, with
// `renamedFrom` named `renamedTo`.
std::vector<Data::Items::ItemDefinition> FindItemListChanges(std::span<const Data::Items::ItemDefinition> slots,
                                                             const Data::Items::ItemOptionFiles& before,
                                                             const Data::Items::ItemOptionFiles& after,
                                                             std::string_view renamedFrom = {},
                                                             std::string_view renamedTo = {});

// Whether the two lists name the same definitions, in any order (as FollowItemOptions compares them).
bool IsSameOptionList(std::span<const std::string> list, std::span<const std::string> other);

// The highest level the definition's value lists give a value of their own (their longest list; a list's last entry
// counts above it); 0 when its values are constants.
int HighestOptionLevel(const Data::Items::ItemOptionDefinition& definition);

// Makes each value list of the definition end at `level` (1 or more): a shorter list goes on by the step between its
// last two entries (a list of one entry by that entry), a longer one is cut. Constants stay. False when nothing
// changed.
bool FillOptionLevels(Data::Items::ItemOptionDefinition& definition, int level);

// Writes the value as a list from `fromLevel` to `toLevel`: `fromValue` at `fromLevel`, `step` more at each level
// after it; the levels below `fromLevel` keep their values (0 where the list had none). `fromLevel` is 1 or more and
// `toLevel` at least `fromLevel`.
void FillOptionValue(Data::Items::ItemOptionValue& value, int fromLevel, int fromValue, int step, int toLevel);
} // namespace MuEditor::Items

#endif // _EDITOR
