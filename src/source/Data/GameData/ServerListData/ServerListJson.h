#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ServerListData/ServerListGroup.h"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The server groups of the login screen (Data/Local/ServerList.json):
//
//   { "formatVersion": 1,
//     "groups": [
//       { "id": 0, "name": "Valhalla", "channels": {"1": "gold", "5": "goldPvp", "6": "nonPvp"} },
//       { "id": 1, "name": "Helheim", "description": "PvP everywhere" },
//       { "id": 9, "name": "Test", "testServer": true } ] }
//
// The list order is the order of the buttons. "id" is the server ID / 20: the
// connect server's servers 0 to 19 are group 0, 20 to 39 group 1. A server the
// list has no group for is not shown. "channels" gives the type of a channel
// by its number (server ID % 20 + 1): normal (the default), nonPvp, goldPvp or
// gold. "description" and "testServer" are optional.
namespace Data::ServerList
{
constexpr int ServerListFormatVersion = 1;

// The file, relative to the client folder.
constexpr const char* ServerListFile = "Data/Local/ServerList.json";

// The name of a channel type in the file ("nonPvp"), and back.
const char* GetChannelTypeName(ChannelType type);
std::optional<ChannelType> FindChannelType(std::string_view name);

// Reads the groups of the file into `groups`, in their order. A group with an
// error is left out; a name or description that is too long is cut (a warning).
// Lengths are counted in UTF-16 units, as the client's wide strings hold them on
// Windows: an emoji counts as two.
void ReadServerListJson(std::string_view text, const std::string& source, std::vector<ServerGroup>& groups,
                        std::vector<Items::ItemDataIssue>& issues);

// The text of the file with these groups, in their order.
std::string WriteServerListJson(std::span<const ServerGroup> groups);

struct ServerListLoadResult
{
    std::vector<ServerGroup> groups;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the file; one that cannot be read is an error.
ServerListLoadResult LoadServerListFile(const std::filesystem::path& file);

// Writes the groups to the file when they read back without fatal problems; the
// problems found are added to `issues`. The old file stays when the new one
// cannot be written whole.
bool SaveServerListFile(const std::filesystem::path& file, std::span<const ServerGroup> groups,
                        std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::ServerList
