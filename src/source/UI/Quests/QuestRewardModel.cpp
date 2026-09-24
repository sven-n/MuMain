#include "stdafx.h"
#include "UI/Quests/QuestRewardModel.h"

#include "GameLogic/Quests/QuestMng.h"
#include "Network/Server/WSclient.h" // QUEST_REQUEST_ITEM / QUEST_REWARD_ITEM
#include "Core/Utilities/StringUtils.h"

namespace
{
UI::Quests::RewardModel::RowStyle ToRowStyle(REQUEST_REWARD_TEXT_KIND kind)
{
    using UI::Quests::RewardModel::RowStyle;
    switch (kind)
    {
    case RRTK_HEADING:
        return RowStyle::Heading;
    case RRTK_REQUIREMENT_UNMET:
        return RowStyle::RequirementUnmet;
    case RRTK_REWARD:
        return RowStyle::Reward;
    case RRTK_RANDOM_REWARD:
        return RowStyle::RandomReward;
    case RRTK_REQUIREMENT:
        break;
    }
    return RowStyle::Requirement;
}
} // namespace

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
                rows.push_back({" ", RowStyle::Plain, 0, nullptr});
                nLoop = 1 + pQuestRequestReward->m_byGeneralRewardCount + i;
            }
            else if (2 == j && pQuestRequestReward->m_byRandRewardCount)
            {
                rows.push_back({" ", RowStyle::Plain, 0, nullptr});
                nLoop = 1 + pQuestRequestReward->m_byRandRewardCount + i;
            }
            else
            {
                nLoop = 0;
            }

            for (; i < nLoop; ++i)
            {
                rows.push_back({StringUtils::WideToNarrow(aRequestRewardText[i].m_szText),
                                ToRowStyle(aRequestRewardText[i].m_eKind), aRequestRewardText[i].m_dwType,
                                aRequestRewardText[i].m_pItem});
            }
        }

        return rows;
    }

    const char* StyleKey(RowStyle style)
    {
        switch (style)
        {
        case RowStyle::Subject:
            return "subject";
        case RowStyle::Summary:
            return "summary";
        case RowStyle::Heading:
            return "heading";
        case RowStyle::Requirement:
            return "requirement";
        case RowStyle::RequirementUnmet:
            return "requirement-unmet";
        case RowStyle::Reward:
            return "reward";
        case RowStyle::RandomReward:
            return "random-reward";
        case RowStyle::Plain:
            break;
        }
        return "plain";
    }

    Entry ToEntry(const RowData& row, int index)
    {
        const bool clickable = row.pItem && (row.dwType == QUEST_REQUEST_ITEM || row.dwType == QUEST_REWARD_ITEM);
        return Entry{row.text, StyleKey(row.style), false, index, clickable};
    }
}
