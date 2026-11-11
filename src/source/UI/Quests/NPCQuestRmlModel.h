#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

// RmlUi model for CNPCQuest (npc_quest.rml/.rcss). Sourced from the legacy g_csQuest/CSQuest quest
// system (DialogStructure's script data + g_lpszMessageBoxCustom/g_lpszDialogAnswer) -- a separate,
// older system from CQuestMng (the one CNPCDialogue/CQuestProgress use). No reuse of
// UI::Quests::RewardModel/QuestProgressRmlModel/NPCDialogueRmlModel here -- this window's row shapes
// don't warrant forcing a cross-window abstraction (see this port's own plan notes).
namespace mu::ui::window
{
    struct NPCQuestTextLine { Rml::String text; };

    // One quest-condition row (item-to-collect or monster-to-kill) -- color pre-computed the same
    // way RenderItemMobText() always colored it (amber = satisfied, red = not -- see that method's
    // own comment for why this mapping isn't reinterpreted here).
    struct NPCQuestConditionRow { Rml::String text; Rml::String color; };

    // index is the answer's absolute slot in DialogStructure::GetEntry()'s answers[] array --
    // RmlClickAnswer() bounds-checks it against that same entry's numAnswer.
    struct NPCQuestAnswerEntry { Rml::String text; int index = 0; };

    struct NPCQuestRmlModel
    {
        float textPx = 0.f; // native text size in physical px (RmlNativeTextSize.h)

        Rml::String npcName;
        Rml::String questTitle;
        bool showQuestTitle = false;

        bool showConditions = false; // QUEST_ING
        std::vector<NPCQuestConditionRow> conditions;
        bool completeEnabled = false;

        bool showCost = false; // QUEST_NO
        Rml::String costAmount;
        Rml::String costTier; // UI::RmlBridge::GoldTierKey() of the cost

        std::vector<NPCQuestTextLine> messageLines; // g_lpszMessageBoxCustom
        std::vector<NPCQuestAnswerEntry> answers;    // g_lpszDialogAnswer (NUM_LINE_DA == 1 in this
                                                      // build, so each answer is already one line)

        // Set once at BuildRmlUi() time (I18N::Game::ProceedWithQuest/Cost/Close388).
        Rml::String completeLabel;
        Rml::String costLabel;
        Rml::String exitTooltip;
    };
}
