#include "stdafx.h"

#include "WarpsJson.h"

#include "Core/Text/AsciiText.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/MapData/WarpMessages.h"

#include <algorithm>

#include <functional>
#include <limits>
#include <utility>

namespace Data::Maps
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

namespace Keys
{
constexpr const char* ClassLevelDiscount = "classLevelDiscount";
constexpr const char* Class = "class";
constexpr const char* Numerator = "numerator";
constexpr const char* Denominator = "denominator";
constexpr const char* Warps = "warps";
constexpr const char* Index = "index";
constexpr const char* Name = "name";
constexpr const char* Command = "command";
constexpr const char* Level = "level";
constexpr const char* Zen = "zen";
constexpr const char* Gate = "gate";
constexpr const char* NoClassDiscount = "noClassDiscount";
constexpr const char* MinMasterLevel = "minMasterLevel";
constexpr const char* MinResets = "minResets";
constexpr const char* PvpServersOnly = "pvpServersOnly";
constexpr const char* GensOnly = "gensOnly";
constexpr const char* NeedsOneOf = "needsOneOf";
constexpr const char* NotWith = "notWith";
constexpr const char* BlockedMessage = "blockedMessage";
constexpr const char* Item = "item";
constexpr const char* Group = "group";
constexpr const char* Number = "number";
constexpr const char* Tag = "tag";
constexpr const char* Except = "except";
constexpr const char* Wings = "wings";
} // namespace Keys

constexpr int MaxNumber = std::numeric_limits<int>::max();

const Items::Json::KeySet WarpKeys{Keys::Index,     Keys::Name,           Keys::Command,         Keys::Level,
                                   Keys::Zen,       Keys::Gate,           Keys::NoClassDiscount, Keys::MinMasterLevel,
                                   Keys::MinResets, Keys::PvpServersOnly, Keys::GensOnly,        Keys::NeedsOneOf,
                                   Keys::NotWith,   Keys::BlockedMessage};
const Items::Json::KeySet DiscountKeys{Keys::Class, Keys::Numerator, Keys::Denominator};

constexpr const char* ItemMatchMessage =
    R"(must be {"item": {"group": 13, "number": 2}}, {"tag": "flying"} (with "except": {"tag": "..."} if needed))"
    R"( or {"wings": true})";

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& source,
              const std::string& field, const std::string& message)
{
    issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
}

// The fields of one warp; false, with a fatal problem added, at the first
// value of the wrong form.
class WarpReader
{
public:
    WarpReader(size_t index, const std::string& source, std::vector<ItemDataIssue>& issues)
        : m_index(index), m_source(source), m_issues(issues)
    {
    }

    bool Read(const OrderedJson& json, Warp& warp)
    {
        if (!json.is_object())
        {
            return Fail("", "must be an object with index, name and command");
        }
        return ReadNumber(json, Keys::Index, 0, MaxWarpIndex, warp.index) && ReadNames(json, warp.names) &&
               ReadCommand(json, warp.command) && ReadNumber(json, Keys::Level, 0, MaxWarpLevel, warp.level) &&
               ReadNumber(json, Keys::Zen, 0, MaxNumber, warp.zen) &&
               ReadNumber(json, Keys::Gate, 0, MaxWarpGate, warp.gate) &&
               ReadFlag(json, Keys::NoClassDiscount, warp.noClassDiscount) &&
               ReadOptionalNumber(json, Keys::MinMasterLevel, MaxWarpLevel, warp.minMasterLevel) &&
               ReadOptionalNumber(json, Keys::MinResets, MaxNumber, warp.minResets) &&
               ReadFlag(json, Keys::PvpServersOnly, warp.pvpServersOnly) &&
               ReadFlag(json, Keys::GensOnly, warp.gensOnly) &&
               ReadItemMatches(json, Keys::NeedsOneOf, warp.needsOneOf) &&
               ReadItemMatches(json, Keys::NotWith, warp.notWith) && ReadBlockedMessage(json, warp.blockedMessage) &&
               WarnUnknown(json);
    }

private:
    bool Fail(const std::string& key, const std::string& message)
    {
        AddIssue(m_issues, ItemDataIssueSeverity::Fatal, m_source, WarpField(m_index, key), message);
        return false;
    }

