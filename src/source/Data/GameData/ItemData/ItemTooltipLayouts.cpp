#include "stdafx.h"

#include "ItemTooltipLayouts.h"
#include "ItemDataValidation.h"
#include "ItemDescriptionPlaceholders.h"
#include "ItemEnumNames.h"
#include "ItemJsonCommon.h"
#include "ItemTooltipConditionsJson.h"

#include <algorithm>
#include <array>
#include <set>

namespace Data::Items
{
namespace
{
using Json::OrderedJson;

namespace Keys
{
constexpr const char* FormatVersion = "formatVersion";
constexpr const char* Texts = "texts";
constexpr const char* Layouts = "layouts";
constexpr const char* Name = "name";
constexpr const char* Lines = "lines";
constexpr const char* Section = "section";
constexpr const char* Text = "text";
constexpr const char* When = "when";
constexpr const char* Color = "color";
constexpr const char* Colors = "colors";
constexpr const char* Bold = "bold";
constexpr const char* Values = "values";
} // namespace Keys

constexpr std::string_view TextReference = "@";

// Same order as TooltipSection, after None.
constexpr std::array<std::string_view, 4> SectionNames = {"name", "description", "classes", "options"};

using Json::KeySet;

const KeySet RootKeys = {Keys::FormatVersion, Keys::Texts, Keys::Layouts};
const KeySet LayoutKeys = {Keys::Name, Keys::Lines};
const KeySet LineKeys = {Keys::Text, Keys::When, Keys::Color, Keys::Colors, Keys::Bold, Keys::Values};
const KeySet SectionLineKeys = {Keys::Section};
const KeySet ColorRuleKeys = {Keys::When, Keys::Color};

std::string ListNames(const std::set<std::string>& names)
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

class ItemTooltipReader
{
public:
    ItemTooltipReader(const std::string& source, std::vector<ItemDataIssue>& issues)
        : m_source(source), m_issues(issues)
    {
    }

