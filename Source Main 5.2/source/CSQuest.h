//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifndef __CSQUEST_H__
#define __CSQUEST_H__

#include <array>
#include <cstdint>

#include "Singleton.h"
#include "_enum.h"
#include "_struct.h"

class CSQuest : public Singleton<CSQuest>
{
private:
    using QuestStateList = std::array<std::uint8_t, MAX_QUESTS / 4>;
    using QuestAttributes = std::array<QUEST_ATTRIBUTE, MAX_QUESTS>;
    using QuestEventCounters = std::array<std::uint8_t, TYPE_QUEST_END>;
    using KillTracker = std::array<int, 5>;

private:
    std::uint8_t getQuestState(int questIndex = -1);
    std::uint8_t CheckQuestState(std::uint8_t state = 0xFF);
    short FindQuestContext(QUEST_ATTRIBUTE* pQuest, int index);
    bool CheckRequestCondition(QUEST_ATTRIBUTE* pQuest, bool bLastCheck = false);
    bool CheckActCondition(QUEST_ATTRIBUTE* pQuest);
    void RenderDevilSquare(void);
    void RenderBloodCastle(void);

public:
    CSQuest(void);
    ~CSQuest(void);

    //  Quest Init Functions
    bool OpenQuestScript(const wchar_t* filename);
    bool IsInit(void);
    void clearQuest(void);

    //  Quest Setting.
    void setQuestLists(const std::uint8_t* byList, int num, CLASS_TYPE Class = CLASS_UNDEFINED);
    void setQuestList(int index, int result);
    std::uint8_t getQuestState2(int questIndex);
    void ShowQuestPreviewWindow(int index = -1);
    void ShowQuestNpcWindow(int index = -1);

    std::uint8_t getCurrQuestState(void);
    wchar_t* GetNPCName(BYTE byQuestIndex);
    wchar_t* getQuestTitle();
    wchar_t* getQuestTitle(BYTE byQuestIndex);
    wchar_t* getQuestTitleWindow();
    void SetEventCount(std::uint8_t type, std::uint8_t count);
    int GetEventCount(std::uint8_t byType);
    std::uint32_t GetNeedZen() { return m_dwNeedZen; }
    QUEST_ATTRIBUTE* GetCurQuestAttribute()
    {
        return &m_Quest[m_byCurrQuestIndex];
    }

    std::uint8_t GetCurrQuestIndex() { return m_byCurrQuestIndex; }
    void SetKillMobInfo(const int* anKillMobInfo);
    bool ProcessNextProgress();
    void ShowDialogText(int iDialogIndex);
    bool BeQuestItem();
    int FindQuestItemsInInven(int nType, int nCount, int nLevel = -1);
    int GetKillMobCount(int nMobType);

private:
    std::uint8_t    m_byClass;

    QuestEventCounters m_byEventCount;
    QuestAttributes    m_Quest;

    QuestStateList m_byQuestList;
    std::uint8_t   m_byCurrQuestIndex;
    std::uint8_t   m_byCurrQuestIndexWnd;

    std::uint8_t   m_byStartQuestList;

    std::uint8_t   m_byViewQuest;
    short          m_shCurrPage;
    std::uint8_t   m_byCurrState;
    std::uint32_t  m_dwNeedZen;

    std::uint8_t   m_byQuestType;
    bool	       m_bOnce;

    KillTracker m_anKillMobType;
    KillTracker m_anKillMobCount;
    std::uint16_t m_wNPCIndex;

    int     m_iStartX;
    int     m_iStartY;
};

#define g_csQuest CSQuest::GetSingleton ()

#endif