    std::function<void(const std::string&, const std::string&)> FatalAt()
    {
        return [this](const std::string& field, const std::string& message)
        { AddIssue(m_issues, ItemDataIssueSeverity::Fatal, m_source, field, message); };
    }

    bool ReadNumber(const OrderedJson& json, const char* key, int low, int high, int& value)
    {
        const auto field = json.find(key);
        if (field == json.end())
        {
            return Fail(key, "missing");
        }
        return Items::Json::ReadWholeNumberInRange(*field, WarpField(m_index, key), low, high, value, FatalAt());
    }

    // A missing field is 0.
    bool ReadOptionalNumber(const OrderedJson& json, const char* key, int high, int& value)
    {
        const auto field = json.find(key);
        return field == json.end() ||
               Items::Json::ReadWholeNumberInRange(*field, WarpField(m_index, key), 0, high, value, FatalAt());
    }

    // A missing field is false.
    bool ReadFlag(const OrderedJson& json, const char* key, bool& value)
    {
        const auto field = json.find(key);
        if (field == json.end())
        {
            return true;
        }
        if (!field->is_boolean())
        {
            return Fail(key, "must be true or false");
        }
        value = field->get<bool>();
        return true;
    }

    bool ReadItemMatches(const OrderedJson& json, const char* key, std::vector<WarpItemMatch>& matches)
    {
        const auto field = json.find(key);
        if (field == json.end())
        {
            return true;
        }
        if (!field->is_array())
        {
            return Fail(key, std::string("must be a list of items, each ") + ItemMatchMessage);
        }
        for (size_t entry = 0; entry < field->size(); ++entry)
        {
            WarpItemMatch match;
            if (!ReadItemMatch((*field)[entry], std::string(key) + "[" + std::to_string(entry) + "]", match))
            {
                return false;
            }
            matches.push_back(std::move(match));
        }
        return true;
    }

    bool ReadItemMatch(const OrderedJson& json, const std::string& key, WarpItemMatch& match)
    {
        if (!json.is_object())
        {
            return Fail(key, ItemMatchMessage);
        }
        const int kinds =
            int(json.contains(Keys::Item)) + int(json.contains(Keys::Tag)) + int(json.contains(Keys::Wings));
        if (kinds != 1 || (json.contains(Keys::Except) && !json.contains(Keys::Tag)))
        {
            return Fail(key, ItemMatchMessage);
        }
        WarnUnknownIn(json, {Keys::Item, Keys::Tag, Keys::Except, Keys::Wings}, key);
        if (json.contains(Keys::Wings))
        {
            match.kind = WarpItemMatch::Kind::Wings;
            return json[Keys::Wings] == true || Fail(key + "." + Keys::Wings, "must be true");
        }
        if (json.contains(Keys::Item))
        {
            match.kind = WarpItemMatch::Kind::Item;
            return ReadItem(json[Keys::Item], key + "." + Keys::Item, match);
        }
        match.kind = WarpItemMatch::Kind::Tag;
        if (!ReadTag(json[Keys::Tag], key + "." + Keys::Tag, match.tag))
        {
            return false;
        }
        const auto except = json.find(Keys::Except);
        if (except == json.end())
        {
            return true;
        }
        const std::string exceptKey = key + "." + Keys::Except;
        if (!except->is_object() || except->size() != 1 || !except->contains(Keys::Tag))
        {
            return Fail(exceptKey, R"(must be {"tag": "..."})");
        }
        return ReadTag((*except)[Keys::Tag], exceptKey + "." + Keys::Tag, match.exceptTag);
    }

    bool ReadItem(const OrderedJson& json, const std::string& key, WarpItemMatch& match)
    {
        if (!json.is_object() || !json.contains(Keys::Group) || !json.contains(Keys::Number))
        {
            return Fail(key, R"(must be {"group": 13, "number": 2})");
        }
        WarnUnknownIn(json, {Keys::Group, Keys::Number}, key);
        return Items::Json::ReadWholeNumberInRange(json[Keys::Group], WarpField(m_index, key + "." + Keys::Group), 0,
                                                   MAX_ITEM_TYPE - 1, match.group, FatalAt()) &&
               Items::Json::ReadWholeNumberInRange(json[Keys::Number], WarpField(m_index, key + "." + Keys::Number), 0,
                                                   MAX_ITEM_INDEX - 1, match.number, FatalAt());
    }

