#include "stdafx.h"

#include "ItemDataValidation.h"
#include "ItemAttributeConversion.h"
#include "ItemDescriptionPlaceholders.h"
#include "ItemEnumNames.h"
#include "ItemType.h"
#include "Core/Text/Utf8.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <tuple>

namespace Data::Items
{
namespace
{
constexpr const char* NameField = "name";

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const ItemDefinition& definition,
              const std::string& message, const char* field = NameField)
{
    issues.push_back({severity, "", definition.group, definition.number, field, message});
}

// The game shows names through the MAX_ITEM_NAME-sized ITEM_ATTRIBUTE name.
bool IsTooLongForGame(const std::string& text)
{
    return Core::Text::FromUtf8(text).size() > static_cast<size_t>(MAX_ITEM_NAME - 1);
}

void ValidateName(const ItemDefinition& definition, std::vector<ItemDataIssue>& issues)
{
    const LocalizedString& names = definition.names;
    if (names.GetNeutral().empty())
    {
        AddIssue(issues, ItemDataIssueSeverity::Fatal, definition, "the English name is missing");
    }

    const auto check = [&](const std::string& locale, const std::string& text)
    {
        if (LocalizedString::ContainsSeparator(text))
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, definition,
                     "the " + locale + " name must not contain \"" + std::string(LocalizedString::Separator) + "\"");
        }
        if (IsTooLongForGame(text))
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                     "the " + locale + " name is longer than " + std::to_string(MAX_ITEM_NAME - 1) +
                         " characters and is cut in the game");
        }
    };

    check(std::string(LocalizedString::NeutralLocale), names.GetNeutral());
    for (const auto& [locale, text] : names.GetTranslations())
    {
        if (!LocalizedString::IsValidLocaleCode(locale))
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, definition,
                     "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
            continue;
        }
        check(locale, text);
    }
}
// A line of an item's description or name lines, and the field the item file writes it at ("description[2]").
struct TooltipLineOf
{
    const ItemDescriptionLine* line;
    std::string field;
};

// The item's description lines, then its name lines.
std::vector<TooltipLineOf> TooltipLinesOf(const ItemDefinition& definition)
{
    std::vector<TooltipLineOf> lines;
    for (size_t index = 0; index < definition.description.size(); ++index)
    {
        lines.push_back({&definition.description[index], "description[" + std::to_string(index) + "]"});
    }
    for (size_t index = 0; index < definition.nameLines.size(); ++index)
    {
        lines.push_back({&definition.nameLines[index], "nameLines[" + std::to_string(index) + "]"});
    }
    return lines;
}

// The field of a line's text in one language, as the item file writes it.
std::string TextField(const std::string& lineField, std::string_view locale)
{
    std::string field = lineField + ".text";
    if (locale != LocalizedString::NeutralLocale)
    {
        field += "." + std::string(locale);
    }
    return field;
}

// The field of a description line's text in one language.
std::string DescriptionTextField(size_t line, std::string_view locale)
{
    return TextField("description[" + std::to_string(line) + "]", locale);
}

std::string ListPlaceholders(const std::set<std::string>& names)
{
    if (names.empty())
    {
        return "none";
    }
    std::string list;
    for (const std::string& name : names)
    {
        list += (list.empty() ? "{" : ", {") + name + "}";
    }
    return list;
}

// An item's own text (a description line's, a name line's, a level value's) is written like a name: an English text,
// and translations under language codes. A blank one has no text in any language.
void ValidateItemText(const ItemDefinition& definition, const LocalizedString& text, const std::string& field,
                      std::vector<ItemDataIssue>& issues)
{
    if (text.IsEmpty())
    {
        return;
    }
    if (text.GetNeutral().empty())
    {
        AddIssue(issues, ItemDataIssueSeverity::Fatal, definition, "the English text is missing", field.c_str());
    }
    for (const auto& [locale, translation] : text.GetTranslations())
    {
        if (!LocalizedString::IsValidLocaleCode(locale))
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, definition,
                     "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)", field.c_str());
        }
    }
}

// The braces of a line's placeholders in every language, and the English text's placeholders in each translation.
void ValidateLinePlaceholderBraces(const ItemDefinition& definition, const LocalizedString& text,
                                   const std::string& lineField, std::vector<ItemDataIssue>& issues)
{
    const PlaceholderScan english = ScanPlaceholders(text.GetNeutral());
    const std::set<std::string> englishNames(english.names.begin(), english.names.end());
    if (!english.error.empty())
    {
        AddIssue(issues, ItemDataIssueSeverity::Fatal, definition, english.error,
                 TextField(lineField, LocalizedString::NeutralLocale).c_str());
    }
    for (const auto& [locale, translation] : text.GetTranslations())
    {
        const std::string field = TextField(lineField, locale);
        const PlaceholderScan scan = ScanPlaceholders(translation);
        if (!scan.error.empty())
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, definition, scan.error, field.c_str());
            continue;
        }
        const std::set<std::string> names(scan.names.begin(), scan.names.end());
        if (english.error.empty() && names != englishNames)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, definition,
                     "has the placeholders " + ListPlaceholders(names) + ", the English text " +
                         ListPlaceholders(englishNames),
                     field.c_str());
        }
    }
}

