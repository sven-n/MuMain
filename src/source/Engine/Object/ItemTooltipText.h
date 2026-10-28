#pragma once

#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"

#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct ItemTooltipValues;

namespace Data::Items
{
struct ItemDefinition;
struct LevelBonusTable;
} // namespace Data::Items

// The texts of the item tooltip's lines (layout lines, description lines, name lines) made ready for drawing once:
// each placeholder's name is resolved to what it shows, so a tooltip drawn every frame only writes numbers and texts
// into its row, comparing no names and allocating nothing (CODING_RULES.md, section 12).
namespace Engine::Items::TooltipText
{
// A piece of a compiled text: `literal`, then the value of `kind` (none for the last piece).
struct Piece
{
    enum class Kind : unsigned char
    {
        None,
        BuiltIn,     // `value` of the item and the hero, or {level}
        Table,       // `table`'s bonus at the item's level
        SkillMana,   // the mana skill `number` costs
        SkillName,   // skill `number`'s name, looked up when written
        MonsterName, // monster `number`'s name, looked up when written
        LevelValue,  // the item's level value: `levelTexts` at the item's level
    };
    std::wstring literal;
    Kind kind = Kind::None;
    Data::Items::TooltipValue value = Data::Items::TooltipValue::Count;
    const Data::Items::LevelBonusTable* table = nullptr;
    int number = 0;
    std::vector<std::wstring> levelTexts;
    // The text from this placeholder on, as written: what the rest of the line shows when the placeholder has no value
    // (a level value past the item's levels, a skill or monster not in its list), as FillPlaceholders leaves a
    // placeholder without a value.
    std::wstring rest;
};

struct Compiled
{
    std::vector<Piece> pieces;
};

// A line's text (already in its language), with its placeholders bound by the line's "values" (`bound`) and resolved
// for the item of `definition` (its level values, in `language`; nullptr where a line has none): a placeholder that
// names nothing, or a text whose braces are wrong, stays as written from there on. Compiled on first use and kept
// until the item data, the item stat file or the tooltip file changes (ItemDatabase::GetVersion). The compiled text is
// found by where `text` and `bound` are, without comparing them: they are the item data's own (or the tooltip file's),
// which stay where they are and as they are until the version changes. Another text needs Clear() first.
const Compiled& Compile(std::string_view text, std::span<const std::pair<std::string, std::string>> bound,
                        const Data::Items::ItemDefinition* definition, std::string_view language);

// Writes the compiled text for the item at `level` into `out` (at most `capacity` characters with the terminating 0)
// and returns its length.
size_t Write(const Compiled& text, int level, const ItemTooltipValues& values, wchar_t* out, size_t capacity);

// Drops every compiled text (the tests, before compiling a text that isn't the item data's).
void Clear();
} // namespace Engine::Items::TooltipText
