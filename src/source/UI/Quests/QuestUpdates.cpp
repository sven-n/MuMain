#include "stdafx.h"

#include "UI/Quests/QuestUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Quests/MyQuestInfoWindow.h"
#include "UI/Quests/QuestProgress.h"
#include "UI/Quests/QuestProgressByEtc.h"
#ifdef ASG_ADD_TIME_LIMIT_QUEST
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "I18N/All.h"
#endif

namespace UI::Quest
{
void DisableCompleteButton()
{
    if (UI::Windows::IsVisible(mu::ui::window::INTERFACE_QUEST_PROGRESS))
        g_pQuestProgress->EnableCompleteBtn(false);
    else if (UI::Windows::IsVisible(mu::ui::window::INTERFACE_QUEST_PROGRESS_ETC))
        g_pQuestProgressByEtc->EnableCompleteBtn(false);
}

void RefreshSelectedQuestReward()
{
    g_pMyQuestInfoWindow->SetSelQuestRequestReward();
}

#ifdef ASG_ADD_TIME_LIMIT_QUEST
void ShowQuestCountLimit()
{
    mu::ui::window::GenericDialogConfig cfg;
    cfg.lines = {{I18N::Game::YouCannotAcceptAnyMoreQuest, false},
                 {I18N::Game::YouCanProceedMaximum10Quests, false},
                 {I18N::Game::AtTheSameTime, false},
                 {I18N::Game::YouNeedToClearAtLeast1QuestTo, false},
                 {I18N::Game::AcceptThisOne, false}};
    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
}
#endif
}