void ValidateDescription(const ItemDefinition& definition, std::vector<ItemDataIssue>& issues)
{
    for (const auto& [line, lineField] : TooltipLinesOf(definition))
    {
        ValidateItemText(definition, line->text, TextField(lineField, LocalizedString::NeutralLocale), issues);
        ValidateLinePlaceholderBraces(definition, line->text, lineField, issues);
    }
    for (const ItemLevelValues& levelValues : definition.levelValues)
    {
        for (size_t level = 0; level < levelValues.values.size(); ++level)
        {
            ValidateItemText(definition, levelValues.values[level].text,
                             "levelValues." + levelValues.name + "[" + std::to_string(level) + "]", issues);
        }
    }
}

// The slot of each weapon kind: the left hand (offHand) for bows, shields and
// books, the right hand (mainHand) for the others. The code looks for a bow or
// a shield only in the left hand and for a crossbow or a spear only in the
// right; one-handed swords, axes, maces, staffs and gloves may also go in the
// left.
ItemSlot GetWeaponKindSlot(WeaponKind kind)
{
    switch (kind)
    {
    case WeaponKind::Bow:
    case WeaponKind::Shield:
    case WeaponKind::Book:
        return ItemSlot::OffHand;
    case WeaponKind::Crossbow:
    case WeaponKind::Spear:
    case WeaponKind::Sword:
    case WeaponKind::Axe:
    case WeaponKind::Mace:
    case WeaponKind::Staff:
    case WeaponKind::Glove:
        return ItemSlot::MainHand;
    default:
        return ItemSlot::None;
    }
}

// The item files store slots by name, so every slot needs one.
void ValidateSlot(const ItemDefinition& definition, std::vector<ItemDataIssue>& issues)
{
    if (definition.slot != ItemSlot::None && FindEnumName(definition.slot) == nullptr)
    {
        AddIssue(issues, ItemDataIssueSeverity::Fatal, definition,
                 "slot " + std::to_string(static_cast<int>(definition.slot)) + " is not an equipment slot", "slot");
    }
    if (definition.wingTier != WingTier::None && definition.slot != ItemSlot::Wings)
    {
        AddIssue(issues, ItemDataIssueSeverity::Warning, definition, "has a wing tier, but its slot is not \"wings\"",
                 "wingTier");
    }
    // Item files saved before a group got its weapon kind have none; nothing fills it in.
    const WeaponKind groupWeaponKind = GetGroupWeaponKind(definition);
    if (definition.weaponKind == WeaponKind::None && groupWeaponKind != WeaponKind::None)
    {
        AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                 std::string("has no weapon kind, but its group gives \"") + FindEnumName(groupWeaponKind) + "\"",
                 "weaponKind");
    }
    const ItemSlot weaponKindSlot = GetWeaponKindSlot(definition.weaponKind);
    if (weaponKindSlot != ItemSlot::None && definition.slot != weaponKindSlot)
    {
        AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                 std::string("has the weapon kind \"") + FindEnumName(definition.weaponKind) +
                     "\", but its slot is not \"" + FindEnumName(weaponKindSlot) + "\"",
                 "weaponKind");
    }
}

// The fixed excellent bonus counts for damage and magic power only.
void ValidateExcellentBonus(const ItemDefinition& definition, std::vector<ItemDataIssue>& issues)
{
    if (definition.excellentBonus != 0 && definition.damageMin == 0 && definition.damageMax == 0 &&
        definition.magicPower == 0)
    {
        AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                 "has an excellent bonus, but no damage or magic power it adds to", "excellentBonus");
    }
}

// Item files saved before the stat profiles have none; the item then has the
// default until an edit or a bmd import gives it the profile its data gives.
void ValidateStatProfileGiven(const ItemDefinition& definition, const ItemStatProfiles& profiles,
                              std::vector<ItemDataIssue>& issues)
{
    const std::string derived = DeriveStatProfile(profiles, definition);
    if (definition.statProfile.empty() && !derived.empty())
    {
        AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                 "has no stat profile, but its data gives \"" + derived + "\" (\"" + StatProfileNames::Default +
                     "\" keeps the default)",
                 "statProfile");
    }
}

// excellentRaisesLevel only matters for an item with a level requirement.
void ValidateExcellentRaisesLevel(const ItemDefinition& definition, std::vector<ItemDataIssue>& issues)
{
    const bool hasLevelRequirement =
        definition.requirements.level > 0 || definition.requirementsByLevel.Find(ItemRequirement::Level) != nullptr;
    if (!definition.excellentRaisesLevel && !hasLevelRequirement)
    {
        AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                 "is set, but the item has no level requirement it would raise", "excellentRaisesLevel");
    }
}

