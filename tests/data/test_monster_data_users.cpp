#include "stdafx.h"

#include "doctest.h"

// The quest data that uses a monster number, which the Monster Browser lists
// before a monster is removed (MN1).
#ifdef _EDITOR
#include "UI/MonsterTools/MonsterDataUsers.h"

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <vector>

using MuEditor::Monsters::FindMonsterDataUsers;
using MuEditor::Monsters::MonsterDataUsers;

namespace
{
constexpr int Marlon = 229;
constexpr int BudgeDragon = 2;
constexpr int Lich = 9;
constexpr DWORD DialogueKey = 0x10000;

QUEST_ATTRIBUTE Quest(std::wstring_view name, WORD npc)
{
    QUEST_ATTRIBUTE quest{};
    std::copy(name.begin(), name.end(), quest.strQuestName);
    quest.wNpcType = npc;
    return quest;
}

void AddAct(QUEST_ATTRIBUTE& quest, BYTE type, WORD target)
{
    QUEST_CLASS_ACT& act = quest.QuestAct[quest.shQuestConditionNum++];
    act.byQuestType = type;
    act.wItemType = target;
}

std::vector<std::string> Names(std::initializer_list<const char*> names)
{
    return {names.begin(), names.end()};
}
} // namespace

TEST_CASE("The quest data uses a monster as a quest's NPC, as a monster to kill and in NPC dialogues "
          "[editor][monsters]")
{
    std::array<QUEST_ATTRIBUTE, 4> quests{};
    quests[0] = Quest(L"Secret of Dark Stone", Marlon);
    AddAct(quests[0], QUEST_MONSTER, BudgeDragon);
    // An item of group 2 is no monster.
    AddAct(quests[0], QUEST_ITEM, BudgeDragon);
    quests[1] = Quest(L"Certificate of Strength", Marlon);
    // quests[2] is a slot the script left empty: its NPC 0 is no monster 0.
    quests[3] = Quest(L"Infiltration", 237);
    // An act past the quest's act count is not read.
    quests[3].QuestAct[5].byQuestType = QUEST_MONSTER;
    quests[3].QuestAct[5].wItemType = Lich;

    NPCDialogueMap dialogues;
    dialogues[Marlon * DialogueKey + 1] = {};
    dialogues[Marlon * DialogueKey + 2] = {};
    dialogues[(Marlon + 1) * DialogueKey] = {};

    const MonsterDataUsers marlon = FindMonsterDataUsers(Marlon, quests, dialogues);
    CHECK(marlon.npcOfQuests == Names({"Secret of Dark Stone", "Certificate of Strength"}));
    CHECK(marlon.killedInQuests.empty());
    CHECK(marlon.dialogueStates == 2);
    CHECK_FALSE(marlon.IsEmpty());

    const MonsterDataUsers dragon = FindMonsterDataUsers(BudgeDragon, quests, dialogues);
    CHECK(dragon.npcOfQuests.empty());
    CHECK(dragon.killedInQuests == Names({"Secret of Dark Stone"}));
    CHECK(dragon.dialogueStates == 0);

    CHECK(FindMonsterDataUsers(0, quests, dialogues).IsEmpty());
    CHECK(FindMonsterDataUsers(Lich, quests, dialogues).IsEmpty());
    CHECK(FindMonsterDataUsers(Marlon + 1, quests, dialogues).dialogueStates == 1);
}

#endif // _EDITOR
