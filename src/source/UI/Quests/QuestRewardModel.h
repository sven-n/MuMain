#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

struct tagITEM;
typedef struct tagITEM ITEM;

// Shared reward-row builder for RmlUi-bound quest windows -- CMyQuestInfoWindow's own reward panel
// and CQuestProgress/CQuestProgressByEtc's own reward list all resolve the exact same
// GetRequestRewardText() requirements/general/random 3-section shape; extracted here so porting the
// latter two off CUIQuestContentsListBox doesn't triple this loop.
namespace UI::Quests::RewardModel
{
    // Raw native data a click handler needs (ITEM*/type) -- kept separate from the display-only
    // Entry pushed into RmlUi, same split CMyQuestInfoWindow's own ContentRowData already used.
    struct RowData
    {
        Rml::String text;
        DWORD dwColor = 0;
        DWORD dwType = 0;
        ITEM* pItem = nullptr;
    };

    // Display-only row for an RmlUi data-for list -- index points back into the RowData vector
    // BuildRows() returned, for a click handler to look up pItem/dwType from.
    struct Entry
    {
        Rml::String text;
        Rml::String color; // "rgba(r,g,b,a)"
        bool bold = false;
        int index = 0;
        bool clickable = false;
    };

    // Wraps GetRequestRewardText()'s requirements/general/random 3-section loop -- the exact logic
    // CQuestProgress::SetCurRequestReward()/CMyQuestInfoWindow::SetSelQuestRequestReward() each
    // duplicated natively before this was extracted. No leading spacer row -- a caller that needs
    // one (CMyQuestInfoWindow, which appends this after other content in the same list) pushes it
    // itself before appending these rows.
    std::vector<RowData> BuildRows(DWORD dwQuestIndex, bool& outRequestComplete);

    // DWORD ARGB -> "rgba(r,g,b,a)" string, plus the "is this a clickable reward/request item" rule
    // -- shared so no caller hand-rolls the conversion.
    Entry ToEntry(const RowData& row, int index);
}