// A requirement given by level takes the place of its base value.
void ValidateRequirementsByLevel(const ItemDefinition& definition, std::vector<ItemDataIssue>& issues)
{
    const WORD bases[] = {definition.requirements.level,     definition.requirements.strength,
                          definition.requirements.dexterity, definition.requirements.energy,
                          definition.requirements.vitality,  definition.requirements.leadership};
    for (size_t requirement = 0; requirement < ItemRequirementCount; ++requirement)
    {
        if (definition.requirementsByLevel.Find(static_cast<ItemRequirement>(requirement)) != nullptr &&
            bases[requirement] != 0)
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                     "gives a requirement by level and also its base value, which is not used", "requirementsByLevel");
        }
    }
}

// Only the potion lists are taken in priority order.
void ValidatePotionPriority(const ItemDefinition& definition, std::vector<ItemDataIssue>& issues)
{
    const ItemTagSet potions{ItemTag::HealingPotion, ItemTag::ManaPotion, ItemTag::ComplexPotion,
                             ItemTag::ShieldPotion};
    if (definition.potionPriority != 0 && !definition.tags.HasAny(potions))
    {
        AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                 "has a potion priority, but no healingPotion, manaPotion, complexPotion or shieldPotion tag",
                 "potionPriority");
    }
}

// The kinds the original client named by these tags in its list of items without class lines
// (IsRequireClassRenderItem); every item of them has the tag noClassLines.
constexpr ItemTag KindsWithoutClassLines[] = {
    ItemTag::ElitePotion, ItemTag::BuffScroll, ItemTag::Elixir, ItemTag::GambleItem, ItemTag::BattleOrStrengthScroll,
    ItemTag::GuardianPet};

// An item of one of those kinds without noClassLines shows class lines in its tooltip, unlike the others: an item
// added later of a kind that used to get it from the list.
void ValidateClassLines(const ItemDefinition& definition, std::vector<ItemDataIssue>& issues)
{
    if (definition.tags.Has(ItemTag::NoClassLines))
    {
        return;
    }
    for (const ItemTag kind : KindsWithoutClassLines)
    {
        if (definition.tags.Has(kind))
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                     std::string("has the tag ") + FindEnumName(kind) +
                         " but not noClassLines: its tooltip shows class lines, unlike the other items of its kind",
                     "tags");
            return;
        }
    }
}
} // namespace

void ValidateItems(std::span<const ItemDefinition> items, std::vector<ItemDataIssue>& issues)
{
    std::set<int> seenItemTypes;
    for (const ItemDefinition& definition : items)
    {
        if (!IsValidItemId(definition.group, definition.number))
        {
            issues.push_back({ItemDataIssueSeverity::Fatal, "", definition.group, definition.number, "number",
                              "the item id is out of range"});
            continue;
        }

        if (!seenItemTypes.insert(MakeItemType(definition.group, definition.number)).second)
        {
            issues.push_back({ItemDataIssueSeverity::Fatal, "", definition.group, definition.number, "number",
                              "the item is defined more than once"});
        }
        // The game does not know an entry without a name; its values are checked once it has one.
        if (definition.entryWithoutName && !definition.Exists())
        {
            continue;
        }

        ValidateName(definition, issues);
        ValidateDescription(definition, issues);
        ValidateSlot(definition, issues);
        ValidatePotionPriority(definition, issues);
        ValidateClassLines(definition, issues);
        ValidateExcellentBonus(definition, issues);
        ValidateExcellentRaisesLevel(definition, issues);
        ValidateRequirementsByLevel(definition, issues);
    }
}

// A fixed excellent bonus takes the place of the excellent term of damage and magic power,
// so it only counts where the item's profile has that term.
void ValidateExcellentBonusTerms(const ItemDefinition& definition, const ItemStatProfile& profile,
                                 std::vector<ItemDataIssue>& issues)
{
    if (definition.excellentBonus == 0)
    {
        return;
    }
    const bool damageWithoutTerm =
        (definition.damageMin > 0 || definition.damageMax > 0) && !profile.Get(ItemStat::Damage).excellent;
    const bool magicPowerWithoutTerm = definition.magicPower > 0 && !profile.Get(ItemStat::MagicPower).excellent;
    if (damageWithoutTerm || magicPowerWithoutTerm)
    {
        AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                 std::string("has an excellent bonus, but its stat profile has no excellent term for its ") +
                     (damageWithoutTerm ? "damage" : "magic power") + ", so the bonus does not count there",
                 "excellentBonus");
    }
}

