#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ServerListData/ServerListData.h"

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The server groups of the login screen, the VIP types of their channels, the
// texts that name a channel and how a server's load shows (Data/Local/ServerList.json):
//
//   { "formatVersion": 2,
//     "vipTypes": [ {"id": "gold", "name": {"en": "Gold", "es": "Oro"}} ],
//     "buttonColors": [ {"vip": "gold", "color": "orange"} ],
//     "names": { "pvp": {"en": "PvP"}, "nonPvp": {"en": "Non-PvP"},
//                "button": {"en": "{name}-{channel} ({vip} {pvp}) {load}"},
//                "login": {"en": "[{name}-{channel} ({vip} {pvp})]"}, ... },
//     "load": { "bar": {"base": "default", "full": "default"},
//               "full": {"text": {"en": "(Full)"}, "bold": true}, ... },
//     "groups": [
//       { "id": 0, "name": "Valhalla", "channels": {"1": {"vip": "gold", "pvp": false}, "6": {"pvp": false}} },
//       { "id": 1, "name": "Helheim", "description": "PvP everywhere" },
//       { "id": 9, "name": "Test", "testServer": true } ] }
//
// The list order is the order of the buttons. "id" is the server ID / 20: the
// connect server's servers 0 to 19 are group 0, 20 to 39 group 1. A server the
// list has no group for is not shown. "channels" gives a channel's settings by
// its number (server ID % 20 + 1): PvP (on by default) and a VIP type (none by
// default). Everything but "formatVersion", "groups" and a group's "id" and
// "name" is optional; a text the file leaves out is the built-in English one.
// docs/server-list.md has the whole format.
namespace Data::ServerList
{
// Version 1 had four fixed channel types; it is not read.
constexpr int ServerListFormatVersion = 2;

// The file, relative to the client folder.
constexpr const char* ServerListFile = "Data/Local/ServerList.json";

// Reads the file into `list`. A group or VIP type with an error is left out; a
// text that is too long is cut (a warning). Lengths are counted in UTF-16
// units, as the client's wide strings hold them on Windows: an emoji counts as
// two.
void ReadServerListJson(std::string_view text, const std::string& source, ServerListData& list,
                        std::vector<Items::ItemDataIssue>& issues);

// Warns about each translation in a language the game has no texts in
// (`uiLocales`, I18N::GetAvailableLocales): it is never shown.
void ValidateServerListLocales(const ServerListData& list, std::span<const char* const> uiLocales,
                               const std::string& source, std::vector<Items::ItemDataIssue>& issues);

// The text of the file with this list.
std::string WriteServerListJson(const ServerListData& list);

struct ServerListLoadResult
{
    ServerListData list;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the file; one that cannot be read is an error. Its translations are
// checked against the game's languages, `uiLocales` (ValidateServerListLocales).
ServerListLoadResult LoadServerListFile(const std::filesystem::path& file, std::span<const char* const> uiLocales);

// Writes the list to the file when it reads back without fatal problems; the
// problems found are added to `issues`. The old file stays when the new one
// cannot be written whole.
bool SaveServerListFile(const std::filesystem::path& file, const ServerListData& list,
                        std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::ServerList
