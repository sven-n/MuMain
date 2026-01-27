//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzTexture.h"
#include "ZzzCharacter.h"
#include "Scenes/SceneCore.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"

#include "CSQuest.h"
#include "UsefulDef.h"
#include "NewUIInventoryCtrl.h"
#include "CharacterManager.h"
#include "NewUISystem.h"

bool bCheckNPC = false;
extern  int  g_iMessageTextStart;
extern  char g_cMessageTextCurrNum;
extern  char g_cMessageTextNum;
extern  int g_iNumLineMessageBoxCustom;
extern  wchar_t g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];
extern  int g_iCurrentDialogScript;
extern  int g_iNumAnswer;
extern  wchar_t g_lpszDialogAnswer[MAX_ANSWER_FOR_DIALOG][NUM_LINE_DA][MAX_LENGTH_CMB];

namespace
{
    constexpr std::uint8_t QUEST_STATE_INVALID = 0xFF;
    constexpr std::uint8_t QUEST_STATE_MASK = 0x03;
    constexpr std::uint8_t QUEST_STATES_PER_ENTRY = 4;
    constexpr std::uint8_t QUEST_STATE_BIT_WIDTH = 2;
    constexpr int QUEST_MONSTER_SLOT_COUNT = 5;
    constexpr int QUEST_MONSTER_INFO_STRIDE = 2;

    struct QuestStateSlot
    {
        std::size_t byteIndex {};
        std::uint8_t bitShift {};
    };

    QuestStateSlot MakeQuestStateSlot(int questIndex)
    {
        QuestStateSlot slot {};
        if (questIndex < 0)
        {
            return slot;
        }

        slot.byteIndex = static_cast<std::size_t>(questIndex) / QUEST_STATES_PER_ENTRY;
        slot.bitShift = static_cast<std::uint8_t>((questIndex % QUEST_STATES_PER_ENTRY) * QUEST_STATE_BIT_WIDTH);
        return slot;
    }

    std::uint8_t ExtractQuestState(const std::uint8_t* questList, std::size_t storageSize, int questIndex)
    {
        if (questList == nullptr)
        {
            return QUEST_STATE_INVALID;
        }

        const auto slot = MakeQuestStateSlot(questIndex);
        if (slot.byteIndex >= storageSize)
        {
            return QUEST_STATE_INVALID;
        }

        return (questList[slot.byteIndex] >> slot.bitShift) & QUEST_STATE_MASK;
    }

    void StoreQuestState(std::uint8_t* questList, std::size_t storageSize, int questIndex, std::uint8_t state)
    {
        if (questList == nullptr)
        {
            return;
        }

        const auto slot = MakeQuestStateSlot(questIndex);
        if (slot.byteIndex >= storageSize)
        {
            return;
        }

        questList[slot.byteIndex] &= ~(QUEST_STATE_MASK << slot.bitShift);
        questList[slot.byteIndex] |= ((state & QUEST_STATE_MASK) << slot.bitShift);
    }

    std::size_t ComputeQuestListByteCount(int questCount)
    {
        if (questCount <= 0)
        {
            return 0;
        }

        return static_cast<std::size_t>(
            (questCount + (QUEST_STATES_PER_ENTRY - 1)) / QUEST_STATES_PER_ENTRY);
    }

    struct QuestAttributeFile
    {
        short   shQuestConditionNum;
        short   shQuestRequestNum;
        WORD	wNpcType;

        char strQuestName[32];

        QUEST_CLASS_ACT     QuestAct[MAX_QUEST_CONDITION];
        QUEST_CLASS_REQUEST QuestRequest[MAX_QUEST_REQUEST];
    };
}

static CSQuest g_csQuestSingleton;


CSQuest::CSQuest(void) : m_byClass(255), m_byCurrQuestIndex(0), m_byCurrQuestIndexWnd(0),
m_byStartQuestList(0), m_shCurrPage(0), m_byViewQuest(QUEST_VIEW_NONE), m_byQuestType(TYPE_QUEST), m_iStartX(640 - 190), m_iStartY(0)
{
}