void ValidateStatProfiles(std::span<const ItemDefinition> items, const ItemStatProfiles& profiles,
                          const std::string& statFile, std::vector<ItemDataIssue>& issues)
{
    std::set<std::string> used;
    for (const ItemDefinition& definition : items)
    {
        // A deleted item keeps its fields in the item database, but is not saved.
        if (!definition.Exists())
        {
            continue;
        }
        const int profile = profiles.Find(definition.statProfile);
        if (profile == ItemStatProfiles::UnknownProfile)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, definition,
                     "\"" + definition.statProfile + "\" is not a profile of " + statFile, "statProfile");
            continue;
        }
        ValidateExcellentBonusTerms(definition, profiles.Get(profile), issues);
        ValidateStatProfileGiven(definition, profiles, issues);
        if (!definition.statProfile.empty())
        {
            used.insert(definition.statProfile);
        }
    }
    for (const std::string& name : profiles.GetProfileNames())
    {
        if (!used.contains(name))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, statFile, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                              "profiles." + name, "no item names this profile"});
        }
    }
}

void ValidateDescriptionPlaceholders(std::span<const ItemDefinition> items, const ItemStatProfiles& profiles,
                                     const ItemTooltipFile& tooltips, const std::string& statFile,
                                     std::vector<ItemDataIssue>& issues)
{
    std::vector<std::string> builtIns(GetBuiltInPlaceholders().begin(), GetBuiltInPlaceholders().end());
    const std::vector<std::string> tables = profiles.GetTableNames();
    for (const std::string& table : tables)
    {
        builtIns.push_back(MakeTablePlaceholder(table));
    }
    const auto isValue = [&](std::string_view name)
    { return IsBuiltInPlaceholder(name) || GetPlaceholderValue(name, 0, profiles).has_value(); };
    // A table written without "table:" is suggested with it. Its own name comes first, also when it is close to a
    // built-in value: {levels} for the table "levels" is not {level}.
    const auto suggest = [&](const std::string& name, std::span<const std::string> candidates)
    {
        if (GetPlaceholderTable(name).empty() && profiles.FindTable(name) != nullptr)
        {
            return MakeTablePlaceholder(name);
        }
        std::string suggestion = SuggestPlaceholder(name, candidates);
        if (suggestion.empty() && GetPlaceholderTable(name).empty())
        {
            suggestion = SuggestPlaceholder(MakeTablePlaceholder(name), candidates);
        }
        return suggestion;
    };
    for (const ItemDefinition& definition : items)
    {
        if (!definition.Exists())
        {
            continue;
        }
        const auto addError = [&](const std::string& field, const std::string& message)
        { issues.push_back({ItemDataIssueSeverity::Fatal, "", definition.group, definition.number, field, message}); };
        std::set<std::string, std::less<>> shownLevelValues;
        for (const auto& [line, field] : TooltipLinesOf(definition))
        {
            const ItemDescriptionLine& entry = *line;
            std::vector<std::string> known = builtIns;
            for (const auto& [placeholder, value] : entry.values)
            {
                known.push_back(placeholder);
                if (HasGameDataValuePrefix(value))
                {
                    if (!ReadGameDataValue(value))
                    {
                        addError(field + ".values." + placeholder,
                                 "\"" + value +
                                     "\" names no skill or monster: \"skillMana:\" and \"skillName:\" "
                                     "take a skill's number below " +
                                     std::to_string(MAX_SKILLS) + ", \"monsterName:\" a monster's below " +
                                     std::to_string(MAX_MONSTER));
                    }
                }
                else if (!isValue(value))
                {
                    const std::string suggestion = suggest(value, builtIns);
                    addError(field + ".values." + placeholder,
                             "\"" + value + "\" is neither a built-in value, a table of " + statFile + " (\"" +
                                 MakeTablePlaceholder("name") +
                                 "\") nor a game data value (\"skillMana:<skill>\", ...)" +
                                 (suggestion.empty() ? "" : "; did you mean \"" + suggestion + "\"?"));
                }
            }
            for (const ItemLevelValues& levelValues : definition.levelValues)
            {
                known.push_back(levelValues.name);
            }
            // The translations have the English text's placeholders (ValidateItems checks that); a shared text the
            // tooltip file does not have is ValidateDescriptionTexts' error.
            const LocalizedString* text = entry.textName.empty() ? &entry.text : tooltips.FindText(entry.textName);
            if (text == nullptr)
            {
                continue;
            }
            const PlaceholderScan scan = ScanPlaceholders(text->GetNeutral());
            // A value the text doesn't show is likely left on the wrong line (as a layout line's).
            for (const auto& [placeholder, value] : entry.values)
            {
                if (std::find(scan.names.begin(), scan.names.end(), placeholder) == scan.names.end())
                {
                    issues.push_back({ItemDataIssueSeverity::Warning, "", definition.group, definition.number,
                                      field + ".values." + placeholder,
                                      "the line's text has no {" + placeholder + "}"});
                }
            }
            std::set<std::string> reported;
            for (const std::string& name : scan.names)
            {
                if (!reported.insert(name).second)
                {
                    continue;
                }
                const bool bound = std::any_of(entry.values.begin(), entry.values.end(),
                                               [&](const auto& value) { return value.first == name; });
                if (bound)
                {
                    continue;
                }
                if (const ItemLevelValues* levelValues = definition.FindLevelValues(name))
                {
                    shownLevelValues.insert(name);
                    if (entry.maxLevel >= static_cast<int>(levelValues->values.size()))
                    {
                        addError(field + ".levels", "the line shows {" + name + "} up to +" +
                                                        std::to_string(entry.maxLevel) + ", but levelValues." + name +
                                                        " has values only up to +" +
                                                        std::to_string(levelValues->values.size() - 1));
                    }
                    continue;
                }
                // A shared text names a table only through the line's "values": the editor's table counts and its
                // rename read an item's own data, not the tooltip file.
                if (!entry.textName.empty() && !GetPlaceholderTable(name).empty())
                {
                    addError(field + ".text", "\"@" + entry.textName + "\" names the table {" + name +
                                                  "} itself; a shared text names a table only through the line's " +
                                                  "\"values\" (\"values\": {\"percent\": \"" + name + "\"})");
                    continue;
                }
                if (isValue(name))
                {
                    continue;
                }
                const std::string suggestion = suggest(name, known);
                const std::string hint = suggestion.empty()
                                             ? "a placeholder is a built-in value (docs/item-data.md), a table of " +
                                                   statFile + " ({" + MakeTablePlaceholder("name") +
                                                   "}), or one of the line's values or the item's levelValues"
                                             : "did you mean {" + suggestion + "}?";
                addError(field + ".text", (entry.textName.empty() ? "has" : "\"@" + entry.textName + "\" has") +
                                              std::string(" the unknown placeholder {") + name + "}; " + hint);
            }
        }
        for (const ItemLevelValues& levelValues : definition.levelValues)
        {
            if (!shownLevelValues.contains(levelValues.name))
            {
                issues.push_back({ItemDataIssueSeverity::Warning, "", definition.group, definition.number,
                                  "levelValues." + levelValues.name, "no line of the description shows it"});
            }
        }
    }
}

