#include "stdafx.h"

#ifdef _EDITOR

#include "ItemFormulaEntry.h"

#include <optional>

namespace MuEditor::Items
{
namespace
{
constexpr std::string_view KeyEnds = ".[";

// The entry of `kind` named after "<key>." up to the next key: "steps" of
// "tables.steps[3]".
std::optional<FormulaEntry> FindNamed(std::string_view field, std::string_view key, FormulaEntry::Kind kind)
{
    const std::string prefix = std::string(key) + ".";
    if (!field.starts_with(prefix))
    {
        return std::nullopt;
    }
    const std::string_view rest = field.substr(prefix.size());
    return FormulaEntry{kind, std::string(rest.substr(0, rest.find_first_of(KeyEnds)))};
}
} // namespace

FormulaEntry FindFormulaEntryForField(std::string_view field)
{
    using Kind = FormulaEntry::Kind;
    if (const std::optional<FormulaEntry> table = FindNamed(field, "tables", Kind::Table))
    {
        return *table;
    }
    if (const std::optional<FormulaEntry> profile = FindNamed(field, "profiles", Kind::Profile))
    {
        return *profile;
    }
    const std::string_view key = field.substr(0, field.find_first_of(KeyEnds));
    if (key == "dropLevelBonus")
    {
        return {Kind::DropLevelBonus, {}};
    }
    if (key == "default")
    {
        return {Kind::Default, {}};
    }
    if (key == "profileRules")
    {
        return {Kind::Rules, {}};
    }
    return {};
}
} // namespace MuEditor::Items

#endif // _EDITOR
