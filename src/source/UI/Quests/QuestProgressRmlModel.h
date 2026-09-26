#pragma once

#include "UI/Quests/QuestRewardModel.h"

#include <RmlUi/Core/Types.h>

#include <vector>

// Shared model-struct shape for CQuestProgress/CQuestProgressByEtc's own independent RmlUi
// documents (quest_progress.rml / quest_progress_etc.rml) -- two separate RmlModelBinder<T>
// instances, one struct type. See those two classes' own headers for why they stay separate
// classes/documents rather than being consolidated into one.
namespace mu::ui::window
{
    struct QuestProgressTextLine
    {
        Rml::String text;
    };

    // One numbered answer choice. `index` is 0-based (the click handler adds 1 to build the real
    // QuestProceedAction) -- text is the raw, un-wrapped "N. <answer>" string; unlike npcLines
    // below, there's no fixed per-page line budget here, so RmlUi's own natural word-wrap is used
    // instead of replicating the legacy DivideStringByPixel() line-break points (the old manual
    // wrapping there existed only to support the old manual Y-position hit-testing math, which a
    // real data-event-click target makes unnecessary -- see QuestProgress.cpp's own comment).
    struct QuestProgressAnswerEntry
    {
        Rml::String text;
        int index = 0;
    };

    struct QuestProgressRmlModel
    {
        // Movable, non-HUD window -- sourced from UI::Scaling::GetActiveTransform(), same
        // convention as CMyQuestInfoWindow's own root_x/root_y/root_scale.
        float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)

        Rml::String subject;

        // CQuestProgress's own .rml includes an NPC-name/player-name preamble neither element
        // exists in quest_progress_etc.rml at all (a structural, not data-driven, difference --
        // see QuestProgress.cpp's own SyncRmlModel()) -- CQuestProgressByEtc's own model instance
        // simply never populates these two fields meaningfully.
        Rml::String npcName;
        Rml::String playerName;

        std::vector<QuestProgressTextLine> npcLines; // current page's up-to-7 pre-wrapped lines
        bool prevEnabled = false;
        bool nextEnabled = false;

        // 0 = NPC dialogue only, 1 = player words + numbered answers, 2 = reward list.
        int activeView = 0;

        Rml::String playerWordsText; // CQuestProgress only; raw text, RmlUi wraps it naturally
        std::vector<QuestProgressAnswerEntry> answers;

        std::vector<UI::Quests::RewardModel::Entry> rewardRows;
        bool requestComplete = false;

        // Set once at BuildRmlUi() time (I18N::Game::OK/Close388) -- legacy CButton had no data
        // binding, so these were previously native ChangeText()/ChangeToolTipText() calls; same
        // "set once after modelCreated" pattern CMyQuestInfoWindow's own tooltip strings use.
        Rml::String okLabel;
        Rml::String exitTooltip;
    };
}