namespace
{
// The tables an item's description and name lines name: in their texts, written in place or shared in `tooltips`
// (the English texts; the translations have their placeholders), and in their "values".
std::set<std::string> FindDescriptionTables(const ItemDefinition& definition, const ItemTooltipFile& tooltips)
{
    std::set<std::string> tables;
    const auto add = [&](std::string_view name)
    {
        if (const std::string_view table = GetPlaceholderTable(name); !table.empty())
        {
            tables.emplace(table);
        }
    };
    for (const auto& [line, field] : TooltipLinesOf(definition))
    {
        for (const auto& [placeholder, value] : line->values)
        {
            add(value);
        }
        const LocalizedString* text = line->textName.empty() ? &line->text : tooltips.FindText(line->textName);
        if (text == nullptr)
        {
            continue;
        }
        const PlaceholderScan scan = ScanPlaceholders(text->GetNeutral());
        for (const std::string& name : scan.names)
        {
            add(name);
        }
    }
    return tables;
}

std::string ListTables(const std::set<std::string>& tables)
{
    if (tables.empty())
    {
        return "no table";
    }
    std::string list;
    for (const std::string& table : tables)
    {
        list += (list.empty() ? "{" : ", {") + MakeTablePlaceholder(table) + "}";
    }
    return list;
}

// A wing's lines as the wing check compares them: each line's shared text ("@name") or English text (its tables
// included), and the values its "values" names.
std::vector<std::string> WingLines(const ItemDefinition& wing)
{
    std::vector<std::string> lines;
    for (const ItemDescriptionLine& line : wing.description)
    {
        std::string text = line.textName.empty() ? line.text.GetNeutral() : "@" + line.textName;
        for (const auto& [placeholder, value] : line.values)
        {
            text += " " + placeholder + "=" + value;
        }
        lines.push_back(std::move(text));
    }
    return lines;
}

std::string DescribeWingLines(const ItemDefinition& wing, const ItemTooltipFile& tooltips)
{
    const size_t count = wing.description.size();
    return std::to_string(count) + (count == 1 ? " line" : " lines") + " naming " +
           ListTables(FindDescriptionTables(wing, tooltips));
}

std::string DescribeWingKind(WingTier tier, bool cape)
{
    return std::string(FindEnumName(tier)) + "-tier " + (cape ? "capes" : "wings");
}

// The wings of a kind: those that have each set of lines, most first.
using WingsByLines = std::vector<std::vector<const ItemDefinition*>>;
WingsByLines SortByLines(const std::vector<const ItemDefinition*>& wings)
{
    std::map<std::vector<std::string>, std::vector<const ItemDefinition*>> byLines;
    for (const ItemDefinition* wing : wings)
    {
        byLines[WingLines(*wing)].push_back(wing);
    }
    WingsByLines sorted;
    for (auto& [lines, having] : byLines)
    {
        sorted.push_back(std::move(having));
    }
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const auto& first, const auto& second) { return first.size() > second.size(); });
    return sorted;
}

