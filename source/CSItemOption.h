//////////////////////////////////////////////////////////////////////////
//  - 아이템 옵션 관련 -
//  
//  
//////////////////////////////////////////////////////////////////////////
#ifndef __CSITEM_OPTION_H__
#define __CSITEM_OPTION_H__

/*+++++++++++++++++++++++++++++++++++++
    INCLUDE.
+++++++++++++++++++++++++++++++++++++*/
#include "Singleton.h"
#include "zzzinfomation.h"

#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
#include <map>
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE


const BYTE MAX_SET_OPTION = 64;
const BYTE MASTERY_OPTION = 24;

const BYTE EXT_A_SET_OPTION  = 1;
const BYTE EXT_B_SET_OPTION  = 2;

/*+++++++++++++++++++++++++++++++++++++
    CLASS.
+++++++++++++++++++++++++++++++++++++*/
//////////////////////////////////////////////////////////////////////////
//  
//////////////////////////////////////////////////////////////////////////
class CSItemOption : public Singleton<CSItemOption>
{
private :
	ITEM_SET_TYPE	m_ItemSetType[MAX_ITEM];
	ITEM_SET_OPTION	m_ItemSetOption[MAX_SET_OPTION];

	bool	m_bViewOptionList;

    BYTE    m_byRenderOptionList;
    BYTE    m_bySelectedItemOption;

    BYTE    m_bySameSetItem;
#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
#ifdef _LANGUAGE_JPN
    char    m_strSetName[2][64];
#else // _LANGUAGE_JPN
    char    m_strSetName[2][32];
#endif // _LANGUAGE_JPN
#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
    char    m_strSetName[2][64];
#else
    char    m_strSetName[2][32];
#endif// SELECTED_LANGUAGE == LANGUAGE_JAPANESE
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE

#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
	typedef std::map<int, string>	MAP_EQUIPPEDSETITEMNAME;
	typedef std::map<BYTE, int>		MAP_EQUIPPEDSETITEM_SEQUENCE;

	MAP_EQUIPPEDSETITEMNAME			m_mapEquippedSetItemName;
	MAP_EQUIPPEDSETITEMNAME::iterator
									m_iterESIN;

	MAP_EQUIPPEDSETITEM_SEQUENCE	m_mapEquippedSetItemSequence;
	MAP_EQUIPPEDSETITEM_SEQUENCE::iterator
									m_iterESIS;
private:
	int		Search_From_EquippedSetItemNameMapTable( char *szSetItemname )
	{
		int		iSizeFindName = strlen( szSetItemname );

		for( m_iterESIN = m_mapEquippedSetItemName.begin(); m_iterESIN != m_mapEquippedSetItemName.end(); ++m_iterESIN )
		{
			string	strCur;

			strCur = m_iterESIN->second;
			int iSizeCurName = strCur.size();
			
			// . 굳이 비교자와 피비교자간에 Size로 모두 하는건 기존 코드가 그렇게 되어 있었기 때문 입니다. 
			if( !strncmp( szSetItemname, strCur.c_str(), iSizeFindName ) 
				&& !strncmp( szSetItemname, strCur.c_str(), iSizeCurName ) )
			{
				return m_iterESIN->first;
			}
		}
		return -1;
	}

public:
	bool	Find_From_EquippedSetItemNameMapTable( char	*szSetItemname )
	{
		int iResult = Search_From_EquippedSetItemNameMapTable( szSetItemname );

		if( -1 == iResult )
		{
			return false;
		}
		else
		{
			return true;
		}
	}

#define	BYTE_MAX		255

	BYTE	Search_From_EquippedSetItemNameSequence( char *szSetItemname )
	{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		BYTE	byCur = 0;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
		int		iResult = Search_From_EquippedSetItemNameMapTable( szSetItemname );
		
		if( -1 != iResult )
		{
			for( m_iterESIS = m_mapEquippedSetItemSequence.begin(); m_iterESIS != m_mapEquippedSetItemSequence.end(); ++m_iterESIS )
			{
				if( (int)(m_iterESIS->second) == iResult )
				{
					return (BYTE)m_iterESIS->first;
				}
			}
		}
		
		return (BYTE)BYTE_MAX;
	}
	
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE	

