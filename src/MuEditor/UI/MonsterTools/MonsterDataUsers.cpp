#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterDataUsers.h"

#include "Core/Text/Utf8.h"
#include "GameLogic/Quests/CSQuest.h"

#include <algorithm>
#include <cwchar>
#include <iterator>

namespace MuEditor::Monsters
{
namespace
{
// An NPC dialogue's key is the NPC's monster number times this, plus the
// dialogue state (CQuestMng::GetNPCDlgNPCWords).
constexpr DWORD DialogueKeyStride = 0x10000;

// The quest's name; empty for a slot the quest script left empty.
std::string NameOf(const QUEST_ATTRIBUTE& quest)
{
    const size_t length = wcsnlen(quest.strQuestName, std::size(quest.strQuestName));
    return Core::Text::ToUtf8(std::wstring(quest.strQuestName, length).c_str());
}

bool AsksToKill(const QUEST_ATTRIBUTE& quest, int number)
{
    const int acts = std::clamp<int>(quest.shQuestConditionNum, 0, MAX_QUEST_CONDITION);
    return std::any_of(quest.QuestAct, quest.QuestAct + acts, [number](const QUEST_CLASS_ACT& act)
                       { return act.byQuestType == QUEST_MONSTER && act.wItemType == number; });
}
} // namespace

MonsterDataUsers FindMonsterDataUsers(int number, std::span<const QUEST_ATTRIBUTE> quests,
                                      const NPCDialogueMap& dialogues)
{
    MonsterDataUsers users;
    for (const QUEST_ATTRIBUTE& quest : quests)
    {
        const std::string name = NameOf(quest);
        if (name.empty())
        {
            continue;
        }
        if (quest.wNpcType == number)
        {
            users.npcOfQuests.push_back(name);
        }
        if (AsksToKill(quest, number))
        {
            users.killedInQuests.push_back(name);
        }
    }
    users.dialogueStates =
        static_cast<int>(std::count_if(dialogues.begin(), dialogues.end(), [number](const auto& dialogue)
                                       { return static_cast<int>(dialogue.first / DialogueKeyStride) == number; }));
    return users;
}

MonsterDataUsers FindMonsterDataUsers(int number)
{
    return FindMonsterDataUsers(number, g_csQuest.GetQuests(), g_QuestMng.GetNPCDialogues());
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
