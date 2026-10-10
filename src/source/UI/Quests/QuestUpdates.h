#pragma once

// Quest window changes the server reports.
namespace UI::Quest
{
// Disables Complete on whichever quest progress window is open.
void DisableCompleteButton();
// The selected quest in the quest log received its requirement and reward details.
void RefreshSelectedQuestReward();
#ifdef ASG_ADD_TIME_LIMIT_QUEST
void ShowQuestCountLimit();
#endif
}
