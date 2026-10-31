#include "stdafx.h"

#include "ServerListJson.h"

#include "ChannelNameBuilder.h"
#include "Core/Text/Utf8.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <optional>
#include <set>

namespace Data::ServerList
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

constexpr const char* GroupsKey = "groups";
constexpr const char* VipTypesKey = "vipTypes";
constexpr const char* NamesKey = "names";
constexpr const char* IdKey = "id";
constexpr const char* NameKey = "name";
constexpr const char* DescriptionKey = "description";
constexpr const char* TestServerKey = "testServer";
constexpr const char* ChannelsKey = "channels";
constexpr const char* PvpKey = "pvp";
constexpr const char* VipKey = "vip";
constexpr const char* ThemeColorKey = "themeColor";
constexpr const char* NonPvpKey = "nonPvp";

class IssueList
{
public:
    IssueList(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues) {}

    void Fatal(const std::string& field, const std::string& message)
    {
        Items::Json::AddFileIssue(m_issues, m_source, ItemDataIssue::NoItem, field, message);
    }

    void Error(const std::string& field, const std::string& message)
    {
        Add(ItemDataIssueSeverity::Error, field, message);
    }

    void Warning(const std::string& field, const std::string& message)
    {
        Add(ItemDataIssueSeverity::Warning, field, message);
    }

private:
    void Add(ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
    {
        m_issues.push_back({severity, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
    }

    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

std::string ListField(const char* key, size_t index)
{
    return std::string(key) + "[" + std::to_string(index) + "]";
}

std::string Field(const std::string& object, const char* key)
{
    return object + "." + key;
}

// For WarnAboutUnknownKeys: each unknown field is a warning.
auto WarnUnknown(IssueList& issues)
{
    return [&issues](const std::string& unknown, const std::string& message) { issues.Warning(unknown, message); };
}

std::string CutWarning(size_t maxUnits)
{
    return "longer than " + std::to_string(maxUnits) + " characters (an emoji counts as two), cut";
}

bool ReadId(const OrderedJson& json, const std::string& field, int& id, IssueList& issues)
{
    const auto value = json.find(IdKey);
    long long number = 0;
    if (value == json.end() || !value->is_number_integer() || !Items::Json::ReadWholeNumber(*value, number) ||
        number < 0 || number > MaxServerGroupId)
    {
        issues.Fatal(Field(field, IdKey),
                     "missing or not a whole number from 0 to " + std::to_string(MaxServerGroupId));
        return false;
    }
    id = static_cast<int>(number);
    return true;
}

// A text of the group; one longer than maxUnits UTF-16 units is cut.
bool ReadText(const OrderedJson& json, const std::string& field, const char* key, size_t maxUnits, std::string& text,
              IssueList& issues)
{
    const auto value = json.find(key);
    if (value == json.end())
    {
        return true;
    }
    if (!value->is_string())
    {
        issues.Fatal(Field(field, key), "must be a text");
        return false;
    }
    text = value->get<std::string>();
    if (Core::Text::CutToUtf16Length(text, maxUnits))
    {
        issues.Warning(Field(field, key), CutWarning(maxUnits));
    }
    return true;
}

bool ReadName(const OrderedJson& json, const std::string& field, std::string& name, IssueList& issues)
{
    if (!ReadText(json, field, NameKey, MaxServerGroupNameLength, name, issues))
    {
        return false;
    }
    if (name.empty())
    {
        issues.Fatal(Field(field, NameKey), "missing or empty");
        return false;
    }
    return true;
}

bool ReadBool(const OrderedJson& json, const std::string& field, const char* key, bool& value, IssueList& issues)
{
    const auto found = json.find(key);
    if (found == json.end())
    {
        return true;
    }
    if (!found->is_boolean())
    {
        issues.Fatal(Field(field, key), "must be true or false");
        return false;
    }
    value = found->get<bool>();
    return true;
}

// A text with its translations, as item names are written: a plain text (the
// English one) or an object of texts by language. Each is cut to maxUnits
// UTF-16 units. `hasEnglish` tells whether the English text was given.
bool ReadLocalizedText(const OrderedJson& json, const std::string& field, size_t maxUnits, LocalizedString& text,
                       bool& hasEnglish, IssueList& issues)
{
    LocalizedString read;
    if (!Items::Json::ReadLocalizedString(json, field, read,
                                          [&issues](const std::string& errorField, const std::string& message)
                                          { issues.Fatal(errorField, message); }))
    {
        return false;
    }
    hasEnglish = json.is_string() || json.contains(std::string(LocalizedString::NeutralLocale));
    bool valid = true;
    // A copy: Set changes the translations while they are gone through.
    const auto translations = read.GetTranslations();
    for (const auto& [locale, translation] : translations)
    {
        if (!LocalizedString::IsValidLocaleCode(locale))
        {
            issues.Fatal(field + "." + locale,
                         "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
            valid = false;
            continue;
        }
        std::string cut = translation;
        if (Core::Text::CutToUtf16Length(cut, maxUnits))
        {
            issues.Warning(field + "." + locale, CutWarning(maxUnits));
            read.Set(locale, std::move(cut));
        }
    }
    std::string neutral = read.GetNeutral();
    if (Core::Text::CutToUtf16Length(neutral, maxUnits))
    {
        issues.Warning(field + "." + std::string(LocalizedString::NeutralLocale), CutWarning(maxUnits));
        read.Set(LocalizedString::NeutralLocale, std::move(neutral));
    }
    if (valid)
    {
        text = std::move(read);
    }
    return valid;
}

std::optional<VipType> ReadVipType(const OrderedJson& json, const std::string& field, IssueList& issues)
{
    if (!json.is_object())
    {
        issues.Fatal(field, "must be an object");
        return std::nullopt;
    }
    VipType vipType;
    bool valid = true;
    const auto id = json.find(IdKey);
    if (id == json.end() || !id->is_string() || !IsVipTypeId(id->get<std::string>()))
    {
        issues.Fatal(Field(field, IdKey), "missing, or not letters and digits starting with a lowercase letter");
        valid = false;
    }
    else
    {
        vipType.id = id->get<std::string>();
    }

    const auto name = json.find(NameKey);
    bool hasEnglish = false;
    if (name == json.end())
    {
        issues.Fatal(Field(field, NameKey), "missing");
        valid = false;
    }
    else if (ReadLocalizedText(*name, Field(field, NameKey), MaxVipTypeNameLength, vipType.name, hasEnglish, issues))
    {
        if (vipType.name.GetNeutral().empty())
        {
            issues.Fatal(Field(field, NameKey), "the English name is missing");
            valid = false;
        }
    }
    else
    {
        valid = false;
    }

    const auto color = json.find(ThemeColorKey);
    if (color != json.end())
    {
        const std::optional<ThemeColor> found =
            color->is_string() ? FindThemeColor(color->get<std::string>()) : std::nullopt;
        if (found)
        {
            vipType.themeColor = *found;
        }
        else
        {
            issues.Warning(Field(field, ThemeColorKey),
                           "not a theme color (default, gray, orange or silver); the default color is used");
        }
    }
    Items::Json::WarnAboutUnknownKeys(json, {IdKey, NameKey, ThemeColorKey}, field + ".", WarnUnknown(issues));
    return valid ? std::optional<VipType>(std::move(vipType)) : std::nullopt;
}

void ReadVipTypes(const OrderedJson& root, std::vector<VipType>& vipTypes, IssueList& issues)
{
    const auto entries = root.find(VipTypesKey);
    if (entries == root.end())
    {
        return;
    }
    if (!entries->is_array())
    {
        issues.Fatal(VipTypesKey, "must be a list of VIP types");
        return;
    }
    std::set<std::string, std::less<>> ids;
    for (size_t index = 0; index < entries->size(); ++index)
    {
        const std::string field = ListField(VipTypesKey, index);
        std::optional<VipType> vipType = ReadVipType((*entries)[index], field, issues);
        if (!vipType)
        {
            continue;
        }
        if (!ids.insert(vipType->id).second)
        {
            issues.Fatal(Field(field, IdKey), "\"" + vipType->id + "\" is the id of more than one VIP type");
            continue;
        }
        vipTypes.push_back(std::move(*vipType));
    }
}

std::string ListPlaceholders(ChannelNamePlace place)
{
    std::string list;
    for (const std::string_view placeholder : GetChannelNamePlaceholders(place))
    {
        list += (list.empty() ? "{" : ", {") + std::string(placeholder) + "}";
    }
    return list;
}

// A template shows what it cannot fill as written: an error for each.
void CheckTemplate(const std::string& text, ChannelNamePlace place, const std::string& field, IssueList& issues)
{
    const Items::PlaceholderScan scan = Items::ScanPlaceholders(text);
    if (!scan.error.empty())
    {
        issues.Error(field, scan.error);
        return;
    }
    const std::span<const std::string_view> known = GetChannelNamePlaceholders(place);
    for (const std::string& name : scan.names)
    {
        if (std::find(known.begin(), known.end(), name) == known.end())
        {
            issues.Error(field, "{" + name + "} is not a placeholder of the " + GetChannelNamePlaceKey(place) +
                                    " template, which has " + ListPlaceholders(place) + "; it is shown as written");
        }
    }
}

// One text of "names"; one the file leaves out stays the built-in one, and so
// does the English text of one that has only translations.
void ReadNamesText(const OrderedJson& names, const char* key, const LocalizedString& builtIn, LocalizedString& text,
                   IssueList& issues)
{
    const auto value = names.find(key);
    if (value == names.end())
    {
        return;
    }
    LocalizedString read;
    bool hasEnglish = false;
    if (!ReadLocalizedText(*value, Field(NamesKey, key), MaxChannelNameTextLength, read, hasEnglish, issues))
    {
        return;
    }
    if (!hasEnglish)
    {
        read.Set(LocalizedString::NeutralLocale, builtIn.GetNeutral());
    }
    text = std::move(read);
}

void ReadNames(const OrderedJson& root, ChannelNames& names, IssueList& issues)
{
    const auto json = root.find(NamesKey);
    if (json == root.end())
    {
        return;
    }
    if (!json->is_object())
    {
        issues.Fatal(NamesKey, "must be an object of texts by name");
        return;
    }
    const ChannelNames builtIn = GetDefaultChannelNames();
    ReadNamesText(*json, PvpKey, builtIn.pvp, names.pvp, issues);
    ReadNamesText(*json, NonPvpKey, builtIn.nonPvp, names.nonPvp, issues);
    Items::Json::KeySet known = {PvpKey, NonPvpKey};
    for (int value = 0; value < ChannelNamePlaceCount; ++value)
    {
        const ChannelNamePlace place = static_cast<ChannelNamePlace>(value);
        const char* key = GetChannelNamePlaceKey(place);
        known.insert(key);
        LocalizedString& text = names.Template(place);
        ReadNamesText(*json, key, builtIn.Template(place), text, issues);
        const std::string field = Field(NamesKey, key);
        CheckTemplate(text.GetNeutral(), place, field + "." + std::string(LocalizedString::NeutralLocale), issues);
        for (const auto& [locale, translation] : text.GetTranslations())
        {
            CheckTemplate(translation, place, field + "." + locale, issues);
        }
    }
    Items::Json::WarnAboutUnknownKeys(*json, known, std::string(NamesKey) + ".", WarnUnknown(issues));
}

// "1" to "20"; nothing for anything else.
std::optional<int> ParseChannelNumber(std::string_view text)
{
    int number = 0;
    const char* end = text.data() + text.size();
    const auto [parsedEnd, error] = std::from_chars(text.data(), end, number);
    if (error != std::errc{} || parsedEnd != end || number < 1 || number > MaxChannelsPerGroup)
    {
        return std::nullopt;
    }
    return number;
}

bool ReadChannel(const std::string& key, const OrderedJson& value, const std::string& field, const ServerListData& list,
                 ServerGroup& group, IssueList& issues)
{
    const std::optional<int> number = ParseChannelNumber(key);
    if (!number)
    {
        issues.Fatal(field, "a channel number is 1 to " + std::to_string(MaxChannelsPerGroup));
        return false;
    }
    if (!value.is_object())
    {
        issues.Fatal(field, R"(must be an object with "pvp" and "vip", e.g. {"vip": "gold", "pvp": false})");
        return false;
    }
    ChannelSettings channel;
    bool valid = ReadBool(value, field, PvpKey, channel.pvp, issues);
    const auto vip = value.find(VipKey);
    if (vip != value.end())
    {
        if (!vip->is_string())
        {
            issues.Fatal(Field(field, VipKey), "must be the id of a VIP type");
            valid = false;
        }
        else if (list.FindVipType(vip->get<std::string>()) == nullptr)
        {
            issues.Error(Field(field, VipKey),
                         "\"" + vip->get<std::string>() + "\" is not the id of a VIP type; the channel has none");
        }
        else
        {
            channel.vip = vip->get<std::string>();
        }
    }
    Items::Json::WarnAboutUnknownKeys(value, {PvpKey, VipKey}, field + ".", WarnUnknown(issues));
    group.channels[*number - 1] = std::move(channel);
    return valid;
}

bool ReadChannels(const OrderedJson& json, const std::string& field, const ServerListData& list, ServerGroup& group,
                  IssueList& issues)
{
    const auto channels = json.find(ChannelsKey);
    if (channels == json.end())
    {
        return true;
    }
    if (!channels->is_object())
    {
        issues.Fatal(Field(field, ChannelsKey), "must be an object of channels by number");
        return false;
    }
    bool valid = true;
    for (const auto& [key, value] : channels->items())
    {
        valid = ReadChannel(key, value, Field(Field(field, ChannelsKey), key.c_str()), list, group, issues) && valid;
    }
    return valid;
}

std::optional<ServerGroup> ReadGroup(const OrderedJson& json, const std::string& field, const ServerListData& list,
                                     IssueList& issues)
{
    if (!json.is_object())
    {
        issues.Fatal(field, "must be an object");
        return std::nullopt;
    }
    ServerGroup group;
    bool valid = ReadId(json, field, group.id, issues);
    valid = ReadName(json, field, group.name, issues) && valid;
    valid = ReadText(json, field, DescriptionKey, MaxServerGroupDescriptionLength, group.description, issues) && valid;
    valid = ReadBool(json, field, TestServerKey, group.testServer, issues) && valid;
    valid = ReadChannels(json, field, list, group, issues) && valid;
    Items::Json::WarnAboutUnknownKeys(json, {IdKey, NameKey, DescriptionKey, TestServerKey, ChannelsKey}, field + ".",
                                      WarnUnknown(issues));
    return valid ? std::optional<ServerGroup>(std::move(group)) : std::nullopt;
}

void ReadGroups(const OrderedJson& root, ServerListData& list, IssueList& issues)
{
    const auto entries = root.find(GroupsKey);
    if (entries == root.end() || !entries->is_array())
    {
        issues.Fatal(GroupsKey, "missing or not a list of server groups");
        return;
    }
    std::set<int> ids;
    for (size_t index = 0; index < entries->size(); ++index)
    {
        const std::string field = ListField(GroupsKey, index);
        std::optional<ServerGroup> group = ReadGroup((*entries)[index], field, list, issues);
        if (!group)
        {
            continue;
        }
        if (!ids.insert(group->id).second)
        {
            issues.Fatal(Field(field, IdKey), "group " + std::to_string(group->id) + " is in the list more than once");
            continue;
        }
        list.groups.push_back(std::move(*group));
    }
}

// The login screen shows the first test server group only.
void WarnAboutMoreTestServers(const std::vector<ServerGroup>& groups, IssueList& issues)
{
    const auto first =
        std::find_if(groups.begin(), groups.end(), [](const ServerGroup& group) { return group.testServer; });
    if (first == groups.end())
    {
        return;
    }
    for (auto group = std::next(first); group != groups.end(); ++group)
    {
        if (group->testServer)
        {
            issues.Warning(TestServerKey, "group " + std::to_string(group->id) +
                                              " is not shown: only the first test server group is, which is group " +
                                              std::to_string(first->id));
        }
    }
}

// Version 1 is read as far as its version, then refused.
bool IsCurrentVersion(const OrderedJson& root, IssueList& issues)
{
    // ReadFileVersion found it, a whole number from 1 to ServerListFormatVersion.
    long long version = 0;
    const auto found = root.find(Items::Json::Keys::FormatVersion);
    if (found != root.end() && Items::Json::ReadWholeNumber(*found, version) && version == ServerListFormatVersion)
    {
        return true;
    }
    issues.Fatal(Items::Json::Keys::FormatVersion,
                 "version " + std::to_string(version) +
                     " is not read: since version 2 a channel has a PvP box and a VIP type (docs/server-list.md)");
    return false;
}

OrderedJson WriteVipType(const VipType& vipType)
{
    OrderedJson json = OrderedJson::object();
    json[IdKey] = vipType.id;
    json[NameKey] = Items::Json::WriteLocalizedString(vipType.name);
    if (vipType.themeColor != ThemeColor::Default)
    {
        json[ThemeColorKey] = GetThemeColorName(vipType.themeColor);
    }
    return json;
}

OrderedJson WriteNames(const ChannelNames& names)
{
    OrderedJson json = OrderedJson::object();
    json[PvpKey] = Items::Json::WriteLocalizedString(names.pvp);
    json[NonPvpKey] = Items::Json::WriteLocalizedString(names.nonPvp);
    for (int value = 0; value < ChannelNamePlaceCount; ++value)
    {
        const ChannelNamePlace place = static_cast<ChannelNamePlace>(value);
        json[GetChannelNamePlaceKey(place)] = Items::Json::WriteLocalizedString(names.Template(place));
    }
    return json;
}

OrderedJson WriteChannels(const ServerGroup& group)
{
    OrderedJson channels = OrderedJson::object();
    for (size_t index = 0; index < group.channels.size(); ++index)
    {
        const ChannelSettings& channel = group.channels[index];
        if (channel == ChannelSettings{})
        {
            continue;
        }
        OrderedJson json = OrderedJson::object();
        if (!channel.vip.empty())
        {
            json[VipKey] = channel.vip;
        }
        if (!channel.pvp)
        {
            json[PvpKey] = false;
        }
        channels[std::to_string(index + 1)] = std::move(json);
    }
    return channels;
}

OrderedJson WriteGroup(const ServerGroup& group)
{
    OrderedJson json = OrderedJson::object();
    json[IdKey] = group.id;
    json[NameKey] = group.name;
    if (!group.description.empty())
    {
        json[DescriptionKey] = group.description;
    }
    if (group.testServer)
    {
        json[TestServerKey] = true;
    }
    OrderedJson channels = WriteChannels(group);
    if (!channels.empty())
    {
        json[ChannelsKey] = std::move(channels);
    }
    return json;
}

// Each text with its translations: the shown ones and those the game never shows.
template <typename TVisit> void ForEachText(const ServerListData& list, TVisit&& visit)
{
    for (size_t index = 0; index < list.vipTypes.size(); ++index)
    {
        visit(Field(ListField(VipTypesKey, index), NameKey), list.vipTypes[index].name);
    }
    visit(Field(NamesKey, PvpKey), list.names.pvp);
    visit(Field(NamesKey, NonPvpKey), list.names.nonPvp);
    for (int value = 0; value < ChannelNamePlaceCount; ++value)
    {
        const ChannelNamePlace place = static_cast<ChannelNamePlace>(value);
        visit(Field(NamesKey, GetChannelNamePlaceKey(place)), list.names.Template(place));
    }
}
} // namespace

void ReadServerListJson(std::string_view text, const std::string& source, ServerListData& list,
                        std::vector<ItemDataIssue>& issues)
{
    IssueList issueList(source, issues);
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, ServerListFormatVersion, root, issues) ||
        !IsCurrentVersion(root, issueList))
    {
        return;
    }
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, VipTypesKey, NamesKey, GroupsKey}, "",
                                      WarnUnknown(issueList));
    // Before the groups: their channels name the VIP types.
    ReadVipTypes(root, list.vipTypes, issueList);
    ReadNames(root, list.names, issueList);
    ReadGroups(root, list, issueList);
    WarnAboutMoreTestServers(list.groups, issueList);
}

