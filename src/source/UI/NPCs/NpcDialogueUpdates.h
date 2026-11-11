#pragma once

#include <cstdint>
#include <span>

// Answers the server sends while the NPC dialogue is open. Each is dropped when the dialogue has
// closed in the meantime. Spans are borrowed for the call.
namespace UI::Npc
{
enum class GensJoinResult : std::uint8_t
{
    Joined,
    AlreadyMember,
    LeftTooRecently,
    LevelTooLow,
    GuildMasterInOtherGens,
    GuildMasterNotMember,
    InParty,
    AllianceMaster,
};

enum class GensLeaveResult : std::uint8_t
{
    Left,
    NotMember,
    GuildMasterCannotLeave,
    WrongNpc,
};

enum class GensRewardResult : std::uint8_t
{
    Granted,
    OutsidePeriod,
    NotEligible,
    InventoryFull,
    AlreadyClaimed,
    WrongNpc,
    NotMember,
};

bool IsDialogueOpen();
// Opens the dialogue for the NPC the quest manager has already selected.
void OpenDialogue(std::uint32_t contributionPoints);

void ShowQuestList(std::span<const std::uint32_t> questIndices);
void GensJoinAnswered(GensJoinResult result, std::uint8_t influence);
void GensLeaveAnswered(GensLeaveResult result);
void GensRewardAnswered(GensRewardResult result);
}
