#include "stdafx.h"

#include "UI/Quests/QuestUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Quests/MyQuestInfoWindow.h"
#include "UI/Quests/QuestProgress.h"
#include "UI/Quests/QuestProgressByEtc.h"
#ifdef ASG_ADD_TIME_LIMIT_QUEST
#include "UI/Dialogs/CommonMessageBox.h"
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
    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CQuestCountLimitMsgBoxLayout));
}
#endif
}
