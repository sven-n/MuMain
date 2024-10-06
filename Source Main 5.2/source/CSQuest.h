//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifndef __CSQUEST_H__
#define __CSQUEST_H__

#include "Singleton.h"
#include "_enum.h"

class CSQuest : public Singleton<CSQuest>
{
private:
    BYTE getQuestState(int questIndex = -1);
    BYTE CheckQuestState(BYTE state = 255);
    short FindQuestContext(QUEST_ATTRIBUTE* pQuest, int index);
    bool CheckRequestCondition(QUEST_ATTRIBUTE* pQuest, bool bLastCheck = 0);
    bool CheckActCondition(QUEST_ATTRIBUTE* pQuest);
    void RenderDevilSquare(void);
    void RenderBloodCastle(void);

public:
    CSQuest(void);
    ~CSQuest(void);

    //  Quest Init Functions
    bool OpenQuestScript(wchar_t* filename);
    bool IsInit(void);
    void clearQuest(void);

    //  Quest Setting.
    void setQuestLists(BYTE* byList, int num, CLASS_TYPE Class = CLASS_UNDEFINED);
    void setQuestList(int index, int result);
    BYTE getQuestState2(int questIndex);
    void ShowQuestPreviewWindow(int index = -1);
    void ShowQuestNpcWindow(int index = -1);

    BYTE getCurrQuestState(void);
    wchar_t* GetNPCName(BYTE byQuestIndex);
    wchar_t* getQuestTitle();
    wchar_t* getQuestTitle(BYTE byQuestIndex);
    wchar_t* getQuestTitleWindow();
    void SetEventCount(BYTE type, BYTE count);
    int GetEventCount(BYTE byType);
    DWORD GetNeedZen() { return m_dwNeedZen; }
    QUEST_ATTRIBUTE* GetCurQuestAttribute()
    {
        return &m_Quest[m_byCurrQuestIndex];
    }

    BYTE GetCurrQuestIndex() { return m_byCurrQuestIndex; }
    void SetKillMobInfo(int* anKillMobInfo);
    bool ProcessNextProgress();
    void ShowDialogText(int iDialogIndex);
    bool BeQuestItem();
    int FindQuestItemsInInven(int nType, int nCount, int nLevel = -1);
    int GetKillMobCount(int nMobType);

private:
    BYTE    m_byClass;

    BYTE    m_byEventCount[TYPE_QUEST_END];
    QUEST_ATTRIBUTE m_Quest[MAX_QUESTS];

    BYTE    m_byQuestList[MAX_QUESTS / 4];
    BYTE    m_byCurrQuestIndex;
    BYTE    m_byCurrQuestIndexWnd;

    BYTE    m_byStartQuestList;

    BYTE    m_byViewQuest;
    short   m_shCurrPage;
    BYTE    m_byCurrState;
    DWORD   m_dwNeedZen;

    BYTE    m_byQuestType;
    bool	m_bOnce;

    int 	m_anKillMobType[5];
    int		m_anKillMobCount[5];
    WORD	m_wNPCIndex;

    int     m_iStartX;
    int     m_iStartY;
};

#define g_csQuest CSQuest::GetSingleton ()

#endif
