#include "stdafx.h"

#include "ItemDataValidation.h"
#include "ItemAttributeConversion.h"
#include "ItemDescriptionPlaceholders.h"
#include "ItemEnumNames.h"
#include "ItemType.h"
#include "Core/Text/Utf8.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>

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

// The braces of every text of the description, and the same placeholders in each translation as in the English text.
void ValidateDescriptionPlaceholderSyntax(const ItemDefinition& definition, std::vector<ItemDataIssue>& issues)
{
    for (size_t line = 0; line < definition.description.size(); ++line)
    {
        const LocalizedString& text = definition.description[line].text;
        const PlaceholderScan english = ScanPlaceholders(text.GetNeutral());
        const std::set<std::string> englishNames(english.names.begin(), english.names.end());
        if (!english.error.empty())
        {
            issues.push_back({ItemDataIssueSeverity::Error, "", definition.group, definition.number,
                              DescriptionTextField(line, LocalizedString::NeutralLocale), english.error});
        }
        for (const auto& [locale, translation] : text.GetTranslations())
        {
            const PlaceholderScan scan = ScanPlaceholders(translation);
            const std::string field = DescriptionTextField(line, locale);
            if (!scan.error.empty())
            {
                issues.push_back(
                    {ItemDataIssueSeverity::Error, "", definition.group, definition.number, field, scan.error});
                continue;
            }
            const std::set<std::string> names(scan.names.begin(), scan.names.end());
            if (english.error.empty() && names != englishNames)
            {
                issues.push_back({ItemDataIssueSeverity::Error, "", definition.group, definition.number, field,
                                  "has the placeholders " + ListPlaceholders(names) + ", the English text " +
                                      ListPlaceholders(englishNames)});
            }
        }
    }
}

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
        ValidateSlot(definition, issues);
        ValidatePotionPriority(definition, issues);
        ValidateExcellentBonus(definition, issues);
        ValidateExcellentRaisesLevel(definition, issues);
        ValidateRequirementsByLevel(definition, issues);
        ValidateDescriptionPlaceholderSyntax(definition, issues);
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
                                     const ItemTooltipFile& tooltips, const std::string& statFile,
                                     std::vector<ItemDataIssue>& issues)
{
    std::vector<std::string> builtIns(GetBuiltInPlaceholders().begin(), GetBuiltInPlaceholders().end());
    const std::vector<std::string> tables = profiles.GetTableNames();
    builtIns.insert(builtIns.end(), tables.begin(), tables.end());
    const auto isValue = [&](std::string_view name)
    { return IsBuiltInPlaceholder(name) || profiles.FindTable(name) != nullptr; };
    std::set<std::string, std::less<>> named;
    for (const ItemDefinition& definition : items)
    {
        if (!definition.Exists())
        {
            continue;
        }
        const auto addError = [&](const std::string& field, const std::string& message)
        { issues.push_back({ItemDataIssueSeverity::Error, "", definition.group, definition.number, field, message}); };
        for (const ItemLevelValues& levelValues : definition.levelValues)
        {
            if (isValue(levelValues.name))
            {
                addError("levelValues." + levelValues.name,
                         "is the name of a built-in value or a table of " + statFile + "; a level value needs its own");
            }
        }
        std::set<std::string, std::less<>> shownLevelValues;
        for (size_t line = 0; line < definition.description.size(); ++line)
        {
            const ItemDescriptionLine& entry = definition.description[line];
            const std::string field = "description[" + std::to_string(line) + "]";
            std::vector<std::string> known = builtIns;
            for (const auto& [placeholder, value] : entry.values)
            {
                named.insert(value);
                known.push_back(placeholder);
                if (value.starts_with(SkillManaPrefix))
                {
                    if (!ReadSkillManaValue(value))
                    {
                        addError(field + ".values." + placeholder,
                                 "\"" + value + "\" names no skill: \"skillMana:\" takes a skill's number, below " +
                                     std::to_string(MAX_SKILLS));
                    }
                }
                else if (!isValue(value))
                {
                    const std::string suggestion = SuggestPlaceholder(value, builtIns);
                    addError(field + ".values." + placeholder,
                             "\"" + value + "\" is neither a built-in value, a table of " + statFile +
                                 " nor \"skillMana:<skill>\"" +
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
                if (isValue(name))
                {
                    named.insert(name);
                    continue;
                }
                const std::string suggestion = SuggestPlaceholder(name, known);
                const std::string hint = suggestion.empty()
                                             ? "a placeholder is a built-in value (docs/item-data.md), a "
                                               "table of " +
                                                   statFile +
                                                   ", or one of the line's values or the "
                                                   "item's levelValues"
                                             : "did you mean {" + suggestion + "}?";
                addError(entry.textName.empty() ? DescriptionTextField(line, LocalizedString::NeutralLocale)
                                                : field + ".text",
                         (entry.textName.empty() ? "has" : "\"@" + entry.textName + "\" has") +
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
    for (const std::string& table : tables)
    {
        if (!profiles.IsTableUsedByStats(table) && !named.contains(table))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, statFile, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                              "tables." + table, "neither a stat nor an item description uses this table"});
        }
    }
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
                issues.push_back({ItemDataIssueSeverity::Error, "", definition.group, definition.number, field,
                                  "\"@" + name + "\" is not a text of " + tooltipFile +
                                      (suggestion.empty() ? "" : "; did you mean \"@" + suggestion + "\"?")});
            }
            else if (isLevelValue && !ScanPlaceholders(text->GetNeutral()).names.empty())
            {
                issues.push_back({ItemDataIssueSeverity::Error, "", definition.group, definition.number, field,
                                  "\"@" + name + "\" has placeholders, which a level value cannot have"});
            }
        };
        for (size_t line = 0; line < definition.description.size(); ++line)
        {
            if (!definition.description[line].textName.empty())
            {
                check(definition.description[line].textName, "description[" + std::to_string(line) + "].text", false);
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
                    issues.push_back({ItemDataIssueSeverity::Error, "", definition.group, definition.number, field,
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
        for (const auto& [locale, text] : definition.names.GetTranslations())
        {
            const bool shown = std::any_of(uiLocales.begin(), uiLocales.end(),
                                           [&](const char* uiLocale) { return locale == uiLocale; });
            if (!shown)
            {
                AddIssue(issues, ItemDataIssueSeverity::Warning, definition,
                         "the " + locale + " name is never shown: the game has no texts in that language");
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