    // An unknown tag is a warning: it matches no item.
    bool ReadTag(const OrderedJson& json, const std::string& key, std::string& tag)
    {
        if (!json.is_string() || json.get_ref<const std::string&>().empty())
        {
            return Fail(key, "must be the name of an item tag, as the item files write it");
        }
        tag = json.get<std::string>();
        Items::ItemTag value{};
        if (!Items::FindEnumValue(tag, value))
        {
            AddIssue(m_issues, ItemDataIssueSeverity::Warning, m_source, WarpField(m_index, key),
                     "\"" + tag + "\" is no item tag (" + Items::JoinEnumNames<Items::ItemTag>() +
                         "): it matches no item");
        }
        return true;
    }

    // An unknown key is a warning: the general text is shown.
    bool ReadBlockedMessage(const OrderedJson& json, std::string& message)
    {
        const auto field = json.find(Keys::BlockedMessage);
        if (field == json.end())
        {
            return true;
        }
        if (!field->is_string())
        {
            return Fail(Keys::BlockedMessage, "must be the key of a game text");
        }
        message = field->get<std::string>();
        if (!message.empty() && !IsWarpMessageKey(message))
        {
            std::string keys;
            for (const std::string_view key : GetWarpMessageKeys())
            {
                keys += (keys.empty() ? "" : ", ") + std::string(key);
            }
            AddIssue(m_issues, ItemDataIssueSeverity::Warning, m_source, WarpField(m_index, Keys::BlockedMessage),
                     "\"" + message + "\" is no warp message (" + keys + "): the general one is shown");
        }
        return true;
    }

    void WarnUnknownIn(const OrderedJson& json, const Items::Json::KeySet& keys, const std::string& key)
    {
        Items::Json::WarnAboutUnknownKeys(
            json, keys, WarpField(m_index, key) + ".", [&](const std::string& field, const std::string& message)
            { AddIssue(m_issues, ItemDataIssueSeverity::Warning, m_source, field, message); });
    }

    bool ReadNames(const OrderedJson& json, LocalizedString& names)
    {
        const auto field = json.find(Keys::Name);
        if (field == json.end())
        {
            return Fail(Keys::Name, "missing");
        }
        if (!Items::Json::ReadLocalizedString(*field, WarpField(m_index, Keys::Name), names, FatalAt()))
        {
            return false;
        }
        return !names.GetNeutral().empty() ||
               Fail(Keys::Name, "must have an English name (\"en\"), which the other languages fall back to");
    }

    bool ReadCommand(const OrderedJson& json, std::string& command)
    {
        const auto field = json.find(Keys::Command);
        if (field == json.end() || !field->is_string() || field->get_ref<const std::string&>().empty())
        {
            return Fail(Keys::Command, "missing or not a text: the name /move takes");
        }
        command = field->get<std::string>();
        return true;
    }

    bool WarnUnknown(const OrderedJson& json)
    {
        Items::Json::WarnAboutUnknownKeys(
            json, WarpKeys, WarpField(m_index, "") + ".", [&](const std::string& field, const std::string& message)
            { AddIssue(m_issues, ItemDataIssueSeverity::Warning, m_source, field, message); });
        return true;
    }

