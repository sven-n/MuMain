#pragma once

#include <functional>
#include <string>
#include <vector>

class CHARACTER;

namespace Data::Monsters
{
struct MonsterSetup;
struct MonsterAltarContract;
} // namespace Data::Monsters

namespace mu::ui::window
{
struct GenericDialogConfig;
}

// What the client does when the hero talks to an NPC (MOVEMENT_TALK), before
// the server answers: the NPC's talk fields in the monster data (MN5, D66),
// and the map rules that stay code. The game's side effects go through a
// TalkWorld, so the monster tests can record them (D50).
namespace GameLogic::Npcs
{
// A Crywolf altar contract's texts in the shown language; canContract has
// "%d" for the contracts left.
struct CrywolfAltarTexts
{
    std::wstring canContract;
    std::wstring proceed;
    std::wstring onMount;
    std::wstring needGuardian;
    std::wstring destroyed;
    std::wstring ongoing;
};

struct TalkWorld
{
    // What the rules read.
    // The NPC's setup now: its entry's on the map and in the states that are on.
    std::function<Data::Monsters::MonsterSetup(const CHARACTER& npc)> talkSetup;
    // The language its texts are shown in.
    std::function<std::string()> locale;
    // The shared talk texts of that name (the list's talkTexts); nullptr for
    // none.
    std::function<const Data::Monsters::MonsterAltarContract*(const std::string& name)> findTalkTexts;
    std::function<int()> heroLevel;
    // The Crywolf 1st map while only elves may make the altar contracts.
    std::function<bool()> isCrywolfElfTime;
    std::function<bool()> isCrywolfMap;
    std::function<bool()> isKanturu2nd;
    std::function<bool()> isCursedTemple;
    // A quest's state (QUEST_ING, QUEST_END, ...) and the hero's zen.
    std::function<int(int quest)> questState;
    std::function<unsigned long()> gold;
    std::function<bool()> hasDarkHorse;
    std::function<bool()> hasDarkRaven;

    // What they do.
    std::function<void(int key)> sendTalk;
    std::function<void()> sendEnterRefuge;
    std::function<void()> sendEnterBarracks;
    std::function<void()> sendDarkHorseInfo;
    std::function<void()> sendDarkRavenInfo;
    std::function<void(mu::ui::window::GenericDialogConfig config)> showDialog;
    std::function<void(const std::wstring& text)> systemMessage;
    std::function<void(const std::wstring& text)> errorMessage;
    std::function<void(bool repair)> setRepairShop;
    // The classes whose quest window shows the NPC's own name and no quest
    // title (Marlon's, Items::GetClassKeys indexes); empty for the others.
    std::function<void(const std::vector<int>& classes)> setNoQuestTitleFor;
    // Every talk: the quest window closes and the legacy quest state is asked.
    std::function<void()> beforeTalk;
    // The map code that stays: the Crywolf altar contract, Kanturu 2nd's
    // entrance, the Illusion Temple's NPCs.
    std::function<void(CHARACTER& npc, const CrywolfAltarTexts& texts)> crywolfAltarTalk;
    std::function<void(CHARACTER& npc)> kanturu2ndTalk;
    std::function<void(CHARACTER& npc)> cursedTempleTalk;
};

// A talk text with each "%d" as the number (a text, not a format: a typed
// "%" cannot break it).
std::wstring WithNumber(std::wstring text, int number);

// The hero talks to the NPC. False when a rule stops before the talk (its
// minimum level): the hero keeps the NPC as its target.
bool Talk(CHARACTER& npc, const TalkWorld& world);
} // namespace GameLogic::Npcs
