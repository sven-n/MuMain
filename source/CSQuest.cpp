//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "zzzInfomation.h"
#include "zzzBmd.h"
#include "zzzObject.h"
#include "zzztexture.h"
#include "zzzCharacter.h"
#include "zzzscene.h"
#include "zzzInterface.h"
#include "zzzinventory.h"
#include "wsclientinline.h"
#include "CSQuest.h"
#include "GIPetManager.h"
#include "UsefulDef.h"
#include "NewUIInventoryCtrl.h"
#include "CharacterManager.h"

bool bCheckNPC;
extern  int  g_iMessageTextStart;
extern  char g_cMessageTextCurrNum;
extern  char g_cMessageTextNum;
extern  int g_iNumLineMessageBoxCustom;
extern  char g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];
extern  int g_iCurrentDialogScript;
extern  int g_iNumAnswer;
extern  char g_lpszDialogAnswer[MAX_ANSWER_FOR_DIALOG][NUM_LINE_DA][MAX_LENGTH_CMB];

static  CSQuest csQuest;

static BYTE bBuxCode[3] = {0xfc,0xcf,0xab};

static void BuxConvert(BYTE *Buffer,int Size)
{
	for(int i=0;i<Size;i++)
		Buffer[i] ^= bBuxCode[i%3];
}

CSQuest::CSQuest(void) : m_byClass( 255 ), m_byCurrQuestIndex( 0 ), m_byCurrQuestIndexWnd( 0 ),
						m_byStartQuestList( 0 ), m_shCurrPage( 0 ), m_byViewQuest( QUEST_VIEW_NONE ), m_byQuestType( TYPE_QUEST ), m_iStartX(640-190), m_iStartY(0) 
{

}

CSQuest::~CSQuest(void)
{

}

bool CSQuest::IsInit(void)
{
    if(m_byClass == 255)
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
	memset(m_byQuestList, 0, sizeof( BYTE )*(MAX_QUESTS/4));
	memset(m_byEventCount, 0, sizeof( BYTE )*TYPE_QUEST_END);

	for (int i = 0; i < 5; ++i)
	{
		m_anKillMobType[i] = m_anKillMobCount[i] = -1;
	}
}

BYTE CSQuest::getCurrQuestState(void)
{
	return CheckQuestState ();
}

unicode::t_char* CSQuest::GetNPCName(BYTE byQuestIndex)
{
	return ::getMonsterName(int(m_Quest[byQuestIndex].wNpcType));
}

unicode::t_char* CSQuest::getQuestTitle()
{
	return m_Quest[m_byCurrQuestIndex].strQuestName;
}

unicode::t_char* CSQuest::getQuestTitle( BYTE byQuestIndex )
{
	return m_Quest[byQuestIndex].strQuestName;
}

unicode::t_char* CSQuest::getQuestTitleWindow()
{
	return m_Quest[m_byCurrQuestIndexWnd].strQuestName;
}

void CSQuest::SetEventCount( BYTE type, BYTE count )
{
	if(type > TYPE_QUEST_END)
	{
		return;
	}

	m_byEventCount[type] = count;
}

int CSQuest::GetEventCount(BYTE byType)
{
	if(byType > TYPE_QUEST_END)
	{
		return 0;
	}

	return m_byEventCount[byType];
}

bool CSQuest::OpenQuestScript ( char* filename )
{
	FILE* fp = fopen ( filename, "rb" );
	if ( fp==NULL )                
	{
		char Text[256];
    	sprintf ( Text,"%s - File not exist.",filename );
		return  FALSE;
	}

    memset ( m_Quest, 0, sizeof( QUEST_ATTRIBUTE )*MAX_QUESTS );

	int     Size    = sizeof ( QUEST_ATTRIBUTE );
	BYTE*   Buffer  = new BYTE [Size];
	for ( int i=0; i<MAX_QUESTS; i++ )
	{
		fread ( Buffer, Size, 1, fp );
		BuxConvert ( Buffer, Size );
		memcpy ( &m_Quest[i], Buffer, Size );
	}
	SAFE_DELETE_ARRAY(Buffer);
	fclose ( fp );

	return  TRUE;
}

