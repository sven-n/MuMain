#pragma once

#include "Data/GameData/BuffData/BuffDefinition.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemOptionDefinition.h"

#include <filesystem>
#include <span>
#include <string>
#include <vector>

// Reads Data/Buffs/Buffs.json (buffs design, B1) and keeps the table (g_BuffDatabase) in step with it.
namespace Data::Buffs
{
// "Data/Buffs/Buffs.json", from the game folder.
std::filesystem::path GetBuffsFile();

struct BuffsLoadResult
{
    std::vector<BuffDefinition> buffs;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the file as the game does: the entries and every problem of the file and its checks (ValidateBuffs,
// ValidateBuffsForGame).
BuffsLoadResult LoadBuffsFile(const std::filesystem::path& file);

// What the game cannot show, as warnings: a number it has no buff state for, a text in a language that is not one of
// `uiLocales` (I18N::GetAvailableLocales), a name longer than the buff info holds. `entries` as for ValidateBuffs.
void ValidateBuffsForGame(std::span<const BuffDefinition> buffs, std::span<const size_t> entries,
                          std::span<const char* const> uiLocales, const std::string& source,
                          std::vector<Items::ItemDataIssue>& issues);

// ValidateBuffItems against the item list as the game has it (g_ItemDatabase), for the file's buffs.
void ValidateBuffItemsInGame(std::span<const BuffDefinition> buffs, const std::string& source,
                             std::vector<Items::ItemDataIssue>& issues);

// "Data/Buffs/BuffTexts.json", the skill tooltip's buff value texts (buffs design B9).
std::filesystem::path GetBuffTextsFile();

struct BuffTextsLoadResult
{
    Items::ItemOptionTexts texts;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the texts and warns about languages the game has no texts in; a file it can't read is a warning, since a
// missing text only leaves its lines out.
BuffTextsLoadResult LoadBuffTextsFile(const std::filesystem::path& file);

// At start: the file into the table in the display language, its problems logged; the table follows later language
// changes. False, with the table left as it was and `errorMessage` describing them, on fatal ones.
bool LoadBuffs(std::string& errorMessage);
} // namespace Data::Buffs
