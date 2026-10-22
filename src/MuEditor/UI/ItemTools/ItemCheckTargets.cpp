#include "stdafx.h"

#ifdef _EDITOR

#include "ItemCheckTargets.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/GameData/Common/LocalizedString.h"

#include <algorithm>
#include <span>

namespace MuEditor::Items
{
namespace
{
// A field names the key of the item or model entry first, then what is in
// it: "glow.color", "classRequirements.darkWizard", "requirementsByLevel.level[3]".
constexpr char SubKeySeparator = '.';
constexpr std::string_view KeyEnds = ".[";

constexpr std::string_view NameKey = "name";
constexpr std::string_view TagsKey = "tags";

// The keys of a model entry in the model files.
constexpr std::string_view ModelKeys[] = {"file",  "model", "textureFolders", "noneBlendMeshes", "inventory", "ground",
                                          "cloth", "glow",  "material",       "renderStyle",     "itemEffect"};
constexpr std::string_view RequirementKeys[] = {"requirements", "requirementsByLevel", "classRequirements",
                                                "excellentRaisesLevel"};
// The rule flags, in the order of the item data (ItemRuleFlags).
constexpr std::string_view RuleKeys[] = {"tradable",
                                         "droppable",
                                         "storable",
                                         "sellable",
                                         "personalShopSellable",
                                         "repairable",
                                         "droppableWhileRented",
                                         "personalShopSellableWhileRented",
                                         "sellableWhenRentalExpired"};
// What makes an item the item it is; the browser moves an item to another number.
constexpr std::string_view IdentityKeys[] = {"number", "group"};

// How the item checks name the language of a name problem: "the de name ...",
// "the English name ...", "\"d e\" is not a language code ...".
constexpr std::string_view NameMessageStart = "the ";
constexpr std::string_view NameMessageAfterLanguage = " name";
constexpr std::string_view EnglishInMessage = "English";
constexpr std::string_view NotALanguageCode = "is not a language code";

bool IsOneOf(std::string_view key, std::span<const std::string_view> keys)
{
    return std::find(keys.begin(), keys.end(), key) != keys.end();
}

// "glow" of "glow.color", "requirementsByLevel" of "requirementsByLevel.level[3]".
std::string_view GetTopKey(std::string_view field)
{
    return field.substr(0, field.find_first_of(KeyEnds));
}

// "de" of "name.de"; empty when the field is only a key.
std::string_view GetSubKey(std::string_view field)
{
    const size_t separator = field.find(SubKeySeparator);
    return separator == std::string_view::npos ? std::string_view{} : field.substr(separator + 1);
}

// A name problem of the item checks names its language in the message.
bool IsAboutTranslatedName(std::string_view message)
{
    if (message.find(NotALanguageCode) != std::string_view::npos)
    {
        return true;
    }
    if (!message.starts_with(NameMessageStart))
    {
        return false;
    }
    const std::string_view afterStart = message.substr(NameMessageStart.size());
    const size_t languageEnd = afterStart.find(NameMessageAfterLanguage);
    if (languageEnd == std::string_view::npos)
    {
        return false;
    }
    const std::string_view language = afterStart.substr(0, languageEnd);
    return language != EnglishInMessage && language != Data::LocalizedString::NeutralLocale;
}

// The English name is a basic of the item; the other languages are in the translations.
ItemTool GetToolForName(std::string_view field)
{
    const std::string_view language = GetSubKey(field);
    const bool english = language.empty() || language == Data::LocalizedString::NeutralLocale;
    return english ? ItemTool::Basics : ItemTool::Translations;
}
} // namespace

ItemTool GetToolForField(std::string_view field)
{
    const std::string_view key = GetTopKey(field);
    if (key.empty() || IsOneOf(key, IdentityKeys))
    {
        return ItemTool::Browser;
    }
    if (key == NameKey)
    {
        return GetToolForName(field);
    }
    if (key == TagsKey)
    {
        return ItemTool::Categories;
    }
    if (IsOneOf(key, ModelKeys))
    {
        return ItemTool::Model;
    }
    if (IsOneOf(key, RequirementKeys))
    {
        return ItemTool::Requirements;
    }
    if (IsOneOf(key, RuleKeys))
    {
        return ItemTool::Rules;
    }
    return ItemTool::Basics;
}

ItemTool GetToolForIssue(const Data::Items::ItemDataIssue& issue)
{
    if (issue.field == NameKey && IsAboutTranslatedName(issue.message))
    {
        return ItemTool::Translations;
    }
    return GetToolForField(issue.field);
}

ItemToolWindowRef GetToolWindow(ItemTool tool)
{
    namespace Ids = MuEditor::WindowIds;
    using MuEditor::ToolWindows;
    switch (tool)
    {
    case ItemTool::Basics:
        return {Ids::ItemBasics, &ToolWindows::itemBasics};
    case ItemTool::Requirements:
        return {Ids::ItemRequirements, &ToolWindows::itemRequirements};
    case ItemTool::Rules:
        return {Ids::ItemRules, &ToolWindows::itemRules};
    case ItemTool::Categories:
        return {Ids::ItemCategories, &ToolWindows::itemCategories};
    case ItemTool::Model:
        return {Ids::ItemModel, &ToolWindows::itemModel};
    case ItemTool::Translations:
        return {Ids::ItemTranslations, &ToolWindows::itemTranslations};
    case ItemTool::Browser:
        break;
    }
    return {Ids::ItemBrowser, &ToolWindows::itemBrowser};
}
} // namespace MuEditor::Items

#endif // _EDITOR
