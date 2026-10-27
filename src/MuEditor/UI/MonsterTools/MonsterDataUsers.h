#pragma once

#ifdef _EDITOR

#include "Core/Globals/_struct.h"
#include "GameLogic/Quests/QuestMng.h"

#include <span>
#include <string>
#include <vector>

namespace MuEditor::Monsters
{
// The game data that uses a monster number and stays at it when the monster
// is removed: the quests of the quest window (Quest_<language>.bmd) and the
// NPC dialogues of the quest system (NPCDialogue.bmd).
struct MonsterDataUsers
{
    // The names of the quests whose NPC the monster is.
    std::vector<std::string> npcOfQuests;
    // The names of the quests that ask to kill it.
    std::vector<std::string> killedInQuests;
    // How many dialogue states the quest system has for it as an NPC.
    int dialogueStates = 0;

    bool IsEmpty() const
    {
        return npcOfQuests.empty() && killedInQuests.empty() && dialogueStates == 0;
    }
};

// The users in these quests and dialogues (keyed by NPC number * 0x10000 plus
// the dialogue state).
MonsterDataUsers FindMonsterDataUsers(int number, std::span<const QUEST_ATTRIBUTE> quests,
                                      const NPCDialogueMap& dialogues);
// The users in the quest data the game has read.
MonsterDataUsers FindMonsterDataUsers(int number);
} // namespace MuEditor::Monsters

#endif // _EDITOR
