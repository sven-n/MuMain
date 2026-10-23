#include "stdafx.h"

#include "ItemDataValidation.h"
#include "ItemAttributeConversion.h"
#include "ItemDescriptionPlaceholders.h"
#include "ItemEnumNames.h"
#include "ItemType.h"
#include "Core/Text/Utf8.h"

#include <algorithm>
#include <cctype>
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

bool ContainsSeparator(const std::string& text)
{
    return text.find(LocalizedString::Separator) != std::string::npos;
}

// Locale codes like "pt" or "zh-TW". Anything else (empty, "=", "||") would
// break the LocalizedString format used for the OpenMU exchange.
bool IsValidLocaleCode(const std::string& locale)
{
    return !locale.empty() &&
           std::all_of(locale.begin(), locale.end(), [](char character)
                       { return std::isalnum(static_cast<unsigned char>(character)) || character == '-'; });
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
        AddIssue(issues, ItemDataIssueSeverity::Error, definition, "the English name is missing");
    }

    const auto check = [&](const std::string& locale, const std::string& text)
    {
        if (ContainsSeparator(text))
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, definition,
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
        if (!IsValidLocaleCode(locale))
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, definition,
                     "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
            continue;
        }
        check(locale, text);
    }
}
// The field of a description line's text in one language, as the item file writes it.
std::string DescriptionTextField(size_t line, std::string_view locale)
{
    std::string field = "description[" + std::to_string(line) + "].text";
    if (locale != LocalizedString::NeutralLocale)
    {
        field += "." + std::string(locale);
    }
    return field;
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

// A description line's text is written like a name: an English text, and translations under language codes. A blank
// line has no text in any language. The braces of its placeholders are checked in every language, and each
// translation has the placeholders of the English text.
void ValidateDescription(const ItemDefinition& definition, std::vector<ItemDataIssue>& issues)
{
    for (size_t index = 0; index < definition.description.size(); ++index)
    {
        const LocalizedString& text = definition.description[index].text;
        if (text.IsEmpty())
        {
            continue;
        }
        const std::string field = DescriptionTextField(index, LocalizedString::NeutralLocale);
        if (text.GetNeutral().empty())
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, definition, "the English text is missing", field.c_str());
        }
        const PlaceholderScan english = ScanPlaceholders(text.GetNeutral());
        const std::set<std::string> englishNames(english.names.begin(), english.names.end());
        if (!english.error.empty())
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, definition, english.error, field.c_str());
        }
        for (const auto& [locale, translation] : text.GetTranslations())
        {
            if (!IsValidLocaleCode(locale))
            {
                AddIssue(issues, ItemDataIssueSeverity::Error, definition,
                         "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)", field.c_str());
            }
            const std::string translationField = DescriptionTextField(index, locale);
            const PlaceholderScan scan = ScanPlaceholders(translation);
            if (!scan.error.empty())
            {
                AddIssue(issues, ItemDataIssueSeverity::Error, definition, scan.error, translationField.c_str());
                continue;
            }
            const std::set<std::string> names(scan.names.begin(), scan.names.end());
            if (english.error.empty() && names != englishNames)
            {
                AddIssue(issues, ItemDataIssueSeverity::Error, definition,
                         "has the placeholders " + ListPlaceholders(names) + ", the English text " +
                             ListPlaceholders(englishNames),
                         translationField.c_str());
            }
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
        AddIssue(issues, ItemDataIssueSeverity::Error, definition,
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
} // namespace

void ValidateItems(std::span<const ItemDefinition> items, std::vector<ItemDataIssue>& issues)
{
    std::set<int> seenItemTypes;
    for (const ItemDefinition& definition : items)
    {
        if (!IsValidItemId(definition.group, definition.number))
        {
            issues.push_back({ItemDataIssueSeverity::Error, "", definition.group, definition.number, "number",
                              "the item id is out of range"});
            continue;
        }

        if (!seenItemTypes.insert(MakeItemType(definition.group, definition.number)).second)
        {
            issues.push_back({ItemDataIssueSeverity::Error, "", definition.group, definition.number, "number",
                              "the item is defined more than once"});
        }

        ValidateName(definition, issues);
        ValidateDescription(definition, issues);
        ValidateSlot(definition, issues);
        ValidatePotionPriority(definition, issues);
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
            AddIssue(issues, ItemDataIssueSeverity::Error, definition,
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
                                     const std::string& statFile, std::vector<ItemDataIssue>& issues)
{
    std::vector<std::string> known(GetBuiltInPlaceholders().begin(), GetBuiltInPlaceholders().end());
    for (const std::string& table : profiles.GetTableNames())
    {
        known.push_back(MakeTablePlaceholder(table));
    }
    for (const ItemDefinition& definition : items)
    {
        if (!definition.Exists())
        {
            continue;
        }
        for (size_t line = 0; line < definition.description.size(); ++line)
        {
            // The translations have the English text's placeholders (ValidateItems checks that).
            const PlaceholderScan scan = ScanPlaceholders(definition.description[line].text.GetNeutral());
            std::set<std::string> reported;
            for (const std::string& name : scan.names)
            {
                if (GetPlaceholderValue(name, 0, profiles) || !reported.insert(name).second)
                {
                    continue;
                }
                // A table written without "table:" is suggested with it. Its own name comes first, also when it is
                // close to a built-in value: {levels} for the table "levels" is not {level}.
                std::string suggestion;
                if (GetPlaceholderTable(name).empty() && profiles.FindTable(name) != nullptr)
                {
                    suggestion = MakeTablePlaceholder(name);
                }
                else
                {
                    suggestion = SuggestPlaceholder(name, known);
                    if (suggestion.empty() && GetPlaceholderTable(name).empty())
                    {
                        suggestion = SuggestPlaceholder(MakeTablePlaceholder(name), known);
                    }
                }
                const std::string hint = suggestion.empty() ? "a placeholder is a built-in value ({" +
                                                                  std::string(LevelPlaceholder) + "}) or a table of " +
                                                                  statFile + " ({" + MakeTablePlaceholder("name") + "})"
                                                            : "did you mean {" + suggestion + "}?";
                issues.push_back({ItemDataIssueSeverity::Error, "", definition.group, definition.number,
                                  DescriptionTextField(line, LocalizedString::NeutralLocale),
                                  "has the unknown placeholder {" + name + "}; " + hint});
            }
        }
    }
}

namespace
{
// The tables an item's description names, by the English texts (the translations have their placeholders).
std::set<std::string> FindDescriptionTables(const ItemDefinition& definition)
{
    std::set<std::string> tables;
    for (const ItemDescriptionLine& line : definition.description)
    {
        const PlaceholderScan scan = ScanPlaceholders(line.text.GetNeutral());
        for (const std::string& name : scan.names)
        {
            if (const std::string_view table = GetPlaceholderTable(name); !table.empty())
            {
                tables.emplace(table);
            }
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

// A wing's lines as the wing check compares them: each line's English text (its tables included).
std::vector<std::string> WingLines(const ItemDefinition& wing)
{
    std::vector<std::string> lines;
    for (const ItemDescriptionLine& line : wing.description)
    {
        lines.push_back(line.text.GetNeutral());
    }
    return lines;
}

std::string DescribeWingLines(const ItemDefinition& wing)
{
    const size_t count = wing.description.size();
    return std::to_string(count) + (count == 1 ? " line" : " lines") + " naming " +
           ListTables(FindDescriptionTables(wing));
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
                       std::vector<ItemDataIssue>& issues)
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
                               " of the other groups: it has " + DescribeWingLines(wing) + ", they have " +
                               DescribeWingLines(*most));
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
                          DescribeWingLines(*having.front());
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
                                std::to_string(group) + ": it has " + DescribeWingLines(*wing) + ", they have " +
                                DescribeWingLines(*most));
            }
        }
    }
}
} // namespace

