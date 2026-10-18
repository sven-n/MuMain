#include "stdafx.h"

#include "ItemDataValidation.h"
#include "ItemAttributeConversion.h"
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
        ValidateSlot(definition, issues);
        ValidatePotionPriority(definition, issues);
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