void CSQuest::setQuestLists ( BYTE* byList, int num, int Class )
{
    if ( Class!=-1 )
    {
		m_bOnce = true;
		bCheckNPC = false;

        m_byClass   = gCharacterManager.GetBaseClass(Class);
    }

    memset ( m_byQuestList, 0, sizeof( BYTE )*(MAX_QUESTS/4) );
	::memcpy(m_byQuestList, byList, sizeof (BYTE) * (num / 4 + 1));

	int i;
	if (CLASS_KNIGHT == m_byClass)
	{
		for (i = 0; i < num; ++i)
		{
			if (getQuestState(i) != QUEST_END)
				break;
		}
	}
	else if (CLASS_DARK == m_byClass || CLASS_DARK_LORD == m_byClass
#ifdef PBG_ADD_NEWCHAR_MONK
			|| CLASS_RAGEFIGHTER == m_byClass
#endif //PBG_ADD_NEWCHAR_MONK
		)
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
	if ( getQuestState( QUEST_COMBO )==QUEST_END )
	{
		Hero->byExtensionSkill = 1;
	}
}

void CSQuest::setQuestList ( int index, int result )
{
    m_byCurrQuestIndex = index;

    m_byCurrQuestIndexWnd = max( index, m_byCurrQuestIndexWnd );
    
    int questIndex = (int)(m_byCurrQuestIndex/4);
    m_byQuestList[questIndex] = result;

	Hero->byExtensionSkill = 0;
	if ( getQuestState( QUEST_COMBO )==QUEST_END )
	{
		Hero->byExtensionSkill = 1;
	}
}

short   CSQuest::FindQuestContext ( QUEST_ATTRIBUTE* pQuest, int index )
{
    for ( int i=0; i<pQuest->shQuestConditionNum; ++i )
    {
        if ( pQuest->QuestAct[i].byRequestClass[m_byClass]>=1  )
        {
            return pQuest->QuestAct[i].shQuestStartText[index];
        }
    }

	m_byCurrQuestIndex--;
    CheckQuestState ();

    return m_shCurrPage;
}