    size_t m_index;
    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

OrderedJson WriteItemMatches(const std::vector<WarpItemMatch>& matches)
{
    OrderedJson list = OrderedJson::array();
    for (const WarpItemMatch& match : matches)
    {
        OrderedJson json = OrderedJson::object();
        switch (match.kind)
        {
        case WarpItemMatch::Kind::Item:
            json[Keys::Item] = OrderedJson::object();
            json[Keys::Item][Keys::Group] = match.group;
            json[Keys::Item][Keys::Number] = match.number;
            break;
        case WarpItemMatch::Kind::Tag:
            json[Keys::Tag] = match.tag;
            if (!match.exceptTag.empty())
            {
                json[Keys::Except] = OrderedJson::object();
                json[Keys::Except][Keys::Tag] = match.exceptTag;
            }
            break;
        case WarpItemMatch::Kind::Wings:
            json[Keys::Wings] = true;
            break;
        }
        list.push_back(std::move(json));
    }
    return list;
}

OrderedJson WriteWarp(const Warp& warp)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Index] = warp.index;
    json[Keys::Name] = Items::Json::WriteLocalizedString(warp.names);
    json[Keys::Command] = warp.command;
    json[Keys::Level] = warp.level;
    json[Keys::Zen] = warp.zen;
    json[Keys::Gate] = warp.gate;
    if (warp.noClassDiscount)
    {
        json[Keys::NoClassDiscount] = true;
    }
    if (warp.minMasterLevel != 0)
    {
        json[Keys::MinMasterLevel] = warp.minMasterLevel;
    }
    if (warp.minResets != 0)
    {
        json[Keys::MinResets] = warp.minResets;
    }
    if (warp.pvpServersOnly)
    {
        json[Keys::PvpServersOnly] = true;
    }
    if (warp.gensOnly)
    {
        json[Keys::GensOnly] = true;
    }
    if (!warp.needsOneOf.empty())
    {
        json[Keys::NeedsOneOf] = WriteItemMatches(warp.needsOneOf);
    }
    if (!warp.notWith.empty())
    {
        json[Keys::NotWith] = WriteItemMatches(warp.notWith);
    }
    if (!warp.blockedMessage.empty())
    {
        json[Keys::BlockedMessage] = warp.blockedMessage;
    }
    return json;
}

OrderedJson WriteDiscount(const ClassLevelDiscount& discount)
{
    OrderedJson json = OrderedJson::object();
    const auto classes = Items::GetClassKeys();
    if (discount.playerClass >= 0 && discount.playerClass < static_cast<int>(classes.size()))
    {
        json[Keys::Class] = classes[discount.playerClass];
    }
    else
    {
        json[Keys::Class] = discount.playerClass;
    }
    json[Keys::Numerator] = discount.numerator;
    json[Keys::Denominator] = discount.denominator;
    return json;
}

// One class level discount; false, with a fatal problem added, at the first
// value of the wrong form.
bool ReadDiscount(const OrderedJson& json, size_t index, const std::string& source, ClassLevelDiscount& discount,
                  std::vector<ItemDataIssue>& issues)
{
    const std::string field = std::string(Keys::ClassLevelDiscount) + "[" + std::to_string(index) + "]";
    const auto addFatal = [&](const std::string& at, const std::string& message)
    { AddIssue(issues, ItemDataIssueSeverity::Fatal, source, at, message); };
    if (!json.is_object())
    {
        addFatal(field, "must be an object with class, numerator and denominator");
        return false;
    }
    Items::Json::WarnAboutUnknownKeys(json, DiscountKeys, field + ".",
                                      [&](const std::string& at, const std::string& message)
                                      { AddIssue(issues, ItemDataIssueSeverity::Warning, source, at, message); });
    const auto classes = Items::GetClassKeys();
    const auto playerClass = json.find(Keys::Class);
    const auto found = playerClass != json.end() && playerClass->is_string()
                           ? std::find(classes.begin(), classes.end(), playerClass->get<std::string>())
                           : classes.end();
    if (found == classes.end())
    {
        std::string names;
        for (const char* name : classes)
        {
            names += (names.empty() ? "\"" : ", \"") + std::string(name) + "\"";
        }
        addFatal(field + "." + Keys::Class, "must be one of " + names);
        return false;
    }
    discount.playerClass = static_cast<int>(found - classes.begin());
    const auto readPart = [&](const char* key, int low, int& value)
    {
        const auto part = json.find(key);
        if (part == json.end())
        {
            addFatal(field + "." + key, "missing");
            return false;
        }
        return Items::Json::ReadWholeNumberInRange(*part, field + "." + key, low, MaxDiscountPart, value, addFatal);
    };
    return readPart(Keys::Numerator, 0, discount.numerator) && readPart(Keys::Denominator, 1, discount.denominator);
}

