#include "stdafx.h"
#include "UI/Quests/QuestRewardModel.h"

#include "GameLogic/Quests/QuestMng.h"
#include "Network/Server/WSclient.h" // QUEST_REQUEST_ITEM / QUEST_REWARD_ITEM
#include "Core/Utilities/StringUtils.h"

namespace UI::Quests::RewardModel
{
    std::vector<RowData> BuildRows(DWORD dwQuestIndex, bool& outRequestComplete)
    {
        std::vector<RowData> rows;
        outRequestComplete = false;

        if (0 == dwQuestIndex)
            return rows;

        const SQuestRequestReward* pQuestRequestReward = g_QuestMng.GetRequestReward(dwQuestIndex);
        if (nullptr == pQuestRequestReward)
            return rows;

        SRequestRewardText aRequestRewardText[13];
        outRequestComplete = g_QuestMng.GetRequestRewardText(aRequestRewardText, 13, dwQuestIndex);

        int i = 0;
        for (int j = 0; j < 3; ++j)
        {
            int nLoop;
            if (0 == j)
            {
                nLoop = 1 + pQuestRequestReward->m_byRequestCount;
            }
            else if (1 == j && pQuestRequestReward->m_byGeneralRewardCount)
            {
                rows.push_back({ " ", 0xffffffff, 0, nullptr });
                nLoop = 1 + pQuestRequestReward->m_byGeneralRewardCount + i;
            }
            else if (2 == j && pQuestRequestReward->m_byRandRewardCount)
            {
                rows.push_back({ " ", 0xffffffff, 0, nullptr });
                nLoop = 1 + pQuestRequestReward->m_byRandRewardCount + i;
            }
            else
            {
                nLoop = 0;
            }

            for (; i < nLoop; ++i)
            {
                rows.push_back({ StringUtils::WideToNarrow(aRequestRewardText[i].m_szText),
                    static_cast<DWORD>(aRequestRewardText[i].m_dwColor), aRequestRewardText[i].m_dwType,
                    aRequestRewardText[i].m_pItem });
            }
        }

        return rows;
    }

    Entry ToEntry(const RowData& row, int index)
    {
        wchar_t colorBuf[32];
        mu_swprintf(colorBuf, L"rgba(%d,%d,%d,%d)", (row.dwColor >> 16) & 0xff, (row.dwColor >> 8) & 0xff,
            row.dwColor & 0xff, (row.dwColor >> 24) & 0xff);
        const bool clickable = row.pItem && (row.dwType == QUEST_REQUEST_ITEM || row.dwType == QUEST_REWARD_ITEM);
        return Entry{ row.text, StringUtils::WideToNarrow(colorBuf), false, index, clickable };
    }
}