// The lines most of the wings that have lines have, or nothing when two sets of lines are as common. Wings without
// lines never set the lines the others are compared with: new wings imported without their lines, more of them than
// there are wings with them, would otherwise point at the wings that have them.
const ItemDefinition* FindMostLines(const WingsByLines& sorted)
{
    WingsByLines withLines;
    std::copy_if(sorted.begin(), sorted.end(), std::back_inserter(withLines),
                 [](const std::vector<const ItemDefinition*>& having) { return !having.front()->description.empty(); });
    if (withLines.empty() || (withLines.size() > 1 && withLines[0].size() == withLines[1].size()))
    {
        return nullptr;
    }
    return withLines[0].front();
}

using WingKey = std::tuple<WingTier, bool, int>;

// The wings of a tier show the same lines: the tooltip code chose them by the tier (and the cape tag) before they were
// data. The wings are compared within their tier, cape tag and group; a wing alone there is compared with the wings of
// its tier and cape tag in the other groups that have more than one, so Cape of Fighter (12) and Cape of Lord (13),
// which the code wrote apart, are not compared with each other.
void ValidateWingLines(const std::map<WingKey, std::vector<const ItemDefinition*>>& wings,
                       const ItemTooltipFile& tooltips, std::vector<ItemDataIssue>& issues)
{
    const auto warn = [&](const ItemDefinition& wing, const std::string& message)
    { issues.push_back({ItemDataIssueSeverity::Warning, "", wing.group, wing.number, "description", message}); };
    for (const auto& [key, kindWings] : wings)
    {
        const auto& [tier, cape, group] = key;
        if (kindWings.size() == 1)
        {
            std::vector<const ItemDefinition*> others;
            for (const auto& [otherKey, otherWings] : wings)
            {
                if (std::get<0>(otherKey) == tier && std::get<1>(otherKey) == cape && std::get<2>(otherKey) != group &&
                    otherWings.size() > 1)
                {
                    others.insert(others.end(), otherWings.begin(), otherWings.end());
                }
            }
            const ItemDefinition* most = FindMostLines(SortByLines(others));
            const ItemDefinition& wing = *kindWings.front();
            if (most != nullptr && WingLines(wing) != WingLines(*most))
            {
                warn(wing, "its wing lines are not those of the " + DescribeWingKind(tier, cape) +
                               " of the other groups: it has " + DescribeWingLines(wing, tooltips) + ", they have " +
                               DescribeWingLines(*most, tooltips));
            }
            continue;
        }
        const WingsByLines sorted = SortByLines(kindWings);
        if (sorted.size() == 1)
        {
            continue;
        }
        const ItemDefinition* most = FindMostLines(sorted);
        if (most == nullptr)
        {
            // A tie: no set of lines is the one the others differ from.
            std::string counts;
            for (const auto& having : sorted)
            {
                counts += (counts.empty() ? "" : ", ") + std::to_string(having.size()) + " have " +
                          DescribeWingLines(*having.front(), tooltips);
            }
            warn(*kindWings.front(), "the " + DescribeWingKind(tier, cape) + " of group " + std::to_string(group) +
                                         " don't agree on their wing lines: " + counts);
            continue;
        }
        for (const ItemDefinition* wing : kindWings)
        {
            if (WingLines(*wing) != WingLines(*most))
            {
                warn(*wing, "its wing lines are not those of the other " + DescribeWingKind(tier, cape) + " of group " +
                                std::to_string(group) + ": it has " + DescribeWingLines(*wing, tooltips) +
                                ", they have " + DescribeWingLines(*most, tooltips));
            }
        }
    }
}
} // namespace

void ValidateTableUses(std::span<const ItemDefinition> items, const ItemStatProfiles& profiles,
                       const ItemTooltipFile& tooltips, const std::string& statFile, std::vector<ItemDataIssue>& issues)
{
    std::set<std::string, std::less<>> named;
    // The wings by tier, cape tag and group.
    std::map<WingKey, std::vector<const ItemDefinition*>> wings;
    for (const ItemDefinition& definition : items)
    {
        if (!definition.Exists())
        {
            continue;
        }
        const std::set<std::string> tables = FindDescriptionTables(definition, tooltips);
        named.insert(tables.begin(), tables.end());
        if (definition.wingTier != WingTier::None)
        {
            wings[{definition.wingTier, definition.tags.Has(ItemTag::Cape), definition.group}].push_back(&definition);
        }
    }
    for (const std::string& table : profiles.GetTableNames())
    {
        if (!profiles.IsTableUsedByStats(table) && !named.contains(table))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, statFile, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                              "tables." + table, "neither a stat nor an item description uses this table"});
        }
    }
    ValidateWingLines(wings, tooltips, issues);
}