bool IsLongerThanTheGameHolds(const std::string& name)
{
    std::string cut = name;
    return Core::Text::CutToUtf16Length(cut, MaxWarpNameLength);
}

void AddWarning(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& field,
                const std::string& message)
{
    AddIssue(issues, ItemDataIssueSeverity::Warning, source, field, message);
}

void ValidateDiscounts(const WarpList& list, const std::string& source, std::vector<ItemDataIssue>& issues)
{
    for (size_t discount = 0; discount < list.classLevelDiscounts.size(); ++discount)
    {
        const ClassLevelDiscount& current = list.classLevelDiscounts[discount];
        const std::string field = std::string(Keys::ClassLevelDiscount) + "[" + std::to_string(discount) + "]";
        for (size_t other = 0; other < discount; ++other)
        {
            if (list.classLevelDiscounts[other].playerClass == current.playerClass)
            {
                AddIssue(issues, ItemDataIssueSeverity::Fatal, source, field + "." + Keys::Class,
                         "is the class of classLevelDiscount[" + std::to_string(other) + "] too: each class has one");
                break;
            }
        }
        if (current.numerator == 0)
        {
            AddWarning(issues, source, field + "." + Keys::Numerator, "is 0: the class needs no level for any warp");
        }
        else if (current.numerator > current.denominator)
        {
            AddWarning(issues, source, field + "." + Keys::Numerator,
                       "is more than the denominator: the class needs more level than the warps ask");
        }
    }
}

// The entries of one item rule list ("needsOneOf" or "notWith").
void ValidateItemMatches(const std::vector<WarpItemMatch>& matches, size_t warp, const char* key,
                         const std::string& source, std::vector<ItemDataIssue>& issues)
{
    for (size_t entry = 0; entry < matches.size(); ++entry)
    {
        const std::string field = WarpField(warp, std::string(key) + "[" + std::to_string(entry) + "]");
        const WarpItemMatch& match = matches[entry];
        const auto end = matches.begin() + static_cast<std::ptrdiff_t>(entry);
        const auto earlier = std::find(matches.begin(), end, match);
        if (earlier != end)
        {
            AddWarning(issues, source, field,
                       "is " + std::string(key) + "[" + std::to_string(earlier - matches.begin()) + "] again");
        }
        if (match.kind == WarpItemMatch::Kind::Tag && match.exceptTag == match.tag)
        {
            AddWarning(issues, source, field + "." + Keys::Except, "is the entry's own tag: the entry matches no item");
        }
    }
}

void WarnIfSpace(const std::string& name, const std::string& field, const std::string& source,
                 std::vector<ItemDataIssue>& issues)
{
    if (name.find(' ') != std::string::npos)
    {
        AddWarning(issues, source, field, "has a space: /move takes the name as one word");
    }
}
} // namespace

std::string WarpField(size_t warp, std::string_view key)
{
    std::string field = std::string(Keys::Warps) + "[" + std::to_string(warp) + "]";
    return key.empty() ? field : field + "." + std::string(key);
}

void ResolveItemTags(WarpList& list)
{
    const auto resolve = [](const std::string& name) -> std::optional<Items::ItemTag>
    {
        Items::ItemTag value{};
        return !name.empty() && Items::FindEnumValue(name, value) ? std::optional(value) : std::nullopt;
    };
    for (Warp& warp : list.warps)
    {
        for (std::vector<WarpItemMatch>* matches : {&warp.needsOneOf, &warp.notWith})
        {
            for (WarpItemMatch& match : *matches)
            {
                match.tagValue = resolve(match.tag);
                match.exceptTagValue = resolve(match.exceptTag);
            }
        }
    }
}

