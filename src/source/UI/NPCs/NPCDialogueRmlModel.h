#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

// RmlUi model for CNPCDialogue (npc_dialogue.rml/.rcss). Two independently paginated lists share
// this one document: NPC words (top) and a sel-text list (bottom) whose rows come from either
// GetNPCDlgAnswer() (normal dialogue) or SetQuestListText()'s quest subjects (quest-list mode) --
// both sources reduce to the same {text, index} row shape, so the model doesn't need to know which
// one is active. No reuse of UI::Quests::RewardModel::Entry here -- this window has no reward list
// and its row shape is already simpler (no color/bold/clickable-item), so sharing that struct would
// just drag in unused fields.
namespace mu::ui::window
{
    struct NPCDialogueTextLine { Rml::String text; };

    // index is the absolute 1-based-minus-one (i.e. plain 0-based) logical entry index across all
    // sel-text pages -- ProcessSelTextResult()'s own m_nSelSelText (1-based) is index+1, kept that
    // way so RmlClickSelectAnswer() can hand off to the untouched native result logic unchanged.
    struct NPCDialogueAnswerEntry { Rml::String text; int index = 0; };

    struct NPCDialogueRmlModel
    {
        float rootX = 0.f;
        float rootY = 0.f;
        float rootScale = 1.f;
        float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)

        Rml::String npcName;

        // Current page's up-to-7 pre-wrapped lines (DivideStringByPixel() output, unchanged) --
        // bound as literal non-wrapping lines, same convention QuestProgressRmlModel's npcLines uses.
        std::vector<NPCDialogueTextLine> npcLines;
        bool npcPrevEnabled = false;
        bool npcNextEnabled = false;

        // Current page's sel-text rows; each row's text may itself be multiple pre-wrapped physical
        // lines joined with '\n' (rendered via white-space:pre-line in CSS) since the click target is
        // the whole logical entry, not each physical line.
        std::vector<NPCDialogueAnswerEntry> answers;
        bool ansPrevEnabled = false;
        bool ansNextEnabled = false;
        bool showAnswers = false; // SEL_TEXTS_MODE

        // Gens contribute-point banner (NPC 543/544 only) -- see RenderContributePoint()'s own gate.
        bool showContribute = false;
        Rml::String contributeText;

        // Set once at BuildRmlUi() time (I18N::Game::Close388) -- same pattern QuestProgressRmlModel
        // uses for its own exitTooltip.
        Rml::String exitTooltip;
    };
}
