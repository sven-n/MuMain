#pragma once

#include <map>
#include <set>
#include <string>
#include <string_view>

// An audit of the per-map file lists (Data/Maps/Files, docs/content-access.md): while it runs, each content read is
// checked against Shared.json, the current map's list and the lists of the monsters spawned so far, and a read outside
// them is logged once per file. Off by default: nothing is loaded and a read costs one flag check.
namespace Content::Audit
{
// The files of each key of a list file, by lower case: Shared.json and Map<n>.json hold one key, "files";
// Monsters.json one per monster number.
std::map<std::string, std::set<std::string>> ParseLists(std::string_view text);

// Starts the audit: reads Shared.json and Monsters.json through the content layer. Returns false, and stays off, when
// Shared.json can't be read.
bool Start();
// Starts it when MU_CONTENT_LISTS is "audit".
bool StartFromEnvironment();
void Stop();
bool Active();

// The map the server sent: its list (Map<n>.json) joins the checks. A map without a list adds nothing.
void EnterMap(int map);
// A monster or NPC spawned: its files (Monsters.json) join the checks for the rest of the run.
void NoteMonster(int type);
// A content read, by its logical path. Called by the content layer.
void NoteRead(std::string_view path);
} // namespace Content::Audit
