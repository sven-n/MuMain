#include "stdafx.h"

#include "ServerListJson.h"

#include "Core/Text/Utf8.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <set>

namespace Data::ServerList
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

constexpr const char* GroupsKey = "groups";
constexpr const char* IdKey = "id";
constexpr const char* NameKey = "name";
constexpr const char* DescriptionKey = "description";
constexpr const char* TestServerKey = "testServer";
constexpr const char* ChannelsKey = "channels";

// In the order of the ChannelType values.
constexpr std::array<const char*, ChannelTypeCount> ChannelTypeNames = {"normal", "nonPvp", "goldPvp", "gold"};

class IssueList
{
public:
    IssueList(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues) {}

    void Fatal(const std::string& field, const std::string& message)
    {
        Items::Json::AddFileIssue(m_issues, m_source, ItemDataIssue::NoItem, field, message);
    }

    void Warning(const std::string& field, const std::string& message)
    {
        m_issues.push_back(
            {ItemDataIssueSeverity::Warning, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
    }

private:
    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

std::string GroupField(size_t index)
{
    return std::string(GroupsKey) + "[" + std::to_string(index) + "]";
}

std::string Field(const std::string& object, const char* key)
{
    return object + "." + key;
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
        issues.Warning(Field(field, key),
                       "longer than " + std::to_string(maxUnits) + " characters (an emoji counts as two), cut");
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

bool ReadTestServer(const OrderedJson& json, const std::string& field, bool& testServer, IssueList& issues)
{
    const auto value = json.find(TestServerKey);
    if (value == json.end())
    {
        return true;
    }
    if (!value->is_boolean())
    {
        issues.Fatal(Field(field, TestServerKey), "must be true or false");
        return false;
    }
    testServer = value->get<bool>();
    return true;
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

bool ReadChannel(const std::string& key, const OrderedJson& value, const std::string& field, ServerGroup& group,
                 IssueList& issues)
{
    const std::optional<int> number = ParseChannelNumber(key);
    if (!number)
    {
        issues.Fatal(field, "a channel number is 1 to " + std::to_string(MaxChannelsPerGroup));
        return false;
    }
    const std::optional<ChannelType> type =
        value.is_string() ? FindChannelType(value.get<std::string>()) : std::nullopt;
    if (!type)
    {
        issues.Fatal(field, "must be normal, nonPvp, goldPvp or gold");
        return false;
    }
    group.channels[*number - 1] = *type;
    return true;
}

bool ReadChannels(const OrderedJson& json, const std::string& field, ServerGroup& group, IssueList& issues)
{
    const auto channels = json.find(ChannelsKey);
    if (channels == json.end())
    {
        return true;
    }
    if (!channels->is_object())
    {
        issues.Fatal(Field(field, ChannelsKey), "must be an object of channel numbers and types");
        return false;
    }
    bool valid = true;
    for (const auto& [key, value] : channels->items())
    {
        valid = ReadChannel(key, value, Field(Field(field, ChannelsKey), key.c_str()), group, issues) && valid;
    }
    return valid;
}

std::optional<ServerGroup> ReadGroup(const OrderedJson& json, const std::string& field, IssueList& issues)
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
    valid = ReadTestServer(json, field, group.testServer, issues) && valid;
    valid = ReadChannels(json, field, group, issues) && valid;
    Items::Json::WarnAboutUnknownKeys(json, {IdKey, NameKey, DescriptionKey, TestServerKey, ChannelsKey}, field + ".",
                                      [&](const std::string& unknown, const std::string& message)
                                      { issues.Warning(unknown, message); });
    return valid ? std::optional<ServerGroup>(std::move(group)) : std::nullopt;
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

OrderedJson WriteChannels(const ServerGroup& group)
{
    OrderedJson channels = OrderedJson::object();
    for (size_t index = 0; index < group.channels.size(); ++index)
    {
        if (group.channels[index] != ChannelType::Normal)
        {
            channels[std::to_string(index + 1)] = GetChannelTypeName(group.channels[index]);
        }
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
} // namespace

const char* GetChannelTypeName(ChannelType type)
{
    return ChannelTypeNames[static_cast<size_t>(type)];
}

std::optional<ChannelType> FindChannelType(std::string_view name)
{
    const auto found = std::find(ChannelTypeNames.begin(), ChannelTypeNames.end(), name);
    if (found == ChannelTypeNames.end())
    {
        return std::nullopt;
    }
    return static_cast<ChannelType>(std::distance(ChannelTypeNames.begin(), found));
}

void ReadServerListJson(std::string_view text, const std::string& source, std::vector<ServerGroup>& groups,
                        std::vector<ItemDataIssue>& issues)
{
    IssueList list(source, issues);
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, ServerListFormatVersion, root, issues))
    {
        return;
    }
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, GroupsKey}, "",
                                      [&](const std::string& unknown, const std::string& message)
                                      { list.Warning(unknown, message); });
    const auto entries = root.find(GroupsKey);
    if (entries == root.end() || !entries->is_array())
    {
        list.Fatal(GroupsKey, "missing or not a list of server groups");
        return;
    }
    std::set<int> ids;
    for (size_t index = 0; index < entries->size(); ++index)
    {
        const std::string field = GroupField(index);
        std::optional<ServerGroup> group = ReadGroup((*entries)[index], field, list);
        if (!group)
        {
            continue;
        }
        if (!ids.insert(group->id).second)
        {
            list.Fatal(Field(field, IdKey), "group " + std::to_string(group->id) + " is in the list more than once");
            continue;
        }
        groups.push_back(std::move(*group));
    }
    WarnAboutMoreTestServers(groups, list);
}

std::string WriteServerListJson(std::span<const ServerGroup> groups)
{
    OrderedJson entries = OrderedJson::array();
    for (const ServerGroup& group : groups)
    {
        entries.push_back(WriteGroup(group));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = ServerListFormatVersion;
    root[GroupsKey] = std::move(entries);
    const std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    return Items::Json::PutObjectsOnOneLine(text, ChannelsKey) + "\n";
}

ServerListLoadResult LoadServerListFile(const std::filesystem::path& file)
{
    ServerListLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = Items::Json::ReadTextFile(file);
    if (!text)
    {
        IssueList(source, result.issues).Fatal("", "could not be read");
        return result;
    }
    ReadServerListJson(*text, source, result.groups, result.issues);
    return result;
}

bool SaveServerListFile(const std::filesystem::path& file, std::span<const ServerGroup> groups,
                        std::vector<ItemDataIssue>& issues)
{
    const std::string source = file.generic_string();
    const std::string text = WriteServerListJson(groups);
    std::vector<ServerGroup> readBack;
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
