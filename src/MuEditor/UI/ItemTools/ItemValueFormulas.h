#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemStatFile.h"

#include <array>
#include <optional>
#include <string>
#include <string_view>

// The terms of a stat profile with where each comes from, for the formula lines
// of the item tools (and later the export to OpenMU). See docs/item-data.md,
// "Item stats by level".
namespace MuEditor::Items
{
struct FormulaSource
{
    enum class Kind : uint8_t
    {
        // The default of the item stat file.
        Default,
        // The profile `name`, which writes the term itself.
        Profile,
        // The item's own field `name` (excellentBonus, requirementsByLevel,
        // excellentRaisesLevel).
        ItemField,
        // The stat file's dropLevelBonus.
        DropLevelBonus,
    };

    Kind kind = Kind::Default;
    std::string name;

    bool operator==(const FormulaSource& other) const = default;
};

template <typename TValue> struct SourcedTerm
{
    TValue value{};
    FormulaSource source;
};

struct ExpandedStatTerms
{
    // The level bonus table's name; nullopt: none.
    SourcedTerm<std::optional<std::string>> table;
    SourcedTerm<std::optional<Data::Items::StatBonusTerm>> excellent;
    SourcedTerm<std::optional<Data::Items::StatBonusTerm>> ancient;
    SourcedTerm<int> perLevelAfterHalving;
};

struct ExpandedRequirementTerms
{
    SourcedTerm<int> levelPerItemLevel;
    SourcedTerm<int> excellentLevelAdd;
    std::array<SourcedTerm<Data::Items::RequirementFormula>, Data::Items::ItemRequirementStatCount> stats;
    SourcedTerm<int> strengthPerOptionLevel;
};

struct ExpandedStatProfile
{
    // "default" for the default.
    std::string name;
    std::array<ExpandedStatTerms, Data::Items::ItemStatCount> stats;
    ExpandedRequirementTerms requirements;
    int excellentDropLevelBonus = 0;
    int ancientDropLevelBonus = 0;

    const ExpandedStatTerms& Get(Data::Items::ItemStat stat) const
    {
        return stats[static_cast<size_t>(stat)];
    }
};

// The profile `name` laid over the default as ItemStatProfiles lays it: a term
// the profile writes comes from it, the others from the default. A name the
// file does not have (also "" and "default") gives the default.
ExpandedStatProfile ExpandStatProfile(const Data::Items::ItemStatFile& file, std::string_view name);
} // namespace MuEditor::Items

#endif // _EDITOR
