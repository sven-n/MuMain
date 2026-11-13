#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <span>
#include <vector>

namespace Data::Monsters
{
// Checks the names of the monsters: the separator of the LocalizedString
// format and a language code that is not one are errors; an empty English name
// (the game shows no name where there is no translation) and a name longer
// than the game holds (it is cut) are warnings. Appends the problems to
// `issues`.
void ValidateMonsterNames(std::span<const MonsterDefinition> monsters, std::vector<Items::ItemDataIssue>& issues);

// A name in a language the game has no texts for is never shown: a warning for
// each. `uiLocales` are the game's languages (I18N::GetAvailableLocales).
void ValidateMonsterNameLocales(std::span<const MonsterDefinition> monsters, std::span<const char* const> uiLocales,
                                std::vector<Items::ItemDataIssue>& issues);

// Old behaviours the setup keeps (Q-MN36), a warning each: `live` false drops
// the character when it appears (the Wolf Status, P24), on the entry, a map
// variant or a state variant; a crywolfAltarContract step on a model that is
// no Crywolf altar only talks.
void ValidateMonsterSetups(std::span<const MonsterDefinition* const> monsters, const std::string& source,
                           std::vector<Items::ItemDataIssue>& issues);
// The talk steps as the editor leaves them, on the entry and its map and state
// variants ("monster 408 mapVariants[0].onTalk[1]..."): a dialog OK whose
// quest has no state never sends (a warning), and an altar contract step
// naming a set the shared talk texts do not have shows empty texts and stops
// Save (an error).
void ValidateMonsterTalkSteps(std::span<const MonsterDefinition* const> monsters, const MonsterTalkTexts& talkTexts,
                              const std::string& source, std::vector<Items::ItemDataIssue>& issues);
// A text of an NPC's talk without a text of its own in one of the game's
// languages (`uiLocales`) shows its English one there (Q-MN47): a warning per
// text naming the languages, on the entry, a map variant, a state variant or
// the shared talk texts, by the fields Monster Translations shows.
void ValidateMonsterTalkTexts(std::span<const MonsterDefinition* const> monsters, const MonsterTalkTexts& talkTexts,
                              std::span<const char* const> uiLocales, const std::string& source,
                              std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Monsters
