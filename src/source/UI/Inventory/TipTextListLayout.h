#pragma once

#include <string>
#include <vector>

// What RenderTipTextList() (ZzzInventory.cpp) draws, laid out without drawing, in reference px:
// its black frame and fill, and each line (RenderText() at x, y in a box of boxWidth with `sort`;
// a coloured text box behind it when bgColor is set). For the windows that draw such tables through
// RmlUi (CItemExplanationWindow, CSetItemExplanation, TipTextListView).
struct TipTextListRecord
{
    struct Box
    {
        float x, y, width, height;
        unsigned int argb;
    };
    struct Line
    {
        std::wstring text;
        float x, y, boxWidth, height;
        int sort;
        bool bold;
        unsigned int color;   // RGBA()
        unsigned int bgColor; // RGBA(), 0 = none
    };
    std::vector<Box> boxes;
    std::vector<Line> lines;
};

namespace UI::TipTextList
{
// RenderTipTextList(sx, sy, textNum, tab, sort, renderPoint, useBackground) over the TextList /
// TextListColor / TextBold globals, appended to `record` instead of drawn.
void Record(TipTextListRecord& record, int sx, int sy, int textNum, int tab, int sort, int renderPoint,
            bool useBackground);

// RenderHelpCategory() / RenderHelpLine() (the item help table's columns), recorded the same way.
void RecordHelpCategory(TipTextListRecord& record, int columnType, int x, int y);
void RecordHelpLine(TipTextListRecord& record, int columnType, const wchar_t* printStyle, int& tabSpace,
                    const wchar_t* gapText = nullptr, int y = 0, int itemType = 0);
} // namespace UI::TipTextList