bool    CSQuest::CheckRequestCondition ( QUEST_ATTRIBUTE* pQuest, bool bLastCheck )
{
    int iRequestType;
    for ( int i=0; i<pQuest->shQuestConditionNum; ++i )
    {
        if ( pQuest->QuestAct[i].byRequestClass[m_byClass]!=0 )
        {
            iRequestType = pQuest->QuestAct[i].byRequestType;
            for ( int j=0; j<pQuest->shQuestRequestNum; ++j )
            {
                if ( pQuest->QuestRequest[j].byType==iRequestType || pQuest->QuestRequest[j].byType==255 )
                {
					if ( pQuest->QuestRequest[j].wCompleteQuestIndex!=65535 )
					{
						if ( getQuestState2( pQuest->QuestRequest[j].wCompleteQuestIndex )!=QUEST_END )
						{
							m_shCurrPage = pQuest->QuestRequest[j].shErrorText;
							m_byCurrState= QUEST_ERROR;
							return false;
						}
					}
                    if ( pQuest->QuestRequest[j].wLevelMin>0 )
                    {
                        WORD level = CharacterAttribute->Level;

                        if ( pQuest->QuestRequest[j].wLevelMin>level )
                        {
                            m_shCurrPage = pQuest->QuestRequest[j].shErrorText;
                            m_byCurrState = QUEST_ERROR;
                            return false;
                        }
                    }
                    if ( pQuest->QuestRequest[j].wLevelMax>0 )
                    {
                        WORD level = CharacterAttribute->Level;

                        if ( pQuest->QuestRequest[j].wLevelMax<level )
                        {
                            m_shCurrPage = pQuest->QuestRequest[j].shErrorText;
                            m_byCurrState = QUEST_ERROR;
                            return false;
                        }
                    }
                    if ( pQuest->QuestRequest[j].dwZen>0 )
                    {
                        if ( bLastCheck )
                        {
							DWORD gold = CharacterMachine->Gold;

                            m_dwNeedZen = pQuest->QuestRequest[j].dwZen;
                            if ( m_dwNeedZen>gold )
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

bool CSQuest::CheckActCondition ( QUEST_ATTRIBUTE* pQuest )
{
    for ( int i=0; i<pQuest->shQuestConditionNum; ++i )
    {
        if ( pQuest->QuestAct[i].byRequestClass[m_byClass]>=1 )
        {
            switch ( pQuest->QuestAct[i].byQuestType )
            {
            case QUEST_ITEM:
                {
					int itemType
						= (pQuest->QuestAct[i].wItemType*MAX_ITEM_INDEX)
							+ pQuest->QuestAct[i].byItemSubType;
					int itemLevel	= -1;
                    int itemNum     = pQuest->QuestAct[i].byItemNum;

					itemLevel	= pQuest->QuestAct[i].byItemLevel;
                    
					if (FindQuestItemsInInven(itemType, itemNum, itemLevel))
                    {
                        m_shCurrPage = FindQuestContext ( pQuest, 1 );
                        return false;
                    }
                }
                break;

            case QUEST_MONSTER:
                {
					bool bFind = false;

					for (int j = 0; j < 5; ++j)
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

BYTE CSQuest::getQuestState ( int questIndex )
{
    int  Index;
	BYTE byCurrState;
    int  SubIndex;

	if (questIndex == -1)
    {
        Index = int(m_byCurrQuestIndex) / 4;
	    SubIndex= m_byCurrQuestIndex % 4;
    }
    else
    {
        Index = questIndex / 4;
	    SubIndex= questIndex % 4;
    }

	byCurrState = (m_byQuestList[Index] >> (SubIndex * 2)) & 0x03;

	if ( questIndex==-1 )
	{
		m_byCurrState = byCurrState;
	}

    return byCurrState;
}


BYTE CSQuest::getQuestState2 ( int questIndex )
{
    int  Index;
	BYTE byCurrState;
    
    Index = questIndex/4;
	int SubIndex = questIndex % 4;
	byCurrState = (m_byQuestList[Index] >> (SubIndex * 2)) & 0x03;

    return byCurrState;
}

BYTE CSQuest::CheckQuestState ( BYTE state )
{
    if ( Hero->Class !=-1 )
    {
		if(m_bOnce)
		{
			m_bOnce		= false;
			m_byClass   = gCharacterManager.GetBaseClass( Hero->Class );
		}
    }

    QUEST_ATTRIBUTE* lpQuest = &m_Quest[m_byCurrQuestIndex];
//    m_byCurrState = m_byQuestList[m_byCurrQuestIndex];

    if ( state==255 )
    {
        getQuestState ();
    }
    else
    {
        m_byCurrState = state;
    }
    switch ( m_byCurrState )
    {
    case QUEST_NO :
        {
            if ( CheckRequestCondition( lpQuest ) )
            {
                m_shCurrPage = FindQuestContext ( lpQuest, 0 );
            }
        }
        break;

    case QUEST_ING : 
        {
            if ( CheckActCondition( lpQuest ) )
            {
                m_shCurrPage = FindQuestContext ( lpQuest, 2 );
                m_byCurrState = QUEST_ITEM;
            }
        }
        break;

    case QUEST_END :
        {
            m_shCurrPage = FindQuestContext ( lpQuest, 3 );
        }
        break;
    }

	return m_byCurrState;
}

void CSQuest::ShowDialogText ( int iDialogIndex )
{
	g_iCurrentDialogScript = iDialogIndex;

	g_iNumLineMessageBoxCustom = SeparateTextIntoLines( g_DialogScript[g_iCurrentDialogScript].m_lpszText, g_lpszMessageBoxCustom[0], NUM_LINE_CMB, MAX_LENGTH_CMB);

	char lpszAnswer[MAX_LENGTH_ANSWER+8];
	g_iNumAnswer = 0;
	ZeroMemory( g_lpszDialogAnswer, MAX_ANSWER_FOR_DIALOG * NUM_LINE_DA * MAX_LENGTH_CMB);

	int iTextSize=0;

	for ( int i = 0; i < g_DialogScript[g_iCurrentDialogScript].m_iNumAnswer; ++i)
	{
		wsprintf( lpszAnswer, "%d) %s", i + 1, g_DialogScript[g_iCurrentDialogScript].m_lpszAnswer[i]);
		int iNumLine = SeparateTextIntoLines( lpszAnswer, g_lpszDialogAnswer[i][0], NUM_LINE_DA, MAX_LENGTH_CMB );
		if ( iNumLine < NUM_LINE_DA - 1)
		{
			g_lpszDialogAnswer[i][iNumLine][0] = '\0';
		}
		g_iNumAnswer++;
		iTextSize = i;
	}

	if ( 0 == g_DialogScript[g_iCurrentDialogScript].m_iNumAnswer)
	{
		wsprintf( lpszAnswer, "%d) %s", iTextSize + 1, GlobalText[609]);
		strcpy( g_lpszDialogAnswer[0][0], lpszAnswer );
		g_iNumAnswer = 1;
	}
}

void CSQuest::ShowQuestPreviewWindow ( int index )
{
    if(index != -1)
	{
		m_byCurrQuestIndex = index;
	}

    m_byViewQuest = QUEST_VIEW_PREVIEW;

    BYTE tmp = m_byCurrQuestIndex;
    m_byCurrQuestIndex = m_byCurrQuestIndexWnd;

    CheckQuestState ();
    ShowDialogText ( m_shCurrPage );

    m_byCurrQuestIndex = tmp;
}

void CSQuest::ShowQuestNpcWindow ( int index )
{
    if ( index!=-1 ) m_byCurrQuestIndex = index;

    g_bEventChipDialogEnable = EVENT_NONE;

    CheckQuestState ();
    ShowDialogText ( m_shCurrPage );
}

bool CSQuest::BeQuestItem ( void )
{
    bool bCompleteItem = false;

    if ( m_byCurrState==QUEST_ING )
    {
        QUEST_ATTRIBUTE* pQuest = &m_Quest[m_byCurrQuestIndex];

        for ( int i=0; i<pQuest->shQuestConditionNum; ++i )
        {
            if ( pQuest->QuestAct[i].byRequestClass[m_byClass]>0 )
            {
                switch ( pQuest->QuestAct[i].byQuestType )
                {
                case QUEST_ITEM:
                    {
                        bCompleteItem = true;

						int itemType
							= (pQuest->QuestAct[i].wItemType*MAX_ITEM_INDEX)
								+ pQuest->QuestAct[i].byItemSubType;
						int itemLevel	= -1;
                        int itemNum     = pQuest->QuestAct[i].byItemNum;

                        itemLevel	= pQuest->QuestAct[i].byItemLevel;
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

						for (int j = 0; j < 5; ++j)
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
	ITEM* pItem = NULL;
	int nFindItemCount = 0;
	
	for (int i = 0; i < nItemsInInven; ++i)
	{
		pItem = pInvenCtrl->GetItem(i);
		if (nType == pItem->Type)
		{
			if (nLevel == -1 || nLevel == (int)((pItem->Level>>3)&15))
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
	for (int i = 0; i < 5; ++i)
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
        SendRequestQuestState(m_byCurrQuestIndex, 1);
		return false;
    }
}

void CSQuest::SetKillMobInfo(int* anKillMobInfo)
{
	for (int i = 0; i < 10; i += 2)
	{
		m_anKillMobType[i / 2] = anKillMobInfo[i];
		m_anKillMobCount[i / 2] = anKillMobInfo[i + 1];
	}
}

void CSQuest::RenderDevilSquare ( void )
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(230, 230, 230, 255);
	g_pRenderText->SetBgColor(20, 20, 20, 255);
	g_pRenderText->RenderText(m_iStartX+95-60, m_iStartY+12, GlobalText[1145],120, 0, RT3_SORT_CENTER);

	g_pRenderText->SetTextColor(200, 220, 255, 255);
//	RenderText ( m_iStartX+95-73, m_iStartY+22, m_Quest[m_byCurrQuestIndex].strQuestName, 150*WindowWidth/640, true );
}

void CSQuest::RenderBloodCastle ( void )
{
    char Text[100];
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(230, 230, 230, 255);
	g_pRenderText->SetBgColor(20, 20, 20, 255);
	g_pRenderText->RenderText(m_iStartX+95-60, m_iStartY+12, GlobalText[1146],120, 0, RT3_SORT_CENTER);

	g_pRenderText->SetTextColor(223, 191, 103, 255);
	g_pRenderText->SetBgColor(0);
	sprintf ( Text, GlobalText[869], BLOODCASTLE_QUEST_NUM, GlobalText[1146], GlobalText[1140]);
	g_pRenderText->RenderText(m_iStartX+95, m_iStartY+80, Text, 0 ,0, RT3_WRITE_CENTER);
	g_pRenderText->SetTextColor(255, 230, 210, 255);
	g_pRenderText->RenderText(m_iStartX+85, m_iStartY+100, GlobalText[877], 0 ,0, RT3_WRITE_CENTER);
    g_pRenderText->RenderText(m_iStartX+105, m_iStartY+120, GlobalText[878], 0 ,0, RT3_WRITE_CENTER);

	g_pRenderText->SetFont(g_hFontBig);
	sprintf ( Text, GlobalText[868], m_byEventCount[m_byQuestType] );
	g_pRenderText->RenderText(m_iStartX+95, m_iStartY+65+60*4, Text, 0 ,0, RT3_WRITE_CENTER);
}