    BYTE	m_bySetOptionANum;
	BYTE	m_bySetOptionBNum;
    BYTE    m_bySetOptionIndex[2];
	BYTE	m_bySetOptionList[16][3];       //  옵션 리스트.
	BYTE	m_bySetOptionListOnOff[16];     //  사용된 옵션.
    int     m_iSetOptionListValue[16][2];   //  옵션 값.

	bool	OpenItemSetType ( const char* filename );
	bool	OpenItemSetOption ( const char* filename );

	void	checkItemType ( BYTE* optioList, const int iType, const int setType );

	void	calcSetOptionList ( BYTE* optioList );

	void	getExplainText ( char* text, const BYTE option, const BYTE value, const BYTE SetA );

#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION 
	void    getAllAddState ( WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma );
#else // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
    void    getAllAddState ( WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality );
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	
	int     GetSetItmeCount( const ITEM* pselecteditem );
	bool	isFullseteffect( const ITEM* ip );

#ifdef LDS_FIX_WRONG_CALCULATEEQUIPEDITEMOPTIONVALUE
public:	

#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	void	getAllAddOptionStatesbyCompare ( WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma, 
								WORD iCompareStrength, WORD iCompareDexterity, WORD iCompareEnergy, WORD iCompareVitality, WORD iC );
#else // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	void	getAllAddOptionStatesbyCompare ( WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality,
								WORD iCompareStrength, WORD iCompareDexterity, WORD iCompareEnergy, WORD iCompareVitality );
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	void	getAllAddStateOnlyAddValue ( WORD* AddStrength, 
										WORD* AddDexterity, 
										WORD* AddEnergy, 
										WORD* AddVitality, 
										WORD* AddCharisma );
#else // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	void	getAllAddStateOnlyAddValue ( WORD* AddStrength, WORD* AddDexterity, WORD* AddEnergy, WORD* AddVitality );
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION

#endif // LDS_FIX_WRONG_CALCULATEEQUIPEDITEMOPTIONVALUE
	
public :
    CSItemOption ( void )  { init(); };
    ~CSItemOption ( void ) {};

	void init( void )
	{
#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
#ifdef _LANGUAGE_JPN
        memset ( m_strSetName, 0, sizeof( char )*64*2 );
#else // _LANGUAGE_JPN
        memset ( m_strSetName, 0, sizeof( char )*32*2 );
#endif // _LANGUAGE_JPN
#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
        memset ( m_strSetName, 0, sizeof( char )*64*2 );
#else
        memset ( m_strSetName, 0, sizeof( char )*32*2 );
#endif// SELECTED_LANGUAGE == LANGUAGE_JAPANESE
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE
		m_bViewOptionList = false;
        m_byRenderOptionList= 0;
        m_bySelectedItemOption = 0;
        m_bySameSetItem = 0;
	}
    void    ClearListOnOff ( void )
    {
        memset ( m_bySetOptionListOnOff, 0, sizeof( BYTE )* 16 );
    }
    bool    OpenItemSetScript ( bool bTestServer );						            //  세트 아이템 정보를 읽는다.

	bool    IsDisableSkill ( int Type, int Energy, int Charisma = 0 );
    BYTE    IsChangeSetItem ( const int Type, const int SubType );                  //  세트 아이템으로 조합 가능한가?
    WORD    GetMixItemLevel ( const int Type );
	bool	GetSetItemName ( char* strName, const int iType, const int setType );	//	세트 아이템 이름을 얻는다.

	void	PlusSpecial ( WORD* Value, const int Special );
	void	PlusSpecialPercent ( WORD* Value, const int Special );
	void	PlusSpecialLevel ( WORD* Value, const WORD SrcValue, const int Special );
    void    PlusMastery ( int* Value, const BYTE MasteryType );

    void    MinusSpecialPercent ( int* Value, const int Special );

	void	GetSpecial ( WORD* Value, int Special );

	void	GetSpecialPercent ( WORD* Value, int Special );
	void	GetSpecialLevel ( WORD* Value, const WORD SrcValue, int Special );
    int     GetDefaultOptionValue ( ITEM* ip, WORD* Value );
    bool    GetDefaultOptionText ( const ITEM* ip, char* Text );
    int     RenderDefaultOptionText ( const ITEM* ip, int TextNum );

