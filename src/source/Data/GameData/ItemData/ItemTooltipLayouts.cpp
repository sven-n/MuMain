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
constexpr const char* Id = "id";
constexpr const char* ForEachClass = "forEachClass";
constexpr const char* Base = "base";
constexpr const char* Hide = "hide";
constexpr const char* Add = "add";
constexpr const char* After = "after";
} // namespace Keys

constexpr std::string_view TextReference = "@";
// The item file's field of an item's own layout changes, in their problems' fields.
constexpr const char* ItemLayoutChangesField = "tooltipLayoutChanges";

// Same order as TooltipSection, after None.
constexpr std::array<std::string_view, 6> SectionNames = {"name",    "itemCases", "description",
                                                          "classes", "options",   "afterOptions"};
constexpr const char* SectionList = "name, itemCases, description, classes, options and afterOptions";

using Json::KeySet;

const KeySet RootKeys = {Keys::FormatVersion, Keys::Texts, Keys::Layouts};
const KeySet LayoutKeys = {Keys::Name, Keys::Lines, Keys::Base, Keys::Hide, Keys::Add};
const KeySet LineKeys = {Keys::Id,     Keys::Text, Keys::When,   Keys::Color,
                         Keys::Colors, Keys::Bold, Keys::Values, Keys::ForEachClass};
const KeySet SectionLineKeys = {Keys::Section};
const KeySet ColorRuleKeys = {Keys::When, Keys::Color};
const KeySet AdditionKeys = {Keys::After, Keys::Lines};

// A layout's lines without those whose id `hide` names.
void HideTooltipLines(std::vector<BuiltTooltipLine>& lines, std::span<const std::string> hide)
{
    if (hide.empty())
    {
        return;
    }
    std::erase_if(lines, [&](const BuiltTooltipLine& line)
                  { return !line.GetId().empty() && std::find(hide.begin(), hide.end(), line.GetId()) != hide.end(); });
}

// A layout's lines without those `hide` names, and each addition's lines right after the line of its `after` id (the
// additions for one line in their order): the changes of a preset or an item (D73). `makeEntry` makes the built line
// of an addition's line.
template <typename TAddition, typename TMakeEntry>
void ApplyLayoutChanges(std::vector<BuiltTooltipLine>& lines, std::span<const std::string> hide,
                        std::span<const TAddition> additions, TMakeEntry&& makeEntry)
{
    HideTooltipLines(lines, hide);
    if (additions.empty())
    {
        return;
    }
    std::vector<BuiltTooltipLine> changed;
    changed.reserve(lines.size());
    for (const BuiltTooltipLine& line : lines)
    {
        changed.push_back(line);
        if (line.GetId().empty())
        {
            continue;
        }
        for (const TAddition& addition : additions)
        {
            if (addition.after == line.GetId())
            {
                for (const auto& added : addition.lines)
                {
                    changed.push_back(makeEntry(added));
                }
            }
        }
    }
    lines = std::move(changed);
}

// Appends the built lines of the layout `name` (its bases' first); false for a layout the file doesn't have or bases
// that go round in a circle (more of them than the file has layouts).
bool AppendLayout(const ItemTooltipFile& file, std::string_view name, std::vector<BuiltTooltipLine>& lines,
                  size_t depth)
{
    const TooltipLayout* layout = file.FindLayout(name);
    if (layout == nullptr || depth > file.layouts.size())
    {
        return false;
    }
    if (!layout->IsPreset())
    {
        for (const TooltipLayoutLine& line : layout->lines)
        {
            lines.push_back({&line, nullptr});
        }
        return true;
    }
    if (!AppendLayout(file, layout->base, lines, depth + 1))
    {
        return false;
    }
    ApplyLayoutChanges(lines, std::span<const std::string>(layout->hide),
                       std::span<const TooltipLayoutAddition>(layout->add),
                       [](const TooltipLayoutLine& added) { return BuiltTooltipLine{&added, nullptr}; });
    return true;
}

// " (the preset "P" leaves it out)" when the layout `name` is a preset that hides the line `id`, itself or through one
// of its bases; empty else.
std::string DescribePresetHiding(const ItemTooltipFile& file, std::string_view name, std::string_view id)
{
    const TooltipLayout* layout = file.FindLayout(name);
    // No more bases than the file has layouts: bases that go round in a circle have their own error.
    for (size_t depth = 0; layout != nullptr && layout->IsPreset() && depth < file.layouts.size(); ++depth)
    {
        if (std::find(layout->hide.begin(), layout->hide.end(), id) != layout->hide.end())
        {
            return " (the preset \"" + layout->name + "\" leaves it out)";
        }
        layout = file.FindLayout(layout->base);
    }
    return {};
}

