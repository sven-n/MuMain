#pragma once

#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Items
{
class ItemStatProfiles;

// Placeholders in the texts of an item's description: "{name}" is a value the tooltip fills in for the item, at the
// item's + level. A placeholder is a built-in item value ("{level}"), or "table:" and the name of a level bonus table
// of the item stat file ("{table:damageIncreaseWeapons}"); names have only letters and digits. The tables are written
// apart, so a table never takes the name of a built-in value, also of one a later version adds. Item texts have no
// braces of their own, so every brace belongs to a placeholder.

// The built-in values: "level" is the item's + level; the others are the values of the tooltip's stat lines
// ("damageMin", "requiredStrength", "missingStrength", ...), which the game computes for the item and the hero
// (Engine/Object/ItemTooltipValues).
inline constexpr std::string_view LevelPlaceholder = "level";
std::span<const std::string_view> GetBuiltInPlaceholders();
bool IsBuiltInPlaceholder(std::string_view name);

// The built-in values by number, in GetBuiltInPlaceholders' order: what a condition or placeholder naming one is
// resolved to when its file is read, so drawing a tooltip compares no names.
enum class TooltipValue : unsigned char
{
    Level,
    DamageMin,
    DamageMax,
    Defense,
    MagicDefense,
    BlockRate,
    AttackSpeed,
    WalkSpeed,
    MagicPower,
    Durability,
    MaxDurability,
    IceResistance,
    PoisonResistance,
    LightningResistance,
    FireResistance,
    EarthResistance,
    WindResistance,
    WaterResistance,
    RequiredLevel,
    RequiredStrength,
    RequiredDexterity,
    RequiredVitality,
    RequiredEnergy,
    RequiredLeadership,
    MissingLevel,
    MissingStrength,
    MissingDexterity,
    MissingVitality,
    MissingEnergy,
    MissingLeadership,
    AddOptionValue1,
    AddOptionValue2,
    AddOptionDuration, // a text
    HeroLevel,
    LevelPlusOne,
    Name, // a text
    ExcellentFlags,
    BundleCount,
    Count,
};
// The built-in value of that name, or nothing for another name.
std::optional<TooltipValue> FindBuiltInPlaceholder(std::string_view name);
std::string_view GetBuiltInPlaceholderName(TooltipValue value);

// "table:<name>" names a level bonus table.
inline constexpr std::string_view TablePlaceholderPrefix = "table:";
// The placeholder of the table (without its braces): "table:<table>".
std::string MakeTablePlaceholder(std::string_view table);
// The table a placeholder names, or empty for another placeholder.
std::string_view GetPlaceholderTable(std::string_view name);

// The values a line's "values" can name besides the built-in values and tables: values of other game data, by number.
// "skillMana:<skill>" is the mana a skill costs and "skillName:<skill>" its name (the skill's number, as an item's
// "skill" writes it); "monsterName:<monster>" is a monster's name (its number in the monster data).
enum class GameDataValueKind : unsigned char
{
    SkillMana,
    SkillName,
    MonsterName,
};
struct GameDataValue
{
    GameDataValueKind kind;
    int number;
};
// Whether the name starts like a game data value ("skillMana:", ...), whatever follows.
bool HasGameDataValuePrefix(std::string_view name);
// The value the name means, or nothing for another name or a number that is no skill's or monster's.
std::optional<GameDataValue> ReadGameDataValue(std::string_view name);

// The placeholders of a text in their order (a repeated one each time), or the first problem of its braces.
struct PlaceholderScan
{
    std::vector<std::string> names;
    std::string error; // empty when the text is fine
};
PlaceholderScan ScanPlaceholders(std::string_view text);

// The placeholders of a text with where each is (from its "{" to past its "}"), up to the first problem of its braces:
// `stop` is where that problem starts, or npos when the whole text was read. FillPlaceholders leaves the text as
// written from `stop` on.
struct PlaceholderSpan
{
    size_t begin = 0;
    size_t end = 0;
    std::string_view name;
};
struct PlaceholderSpans
{
    std::vector<PlaceholderSpan> spans;
    size_t stop = std::string_view::npos;
};
PlaceholderSpans FindPlaceholders(std::string_view text);

// The value of {level} or "table:" and a table of `profiles` for an item at `level`, or nothing for another name (the
// stat line values come from the game, see FillPlaceholders with valueOf).
std::optional<int> GetPlaceholderValue(std::string_view name, int level, const ItemStatProfiles& profiles);

// The text with each placeholder replaced by its value. A placeholder without a value, or a text ScanPlaceholders
// refuses, stays as written from there on. The first takes {level} and the tables; the second asks `valueOf`, whose
// value is a number or a text (an item's level value: "ENG").
std::string FillPlaceholders(std::string_view text, int level, const ItemStatProfiles& profiles);
using PlaceholderValueOf = std::function<std::optional<std::string>(std::string_view name)>;
std::string FillPlaceholders(std::string_view text, const PlaceholderValueOf& valueOf);

// The known name nearest to `name` when it is at most two letters away (a typo), else empty.
std::string SuggestPlaceholder(std::string_view name, std::span<const std::string> known);

// Whether the text names the table as a placeholder.
bool NamesTable(std::string_view text, std::string_view table);
// The text with every placeholder of the table `from` naming the table `to` instead.
std::string RenameTablePlaceholders(std::string_view text, std::string_view from, std::string_view to);
} // namespace Data::Items