CSQuest::~CSQuest(void)
{
}

bool CSQuest::IsInit(void)
{
    if (m_byClass == 255)
    {
        return true;
    }

    return false;
}

void CSQuest::clearQuest(void)
{
    m_byClass = 255;
    m_shCurrPage = 0;
    m_byViewQuest = QUEST_VIEW_NONE;
    m_byStartQuestList = 0;
    m_byCurrQuestIndex = 0;
    m_byCurrQuestIndexWnd = 0;
    m_byQuestList.fill(0);
    m_byEventCount.fill(0);
    std::fill(m_anKillMobType.begin(), m_anKillMobType.end(), -1);
    std::fill(m_anKillMobCount.begin(), m_anKillMobCount.end(), -1);
}

std::uint8_t CSQuest::getCurrQuestState(void)
{
    return CheckQuestState();
}

const wchar_t* CSQuest::GetNPCName(BYTE byQuestIndex)
{
    return getMonsterName(int(m_Quest[byQuestIndex].wNpcType));
}

wchar_t* CSQuest::getQuestTitle()
{
    return m_Quest[m_byCurrQuestIndex].strQuestName;
}

wchar_t* CSQuest::getQuestTitle(BYTE byQuestIndex)
{
    return m_Quest[byQuestIndex].strQuestName;
}

wchar_t* CSQuest::getQuestTitleWindow()
{
    return m_Quest[m_byCurrQuestIndexWnd].strQuestName;
}

void CSQuest::SetEventCount(std::uint8_t type, std::uint8_t count)
{
    if (type >= TYPE_QUEST_END)
    {
        return;
    }

    m_byEventCount[type] = count;
}

int CSQuest::GetEventCount(std::uint8_t byType)
{
    if (byType >= TYPE_QUEST_END)
    {
        return 0;
    }

    return m_byEventCount[byType];
}

bool CSQuest::OpenQuestScript(const wchar_t* filename)
{
    if (filename == nullptr || filename[0] == L'\0')
    {
        return false;
    }

    const std::filesystem::path filePath(filename);
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        wchar_t text[256];
        mu_swprintf_s(text, std::size(text), L"%ls - File not exist.", filename);
        return false;
    }

    m_Quest.fill({});

    const auto recordSize = sizeof(QuestAttributeFile);
    std::vector<std::uint8_t> buffer(recordSize);

    for (auto& quest : m_Quest)
    {
        if (!file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(recordSize)))
        {
            return false;
        }

        BuxConvert(buffer.data(), static_cast<int>(buffer.size()));
        auto* current = reinterpret_cast<QuestAttributeFile*>(buffer.data());

        quest.shQuestConditionNum = current->shQuestConditionNum;
        quest.shQuestRequestNum = current->shQuestRequestNum;
        quest.wNpcType = current->wNpcType;
        CMultiLanguage::ConvertFromUtf8(quest.strQuestName, current->strQuestName);

        std::memcpy(quest.QuestAct, current->QuestAct, sizeof quest.QuestAct);
        std::memcpy(quest.QuestRequest, current->QuestRequest, sizeof quest.QuestRequest);
    }

    return true;
}

void CSQuest::setQuestLists(const std::uint8_t* byList, int num, CLASS_TYPE Class)
{
    if (Class != -1)
    {
        m_bOnce = true;
        bCheckNPC = false;

        m_byClass = gCharacterManager.GetBaseClass(Class);
    }

    m_byQuestList.fill(0);
    if (byList != nullptr)
    {
        const auto bytesToCopy = std::min<std::size_t>(ComputeQuestListByteCount(num), m_byQuestList.size());
        std::memcpy(m_byQuestList.data(), byList, bytesToCopy);
    }

    int i;
    if (CLASS_KNIGHT == m_byClass)
    {
        for (i = 0; i < num; ++i)
        {
            if (getQuestState(i) != QUEST_END)
                break;
        }
    }
    else if (CLASS_DARK == m_byClass || CLASS_DARK_LORD == m_byClass || CLASS_RAGEFIGHTER == m_byClass)
    {
        for (i = 4; i < num; ++i)
        {
            if (getQuestState(i) != QUEST_END)
                break;
        }
    }
    else
    {
        for (i = 0; i < num; ++i)
        {
            if (QUEST_COMBO == i)
                continue;
            if (getQuestState(i) != QUEST_END)
                break;
        }
    }

    m_byCurrQuestIndex = i;
    m_byCurrQuestIndexWnd = i;

    Hero->byExtensionSkill = 0;
    if (getQuestState(QUEST_COMBO) == QUEST_END)
    {
        Hero->byExtensionSkill = 1;
    }
}

