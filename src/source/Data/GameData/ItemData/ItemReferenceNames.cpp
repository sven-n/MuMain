#include "stdafx.h"

#include "Data/GameData/ItemData/ItemReferenceNames.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/ItemTooltipLayouts.h"
#include "Data/GameData/ItemData/ItemType.h"

namespace Data::Items
{
namespace
{
using Translation::ItemReference;

constexpr std::string_view NamePlaceholder = "name";
constexpr std::string_view LevelPlusOnePlaceholder = "levelPlusOne";
// How many name lines deep a name may come from name lines that name other items; deeper (a line that names its own
// level) cannot be filled.
constexpr int MaxNameDepth = 4;
// What a text would read as its own in a name put into it: a printf conversion of the UI texts, a placeholder of the
// item data's texts.
constexpr std::string_view UnshowableNameCharacters = "%{}";

ReferencedName Resolve(const ItemDatabase& database, const ItemReference& reference, std::string_view locale,
                       int depth);

// The name line that names the item at `level`, as the tooltip takes it: the first one shown at that level, skipping
// lines with conditions (those need the item and the hero). None when there is none, or when that line has no text:
// the tooltip then shows the standard name.
const ItemNameLine* FindNameLineAt(const ItemDefinition& definition, int level)
{
    for (const ItemNameLine& line : definition.nameLines)
    {
        if (line.when.empty() && line.IsShownAt(level))
        {
            return line.hasText ? &line : nullptr;
        }
    }
    return nullptr;
}

// A text with translations, or the shared text of the tooltip file it names.
const LocalizedString* LineText(const ItemDatabase& database, const LocalizedString& text, const std::string& textName)
{
    return textName.empty() ? &text : database.GetTooltipFile().FindText(textName);
}

// The name in the locale, or its form for the case. A name the locale does not translate is the English name, which
// has no forms, and a case the locale does not decline in (NameCases.json) shows no form.
ReferencedName NameIn(const ItemDatabase& database, const LocalizedString& names, const ItemNameForms& forms,
                      std::string_view locale, std::string_view caseKey)
{
    ReferencedName name{names.Get(locale), ItemReferenceProblem::None};
    if (caseKey.empty() || !names.HasOwn(locale) || locale == LocalizedString::NeutralLocale)
    {
        return name;
    }
    const std::string& form = forms.Find(locale, caseKey);
    if (form.empty() || !database.GetNameCases().Uses(locale, caseKey))
    {
        name.problem = ItemReferenceProblem::MissingForm;
        return name;
    }
    name.text = form;
    return name;
}

// The item's value of a level value placeholder at `level` ("{stat}" of the Fruits), or nothing.
std::optional<std::string> LevelValueOf(const ItemDatabase& database, const ItemDefinition& definition,
                                        std::string_view placeholder, int level, std::string_view locale)
{
    for (const ItemLevelValues& values : definition.levelValues)
    {
        if (values.name != placeholder || level >= static_cast<int>(values.values.size()))
        {
            continue;
        }
        const ItemLevelValue& value = values.values[static_cast<size_t>(level)];
        const LocalizedString* text = LineText(database, value.text, value.textName);
        return text != nullptr ? std::optional<std::string>(text->Get(locale)) : std::nullopt;
    }
    return std::nullopt;
}

// A name line's text with {name} (in the case's form), {level}, {levelPlusOne}, the level values and other items' names
// ({item:14,13}, in `textLocale`, the language the line is written in) filled in.
ReferencedName FillNameLine(const ItemDatabase& database, const ItemDefinition& definition, std::string_view text,
                            const ItemReference& reference, std::string_view locale, std::string_view textLocale,
                            int depth)
{
    const int level = reference.level.value_or(0);
    const ReferencedName base = NameIn(database, definition.names, definition.nameForms, locale, reference.form);
    bool filled = true;
    bool namesItem = false;
    const auto valueOf = [&](std::string_view placeholder) -> std::optional<std::string>
    {
        std::optional<std::string> value;
        if (placeholder == NamePlaceholder)
        {
            value = base.text;
            namesItem = true;
        }
        else if (placeholder == LevelPlaceholder)
        {
            value = std::to_string(level);
        }
        else if (placeholder == LevelPlusOnePlaceholder)
        {
            value = std::to_string(level + 1);
        }
        else if (const std::optional<ItemReference> other = Translation::ParseItemReferenceName(placeholder))
        {
            // No name (no such item or level, a line that cannot be filled): the line cannot be filled either.
            const ReferencedName name = Resolve(database, *other, textLocale, depth + 1);
            if (!name.text.empty())
            {
                value = name.text;
            }
        }
        else
        {
            value = LevelValueOf(database, definition, placeholder, level, locale);
        }
        filled = filled && value.has_value();
        return value;
    };
    std::string name = FillPlaceholders(text, valueOf);
    if (!filled)
    {
        return {{}, ItemReferenceProblem::UnfillableLine};
    }
    // The case is the item's form in {name}; a line without it has nothing to decline.
    if (!namesItem && !reference.form.empty())
    {
        return {std::move(name), ItemReferenceProblem::UndeclinedLine};
    }
    return {std::move(name), base.problem};
}

ReferencedName NameAtLevel(const ItemDatabase& database, const ItemDefinition& definition,
                           const ItemReference& reference, std::string_view locale, int depth)
{
    const int level = *reference.level;
    const ItemNameLine* line = FindNameLineAt(definition, level);
    if (line == nullptr)
    {
        // The standard name: at +0 the item's name itself, above it with its level.
        ReferencedName name = NameIn(database, definition.names, definition.nameForms, locale, reference.form);
        if (level > 0)
        {
            name.text += " +" + std::to_string(level);
            name.problem = ItemReferenceProblem::StandardName;
        }
        return name;
    }
    const LocalizedString* text = LineText(database, line->text, line->textName);
    if (text == nullptr)
    {
        return {{}, ItemReferenceProblem::UnfillableLine};
    }
    if (text->Get(locale).find('{') != std::string::npos)
    {
        const std::string_view textLocale = text->HasOwn(locale) ? locale : LocalizedString::NeutralLocale;
        return FillNameLine(database, definition, text->Get(locale), reference, locale, textLocale, depth);
    }
    return NameIn(database, *text, line->nameForms, locale, reference.form);
}

ReferencedName Resolve(const ItemDatabase& database, const ItemReference& reference, std::string_view locale, int depth)
{
    if (depth > MaxNameDepth)
    {
        return {{}, ItemReferenceProblem::UnfillableLine};
    }
    const ItemDefinition* definition =
        IsValidItemId(reference.group, reference.number) ? database.Find(reference.group, reference.number) : nullptr;
    if (definition == nullptr)
    {
        return {{}, ItemReferenceProblem::UnknownItem};
    }
    if (reference.level && *reference.level >= ItemLevelCount)
    {
        return {{}, ItemReferenceProblem::NoSuchLevel};
    }
    if (reference.level)
    {
        return NameAtLevel(database, *definition, reference, locale, depth);
    }
    return NameIn(database, definition->names, definition->nameForms, locale, reference.form);
}
} // namespace

ReferencedName ResolveReferencedName(const ItemDatabase& database, const ItemReference& reference,
                                     std::string_view locale)
{
    ReferencedName name = Resolve(database, reference, locale, 0);
    if (name.text.find_first_of(UnshowableNameCharacters) != std::string::npos)
    {
        return {{}, ItemReferenceProblem::UnshowableName};
    }
    return name;
}
} // namespace Data::Items
