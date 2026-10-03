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
void AppendRows(std::vector<UI::Quests::RewardModel::RowData>& rows,
                const SRequestRewardText* text, int& nextRow, int count)
{
    const int endRow = nextRow + count;
    for (; nextRow < endRow; ++nextRow)
    {
        const auto& row = text[nextRow];
        rows.push_back({StringUtils::WideToNarrow(row.m_szText), ToRowStyle(row.m_eKind),
                        row.m_dwType, row.m_pItem});
    }
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

        constexpr int RequestRewardTextCapacity = 13;
        SRequestRewardText text[RequestRewardTextCapacity];
        outRequestComplete = g_QuestMng.GetRequestRewardText(text, RequestRewardTextCapacity, dwQuestIndex);

        // The producer always emits a requirements heading, even with no requirements.
        int nextRow = 0;
        AppendRows(rows, text, nextRow, 1 + pQuestRequestReward->m_byRequestCount);
        for (const int rewardCount : {pQuestRequestReward->m_byGeneralRewardCount,
                                      pQuestRequestReward->m_byRandRewardCount})
        {
            if (rewardCount == 0)
                continue;
            rows.push_back({BlankRowText, RowStyle::Plain, 0, nullptr});
            AppendRows(rows, text, nextRow, 1 + rewardCount);
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
        // GetRequestRewardText() draws its headings in g_hFontBold.
        return Entry{row.text, StyleKey(row.style), row.style == RowStyle::Heading, index, clickable};
    }
}