void CSQuest::setQuestList(int index, int result)
{
    if (index < 0 || index >= MAX_QUESTS)
    {
        return;
    }

    m_byCurrQuestIndex = static_cast<std::uint8_t>(index);
    m_byCurrQuestIndexWnd = std::max<std::uint8_t>(static_cast<std::uint8_t>(index), m_byCurrQuestIndexWnd);

    const auto questState = static_cast<std::uint8_t>(result & QUEST_STATE_MASK);
    StoreQuestState(m_byQuestList.data(), m_byQuestList.size(), index, questState);

    Hero->byExtensionSkill = 0;
    if (getQuestState(QUEST_COMBO) == QUEST_END)
    {
        Hero->byExtensionSkill = 1;
    }
}

short   CSQuest::FindQuestContext(QUEST_ATTRIBUTE* pQuest, int index)
{
    for (int i = 0; i < pQuest->shQuestConditionNum; ++i)
    {
        if (pQuest->QuestAct[i].byRequestClass[m_byClass] >= 1)
        {
            return pQuest->QuestAct[i].shQuestStartText[index];
        }
    }

    m_byCurrQuestIndex--;
    CheckQuestState();

    return m_shCurrPage;
}

bool    CSQuest::CheckRequestCondition(QUEST_ATTRIBUTE* pQuest, bool bLastCheck)
{
    int iRequestType;
    for (int i = 0; i < pQuest->shQuestConditionNum; ++i)
    {
        if (pQuest->QuestAct[i].byRequestClass[m_byClass] != 0)
        {
            iRequestType = pQuest->QuestAct[i].byRequestType;
            for (int j = 0; j < pQuest->shQuestRequestNum; ++j)
            {
                if (pQuest->QuestRequest[j].byType == iRequestType || pQuest->QuestRequest[j].byType == 255)
                {
                    if (pQuest->QuestRequest[j].wCompleteQuestIndex != 65535)
                    {
                        if (getQuestState2(pQuest->QuestRequest[j].wCompleteQuestIndex) != QUEST_END)
                        {
                            m_shCurrPage = pQuest->QuestRequest[j].shErrorText;
                            m_byCurrState = QUEST_ERROR;
                            return false;
                        }
                    }
                    if (pQuest->QuestRequest[j].wLevelMin > 0)
                    {
                        WORD level = CharacterAttribute->Level;

                        if (pQuest->QuestRequest[j].wLevelMin > level)
                        {
                            m_shCurrPage = pQuest->QuestRequest[j].shErrorText;
                            m_byCurrState = QUEST_ERROR;
                            return false;
                        }
                    }
                    if (pQuest->QuestRequest[j].wLevelMax > 0)
                    {
                        WORD level = CharacterAttribute->Level;

                        if (pQuest->QuestRequest[j].wLevelMax < level)
                        {
                            m_shCurrPage = pQuest->QuestRequest[j].shErrorText;
                            m_byCurrState = QUEST_ERROR;
                            return false;
                        }
                    }
                    if (pQuest->QuestRequest[j].dwZen > 0)
                    {
                        if (bLastCheck)
                        {
                            DWORD gold = CharacterMachine->Gold;

                            m_dwNeedZen = pQuest->QuestRequest[j].dwZen;
                            if (m_dwNeedZen > gold)
                            {
                                m_shCurrPage = pQuest->QuestRequest[j].shErrorText;
                                m_byCurrState = QUEST_ERROR;
                                return false;
                            }
                        }
                        else
                        {
                            m_dwNeedZen = pQuest->QuestRequest[j].dwZen;
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool CSQuest::CheckActCondition(QUEST_ATTRIBUTE* pQuest)
{
    for (int i = 0; i < pQuest->shQuestConditionNum; ++i)
    {
        if (pQuest->QuestAct[i].byRequestClass[m_byClass] >= 1)
        {
            switch (pQuest->QuestAct[i].byQuestType)
            {
            case QUEST_ITEM:
            {
                int itemType
                    = (pQuest->QuestAct[i].wItemType * MAX_ITEM_INDEX)
                    + pQuest->QuestAct[i].byItemSubType;
                int itemLevel = -1;
                int itemNum = pQuest->QuestAct[i].byItemNum;

                itemLevel = pQuest->QuestAct[i].byItemLevel;

                if (FindQuestItemsInInven(itemType, itemNum, itemLevel))
                {
                    m_shCurrPage = FindQuestContext(pQuest, 1);
                    return false;
                }
            }
            break;

            case QUEST_MONSTER:
            {
                bool bFind = false;

                for (int j = 0; j < QUEST_MONSTER_SLOT_COUNT; ++j)
                {
                    if (m_anKillMobType[j] == int(pQuest->QuestAct[i].wItemType)
                        && m_anKillMobCount[j] >= int(pQuest->QuestAct[i].byItemNum))
                    {
                        bFind = true;
                        break;
                    }
                }

                if (!bFind)
                {
                    m_shCurrPage = FindQuestContext(pQuest, 1);
                    return false;
                }
            }
            break;
            }
        }
    }
    return true;
}

std::uint8_t CSQuest::getQuestState(int questIndex)
{
    const int targetIndex = (questIndex < 0) ? m_byCurrQuestIndex : questIndex;
    const auto state = ExtractQuestState(m_byQuestList.data(), m_byQuestList.size(), targetIndex);
    const std::uint8_t normalizedState = (state == QUEST_STATE_INVALID) ? 0 : state;

    if (questIndex == -1)
    {
        m_byCurrState = normalizedState;
    }

    return normalizedState;
}

std::uint8_t CSQuest::getQuestState2(int questIndex)
{
    const auto state = ExtractQuestState(m_byQuestList.data(), m_byQuestList.size(), questIndex);
    return (state == QUEST_STATE_INVALID) ? 0 : state;
}

std::uint8_t CSQuest::CheckQuestState(std::uint8_t state)
{
    if (Hero->Class != -1)
    {
        if (m_bOnce)
        {
            m_bOnce = false;
            m_byClass = gCharacterManager.GetBaseClass(Hero->Class);
        }
    }

    QUEST_ATTRIBUTE* lpQuest = &m_Quest[m_byCurrQuestIndex];
    //    m_byCurrState = m_byQuestList[m_byCurrQuestIndex];

    if (state == QUEST_STATE_INVALID)
    {
        getQuestState();
    }
    else
    {
        m_byCurrState = state & QUEST_STATE_MASK;
    }
    switch (m_byCurrState)
    {
    case QUEST_NO:
    {
        if (CheckRequestCondition(lpQuest))
        {
            m_shCurrPage = FindQuestContext(lpQuest, 0);
        }
    }
    break;

    case QUEST_ING:
    {
        if (CheckActCondition(lpQuest))
        {
            m_shCurrPage = FindQuestContext(lpQuest, 2);
            m_byCurrState = QUEST_ITEM;
        }
    }
    break;

    case QUEST_END:
    {
        m_shCurrPage = FindQuestContext(lpQuest, 3);
    }
    break;
    }

    return m_byCurrState;
}

void CSQuest::ShowDialogText(int iDialogIndex)
{
    g_iCurrentDialogScript = iDialogIndex;

    wchar_t Text[300] {};
    CMultiLanguage::ConvertFromUtf8(Text, g_DialogScript[g_iCurrentDialogScript].m_lpszText);

    g_iNumLineMessageBoxCustom = SeparateTextIntoLines(Text, g_lpszMessageBoxCustom[0], NUM_LINE_CMB, MAX_LENGTH_CMB);

    wchar_t lpszAnswer[MAX_LENGTH_ANSWER + 8] {};
    g_iNumAnswer = 0;
    std::memset(g_lpszDialogAnswer, 0, sizeof g_lpszDialogAnswer);

    int iTextSize = 0;

    for (int i = 0; i < g_DialogScript[g_iCurrentDialogScript].m_iNumAnswer; ++i)
    {
        wchar_t answerText[64] {};
        CMultiLanguage::ConvertFromUtf8(answerText, g_DialogScript[g_iCurrentDialogScript].m_lpszAnswer[i]);
        mu_swprintf_s(lpszAnswer, std::size(lpszAnswer), L"%d) %ls", i + 1, answerText);
        int iNumLine = SeparateTextIntoLines(lpszAnswer, g_lpszDialogAnswer[i][0], NUM_LINE_DA, MAX_LENGTH_CMB);
        if (iNumLine < NUM_LINE_DA - 1)
        {
            g_lpszDialogAnswer[i][iNumLine][0] = '\0';
        }

        g_iNumAnswer++;
        iTextSize = i;
    }

    if (0 == g_DialogScript[g_iCurrentDialogScript].m_iNumAnswer)
    {
        mu_swprintf_s(lpszAnswer, std::size(lpszAnswer), L"%d) %ls", iTextSize + 1, GlobalText[609]);
        wcscpy_s(g_lpszDialogAnswer[0][0], MAX_LENGTH_CMB, lpszAnswer);
        g_iNumAnswer = 1;
    }
}

void CSQuest::ShowQuestPreviewWindow(int index)
{
    if (index != -1)
    {
        m_byCurrQuestIndex = static_cast<std::uint8_t>(index);
    }

    m_byViewQuest = QUEST_VIEW_PREVIEW;

    const std::uint8_t previousIndex = m_byCurrQuestIndex;
    m_byCurrQuestIndex = m_byCurrQuestIndexWnd;

    CheckQuestState();
    ShowDialogText(m_shCurrPage);

    m_byCurrQuestIndex = previousIndex;
}

void CSQuest::ShowQuestNpcWindow(int index)
{
    if (index != -1)
    {
        m_byCurrQuestIndex = static_cast<std::uint8_t>(index);
    }

    g_bEventChipDialogEnable = EVENT_NONE;

    CheckQuestState();
    ShowDialogText(m_shCurrPage);
}

bool CSQuest::BeQuestItem(void)
{
    bool bCompleteItem = false;

    if (m_byCurrState == QUEST_ING)
    {
        QUEST_ATTRIBUTE* pQuest = &m_Quest[m_byCurrQuestIndex];

        for (int i = 0; i < pQuest->shQuestConditionNum; ++i)
        {
            if (pQuest->QuestAct[i].byRequestClass[m_byClass] > 0)
            {
                switch (pQuest->QuestAct[i].byQuestType)
                {
                case QUEST_ITEM:
                {
                    bCompleteItem = true;

                    int itemType
                        = (pQuest->QuestAct[i].wItemType * MAX_ITEM_INDEX)
                        + pQuest->QuestAct[i].byItemSubType;
                    int itemLevel = -1;
                    int itemNum = pQuest->QuestAct[i].byItemNum;

                    itemLevel = pQuest->QuestAct[i].byItemLevel;
                    if (FindQuestItemsInInven(itemType, itemNum, itemLevel))
                    {
                        return false;
                    }
                }
                break;
                case QUEST_MONSTER:
                {
                    bCompleteItem = true;
                    bool bFind = false;

                    for (int j = 0; j < QUEST_MONSTER_SLOT_COUNT; ++j)
                    {
                        if (m_anKillMobType[j] == int(pQuest->QuestAct[i].wItemType)
                            && m_anKillMobCount[j] >= int(pQuest->QuestAct[i].byItemNum))
                        {
                            bFind = true;
                            break;
                        }
                    }

                    if (!bFind)
                        return false;
                }
                break;
                }
            }
        }
    }

    return bCompleteItem;
}

int CSQuest::FindQuestItemsInInven(int nType, int nCount, int nLevel)
{
    SEASON3B::CNewUIInventoryCtrl* pInvenCtrl = g_pMyInventory->GetInventoryCtrl();

    int nItemsInInven = pInvenCtrl->GetNumberOfItems();
    ITEM* pItem = nullptr;
    int nFindItemCount = 0;

    for (int i = 0; i < nItemsInInven; ++i)
    {
        pItem = pInvenCtrl->GetItem(i);
        if (nType == pItem->Type)
        {
            if (nLevel == -1 || nLevel == pItem->Level)
            {
                if (nCount <= ++nFindItemCount)
                    return 0;
            }
        }
    }

    return nCount - nFindItemCount;
}

int CSQuest::GetKillMobCount(int nMobType)
{
    for (int i = 0; i < QUEST_MONSTER_SLOT_COUNT; ++i)
    {
        if (nMobType == m_anKillMobType[i])
            return m_anKillMobCount[i];
    }

    return 0;
}

bool CSQuest::ProcessNextProgress()
{
    QUEST_ATTRIBUTE* lpQuest = &m_Quest[m_byCurrQuestIndex];
    if (!CheckRequestCondition(lpQuest, true))
    {
        ShowDialogText(m_shCurrPage);
        return true;
    }
    else
    {
        SocketClient->ToGameServer()->SendLegacyQuestStateSetRequest(m_byCurrQuestIndex, 1);
        return false;
    }
}

void CSQuest::SetKillMobInfo(const int* anKillMobInfo)
{
    if (anKillMobInfo == nullptr)
    {
        std::fill(m_anKillMobType.begin(), m_anKillMobType.end(), -1);
        std::fill(m_anKillMobCount.begin(), m_anKillMobCount.end(), -1);
        return;
    }

    for (int i = 0; i < QUEST_MONSTER_SLOT_COUNT; ++i)
    {
        const int typeIndex = i * QUEST_MONSTER_INFO_STRIDE;
        m_anKillMobType[i] = anKillMobInfo[typeIndex];
        m_anKillMobCount[i] = anKillMobInfo[typeIndex + 1];
    }
}

void CSQuest::RenderDevilSquare(void)
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(230, 230, 230, 255);
    g_pRenderText->SetBgColor(20, 20, 20, 255);
    g_pRenderText->RenderText(m_iStartX + 95 - 60, m_iStartY + 12, GlobalText[1145], 120, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(200, 220, 255, 255);
    //	RenderText ( m_iStartX+95-73, m_iStartY+22, m_Quest[m_byCurrQuestIndex].strQuestName, 150*WindowWidth/640, true );
}

void CSQuest::RenderBloodCastle(void)
{
    wchar_t Text[100] {};
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(230, 230, 230, 255);
    g_pRenderText->SetBgColor(20, 20, 20, 255);
    g_pRenderText->RenderText(m_iStartX + 95 - 60, m_iStartY + 12, GlobalText[1146], 120, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(223, 191, 103, 255);
    g_pRenderText->SetBgColor(0);
    mu_swprintf_s(Text, std::size(Text), GlobalText[869], BLOODCASTLE_QUEST_NUM, GlobalText[1146], GlobalText[1140]);
    g_pRenderText->RenderText(m_iStartX + 95, m_iStartY + 80, Text, 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->SetTextColor(255, 230, 210, 255);
    g_pRenderText->RenderText(m_iStartX + 85, m_iStartY + 100, GlobalText[877], 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->RenderText(m_iStartX + 105, m_iStartY + 120, GlobalText[878], 0, 0, RT3_WRITE_CENTER);

    g_pRenderText->SetFont(g_hFontBig);
    mu_swprintf_s(Text, std::size(Text), GlobalText[868], m_byEventCount[m_byQuestType]);
    g_pRenderText->RenderText(m_iStartX + 95, m_iStartY + 65 + 60 * 4, Text, 0, 0, RT3_WRITE_CENTER);
}