void ReadWarpsJson(std::string_view text, const std::string& source, WarpList& list,
                   std::vector<Items::ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, WarpsFormatVersion, root, issues))
    {
        return;
    }
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, Keys::ClassLevelDiscount, Keys::Warps},
                                      "", [&](const std::string& field, const std::string& message)
                                      { AddIssue(issues, ItemDataIssueSeverity::Warning, source, field, message); });
    const auto discounts = root.find(Keys::ClassLevelDiscount);
    if (discounts != root.end())
    {
        if (!discounts->is_array())
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, source, Keys::ClassLevelDiscount,
                     R"(must be a list of {"class": "darkLord", "numerator": 2, "denominator": 3})");
            return;
        }
        for (size_t index = 0; index < discounts->size(); ++index)
        {
            ClassLevelDiscount discount;
            if (ReadDiscount((*discounts)[index], index, source, discount, issues))
            {
                list.classLevelDiscounts.push_back(discount);
            }
        }
    }
    const auto warps = root.find(Keys::Warps);
    if (warps == root.end() || !warps->is_array())
    {
        AddIssue(issues, ItemDataIssueSeverity::Fatal, source, Keys::Warps, "missing or not a list of warps");
        return;
    }
    for (size_t index = 0; index < warps->size(); ++index)
    {
        Warp warp;
        if (WarpReader(index, source, issues).Read((*warps)[index], warp))
        {
            list.warps.push_back(std::move(warp));
        }
    }
    ResolveItemTags(list);
}