// The problems of changes to the layout `name` whose built lines are `lines` (a preset's changes to its base, an
// item's to the layout it names), each a field after the changes' own ("hide[0]", "add[1].after") and its message: an
// id `hide` names that the lines don't have, and an id an addition comes after that they don't have once those are
// hidden. `changer` names who makes the changes ("the preset", "the item"), for a line it hides itself.
template <typename TAddition>
std::vector<std::pair<std::string, std::string>>
CheckLayoutChanges(const ItemTooltipFile& file, std::string_view name, std::vector<BuiltTooltipLine> lines,
                   std::span<const std::string> hide, std::span<const TAddition> additions, std::string_view changer)
{
    std::vector<std::pair<std::string, std::string>> problems;
    const auto noSuchLine = [&](const std::string& id)
    {
        return "the layout \"" + std::string(name) + "\" has no line with the id \"" + id + "\"" +
               DescribePresetHiding(file, name, id);
    };
    for (size_t index = 0; index < hide.size(); ++index)
    {
        if (!HasTooltipLine(lines, hide[index]))
        {
            problems.emplace_back("hide[" + std::to_string(index) + "]", noSuchLine(hide[index]));
        }
    }
    // Whether the layout has each addition's line before the changes hide theirs.
    std::vector<char> inLayout;
    for (const TAddition& addition : additions)
    {
        inLayout.push_back(HasTooltipLine(lines, addition.after));
    }
    HideTooltipLines(lines, hide);
    for (size_t index = 0; index < additions.size(); ++index)
    {
        const std::string& after = additions[index].after;
        if (HasTooltipLine(lines, after))
        {
            continue;
        }
        problems.emplace_back("add[" + std::to_string(index) + "].after",
                              inLayout[index] ? std::string(changer) + " hides the line \"" + after +
                                                    "\" itself, so no line can come after it"
                                              : noSuchLine(after));
    }
    return problems;
}

// The first id the lines have twice, or empty.
std::string_view FindRepeatedId(std::span<const BuiltTooltipLine> lines)
{
    std::set<std::string_view> ids;
    for (const BuiltTooltipLine& line : lines)
    {
        if (!line.GetId().empty() && !ids.insert(line.GetId()).second)
        {
            return line.GetId();
        }
    }
    return {};
}

constexpr const char* ClassNameOutsideMessage =
    "has {className}, which only a layout line with \"forEachClass\" fills in: it is empty here";
constexpr const char* ClassUsableOutsideMessage =
    "names classUsable, which holds only on a layout line with \"forEachClass\": it never holds here";

bool NamesClassUsable(std::span<const TooltipCondition> conditions)
{
    return std::any_of(
        conditions.begin(), conditions.end(), [](const TooltipCondition& condition)
        { return condition.kind == TooltipCondition::Kind::State && condition.state == TooltipState::ClassUsable; });
}

// CheckClassOnlyValues for a layout line or an item's own line: its conditions and color rules, then the
// placeholders of its text, each showing the value its "values" names for it, else the built-in value of its name.
template <typename TLine> std::string CheckLineClassOnlyValues(const TLine& line, const ItemTooltipFile& file)
{
    const bool colorNamesIt = std::any_of(line.colors.begin(), line.colors.end(),
                                          [](const TooltipColorRule& rule) { return NamesClassUsable(rule.when); });
    if (NamesClassUsable(line.when) || colorNamesIt)
    {
        return ClassUsableOutsideMessage;
    }
    const LocalizedString* text = line.textName.empty() ? &line.text : file.FindText(line.textName);
    if (text == nullptr)
    {
        return {};
    }
    const PlaceholderScan scan = ScanPlaceholders(text->GetNeutral());
    for (const std::string& placeholder : scan.names)
    {
        const auto named = std::find_if(line.values.begin(), line.values.end(),
                                        [&](const auto& value) { return value.first == placeholder; });
        const std::string_view shown =
            named != line.values.end() ? std::string_view(named->second) : std::string_view(placeholder);
        if (FindBuiltInPlaceholder(shown) == TooltipValue::ClassName)
        {
            return ClassNameOutsideMessage;
        }
    }
    return {};
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

    void AddFatal(const std::string& field, const std::string& message)
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field, message);
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
    void ReadLines(const OrderedJson& json, const std::string& field, const ItemTooltipFile& file,
                   TooltipLayout& layout);
    void ReadPreset(const OrderedJson& json, const std::string& field, const ItemTooltipFile& file,
                    TooltipLayout& layout);
    void CheckPresets(const ItemTooltipFile& file);
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
                                   { AddFatal(errorField, message); }))
    {
        return false;
    }
    if (!text.IsEmpty() && text.GetNeutral().empty())
    {
        AddFatal(field, "the English text is missing");
    }
    for (const auto& [locale, translation] : text.GetTranslations())
    {
        if (!LocalizedString::IsValidLocaleCode(locale))
        {
            AddFatal(field, "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
        }
    }
    return true;
}

