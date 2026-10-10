#include "stdafx.h"

#include "GameLogic/Items/CSItemOption.h"

#include "Engine/Object/ZzzInventory.h"
#include "I18N/All.h"

#include <algorithm>
#include <iterator>

// The original RenderOptionHelper()'s table, line for line, without its RenderTipTextList().
int CSItemOption::BuildOptionHelperTextList()
{
    if (m_byRenderOptionList == 0)
        return 0;

    int TextNum = 0;
    std::fill(std::begin(TextListColor), std::end(TextListColor), 0);
    for (int i = 0; i < 30; i++)
    {
        TextList[i][0] = L'\0';
    }

    const ITEM_SET_OPTION& setOption = m_ItemSetOption[m_byRenderOptionList - 1];
    if (setOption.byOptionCount >= 255)
    {
        m_byRenderOptionList = 0;
        return 0;
    }

    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;
    mu_swprintf(TextList[TextNum], L"%ls %ls %ls", setOption.strSetName, I18N::Game::Set, I18N::Game::ItemOptionInfo);
    TextListColor[TextNum] = TEXT_COLOR_YELLOW;
    TextNum++;

    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;
    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;

    for (int o = 0; o < MAX_ITEM_SET_STANDARD_OPTION_COUNT; ++o)
    {
        for (int n = 0; n < MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT; ++n)
        {
            if (getExplainText(TextList[TextNum], setOption.byStandardOption[o][n],
                               setOption.byStandardOptionValue[o][n]))
            {
                TextListColor[TextNum] = TEXT_COLOR_BLUE;
                TextBold[TextNum] = false;
                TextNum++;
            }
        }
    }

    for (int o = 0; o < MAX_ITEM_SET_EXT_OPTION_COUNT; ++o)
    {
        if (getExplainText(TextList[TextNum], setOption.byExtOption[o], setOption.byExtOption[o]))
        {
            TextListColor[TextNum] = TEXT_COLOR_GREEN;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    for (int o = 0; o < MAX_ITEM_SET_FULL_OPTION_COUNT; ++o)
    {
        if (getExplainText(TextList[TextNum], setOption.byFullOption[o], setOption.byFullOption[o]))
        {
            TextListColor[TextNum] = TEXT_COLOR_YELLOW;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;
    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;
    return TextNum;
}