std::string WriteWarpsJson(const WarpList& list)
{
    OrderedJson warps = OrderedJson::array();
    for (const Warp& warp : list.warps)
    {
        warps.push_back(WriteWarp(warp));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = WarpsFormatVersion;
    if (!list.classLevelDiscounts.empty())
    {
        OrderedJson discounts = OrderedJson::array();
        for (const ClassLevelDiscount& discount : list.classLevelDiscounts)
        {
            discounts.push_back(WriteDiscount(discount));
        }
        root[Keys::ClassLevelDiscount] = std::move(discounts);
    }
    root[Keys::Warps] = std::move(warps);
    const std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    const std::string discountsOnOneLine = Items::Json::PutListEntriesOnOneLine(text, Keys::ClassLevelDiscount);
    return Items::Json::PutListEntriesOnOneLine(discountsOnOneLine, Keys::Warps) + "\n";
}

namespace
{
// The languages either warp has a name in, English first.
std::vector<std::string> GetNameLocales(const Warp& first, const Warp& second)
{
    std::vector<std::string> locales{std::string(LocalizedString::NeutralLocale)};
    for (const Warp* warp : {&first, &second})
    {
        for (const auto& [locale, name] : warp->names.GetTranslations())
        {
            if (std::find(locales.begin(), locales.end(), locale) == locales.end())
            {
                locales.push_back(locale);
            }
        }
    }
    return locales;
}

// The /move name or a name (in the language the M window shows) of warp `warp`
// that an earlier warp has as its name or /move name: /move, the warp word and
// the chat's check take the first warp a text names. One warning per field.
void WarnAboutSharedNames(const WarpList& list, size_t warp, const std::string& source,
                          std::vector<ItemDataIssue>& issues)
{
    const Warp& current = list.warps[warp];
    const auto warn = [&](const std::string& field, size_t other, const std::string& what)
    {
        AddWarning(issues, source, WarpField(warp, field),
                   std::string("is ") + what + " of warps[" + std::to_string(other) +
                       "] too: /move and the M window take the first of them");
    };
    const auto commandShared = [&]() -> bool
    {
        for (size_t other = 0; other < warp; ++other)
        {
            const Warp& earlier = list.warps[other];
            if (Core::Text::EqualsIgnoringAsciiCase(earlier.command, current.command))
            {
                warn(Keys::Command, other, "the /move name");
                return true;
            }
            for (const std::string& locale : GetNameLocales(current, earlier))
            {
                if (Core::Text::EqualsIgnoringAsciiCase(earlier.names.Get(locale), current.command))
                {
                    warn(Keys::Command, other, "a name");
                    return true;
                }
            }
        }
        return false;
    };
    commandShared();
    std::vector<std::string> warned;
    // The languages with the warp's own name first, so a clash where the English name only stands in for another
    // language doesn't take the place of a clash of the English name itself.
    for (const bool ownNamePass : {true, false})
    {
        for (size_t other = 0; other < warp; ++other)
        {
            const Warp& earlier = list.warps[other];
            for (const std::string& locale : GetNameLocales(current, earlier))
            {
                // Without a name of its own in the language, the English one is shown there: that is the field.
                const bool ownName = locale == LocalizedString::NeutralLocale || current.names.HasOwn(locale);
                const std::string fieldLocale = ownName ? locale : std::string(LocalizedString::NeutralLocale);
                if (ownName != ownNamePass || std::find(warned.begin(), warned.end(), fieldLocale) != warned.end())
                {
                    continue;
                }
                const std::string name = current.names.Get(locale);
                // In any case, as /warp and the M window find a warp by its name.
                const bool sameName = Core::Text::EqualsIgnoringAsciiCase(earlier.names.Get(locale), name);
                if (sameName || Core::Text::EqualsIgnoringAsciiCase(earlier.command, name))
                {
                    const std::string what = sameName ? "the name" : "the /move name";
                    warn(std::string(Keys::Name) + "." + fieldLocale, other,
                         ownName ? what : what + " (" + locale + ", where the English name is shown)");
                    warned.push_back(fieldLocale);
                }
            }
        }
    }
}
} // namespace

void ValidateWarps(const WarpList& list, const std::string& source, std::vector<Items::ItemDataIssue>& issues)
{
    ValidateDiscounts(list, source, issues);
    for (size_t warp = 0; warp < list.warps.size(); ++warp)
    {
        const Warp& current = list.warps[warp];
        for (size_t other = 0; other < warp; ++other)
        {
            if (list.warps[other].index == current.index)
            {
                AddIssue(issues, ItemDataIssueSeverity::Fatal, source, WarpField(warp, Keys::Index),
                         std::to_string(current.index) + " is the index of warps[" + std::to_string(other) +
                             "] too: each warp needs its own");
                break;
            }
        }
        WarnAboutSharedNames(list, warp, source, issues);
        const auto checkName = [&](const std::string& field, const std::string& name)
        {
            if (IsLongerThanTheGameHolds(name))
            {
                AddIssue(issues, ItemDataIssueSeverity::Warning, source, field,
                         "is longer than " + std::to_string(MaxWarpNameLength) + " characters: the game cuts it");
            }
            WarnIfSpace(name, field, source, issues);
        };
        checkName(WarpField(warp, std::string(Keys::Name) + "." + std::string(LocalizedString::NeutralLocale)),
                  current.names.GetNeutral());
        for (const auto& [locale, name] : current.names.GetTranslations())
        {
            checkName(WarpField(warp, std::string(Keys::Name) + "." + locale), name);
        }
        checkName(WarpField(warp, Keys::Command), current.command);
        ValidateItemMatches(current.needsOneOf, warp, Keys::NeedsOneOf, source, issues);
        ValidateItemMatches(current.notWith, warp, Keys::NotWith, source, issues);
        if (!current.blockedMessage.empty() && current.needsOneOf.empty() && current.notWith.empty())
        {
            AddWarning(issues, source, WarpField(warp, Keys::BlockedMessage),
                       "is never shown: the warp has no equipment rules (needsOneOf, notWith)");
        }
    }
}

void ValidateWarpItems(const WarpList& list, const std::function<bool(int group, int number)>& itemExists,
                       const std::string& source, std::vector<Items::ItemDataIssue>& issues)
{
    for (size_t warp = 0; warp < list.warps.size(); ++warp)
    {
        const Warp& current = list.warps[warp];
        for (const auto& [key, matches] :
             {std::pair{Keys::NeedsOneOf, &current.needsOneOf}, std::pair{Keys::NotWith, &current.notWith}})
        {
            for (size_t entry = 0; entry < matches->size(); ++entry)
            {
                const WarpItemMatch& match = (*matches)[entry];
                if (match.kind != WarpItemMatch::Kind::Item || itemExists(match.group, match.number))
                {
                    continue;
                }
                AddIssue(issues, ItemDataIssueSeverity::Error, source,
                         WarpField(warp, std::string(key) + "[" + std::to_string(entry) + "]." + Keys::Item),
                         "names item (" + std::to_string(match.group) + ", " + std::to_string(match.number) +
                             "), which the item list does not have: the entry matches no item");
            }
        }
    }
}
} // namespace Data::Maps