void ValidateServerListLocales(const ServerListData& list, std::span<const char* const> uiLocales,
                               const std::string& source, std::vector<ItemDataIssue>& issues)
{
    IssueList issueList(source, issues);
    ForEachText(list,
                [&](const std::string& field, const LocalizedString& text)
                {
                    for (const auto& [locale, translation] : text.GetTranslations())
                    {
                        const bool shown = std::any_of(uiLocales.begin(), uiLocales.end(),
                                                       [&locale](const char* uiLocale) { return locale == uiLocale; });
                        if (!shown)
                        {
                            issueList.Warning(field + "." + locale,
                                              "the game has no language \"" + locale + "\"; the text is not used");
                        }
                    }
                });
}

std::string WriteServerListJson(const ServerListData& list)
{
    OrderedJson vipTypes = OrderedJson::array();
    for (const VipType& vipType : list.vipTypes)
    {
        vipTypes.push_back(WriteVipType(vipType));
    }
    OrderedJson groups = OrderedJson::array();
    for (const ServerGroup& group : list.groups)
    {
        groups.push_back(WriteGroup(group));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = ServerListFormatVersion;
    root[VipTypesKey] = std::move(vipTypes);
    root[NamesKey] = WriteNames(list.names);
    root[GroupsKey] = std::move(groups);
    std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    text = Items::Json::PutObjectsOnOneLine(text, ChannelsKey);
    // A VIP type's names, by language, on the line of its name.
    text = Items::Json::PutObjectsOnOneLine(text, NameKey);
    return text + "\n";
}

ServerListLoadResult LoadServerListFile(const std::filesystem::path& file, std::span<const char* const> uiLocales)
{
    ServerListLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = Items::Json::ReadTextFile(file);
    if (!text)
    {
        IssueList(source, result.issues).Fatal("", "could not be read");
        return result;
    }
    ReadServerListJson(*text, source, result.list, result.issues);
    if (!uiLocales.empty())
    {
        ValidateServerListLocales(result.list, uiLocales, source, result.issues);
    }
    return result;
}

bool SaveServerListFile(const std::filesystem::path& file, const ServerListData& list,
                        std::vector<ItemDataIssue>& issues)
{
    const std::string source = file.generic_string();
    const std::string text = WriteServerListJson(list);
    ServerListData readBack;
    ReadServerListJson(text, source, readBack, issues);
    if (Items::HasFatal(issues))
    {
        return false;
    }

    std::string error;
    if (!WriteTextFile(file, text, error))
    {
        IssueList(source, issues).Fatal("", error);
        return false;
    }
    return true;
}
} // namespace Data::ServerList