void ValidateDescriptionTexts(std::span<const ItemDefinition> items, const ItemTooltipFile& tooltips,
                              const std::string& tooltipFile, std::vector<ItemDataIssue>& issues)
{
    std::vector<std::string> textNames;
    for (const auto& [name, text] : tooltips.texts)
    {
        textNames.push_back(name);
    }
    std::set<std::string, std::less<>> used;
    for (const TooltipLayout& layout : tooltips.layouts)
    {
        for (const TooltipLayoutLine& line : layout.lines)
        {
            used.insert(line.textName);
        }
    }
    for (const ItemDefinition& definition : items)
    {
        if (!definition.Exists())
        {
            continue;
        }
        const auto check = [&](const std::string& name, const std::string& field, bool isLevelValue)
        {
            used.insert(name);
            const LocalizedString* text = tooltips.FindText(name);
            if (text == nullptr)
            {
                const std::string suggestion = SuggestPlaceholder(name, textNames);
                issues.push_back({ItemDataIssueSeverity::Fatal, "", definition.group, definition.number, field,
                                  "\"@" + name + "\" is not a text of " + tooltipFile +
                                      (suggestion.empty() ? "" : "; did you mean \"@" + suggestion + "\"?")});
            }
            else if (isLevelValue && !ScanPlaceholders(text->GetNeutral()).names.empty())
            {
                issues.push_back({ItemDataIssueSeverity::Fatal, "", definition.group, definition.number, field,
                                  "\"@" + name + "\" has placeholders, which a level value cannot have"});
            }
        };
        for (const auto& [line, field] : TooltipLinesOf(definition))
        {
            if (!line->textName.empty())
            {
                check(line->textName, field + ".text", false);
            }
        }
        for (const ItemLevelValues& levelValues : definition.levelValues)
        {
            for (size_t level = 0; level < levelValues.values.size(); ++level)
            {
                const ItemLevelValue& value = levelValues.values[level];
                const std::string field = "levelValues." + levelValues.name + "[" + std::to_string(level) + "]";
                if (!value.textName.empty())
                {
                    check(value.textName, field, true);
                }
                else if (!ScanPlaceholders(value.text.GetNeutral()).names.empty())
                {
                    issues.push_back({ItemDataIssueSeverity::Fatal, "", definition.group, definition.number, field,
                                      "has placeholders, which a level value cannot have"});
                }
            }
        }
    }
    for (const std::string& name : textNames)
    {
        if (!used.contains(name))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, tooltipFile, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                              "texts." + name, "neither a layout nor an item description uses this text"});
        }
    }
}

void ValidateNameLocales(std::span<const ItemDefinition> items, std::span<const char* const> uiLocales,
                         std::vector<ItemDataIssue>& issues)
{
    for (const ItemDefinition& definition : items)
    {
        if (!definition.Exists())
        {
            continue;
        }
        const auto isShown = [&](const std::string& locale)
        {
            return std::any_of(uiLocales.begin(), uiLocales.end(),
                               [&](const char* uiLocale) { return locale == uiLocale; });
        };
        for (const auto& [locale, text] : definition.names.GetTranslations())
        {
            if (!isShown(locale))
            {
                AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                         "the " + locale + " name is never shown: the game has no texts in that language");
            }
        }
        const auto warnAbout = [&](const LocalizedString& text, const std::string& field)
        {
            for (const auto& [locale, translation] : text.GetTranslations())
            {
                if (!isShown(locale))
                {
                    AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                             "the " + locale + " text is never shown: the game has no texts in that language",
                             field.c_str());
                }
            }
        };
        for (size_t index = 0; index < definition.description.size(); ++index)
        {
            warnAbout(definition.description[index].text, DescriptionTextField(index, LocalizedString::NeutralLocale));
        }
        for (size_t index = 0; index < definition.nameLines.size(); ++index)
        {
            warnAbout(definition.nameLines[index].text, "nameLines[" + std::to_string(index) + "].text");
        }
        for (const ItemLevelValues& levelValues : definition.levelValues)
        {
            for (size_t level = 0; level < levelValues.values.size(); ++level)
            {
                warnAbout(levelValues.values[level].text,
                          "levelValues." + levelValues.name + "[" + std::to_string(level) + "]");
            }
        }
    }
}

void ValidateItemModels(std::span<const ItemModelDefinition> models, std::vector<ItemDataIssue>& issues)
{
    std::set<int> seenItemTypes;
    for (const ItemModelDefinition& model : models)
    {
        if (!seenItemTypes.insert(MakeItemType(model.group, model.number)).second)
        {
            issues.push_back({ItemDataIssueSeverity::Fatal, "", model.group, model.number, "number",
                              "the model is defined more than once"});
        }
    }
}