#ifdef PJH_FIX_4_BUGFIX_33
	bool    Special_Option_Check(int Kind = 0);
#endif //#ifdef PJH_FIX_4_BUGFIX_33

	void	CheckItemSetOptions ( void );
	void	MoveSetOptionList ( const int StartX, const int StartY );
	void	RenderSetOptionButton ( const int StartX, const int StartY );
	void	RenderSetOptionList ( const int StartX, const int StartY );
 
#ifdef LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
	// 각 착용중인 아이템별로 출력해야할 세트 추가 옵션리스트의 갯수 제한 수치
	int		m_arLimitSetItemOptionCount[MAX_EQUIPMENT];		

	// 모든 착용중인 아이템 별로 출력해야할 세트 추가 옵션 갯수 Counting
	// * 주의 : checkItemType 함수 연산 이후 호출 되어야 합니다.
	void	UpdateCount_SetOptionPerEquippedSetItem( const BYTE* byOptionList,			// 장착 세트아이템 정보
												int* arLimitSetItemOptionCount,		// 장착 아이템별 출력 옵션 갯수 저장될 반환 변수
												ITEM* ItemsEquipment				// 현재 장착된 모든 아이템들
												 )
	{
		for( int iE = 0; iE < MAX_EQUIPMENT_INDEX; ++iE )
		{
			int &iCurCount = arLimitSetItemOptionCount[iE];
			ITEM &CurItem = ItemsEquipment[iE];
			
			iCurCount = GetCurrentTypeSetitemCount( CurItem, byOptionList );
		}
	}

	// checkItemType 함수 연산 이후에 적용된 각 장착 세트 아이템 별 갯수 반환
	int		GetCurrentTypeSetitemCount( const ITEM &CurItem_, const BYTE* byOptionList )
	{
		BYTE bySetType = CurItem_.ExtOption;

		int setItemType = (bySetType%0x04);

		ITEM_SET_TYPE& itemSType = m_ItemSetType[CurItem_.Type];


		// 총 4번 즉 세트 아이템 종류별 최대 4개 까지만 가능합니다.
#ifdef YDG_FIX_OVER_5_SETITEM_TOOLTIP_BUG
		for (int i = 0; i < 30; i += 3)
#else	// YDG_FIX_OVER_5_SETITEM_TOOLTIP_BUG
#ifdef PBG_FIX_SETITEM_4OVER
		for( int i = 0; i < 13; i+=3 )
#else //PBG_FIX_SETITEM_4OVER
		for( int i = 0; i < 10; i+=3 )
#endif //PBG_FIX_SETITEM_4OVER
#endif	// YDG_FIX_OVER_5_SETITEM_TOOLTIP_BUG
		{
			ITEM_SET_OPTION& itemOption = m_ItemSetOption[byOptionList[i]];
			
			if( byOptionList[i] == itemSType.byOption[setItemType-1] )
			{
				int iEquippedCount = byOptionList[i+1];
				
				// 풀세트에 해당
				if ( iEquippedCount>=itemOption.byOptionCount-1 )
				{
					return BYTE_MAX;
				}
				
				return iEquippedCount;
			}
		}

		return 0;
	}

#endif // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE

#ifdef LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY
	int     RenderSetOptionListInItem ( const ITEM* ip, int TextNum, bool bIsEquippedItem = false );
#else // LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY
	int     RenderSetOptionListInItem ( const ITEM* ip, int TextNum );
#endif // LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY


    void    ClearOptionHelper ( void ) { m_byRenderOptionList = 0; }
    void    CheckRenderOptionHelper ( const char* FilterName );
    void    RenderOptionHelper ( void );
	
	BYTE GetSetOptionANum();
	BYTE GetSetOptionBNum();

	void SetViewOptionList(bool bView);
	bool IsViewOptionList();
};

#define g_csItemOption CSItemOption::GetSingleton ()

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
extern string g_strSelectedML;
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#endif
