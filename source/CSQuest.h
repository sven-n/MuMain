//////////////////////////////////////////////////////////////////////////
//  - 퀘스트 관련 -
//  
//  
//////////////////////////////////////////////////////////////////////////
#ifndef __CSQUEST_H__
#define __CSQUEST_H__

/*+++++++++++++++++++++++++++++++++++++
    INCLUDE.
+++++++++++++++++++++++++++++++++++++*/
#include "Singleton.h"

class CSQuest : public Singleton<CSQuest>
{
private:
	// 현 퀘스트의 상태.
    BYTE getQuestState(int questIndex=-1);
	// 퀘스트의 상태를 검사한다.
    BYTE CheckQuestState(BYTE state=255);
	// 해당 퀘스트의 대화를 찾는다.
    short FindQuestContext(QUEST_ATTRIBUTE* pQuest, int index);    
	// 요구 조건을 검사한다.
    bool CheckRequestCondition(QUEST_ATTRIBUTE* pQuest, bool bLastCheck=0); 
	// 수행 조건을 검사한다.
    bool CheckActCondition(QUEST_ATTRIBUTE* pQuest);              
    void RenderDevilSquare(void);         //  데빌 스퀘어.
    void RenderBloodCastle(void);         //  블러드캐슬.

public :
    CSQuest(void);
    ~CSQuest(void);

    //  Quest Init Functions
    bool OpenQuestScript(char* filename);     //  퀘스트 정보를 읽는다.
    bool IsInit(void);                        //  퀘스트 초기화?
    void clearQuest(void);                    //  퀘스트 제거.

    //  Quest Setting.
    void setQuestLists( BYTE* byList, int num, int Class=-1 );	//  퀘스트 수행정보를 설정한다. ( 처음 서버에서 받는다. )
    void setQuestList( int index, int result );					//  퀘스트 수행정보 설정 ( 해당 퀘스트 정보를 갱신한다. )
	BYTE getQuestState2( int questIndex );	//  현 퀘스트의 상태.
    void ShowQuestPreviewWindow( int index = -1 );  //  자신의 퀘스트 정보를 보여주는 창.
    void ShowQuestNpcWindow( int index = -1 );      //  NPC와의 대화를 통해 나오는 퀘스트 내용을 보여주는 창.
    

	BYTE getCurrQuestState(void);				// 현재 퀘스트의 상태.
	unicode::t_char* GetNPCName(BYTE byQuestIndex);
	unicode::t_char* getQuestTitle();			// 현재 퀘스트 타이틀.
	unicode::t_char* getQuestTitle(BYTE byQuestIndex);		// 현재 퀘스트 타이틀.
	unicode::t_char* getQuestTitleWindow();
    void SetEventCount(BYTE type, BYTE count);
	int GetEventCount(BYTE byType);
	DWORD GetNeedZen() { return m_dwNeedZen; }
	QUEST_ATTRIBUTE* GetCurQuestAttribute()
	{ return &m_Quest[m_byCurrQuestIndex]; }

	BYTE GetCurrQuestIndex(){ return m_byCurrQuestIndex; }
	void SetKillMobInfo(int* anKillMobInfo);
	bool ProcessNextProgress();
    void ShowDialogText(int iDialogIndex);
	bool BeQuestItem();
	int FindQuestItemsInInven(int nType, int nCount, int nLevel = -1);
	int GetKillMobCount(int nMobType);

private :
    BYTE    m_byClass;                          //  주인공 클래스.

    BYTE    m_byEventCount[TYPE_QUEST_END];      //  남은 퀘스트(이벤트) 진행 횟수.
    QUEST_ATTRIBUTE m_Quest[MAX_QUESTS];        //  퀘스트 정보.

    BYTE    m_byQuestList[MAX_QUESTS/4];        //  퀘스트 수행정보.
	//(0x00:전송 받지 않았음, 0x01:퀘스트를 받음, 0x02:퀘스트 수행 완료, 0x03:아직 퀘스트 진행 안함)
    BYTE    m_byCurrQuestIndex;                 //  현재 퀘스트 인덱스.
    BYTE    m_byCurrQuestIndexWnd;              //  퀘스트창에서의 현재 퀘스트 인덱스.

    BYTE    m_byStartQuestList;                 //  퀘스트 리스트창에 

    BYTE    m_byViewQuest;                      //  퀘스트 창의 활성/비활성.
    short   m_shCurrPage;                       //  현재 퀘스트 내용 페이지.
    BYTE    m_byCurrState;                      //  현재 퀘스트 상태.
    DWORD   m_dwNeedZen;                         //  필요 젠.

    BYTE    m_byQuestType;                      //  퀘스트 종류.
	bool	m_bOnce;							//  0xF3(케리터 정보)보다 0xA0이 먼저 오는것에 대비한 채크
	
	int 	m_anKillMobType[5];					//	현 진행중인 퀘스트 조건인 죽인 몬스터 타잎.
	int		m_anKillMobCount[5];				//	현 진행중인 퀘스트 조건인 죽인 몬스터 개수.
	WORD	m_wNPCIndex;						//	NPC 쿼렐

    int     m_iStartX;
    int     m_iStartY;
};

#define g_csQuest CSQuest::GetSingleton ()

#endif