// The braces of every language, and the translations have the English text's placeholders.
void ItemTooltipReader::CheckPlaceholders(const LocalizedString& text, const std::string& field)
{
    for (const auto& [problemField, message] : CheckTextPlaceholders(text, field))
    {
        AddFatal(problemField, message);
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
        AddFatal(Keys::Texts, "must be an object of texts by name");
        return;
    }
    for (const auto& [name, json] : texts->items())
    {
        const std::string field = std::string(Keys::Texts) + "." + name;
        if (!Json::IsName(name))
        {
            AddFatal(field, "a text's name has only letters and digits");
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
                          { AddFatal(errorField, message); });
}

void ItemTooltipReader::ReadColor(const OrderedJson& json, const std::string& field, ItemTextColor& color)
{
    if (!json.is_string() || !FindEnumValue(json.get<std::string>(), color))
    {
        AddFatal(field, "must be a color name (docs/item-data.md)");
    }
}

void ItemTooltipReader::ReadColorRules(const OrderedJson& json, const std::string& field,
                                       std::vector<TooltipColorRule>& rules)
{
    if (!json.is_array())
    {
        AddFatal(field, "must be a list of {\"when\": [...], \"color\": ...}");
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
            AddFatal(entryField, "must be {\"when\": [...], \"color\": ...}");
            continue;
        }
        TooltipColorRule rule;
        ReadConditions(*when, entryField + "." + Keys::When, rule.when);
        if (rule.when.empty())
        {
            AddFatal(entryField + "." + Keys::When, "a color rule needs a condition; the line's \"color\" is the "
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
        AddFatal(field, "must be an object of placeholders and the built-in values they show");
        return;
    }
    for (const auto& [placeholder, value] : json.items())
    {
        const std::string entryField = field + "." + placeholder;
        if (!Json::IsName(placeholder))
        {
            AddFatal(entryField, "a placeholder has only letters and digits");
            continue;
        }
        if (!value.is_string() || !IsBuiltInPlaceholder(value.get<std::string>()))
        {
            AddFatal(entryField, "must name a built-in value (docs/item-data.md)");
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
        AddFatal(field + "." + Keys::Text,
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
    AddFatal(field, std::string("must be one of the sections ") + SectionList);
    return false;
}

void ItemTooltipReader::ReadLine(const OrderedJson& json, const std::string& field, const ItemTooltipFile& file,
                                 TooltipLayoutLine& line)
{
    if (const auto id = json.find(Keys::Id); id != json.end())
    {
        if (id->is_string() && Json::IsName(id->get<std::string>()))
        {
            line.id = id->get<std::string>();
        }
        else
        {
            AddFatal(field + "." + Keys::Id, "must be a name of letters and digits");
        }
    }
    if (const auto forEachClass = json.find(Keys::ForEachClass); forEachClass != json.end())
    {
        if (forEachClass->is_boolean())
        {
            line.forEachClass = forEachClass->get<bool>();
        }
        else
        {
            AddFatal(field + "." + Keys::ForEachClass, "must be true or false");
        }
    }
    const auto text = json.find(Keys::Text);
    if (text == json.end())
    {
        AddFatal(field, "must have a \"text\" (\"\" for a blank line) or a \"section\"");
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
            AddFatal(field + "." + Keys::Text,
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
            AddFatal(field + "." + Keys::Bold, "must be true or false");
        }
    }
    if (const auto values = json.find(Keys::Values); values != json.end())
    {
        ReadLineValues(*values, field + "." + Keys::Values, line);
    }
    CheckLinePlaceholders(line, file, field);
    if (const std::string warning = CheckClassOnlyValues(line, file); !warning.empty())
    {
        AddIssue(ItemDataIssueSeverity::Warning, field, warning);
    }
    WarnAboutUnknownKeys(json, LineKeys, field + ".");
}

void ItemTooltipReader::ReadLayout(const OrderedJson& json, const std::string& field, const ItemTooltipFile& file,
                                   TooltipLayout& layout)
{
    const auto name = json.find(Keys::Name);
    if (name == json.end() || !name->is_string() || !Json::IsName(name->get<std::string>()))
    {
        AddFatal(field + "." + Keys::Name, "must be a name of letters and digits");
    }
    else
    {
        layout.name = name->get<std::string>();
    }
    if (json.contains(Keys::Base))
    {
        ReadPreset(json, field, file, layout);
    }
    else
    {
        ReadLines(json, field, file, layout);
    }
}

void ItemTooltipReader::ReadLines(const OrderedJson& json, const std::string& field, const ItemTooltipFile& file,
                                  TooltipLayout& layout)
{
    for (const char* key : {Keys::Hide, Keys::Add})
    {
        if (json.contains(key))
        {
            AddFatal(field + "." + key, "only a preset (a layout with a \"base\") has changes");
        }
    }
    const auto lines = json.find(Keys::Lines);
    if (lines == json.end() || !lines->is_array())
    {
        AddFatal(field + "." + Keys::Lines, "must be a list of lines");
        return;
    }

    TooltipSection lastSection = TooltipSection::None;
    for (size_t index = 0; index < lines->size(); ++index)
    {
        const OrderedJson& entry = (*lines)[index];
        const std::string lineField = field + "." + Keys::Lines + "[" + std::to_string(index) + "]";
        if (!entry.is_object())
        {
            AddFatal(lineField, "must be an object");
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
                AddFatal(lineField + "." + Keys::Section,
                         std::string("the sections come once each, in the order ") + SectionList);
            }
            lastSection = std::max(lastSection, line.section);
            WarnAboutUnknownKeys(entry, SectionLineKeys, lineField + ".");
        }
        else
        {
            ReadLine(entry, lineField, file, line);
        }
        layout.lines.push_back(std::move(line));
    }
    const size_t sections = std::count_if(layout.lines.begin(), layout.lines.end(), [](const TooltipLayoutLine& line)
                                          { return line.section != TooltipSection::None; });
    if (sections != SectionNames.size() || lastSection != TooltipSection::AfterOptions)
    {
        AddFatal(field + "." + Keys::Lines, std::string("must have the sections ") + SectionList);
    }
    std::vector<BuiltTooltipLine> built;
    for (const TooltipLayoutLine& line : layout.lines)
    {
        built.push_back({&line, nullptr});
    }
    if (const std::string_view repeated = FindRepeatedId(built); !repeated.empty())
    {
        AddFatal(field + "." + Keys::Lines, "has more than one line with the id \"" + std::string(repeated) + "\"");
    }
}

void ItemTooltipReader::ReadPreset(const OrderedJson& json, const std::string& field, const ItemTooltipFile& file,
                                   TooltipLayout& layout)
{
    const auto base = json.find(Keys::Base);
    if (!base->is_string() || !Json::IsName(base->get<std::string>()))
    {
        AddFatal(field + "." + Keys::Base, "must name the layout the preset changes");
    }
    else
    {
        layout.base = base->get<std::string>();
    }
    if (json.contains(Keys::Lines))
    {
        AddFatal(field + "." + Keys::Lines, "a preset has no lines of its own: it changes its base with "
                                            "\"hide\" and \"add\"");
    }
    if (const auto hide = json.find(Keys::Hide); hide != json.end() && !Json::ReadLineIds(*hide, layout.hide))
    {
        AddFatal(field + "." + Keys::Hide, Json::LineIdsMessage);
    }
    const auto add = json.find(Keys::Add);
    if (add == json.end())
    {
        return;
    }
    if (!add->is_array())
    {
        AddFatal(field + "." + Keys::Add, Json::LineAdditionsMessage);
        return;
    }
    for (size_t index = 0; index < add->size(); ++index)
    {
        const OrderedJson& entry = (*add)[index];
        const std::string entryField = field + "." + Keys::Add + "[" + std::to_string(index) + "]";
        TooltipLayoutAddition addition;
        const OrderedJson* lines = Json::ReadLineAddition(entry, addition.after);
        if (lines == nullptr)
        {
            AddFatal(entryField, Json::LineAdditionMessage);
            continue;
        }
        for (size_t lineIndex = 0; lineIndex < lines->size(); ++lineIndex)
        {
            const OrderedJson& lineJson = (*lines)[lineIndex];
            const std::string lineField = entryField + "." + Keys::Lines + "[" + std::to_string(lineIndex) + "]";
            if (!lineJson.is_object() || lineJson.contains(Keys::Section))
            {
                AddFatal(lineField, "must be a line: a preset can't add a section");
                continue;
            }
            TooltipLayoutLine line;
            ReadLine(lineJson, lineField, file, line);
            addition.lines.push_back(std::move(line));
        }
        WarnAboutUnknownKeys(entry, AdditionKeys, entryField + ".");
        layout.add.push_back(std::move(addition));
    }
}

// Each preset's base is a layout of the file, its bases don't go round in a circle, and its changes name lines its
// base has.
void ItemTooltipReader::CheckPresets(const ItemTooltipFile& file)
{
    for (size_t index = 0; index < file.layouts.size(); ++index)
    {
        const TooltipLayout& layout = file.layouts[index];
        if (!layout.IsPreset())
        {
            continue;
        }
        const std::string field = std::string(Keys::Layouts) + "[" + std::to_string(index) + "]";
        if (file.FindLayout(layout.base) == nullptr)
        {
            AddFatal(field + "." + Keys::Base, "\"" + layout.base + "\" is not a layout of this file");
            continue;
        }
        std::vector<BuiltTooltipLine> base;
        BuildTooltipLayout(file, layout.base, base);
        if (base.empty())
        {
            AddFatal(field + "." + Keys::Base, "its bases go round in a circle, or one of them is not a layout of "
                                               "this file");
            continue;
        }
        for (const auto& [changeField, message] :
             CheckLayoutChanges(file, layout.base, std::move(base), std::span<const std::string>(layout.hide),
                                std::span<const TooltipLayoutAddition>(layout.add), "the preset"))
        {
            AddFatal(field + "." + changeField, message);
        }
        std::vector<BuiltTooltipLine> built;
        BuildTooltipLayout(file, layout.name, built);
        if (const std::string_view repeated = FindRepeatedId(built); !repeated.empty())
        {
            AddFatal(field + "." + Keys::Add,
                     "adds a line with the id \"" + std::string(repeated) + "\", which its layout has already");
        }
    }
}

void ItemTooltipReader::ReadLayouts(const OrderedJson& root, ItemTooltipFile& file)
{
    const auto layouts = root.find(Keys::Layouts);
    if (layouts == root.end() || !layouts->is_array())
    {
        AddFatal(Keys::Layouts, "must be a list of layouts");
        return;
    }
    for (size_t index = 0; index < layouts->size(); ++index)
    {
        const OrderedJson& entry = (*layouts)[index];
        const std::string field = std::string(Keys::Layouts) + "[" + std::to_string(index) + "]";
        if (!entry.is_object())
        {
            AddFatal(field, "must be an object");
            continue;
        }
        TooltipLayout layout;
        ReadLayout(entry, field, file, layout);
        WarnAboutUnknownKeys(entry, LayoutKeys, field + ".");
        if (!layout.name.empty() && file.FindLayout(layout.name) != nullptr)
        {
            AddFatal(field + "." + Keys::Name, "the layout \"" + layout.name + "\" is defined more than once");
            continue;
        }
        if (layout.name == DefaultTooltipLayout && layout.IsPreset())
        {
            AddFatal(field + "." + Keys::Base, "the default layout has lines of its own, it is no preset");
        }
        file.layouts.push_back(std::move(layout));
    }
    if (file.FindLayout(DefaultTooltipLayout) == nullptr)
    {
        AddFatal(Keys::Layouts, "must have the layout \"default\", which every item's tooltip uses");
    }
    CheckPresets(file);
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

void BuildTooltipLayout(const ItemTooltipFile& file, std::string_view name, std::vector<BuiltTooltipLine>& lines)
{
    lines.clear();
    if (!AppendLayout(file, name, lines, 0))
    {
        lines.clear();
    }
}

void BuildItemTooltipLayout(const ItemTooltipFile& file, const ItemDefinition& definition,
                            std::vector<BuiltTooltipLine>& lines)
{
    BuildTooltipLayout(file, GetItemTooltipLayoutName(definition), lines);
    const ItemTooltipLayoutChanges& changes = definition.tooltipLayoutChanges;
    ApplyLayoutChanges(lines, std::span<const std::string>(changes.hide),
                       std::span<const ItemTooltipLayoutAddition>(changes.add),
                       [](const ItemDescriptionLine& added) { return BuiltTooltipLine{nullptr, &added}; });
}

std::string CheckClassOnlyValues(const TooltipLayoutLine& line, const ItemTooltipFile& file)
{
    return line.forEachClass ? std::string() : CheckLineClassOnlyValues(line, file);
}

std::string CheckClassOnlyValues(const ItemDescriptionLine& line, const ItemTooltipFile& file)
{
    return CheckLineClassOnlyValues(line, file);
}

std::string CheckClassOnlyConditions(std::span<const TooltipCondition> conditions)
{
    return NamesClassUsable(conditions) ? ClassUsableOutsideMessage : std::string();
}

bool HasTooltipLine(std::span<const BuiltTooltipLine> lines, std::string_view id)
{
    return std::any_of(lines.begin(), lines.end(), [&](const BuiltTooltipLine& line) { return line.GetId() == id; });
}

std::string_view GetItemTooltipLayoutName(const ItemDefinition& definition)
{
    return definition.tooltipLayout.empty() ? std::string_view(DefaultTooltipLayout)
                                            : std::string_view(definition.tooltipLayout);
}

std::vector<std::pair<std::string, std::string>> CheckItemTooltipLayout(const ItemTooltipFile& file,
                                                                        const ItemDefinition& definition)
{
    std::vector<std::pair<std::string, std::string>> problems;
    const std::string name(GetItemTooltipLayoutName(definition));
    // A file without the default layout has that error of its own.
    if (definition.tooltipLayout.empty() && file.FindLayout(name) == nullptr)
    {
        return problems;
    }
    if (file.FindLayout(name) == nullptr)
    {
        std::vector<std::string> names;
        for (const TooltipLayout& layout : file.layouts)
        {
            names.push_back(layout.name);
        }
        const std::string suggestion = SuggestPlaceholder(name, names);
        problems.emplace_back("tooltipLayout",
                              "\"" + name + "\" is not a layout of the tooltip file" +
                                  (suggestion.empty() ? "" : "; did you mean \"" + suggestion + "\"?"));
        return problems;
    }
    return CheckTooltipLayoutChanges(file, name, definition.tooltipLayoutChanges);
}

std::vector<std::pair<std::string, std::string>>
CheckTooltipLayoutChanges(const ItemTooltipFile& file, std::string_view name, const ItemTooltipLayoutChanges& changes)
{
    // Most items change nothing: no layout to build for them.
    if (changes.IsEmpty())
    {
        return {};
    }
    std::vector<BuiltTooltipLine> lines;
    BuildTooltipLayout(file, name, lines);
    if (lines.empty())
    {
        return {};
    }
    std::vector<std::pair<std::string, std::string>> problems =
        CheckLayoutChanges(file, name, std::move(lines), std::span<const std::string>(changes.hide),
                           std::span<const ItemTooltipLayoutAddition>(changes.add), "the item");
    for (auto& [field, message] : problems)
    {
        field = std::string(ItemLayoutChangesField) + "." + field;
    }
    return problems;
}

void ReadItemTooltipJson(std::string_view text, const std::string& source, ItemTooltipFile& file,
                         std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Json::ReadFileVersion(text, source, ItemTooltipJsonFormatVersion, root, issues))
    {
        return;
    }
    // Read as this version, an older file only gives errors about what it lacks.
    if (root[Keys::FormatVersion].get<int>() < ItemTooltipJsonOldestFormatVersion)
    {
        Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, Keys::FormatVersion,
                           "version " + root[Keys::FormatVersion].dump() +
                               " is from before the tooltip's code lines became layout lines: its layout lacks them "
                               "(the first row, the class lines, the boots' and gloves' texts, a timed item's end) "
                               "and their sections, and names the tags they replaced; copy your changes into the "
                               "Tooltip.json of this client (docs/item-data.md, Tooltip layouts)");
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
        const std::string field = std::string(Keys::Layouts) + "[" + std::to_string(layout) + "]";
        const std::vector<TooltipLayoutLine>& lines = file.layouts[layout].lines;
        for (size_t index = 0; index < lines.size(); ++index)
        {
            warn(lines[index].text, field + ".lines[" + std::to_string(index) + "].text");
        }
        const std::vector<TooltipLayoutAddition>& additions = file.layouts[layout].add;
        for (size_t addition = 0; addition < additions.size(); ++addition)
        {
            for (size_t index = 0; index < additions[addition].lines.size(); ++index)
            {
                warn(additions[addition].lines[index].text,
                     field + ".add[" + std::to_string(addition) + "].lines[" + std::to_string(index) + "].text");
            }
        }
    }
}

} // namespace Data::Items
