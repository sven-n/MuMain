#pragma once

#ifdef _EDITOR

#include <cstdint>
#include <string>
#include <string_view>

namespace MuEditor::Items
{
// An entry of Item Formulas: a part of the item stat file.
struct FormulaEntry
{
    enum class Kind : uint8_t
    {
        None,
        DropLevelBonus,
        Default,
        Profile,
        Table,
        Rules,
    };

    Kind kind = Kind::None;
    // The profile's or the table's name.
    std::string name;

    bool operator==(const FormulaEntry& other) const = default;
};

// The entry a field of the item stat file names, as the file's problems name
// them: "tables.steps[3]" the table, "profiles.shield.defense" the profile,
// "default.damage" the default, "dropLevelBonus.excellent" the drop level
// bonus, "profileRules[2].slot" the rules; None for others.
FormulaEntry FindFormulaEntryForField(std::string_view field);
} // namespace MuEditor::Items

#endif // _EDITOR