void ValidateTableUses(std::span<const ItemDefinition> items, const ItemStatProfiles& profiles,
                       const std::string& statFile, std::vector<ItemDataIssue>& issues)
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
        const std::set<std::string> tables = FindDescriptionTables(definition);
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
    ValidateWingLines(wings, issues);
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
        for (size_t index = 0; index < definition.description.size(); ++index)
        {
            for (const auto& [locale, text] : definition.description[index].text.GetTranslations())
            {
                if (!isShown(locale))
                {
                    AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                             "the " + locale + " text is never shown: the game has no texts in that language",
                             DescriptionTextField(index, LocalizedString::NeutralLocale).c_str());
                }
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
            issues.push_back({ItemDataIssueSeverity::Error, "", model.group, model.number, "number",
                              "the model is defined more than once"});
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
            issues.push_back({ItemDataIssueSeverity::Error, sharedModelsSource, ItemDataIssue::NoItem,
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
            issues.push_back({ItemDataIssueSeverity::Error, "", model.group, model.number, "model",
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
            issues.push_back({ItemDataIssueSeverity::Error, colorList, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
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
                issues.push_back({ItemDataIssueSeverity::Error, "", model.group, model.number, field,
                                  "\"" + *name + "\" is not in " + colorList});
            }
        }
    }
}
} // namespace Data::Items
