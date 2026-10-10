#include "stdafx.h"

#include "UI/NPCs/NpcDialogueUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "UI/NPCs/NPCDialogue.h"

namespace UI::Npc
{
bool IsDialogueOpen()
{
    return UI::Windows::IsVisible(mu::ui::window::INTERFACE_NPC_DIALOGUE);
}

void OpenDialogue(std::uint32_t contributionPoints)
{
    g_pNPCDialogue->SetContributePoint(contributionPoints);
    UI::Windows::Show(mu::ui::window::INTERFACE_NPC_DIALOGUE);
}

void ShowQuestList(std::span<const std::uint32_t> questIndices)
{
    if (IsDialogueOpen())
        g_pNPCDialogue->ProcessQuestListReceive(questIndices);
}

void GensJoinAnswered(GensJoinResult result, std::uint8_t influence)
{
    if (IsDialogueOpen())
        g_pNPCDialogue->ProcessGensJoiningReceive(result, influence);
}

void GensLeaveAnswered(GensLeaveResult result)
{
    if (IsDialogueOpen())
        g_pNPCDialogue->ProcessGensSecessionReceive(result);
}

void GensRewardAnswered(GensRewardResult result)
{
    if (IsDialogueOpen())
        g_pNPCDialogue->ProcessGensRewardReceive(result);
}
}