void ValidateModelsHaveItemEntries(std::span<const ItemDefinition> items, std::span<const ItemModelDefinition> models,
                                   std::vector<ItemDataIssue>& issues)
{
    std::set<int> entries;
    for (const ItemDefinition& item : items)
    {
        if (item.HasEntry() && IsValidItemId(item.group, item.number))
        {
            entries.insert(MakeItemType(item.group, item.number));
        }
    }
    for (const ItemModelDefinition& model : models)
    {
        if (model.Exists() && IsValidItemId(model.group, model.number) &&
            !entries.contains(MakeItemType(model.group, model.number)))
        {
            issues.push_back({ItemDataIssueSeverity::Error, "", model.group, model.number, "number",
                              "has a model but no entry in the item file; add {\"number\": " +
                                  std::to_string(model.number) + "} there"});
        }
    }
}

void ApplySharedItemModels(std::vector<ItemModelDefinition>& models, std::span<const SharedItemModel> sharedModels,
                           const std::string& sharedModelsSource, std::vector<ItemDataIssue>& issues)
{
    std::map<std::string, const SharedItemModel*, std::less<>> byName;
    for (const SharedItemModel& shared : sharedModels)
    {
        if (!byName.emplace(shared.name, &shared).second)
        {
            issues.push_back({ItemDataIssueSeverity::Fatal, sharedModelsSource, ItemDataIssue::NoItem,
                              ItemDataIssue::NoItem, shared.name, "the shared model is defined more than once"});
        }
    }

    std::set<std::string, std::less<>> usedNames;
    for (ItemModelDefinition& model : models)
    {
        if (model.model.empty())
        {
            continue;
        }
        const auto shared = byName.find(model.model);
        if (shared == byName.end())
        {
            // An entry that could not be read is left out, with an error of its own.
            issues.push_back({ItemDataIssueSeverity::Fatal, "", model.group, model.number, "model",
                              "the shared model \"" + model.model + "\" is not in " + sharedModelsSource +
                                  " or could not be read there"});
            continue;
        }
        model.file = shared->second->file;
        model.textureFolders = shared->second->textureFolders;
        model.noneBlendMeshes = shared->second->noneBlendMeshes;
        usedNames.insert(model.model);
    }

    for (const auto& [name, shared] : byName)
    {
        if (!usedNames.contains(name))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, sharedModelsSource, ItemDataIssue::NoItem,
                              ItemDataIssue::NoItem, name, "no item uses the shared model"});
        }
    }
}

void ValidateItemModelLooks(std::span<const ItemModelDefinition> models, LookNameExists renderStyleExists,
                            LookNameExists itemEffectExists, std::vector<ItemDataIssue>& issues)
{
    const auto check =
        [&](const ItemModelDefinition& model, const char* field, const std::string& name, LookNameExists exists)
    {
        if (!name.empty() && !exists(name))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, "", model.group, model.number, field,
                              "there is no look named \"" + name + "\"; the model is drawn plainly"});
        }
    };
    for (const ItemModelDefinition& model : models)
    {
        check(model, "renderStyle", model.renderStyle, renderStyleExists);
        check(model, "itemEffect", model.itemEffect, itemEffectExists);
    }
}

void ValidateDirectlySharedModelFiles(std::span<const ItemModelDefinition> models, std::vector<ItemDataIssue>& issues)
{
    std::map<std::string, int, std::less<>> entriesByFile;
    for (const ItemModelDefinition& model : models)
    {
        if (model.model.empty() && !model.file.empty())
        {
            ++entriesByFile[model.file];
        }
    }
    for (const ItemModelDefinition& model : models)
    {
        if (model.model.empty() && !model.file.empty() && entriesByFile[model.file] > 1)
        {
            issues.push_back({ItemDataIssueSeverity::Warning, "", model.group, model.number, "file",
                              "other items name " + model.file + " too; a shared model would open it once"});
        }
    }
}

void ValidateItemModelGlowColors(std::span<const ItemModelDefinition> models,
                                 std::span<const Effects::GlowColor> colors, std::vector<ItemDataIssue>& issues)
{
    std::set<std::string, std::less<>> names;
    for (const Effects::GlowColor& color : colors)
    {
        names.insert(color.name);
    }

    const std::string colorList = Effects::GlowColorsFile;
    for (const char* name : {ItemGlow::DefaultColor, ItemGlow::DefaultShineColor, ItemGlow::DefaultAncientColor})
    {
        if (!names.contains(name))
        {
            issues.push_back({ItemDataIssueSeverity::Fatal, colorList, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                              "colors", "the default glow color \"" + std::string(name) + "\" is missing"});
        }
    }

    for (const ItemModelDefinition& model : models)
    {
        const std::pair<const char*, const std::string*> fields[] = {{"glow.color", &model.glow.color},
                                                                     {"glow.shineColor", &model.glow.shineColor},
                                                                     {"glow.ancientColor", &model.glow.ancientColor}};
        for (const auto& [field, name] : fields)
        {
            if (!names.contains(*name))
            {
                issues.push_back({ItemDataIssueSeverity::Fatal, "", model.group, model.number, field,
                                  "\"" + *name + "\" is not in " + colorList});
            }
        }
    }
}
} // namespace Data::Items