    void Read(const OrderedJson& root, ItemTooltipFile& file);

private:
    void AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
    {
        m_issues.push_back({severity, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
    }

    void AddError(const std::string& field, const std::string& message)
    {
        AddIssue(ItemDataIssueSeverity::Error, field, message);
    }

    void WarnAboutUnknownKeys(const OrderedJson& json, const KeySet& knownKeys, const std::string& prefix)
    {
        Json::WarnAboutUnknownKeys(json, knownKeys, prefix, [this](const std::string& field, const std::string& message)
                                   { AddIssue(ItemDataIssueSeverity::Warning, field, message); });
    }

    bool ReadText(const OrderedJson& json, const std::string& field, LocalizedString& text);
    void CheckPlaceholders(const LocalizedString& text, const std::string& field);
    void ReadTexts(const OrderedJson& root, ItemTooltipFile& file);
    void ReadLayouts(const OrderedJson& root, ItemTooltipFile& file);
    void ReadLayout(const OrderedJson& json, const std::string& field, const ItemTooltipFile& file,
                    TooltipLayout& layout);
    bool ReadSection(const OrderedJson& json, const std::string& field, TooltipSection& section);
    void ReadLine(const OrderedJson& json, const std::string& field, const ItemTooltipFile& file,
                  TooltipLayoutLine& line);
    void ReadLineValues(const OrderedJson& json, const std::string& field, TooltipLayoutLine& line);
    void CheckLinePlaceholders(const TooltipLayoutLine& line, const ItemTooltipFile& file, const std::string& field);
    void ReadConditions(const OrderedJson& json, const std::string& field, std::vector<TooltipCondition>& conditions);
    void ReadColor(const OrderedJson& json, const std::string& field, ItemTextColor& color);
    void ReadColorRules(const OrderedJson& json, const std::string& field, std::vector<TooltipColorRule>& rules);

    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

// A text is written like an item name and checked like one: an English text (unless it is a blank line), and
// translations under language codes.
bool ItemTooltipReader::ReadText(const OrderedJson& json, const std::string& field, LocalizedString& text)
{
    if (!Json::ReadLocalizedString(json, field, text, [this](const std::string& errorField, const std::string& message)
                                   { AddError(errorField, message); }))
    {
        return false;
    }
    if (!text.IsEmpty() && text.GetNeutral().empty())
    {
        AddError(field, "the English text is missing");
    }
    for (const auto& [locale, translation] : text.GetTranslations())
    {
        if (!LocalizedString::IsValidLocaleCode(locale))
        {
            AddError(field, "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
        }
    }
    return true;
}

// The braces of every language, and the translations have the English text's placeholders.
void ItemTooltipReader::CheckPlaceholders(const LocalizedString& text, const std::string& field)
{
    const PlaceholderScan english = ScanPlaceholders(text.GetNeutral());
    if (!english.error.empty())
    {
        AddError(field, english.error);
        return;
    }
    const std::set<std::string> englishNames(english.names.begin(), english.names.end());
    for (const auto& [locale, translation] : text.GetTranslations())
    {
        const PlaceholderScan scan = ScanPlaceholders(translation);
        if (!scan.error.empty())
        {
            AddError(field + "." + locale, scan.error);
            continue;
        }
        const std::set<std::string> names(scan.names.begin(), scan.names.end());
        if (names != englishNames)
        {
            AddError(field + "." + locale,
                     "has the placeholders " + ListNames(names) + ", the English text " + ListNames(englishNames));
        }
    }
}

void ItemTooltipReader::ReadTexts(const OrderedJson& root, ItemTooltipFile& file)
{
    const auto texts = root.find(Keys::Texts);
    if (texts == root.end())
    {
        return;
    }
    if (!texts->is_object())
    {
        AddError(Keys::Texts, "must be an object of texts by name");
        return;
    }
    for (const auto& [name, json] : texts->items())
    {
        const std::string field = std::string(Keys::Texts) + "." + name;
        if (!Json::IsName(name))
        {
            AddError(field, "a text's name has only letters and digits");
            continue;
        }
        LocalizedString text;
        if (ReadText(json, field, text))
        {
            CheckPlaceholders(text, field);
            file.texts.emplace(name, std::move(text));
        }
    }
}

void ItemTooltipReader::ReadConditions(const OrderedJson& json, const std::string& field,
                                       std::vector<TooltipCondition>& conditions)
{
    ReadTooltipConditions(json, field, R"(["defense", "!excellent", "tag:sceptre"])", conditions,
                          [this](const std::string& errorField, const std::string& message)
                          { AddError(errorField, message); });
}

void ItemTooltipReader::ReadColor(const OrderedJson& json, const std::string& field, ItemTextColor& color)
{
    if (!json.is_string() || !FindEnumValue(json.get<std::string>(), color))
    {
        AddError(field, "must be a color name (docs/item-data.md)");
    }
}

void ItemTooltipReader::ReadColorRules(const OrderedJson& json, const std::string& field,
                                       std::vector<TooltipColorRule>& rules)
{
    if (!json.is_array())
    {
        AddError(field, "must be a list of {\"when\": [...], \"color\": ...}");
        return;
    }
    for (size_t index = 0; index < json.size(); ++index)
    {
        const OrderedJson& entry = json[index];
        const std::string entryField = field + "[" + std::to_string(index) + "]";
        const auto when = entry.is_object() ? entry.find(Keys::When) : entry.end();
        const auto color = entry.is_object() ? entry.find(Keys::Color) : entry.end();
        if (when == entry.end() || color == entry.end())
        {
            AddError(entryField, "must be {\"when\": [...], \"color\": ...}");
            continue;
        }
        TooltipColorRule rule;
        ReadConditions(*when, entryField + "." + Keys::When, rule.when);
        if (rule.when.empty())
        {
            AddError(entryField + "." + Keys::When, "a color rule needs a condition; the line's \"color\" is the "
                                                    "color when no rule holds");
        }
        ReadColor(*color, entryField + "." + Keys::Color, rule.color);
        WarnAboutUnknownKeys(entry, ColorRuleKeys, entryField + ".");
        rules.push_back(std::move(rule));
    }
}

void ItemTooltipReader::ReadLineValues(const OrderedJson& json, const std::string& field, TooltipLayoutLine& line)
{
    if (!json.is_object())
    {
        AddError(field, "must be an object of placeholders and the built-in values they show");
        return;
    }
    for (const auto& [placeholder, value] : json.items())
    {
        const std::string entryField = field + "." + placeholder;
        if (!Json::IsName(placeholder))
        {
            AddError(entryField, "a placeholder has only letters and digits");
            continue;
        }
        if (!value.is_string() || !IsBuiltInPlaceholder(value.get<std::string>()))
        {
            AddError(entryField, "must name a built-in value (docs/item-data.md)");
            continue;
        }
        line.values.emplace_back(placeholder, value.get<std::string>());
    }
}

// Every placeholder of the line's text is a built-in value or one its "values" names, and each of those is used.
void ItemTooltipReader::CheckLinePlaceholders(const TooltipLayoutLine& line, const ItemTooltipFile& file,
                                              const std::string& field)
{
    const LocalizedString* text = line.textName.empty() ? &line.text : file.FindText(line.textName);
    if (text == nullptr)
    {
        return;
    }
    const PlaceholderScan scan = ScanPlaceholders(text->GetNeutral());
    std::vector<std::string> known(GetBuiltInPlaceholders().begin(), GetBuiltInPlaceholders().end());
    for (const auto& [placeholder, value] : line.values)
    {
        known.push_back(placeholder);
    }
    std::set<std::string> reported;
    for (const std::string& name : scan.names)
    {
        if (std::find(known.begin(), known.end(), name) != known.end() || !reported.insert(name).second)
        {
            continue;
        }
        const std::string suggestion = SuggestPlaceholder(name, known);
        AddError(field + "." + Keys::Text,
                 "has the unknown placeholder {" + name + "}; " +
                     (suggestion.empty() ? "a placeholder is a built-in value (docs/item-data.md) or one the line's "
                                           "\"values\" names"
                                         : "did you mean {" + suggestion + "}?"));
    }
    for (const auto& [placeholder, value] : line.values)
    {
        if (std::find(scan.names.begin(), scan.names.end(), placeholder) == scan.names.end())
        {
            AddIssue(ItemDataIssueSeverity::Warning, field + "." + Keys::Values + "." + placeholder,
                     "the line's text has no {" + placeholder + "}");
        }
    }
}

bool ItemTooltipReader::ReadSection(const OrderedJson& json, const std::string& field, TooltipSection& section)
{
    if (json.is_string())
    {
        const auto found = std::find(SectionNames.begin(), SectionNames.end(), json.get<std::string>());
        if (found != SectionNames.end())
        {
            section = static_cast<TooltipSection>(found - SectionNames.begin() + 1);
            return true;
        }
    }
    AddError(field, "must be \"name\", \"description\", \"classes\" or \"options\"");
    return false;
}

void ItemTooltipReader::ReadLine(const OrderedJson& json, const std::string& field, const ItemTooltipFile& file,
                                 TooltipLayoutLine& line)
{
    const auto text = json.find(Keys::Text);
    if (text == json.end())
    {
        AddError(field, "must have a \"text\" (\"\" for a blank line) or a \"section\"");
        return;
    }
    if (text->is_string() && text->get<std::string>().starts_with(TextReference))
    {
        line.textName = text->get<std::string>().substr(TextReference.size());
        if (file.FindText(line.textName) == nullptr)
        {
            std::vector<std::string> names;
            for (const auto& [name, shared] : file.texts)
            {
                names.push_back(name);
            }
            const std::string suggestion = SuggestPlaceholder(line.textName, names);
            AddError(field + "." + Keys::Text,
                     "\"@" + line.textName + "\" is not a text of \"texts\"" +
                         (suggestion.empty() ? "" : "; did you mean \"@" + suggestion + "\"?"));
        }
    }
    else if (ReadText(*text, field + "." + Keys::Text, line.text))
    {
        CheckPlaceholders(line.text, field + "." + Keys::Text);
    }

    if (const auto when = json.find(Keys::When); when != json.end())
    {
        ReadConditions(*when, field + "." + Keys::When, line.when);
    }
    if (const auto color = json.find(Keys::Color); color != json.end())
    {
        ReadColor(*color, field + "." + Keys::Color, line.color);
    }
    if (const auto colors = json.find(Keys::Colors); colors != json.end())
    {
        ReadColorRules(*colors, field + "." + Keys::Colors, line.colors);
    }
    if (const auto bold = json.find(Keys::Bold); bold != json.end())
    {
        if (bold->is_boolean())
        {
            line.bold = bold->get<bool>();
        }
        else
        {
            AddError(field + "." + Keys::Bold, "must be true or false");
        }
    }
    if (const auto values = json.find(Keys::Values); values != json.end())
    {
        ReadLineValues(*values, field + "." + Keys::Values, line);
    }
    CheckLinePlaceholders(line, file, field);
    WarnAboutUnknownKeys(json, LineKeys, field + ".");
}

void ItemTooltipReader::ReadLayout(const OrderedJson& json, const std::string& field, const ItemTooltipFile& file,
                                   TooltipLayout& layout)
{
    const auto name = json.find(Keys::Name);
    if (name == json.end() || !name->is_string() || !Json::IsName(name->get<std::string>()))
    {
        AddError(field + "." + Keys::Name, "must be a name of letters and digits");
    }
    else
    {
        layout.name = name->get<std::string>();
    }
    const auto lines = json.find(Keys::Lines);
    if (lines == json.end() || !lines->is_array())
    {
        AddError(field + "." + Keys::Lines, "must be a list of lines");
        return;
    }

    TooltipSection lastSection = TooltipSection::None;
    for (size_t index = 0; index < lines->size(); ++index)
    {
        const OrderedJson& entry = (*lines)[index];
        const std::string lineField = field + "." + Keys::Lines + "[" + std::to_string(index) + "]";
        if (!entry.is_object())
        {
            AddError(lineField, "must be an object");
            continue;
        }
        TooltipLayoutLine line;
        if (const auto section = entry.find(Keys::Section); section != entry.end())
        {
            if (!ReadSection(*section, lineField + "." + Keys::Section, line.section))
            {
                continue;
            }
            if (line.section <= lastSection)
            {
                AddError(lineField + "." + Keys::Section,
                         "the sections come once each, in the order name, description, classes, options");
            }
            if (lastSection == TooltipSection::None && index != 0)
            {
                AddError(lineField + "." + Keys::Section, "the lines start after the name section");
            }
            lastSection = std::max(lastSection, line.section);
            WarnAboutUnknownKeys(entry, SectionLineKeys, lineField + ".");
        }
        else
        {
            if (lastSection == TooltipSection::Options)
            {
                AddError(lineField, "the options section ends the tooltip, so no line comes after it");
            }
            ReadLine(entry, lineField, file, line);
        }
        layout.lines.push_back(std::move(line));
    }
    const size_t sections = std::count_if(layout.lines.begin(), layout.lines.end(), [](const TooltipLayoutLine& line)
                                          { return line.section != TooltipSection::None; });
    if (sections != SectionNames.size() || lastSection != TooltipSection::Options)
    {
        AddError(field + "." + Keys::Lines, "must have the sections name, description, classes and options");
    }
}

void ItemTooltipReader::ReadLayouts(const OrderedJson& root, ItemTooltipFile& file)
{
    const auto layouts = root.find(Keys::Layouts);
    if (layouts == root.end() || !layouts->is_array())
    {
        AddError(Keys::Layouts, "must be a list of layouts");
        return;
    }
    for (size_t index = 0; index < layouts->size(); ++index)
    {
        const OrderedJson& entry = (*layouts)[index];
        const std::string field = std::string(Keys::Layouts) + "[" + std::to_string(index) + "]";
        if (!entry.is_object())
        {
            AddError(field, "must be an object");
            continue;
        }
        TooltipLayout layout;
        ReadLayout(entry, field, file, layout);
        WarnAboutUnknownKeys(entry, LayoutKeys, field + ".");
        if (!layout.name.empty() && file.FindLayout(layout.name) != nullptr)
        {
            AddError(field + "." + Keys::Name, "the layout \"" + layout.name + "\" is defined more than once");
            continue;
        }
        if (!layout.name.empty() && layout.name != DefaultTooltipLayout)
        {
            AddIssue(ItemDataIssueSeverity::Warning, field + "." + Keys::Name,
                     "only the \"default\" layout is used for now");
        }
        file.layouts.push_back(std::move(layout));
    }
    if (file.FindLayout(DefaultTooltipLayout) == nullptr)
    {
        AddError(Keys::Layouts, "must have the layout \"default\", which every item's tooltip uses");
    }
}

void ItemTooltipReader::Read(const OrderedJson& root, ItemTooltipFile& file)
{
    WarnAboutUnknownKeys(root, RootKeys, "");
    ReadTexts(root, file);
    ReadLayouts(root, file);
}
} // namespace

std::string_view GetTooltipSectionName(TooltipSection section)
{
    return section == TooltipSection::None ? std::string_view() : SectionNames[static_cast<size_t>(section) - 1];
}

const LocalizedString* ItemTooltipFile::FindText(std::string_view name) const
{
    const auto found = texts.find(name);
    return found != texts.end() ? &found->second : nullptr;
}

const TooltipLayout* ItemTooltipFile::FindLayout(std::string_view name) const
{
    const auto found =
        std::find_if(layouts.begin(), layouts.end(), [&](const TooltipLayout& layout) { return layout.name == name; });
    return found != layouts.end() ? &*found : nullptr;
}

void ReadItemTooltipJson(std::string_view text, const std::string& source, ItemTooltipFile& file,
                         std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Json::ReadFileVersion(text, source, ItemTooltipJsonFormatVersion, root, issues))
    {
        return;
    }
    ItemTooltipReader(source, issues).Read(root, file);
}
void ValidateTooltipTextLocales(const ItemTooltipFile& file, std::span<const char* const> uiLocales,
                                const std::string& source, std::vector<ItemDataIssue>& issues)
{
    const auto warn = [&](const LocalizedString& text, const std::string& field)
    {
        for (const auto& [locale, translation] : text.GetTranslations())
        {
            const bool shown = std::any_of(uiLocales.begin(), uiLocales.end(),
                                           [&](const char* uiLocale) { return locale == uiLocale; });
            if (!shown)
            {
                issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                                  field,
                                  "the " + locale + " text is never shown: the game has no texts in that language"});
            }
        }
    };
    for (const auto& [name, text] : file.texts)
    {
        warn(text, std::string(Keys::Texts) + "." + name);
    }
    // Named as the reader names them: by the layout's place in the list.
    for (size_t layout = 0; layout < file.layouts.size(); ++layout)
    {
        const std::vector<TooltipLayoutLine>& lines = file.layouts[layout].lines;
        for (size_t index = 0; index < lines.size(); ++index)
        {
            warn(lines[index].text, std::string(Keys::Layouts) + "[" + std::to_string(layout) + "].lines[" +
                                        std::to_string(index) + "].text");
        }
    }
}

} // namespace Data::Items
