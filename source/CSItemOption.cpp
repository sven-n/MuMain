//////////////////////////////////////////////////////////////////////////
//  - 아이템 옵션 관련 -
//  
//  
//////////////////////////////////////////////////////////////////////////
/*+++++++++++++++++++++++++++++++++++++
    INCLUDE.
+++++++++++++++++++++++++++++++++++++*/
#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "zzzInfomation.h"
#include "zzzBmd.h"
#include "zzzObject.h"
#include "zzztexture.h"
#include "zzzCharacter.h"
#include "zzzscene.h"
#include "zzzInterface.h"
#include "zzzinventory.h"
#include "dsplaysound.h"
#include "CSItemOption.h"
#include "UIControls.h"
#include "NewUISystem.h"

/*+++++++++++++++++++++++++++++++++++++
    Extern.
+++++++++++++++++++++++++++++++++++++*/
extern	char TextList[30][100];
extern	int  TextListColor[30];
extern	int  TextBold[30];
extern float g_fScreenRate_x;	// ※
extern float g_fScreenRate_y;


/*+++++++++++++++++++++++++++++++++++++
    Global.
+++++++++++++++++++++++++++++++++++++*/
static  CSItemOption csItemOption;

//static	const	BYTE g_bySetOptionListValue[35] = {  2,  2, 15,  5,  5,  5, 10, 20, 10, 20, 10, 20, 10, 20, 40, 80, 40, 80,  
//													 5, 10,  5, 60, 50, 70, 20,  3, 50, 40, 10, 10, 10, 10, 10, 10, 10 };

/*+++++++++++++++++++++++++++++++++++++
    FUNCTIONS.
+++++++++++++++++++++++++++++++++++++*/
static BYTE bBuxCode[3] = {0xfc,0xcf,0xab};

static void BuxConvert(BYTE *Buffer,int Size)
{
	for(int i=0;i<Size;i++)
		Buffer[i] ^= bBuxCode[i%3];
}


//////////////////////////////////////////////////////////////////////////
//	세트 아이템 옵션을 위한 데이터를 읽는다.
//////////////////////////////////////////////////////////////////////////
bool    CSItemOption::OpenItemSetScript ( bool bTestServer )     //  세트 아이템 정보를 읽는다.
{
// ItemSetType.bmd는 Localization-free 파일이지만, ItemSetOption은 국가에 따라 다름
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	string strFileName = "";
	string strTest = (bTestServer) ? "Test" : "";

	strFileName = "Data\\Local\\ItemSetType" + strTest + ".bmd";
	if ( !OpenItemSetType(strFileName.c_str()) )		return false;

	strFileName = "Data\\Local\\" + g_strSelectedML + "\\ItemSetOption" + strTest + "_" + g_strSelectedML + ".bmd";
	if ( !OpenItemSetOption(strFileName.c_str()) )	 	return false;

#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	if ( bTestServer )
	{
		if ( !OpenItemSetType( "Data\\Local\\ItemSetTypeTest.bmd" ) )		return false;
		if ( !OpenItemSetOption( "Data\\Local\\ItemSetOptionTest.bmd" ) ) 	return false;
	}
	else
	{
		if ( !OpenItemSetType( "Data\\Local\\ItemSetType.bmd" ) )		return false;
		if ( !OpenItemSetOption( "Data\\Local\\ItemSetOption.bmd" ) ) 	return false;
	}
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

	return true;
}



//////////////////////////////////////////////////////////////////////////
//	세트 아이템 타입을 "ItemSetType.bmd" 읽는다.
//////////////////////////////////////////////////////////////////////////
bool	CSItemOption::OpenItemSetType ( const char* filename )
{
	FILE* fp = fopen ( filename, "rb" );
	if(fp != NULL)
	{
		int Size = sizeof(ITEM_SET_TYPE);
		BYTE *Buffer = new BYTE [Size*MAX_ITEM];
		fread(Buffer,Size*MAX_ITEM,1,fp);

		DWORD dwCheckSum;
		fread(&dwCheckSum,sizeof ( DWORD),1,fp);
		fclose(fp);

		if ( dwCheckSum != GenerateCheckSum2( Buffer, Size*MAX_ITEM, 0xE5F1))
		{
			char Text[256];
    		sprintf(Text,"%s - File corrupted.",filename);
			g_ErrorReport.Write( Text);
			MessageBox(g_hWnd,Text,NULL,MB_OK);
			SendMessage(g_hWnd,WM_DESTROY,0,0);
		}
		else
		{
			BYTE *pSeek = Buffer;
			for(int i=0;i<MAX_ITEM;i++)
			{
				BuxConvert(pSeek,Size);
				memcpy(&m_ItemSetType[i],pSeek,Size);

				pSeek += Size;
			}
		}
		delete [] Buffer;
	}
	else
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",filename);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//	세트 아이템 옵션을 "ItemSetOption.bmd" 읽는다.
//////////////////////////////////////////////////////////////////////////
bool	CSItemOption::OpenItemSetOption ( const char* filename )
{
	FILE* fp = fopen ( filename, "rb" );
	if(fp != NULL)
	{
		int Size = sizeof(ITEM_SET_OPTION);
		BYTE *Buffer = new BYTE [Size*MAX_SET_OPTION];
		fread(Buffer,Size*MAX_SET_OPTION,1,fp);

		DWORD dwCheckSum;
		fread(&dwCheckSum,sizeof ( DWORD),1,fp);
		fclose(fp);

		if ( dwCheckSum != GenerateCheckSum2( Buffer, Size*MAX_SET_OPTION, 0xA2F1))
		{
			char Text[256];
    		sprintf(Text,"%s - File corrupted.",filename);
			g_ErrorReport.Write( Text);
			MessageBox(g_hWnd,Text,NULL,MB_OK);
			SendMessage(g_hWnd,WM_DESTROY,0,0);
		}
		else
		{
			BYTE *pSeek = Buffer;
			for(int i=0;i<MAX_SET_OPTION;i++)
			{
				BuxConvert(pSeek,Size);
				memcpy(&m_ItemSetOption[i],pSeek,Size);

				pSeek += Size;
			}
		}
		delete [] Buffer;
	}
	else
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",filename);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//  
//////////////////////////////////////////////////////////////////////////
bool CSItemOption::IsDisableSkill ( int Type, int Energy, int Charisma )
{

	int SkillEnergy = 20+SkillAttribute[Type].Energy*(SkillAttribute[Type].Level)*4/100;

	// 익스플로전, 레퀴엠
	if( Type == AT_SKILL_SUMMON_EXPLOSION || Type == AT_SKILL_SUMMON_REQUIEM ) {
		SkillEnergy = 20+SkillAttribute[Type].Energy*(SkillAttribute[Type].Level)*3/100;
	}

#ifdef PSW_BUGFIX_CLASS_KNIGHT_REQUIRESKILL
	if( GetBaseClass ( Hero->Class ) == CLASS_KNIGHT ) {
		SkillEnergy = 10+SkillAttribute[Type].Energy*(SkillAttribute[Type].Level)*4/100;
	}
#endif //PSW_BUGFIX_CLASS_KNIGHT_REQUIRESKILL
    
    switch ( Type )
    {
    case 17:SkillEnergy = 0;break;      //  에너지볼
	case 30:SkillEnergy = 30;break;     //  고블린소환	
	case 31:SkillEnergy = 60;break;     //  돌괴물소환	
	case 32:SkillEnergy = 90;break;     //  암살자소환
	case 33:SkillEnergy = 130;break;    //  설인대장소환
	case 34:SkillEnergy = 170;break;    //  다크나이트소환
	case 35:SkillEnergy = 210;break;    //  발리소환
	case 36:SkillEnergy = 300;break;    //  솔져소환
#ifdef ADD_ELF_SUMMON
	case 37:SkillEnergy = 500;break;    //  쉐도우나이트소환
#endif // ADD_ELF_SUMMON
    case 60:SkillEnergy = 15; break;    //  포스.
	case AT_SKILL_ASHAKE_UP:
	case AT_SKILL_ASHAKE_UP+1:
	case AT_SKILL_ASHAKE_UP+2:
	case AT_SKILL_ASHAKE_UP+3:
	case AT_SKILL_ASHAKE_UP+4:
    case AT_SKILL_DARK_HORSE:    SkillEnergy = 0; break;    //  
    case AT_PET_COMMAND_DEFAULT: SkillEnergy = 0; break;    //  
    case AT_PET_COMMAND_RANDOM:  SkillEnergy = 0; break;    //  
    case AT_PET_COMMAND_OWNER:   SkillEnergy = 0; break;    //  
    case AT_PET_COMMAND_TARGET:  SkillEnergy = 0; break;    //  
	case AT_SKILL_PLASMA_STORM_FENRIR: SkillEnergy = 0; break;	// 플라즈마스톰
	case AT_SKILL_INFINITY_ARROW: SkillEnergy = 0; 
		break;	// 인피니티 애로우
		
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
	case AT_SKILL_BLOW_OF_DESTRUCTION: SkillEnergy = 0;	// 파괴의일격
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION

#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
	case AT_SKILL_RECOVER:
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
	case AT_SKILL_GAOTIC:
#endif //PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
	case AT_SKILL_MULTI_SHOT:
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_FIRE_SCREAM_UP:
	case AT_SKILL_FIRE_SCREAM_UP+1:
	case AT_SKILL_FIRE_SCREAM_UP+2:
	case AT_SKILL_FIRE_SCREAM_UP+3:
	case AT_SKILL_FIRE_SCREAM_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_DARK_SCREAM: 
		SkillEnergy = 0; 
		break;

	case AT_SKILL_EXPLODE:	
		SkillEnergy = 0; 
		break;
    }

	/*
    AT_SKILL_STUN               ,       //  스턴.
    AT_SKILL_REMOVAL_STUN       ,       //  스턴 제거.
    AT_SKILL_MANA               ,       //  마나증가.
    AT_SKILL_INVISIBLE          ,       //  투명.
    AT_SKILL_REMOVAL_INVISIBLE  ,       //  투명 해제.
    AT_SKILL_REMOVAL_BUFF       ,       //  보조마법 일시 제거.
	*/

	if(Type >= AT_SKILL_STUN && Type <= AT_SKILL_REMOVAL_BUFF)
	{
		SkillEnergy = 0;
	}
	else
    //  17, 18, 19, 20, 21, 22, 23, 41, 47, 49, 48, 43, 42, 24, 52, 51, 30, 31, 32, 33, 34, 35, 36, 55, 56
    //      방어    ~    베기       회오리베기 ~  블로우         창찌르기   ~ 레이드 슛       다발        관통   아이스애로우   블러드어택  파워슬레쉬
#ifdef KJH_FIX_SKILL_REQUIRE_ENERGY
	if ( ( Type>=18 && Type<=23 ) || ( Type>=41 && Type<=43 ) || ( Type>=47 && Type<=49 ) || Type==24 || Type==51 || Type==52 || Type==55 || Type==56 )
#else // KJH_FIX_SKILL_REQUIRE_ENERGY
	if ( ( Type>=18 && Type<23 ) || ( Type>=41 && Type<=43 ) || ( Type>=47 && Type<=49 ) || Type==24 || Type==51 || Type==52 || Type==55 || Type==56 )
#endif // KJH_FIX_SKILL_REQUIRE_ENERGY
    {
        SkillEnergy = 0;
    }
    else if ( Type==44 || Type==45 || Type==46 || Type==57 || Type==73 || Type==74 )
    {	// 공성스킬은 최소요구 에너지가 없다. 
        SkillEnergy = 0;
    }
	
	// 마이너스 열매 관련
	if(Charisma > 0)
	{
		int SkillCharisma = SkillAttribute[Type].Charisma;
		if(Charisma < SkillCharisma)	// 통솔포인트 비교 해서 적으면 스킬 사용 못하는거다.
		{
			return true;
		}
	}


    if( Energy < SkillEnergy )
	{
        return true;
	}

    return false;
}


//////////////////////////////////////////////////////////////////////////
//	세트 아이템으로 조합 가능?
//////////////////////////////////////////////////////////////////////////
BYTE CSItemOption::IsChangeSetItem ( const int Type, const int SubType=-1 )
{
	ITEM_SET_TYPE& itemSType = m_ItemSetType[Type];

    if ( SubType==-1 )  //  A, B세트중 하나.
    {
        if ( itemSType.byOption[0]==255 && itemSType.byOption[1]==255 )
            return 0;
        return 255;
    }
    else                //  해당되는 세트.
    {
        if ( itemSType.byOption[SubType]==255 )
            return 0;

        return SubType+1;
    }
}

//////////////////////////////////////////////////////////////////////////
//  세트아이템 조합 레벨.
//////////////////////////////////////////////////////////////////////////
WORD CSItemOption::GetMixItemLevel ( const int Type )
{
    if ( Type<0 ) return 0;

    WORD MixLevel = 0;
    ITEM_SET_TYPE& itemSType = m_ItemSetType[Type];

    MixLevel = MAKEWORD ( itemSType.byMixItemLevel[0], itemSType.byMixItemLevel[1] );

    return MixLevel;
}


//////////////////////////////////////////////////////////////////////////
//	해당 아이템의 세트아이템 이름을 얻는다.
//////////////////////////////////////////////////////////////////////////
bool	CSItemOption::GetSetItemName( char* strName, const int iType, const int setType )
{
    int setItemType = (setType%0x04);

	if ( setItemType>0 )
	{
		ITEM_SET_TYPE& itemSType = m_ItemSetType[iType];
		if ( itemSType.byOption[setItemType-1]!=255 && itemSType.byOption[setItemType-1]!=0 )
		{
			ITEM_SET_OPTION& itemOption = m_ItemSetOption[itemSType.byOption[setItemType-1]];

#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
#ifdef _LANGUAGE_JPN
			memcpy ( strName, itemOption.strSetName, sizeof( char )*64 );
#else // _LANGUAGE_JPN
			memcpy ( strName, itemOption.strSetName, sizeof( char )*32 );
#endif // _LANGUAGE_JPN
#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
            memcpy ( strName, itemOption.strSetName, sizeof( char )*64 );
#else
			memcpy ( strName, itemOption.strSetName, sizeof( char )*32 );
#endif// SELECTED_LANGUAGE == LANGUAGE_JAPANESE
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE
            int length = strlen ( strName );
            strName[length] = ' ';
            strName[length+1]= 0;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//	해당 아이템의 세트 타입을 검색한다.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::checkItemType ( BYTE* optionList, const int iType, const int setType )
{
	int setItemType = (setType%0x04);

    if ( setItemType>0 )
	{
		ITEM_SET_TYPE& itemSType = m_ItemSetType[iType];

        //  세트 옵션이 존재한다.
		if ( itemSType.byOption[setItemType-1]!=255 && itemSType.byOption[setItemType-1]!=0 )
		{
            //  같은 무기이면서 같은 세트인 아이템은 제외한다.
			//  총 4번의 루핑이 돌며, 즉 세트 아이템 4개 까지만 가능하다. // 
#ifdef YDG_FIX_OVER_5_SETITEM_TOOLTIP_BUG
			for (int i = 0; i < 30; i += 3)
#else	// YDG_FIX_OVER_5_SETITEM_TOOLTIP_BUG
#ifdef PBG_FIX_SETITEM_4OVER
			for ( int i=0; i<13; i+=3 )
#else //PBG_FIX_SETITEM_4OVER
			for ( int i=0; i<10; i+=3 )
#endif //PBG_FIX_SETITEM_4OVER
#endif	// YDG_FIX_OVER_5_SETITEM_TOOLTIP_BUG
			{
				if ( optionList[i]==0 )
				{
					optionList[i] = itemSType.byOption[setItemType-1];          //  옵션.
					optionList[i+1]++;                                          //  갯수.
                    optionList[i+2] = setItemType-1;                            //  세트 종류.
					break;
				}
				else if ( optionList[i]==itemSType.byOption[setItemType-1] )    //  옵션.
				{
					optionList[i+1]++;
                    optionList[i+2] = setItemType-1;                            //  세트 종류.
					break;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//	옵션들의 정보를 계산한다.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::calcSetOptionList ( BYTE* optionList )
{
    int Class = GetBaseClass ( Hero->Class );
	int ExClass = IsSecondClass(Hero->Class);

    BYTE    bySetOptionListTmp[2][16];  //  세트 옵션 리스트.
    BYTE    bySetOptionListTmp2[2][16]; //  세트 옵션 가능 리스트.
    int     iSetOptionListValue[2][16]; //  옵션 값.

#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
	unsigned int    
			arruiSetItemTypeSequence[2][16];
	memset( arruiSetItemTypeSequence, 0, sizeof( unsigned int ) * 32 );
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE

    BYTE    optionCount[2] = { 0, 0 };  //  
    BYTE    setType = 0;

    m_bySameSetItem = 0;
#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
#ifdef _LANGUAGE_JPN
    ZeroMemory ( m_strSetName, sizeof(char)*2*64 );
#else // _LANGUAGE_JPN
    ZeroMemory ( m_strSetName, sizeof(char)*2*32 );
#endif // _LANGUAGE_JPN
#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
    ZeroMemory ( m_strSetName, sizeof(char)*2*64 );
#else
    ZeroMemory ( m_strSetName, sizeof(char)*2*32 );
#endif// SELECTED_LANGUAGE == LANGUAGE_JAPANESE
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE

    m_bySetOptionIndex[0] = 0;
    m_bySetOptionIndex[1] = 0;
	m_bySetOptionANum = 0;
	m_bySetOptionBNum = 0;
    Hero->ExtendState = 0;

#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
	unsigned int iSetItemTypeSequence = 0, iCurrentSetItemTypeSequence = 0;

	m_mapEquippedSetItemName.clear();
	m_mapEquippedSetItemSequence.clear();
#endif //LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE

#ifdef YDG_FIX_OVER_5_SETITEM_TOOLTIP_BUG
	for (int i = 0; i < 30; i += 3)
#else	// YDG_FIX_OVER_5_SETITEM_TOOLTIP_BUG
#ifdef PBG_FIX_SETITEM_4OVER
	for ( int i=0; i<13; i+=3 )
#else //PBG_FIX_SETITEM_4OVER
	for ( int i=0; i<10; i+=3 )
#endif //PBG_FIX_SETITEM_4OVER
#endif	// YDG_FIX_OVER_5_SETITEM_TOOLTIP_BUG
	{
		// 동일한 세트가 2개 이상이다.
		if ( optionList[i+1]>=2 )
		{
			int count = optionList[i+1]-1;									// 같은 세트 옵션 아이템 갯수 - 1 // 인덱스 접근
			int standardCount = min ( count, 6 );
			ITEM_SET_OPTION& itemOption = m_ItemSetOption[optionList[i]];		

            BYTE RequireClass = 0;
            if ( itemOption.byRequireClass[0]==1 && Class==CLASS_WIZARD ) RequireClass = 1;
            if ( itemOption.byRequireClass[0]==2 && Class==CLASS_WIZARD && ExClass ) RequireClass = 1;
            if ( itemOption.byRequireClass[1]==1 && Class==CLASS_KNIGHT ) RequireClass = 1;
            if ( itemOption.byRequireClass[1]==2 && Class==CLASS_KNIGHT && ExClass ) RequireClass = 1;
            if ( itemOption.byRequireClass[2]==1 && Class==CLASS_ELF    ) RequireClass = 1;
            if ( itemOption.byRequireClass[2]==2 && Class==CLASS_ELF    && ExClass ) RequireClass = 1;
            if ( itemOption.byRequireClass[3]==1 && Class==CLASS_DARK   ) RequireClass = 1;
#ifdef PJH_ADD_SET_NEWJOB
            if ( itemOption.byRequireClass[3]==1 && Class==CLASS_DARK && ExClass) RequireClass = 1;
            if ( itemOption.byRequireClass[4]==1 && Class==CLASS_DARK_LORD ) RequireClass = 1;
			if ( itemOption.byRequireClass[4]==1 && Class==CLASS_DARK_LORD && ExClass) RequireClass = 1;
            if ( itemOption.byRequireClass[5]==1 && Class==CLASS_SUMMONER ) RequireClass = 1;
			if ( itemOption.byRequireClass[5]==1 && Class==CLASS_SUMMONER && ExClass) RequireClass = 1;
#endif //PJH_ADD_SET_NEWJOB
#ifdef PBG_ADD_NEWCHAR_MONK
			if ( itemOption.byRequireClass[6]==1 && Class==CLASS_RAGEFIGHTER ) RequireClass = 1;
			if ( itemOption.byRequireClass[6]==1 && Class==CLASS_RAGEFIGHTER && ExClass) RequireClass = 1;
#endif //PBG_ADD_NEWCHAR_MONK
            setType = optionList[i+2];
            m_bySetOptionIndex[setType] = optionList[i];
            if ( m_strSetName[setType][0]!=0 && strcmp( m_strSetName[setType], itemOption.strSetName )!=NULL )
            {
                if ( m_strSetName[0][0]==0 )
                    strcpy ( m_strSetName[0], itemOption.strSetName );
                else
                    strcpy ( m_strSetName[1], itemOption.strSetName );
                m_bySameSetItem = count;
            }
            else
            {
                strcpy ( m_strSetName[setType], itemOption.strSetName );
            }


#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
			// 1. SetItemNameMap에서 찾아보고 없으면 추가. 추가된 아이템의 TypeSequence를 알아놓고 있는다.
			// 2. SetItemSequence Table에 현재 TypeSequence와 Index Sequence를 저장
			
			bool	bFind	=	false;
			for( m_iterESIN = m_mapEquippedSetItemName.begin(); m_iterESIN != m_mapEquippedSetItemName.end(); ++m_iterESIN )
			{
				std::string strCur = m_iterESIN->second;
				
				if( strcmp( itemOption.strSetName, strCur.c_str() ) == 0 )
				{
					bFind = true;
					break;
				}
			}

			if( false == bFind )
			{
				iCurrentSetItemTypeSequence = iSetItemTypeSequence++;
				m_mapEquippedSetItemName.insert( pair<int, string>(iCurrentSetItemTypeSequence, itemOption.strSetName) );
			}
			
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE


			BYTE option[2];
            int  value[2];
			for ( int j=0; j<count; ++j )
			{
				option[0] = 255; option[1] = 255;
				if ( j<standardCount )
				{
					option[0] = itemOption.byStandardOption[j][0];
                    value[0]  = itemOption.byStandardOptionValue[j][0];
					option[1] = itemOption.byStandardOption[j][1];
                    value[1]  = itemOption.byStandardOptionValue[j][1];
				}
				else if ( j<(count-standardCount) )
				{
					option[0] = itemOption.byExtOption[j];
                    value[0]  = itemOption.byExtOptionValue[j];
				}

				if ( option[0]!=255 )
				{
                    if ( option[0]<MASTERY_OPTION )
                    {
						
#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
						arruiSetItemTypeSequence[setType][optionCount[setType]] = 
																			iCurrentSetItemTypeSequence;
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE

                        bySetOptionListTmp[setType][optionCount[setType]] = option[0];
                        bySetOptionListTmp2[setType][optionCount[setType]]= RequireClass;
                        iSetOptionListValue[setType][optionCount[setType]]= value[0];
                        optionCount[setType]++;
						
                    }
                    else
                    {
			            if ( itemOption.byRequireClass[Class] && ExClass>=itemOption.byRequireClass[Class]-1 )
                        {
							
#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
							arruiSetItemTypeSequence[setType][optionCount[setType]] = 
																			iCurrentSetItemTypeSequence;
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
							
                            bySetOptionListTmp[setType][optionCount[setType]] = option[0];
                            bySetOptionListTmp2[setType][optionCount[setType]]= RequireClass;
                            iSetOptionListValue[setType][optionCount[setType]]= value[0];
                            optionCount[setType]++;
                        }
                    }
				}
				if ( option[1]!=255 )
				{
                    if ( option[1]<MASTERY_OPTION )
                    {
#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
						arruiSetItemTypeSequence[setType][optionCount[setType]] = 
																	iCurrentSetItemTypeSequence;
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE

                        bySetOptionListTmp[setType][optionCount[setType]] = option[1];
                        bySetOptionListTmp2[setType][optionCount[setType]]= RequireClass;
                        iSetOptionListValue[setType][optionCount[setType]]= value[1];
                        optionCount[setType]++;
                    }
                    else
                    {
			            if ( itemOption.byRequireClass[Class] && ExClass>=itemOption.byRequireClass[Class]-1 )
                        {
#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
							arruiSetItemTypeSequence[setType][optionCount[setType]] = 
																	iCurrentSetItemTypeSequence;
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE

                            bySetOptionListTmp[setType][optionCount[setType]] = option[1];
                            bySetOptionListTmp2[setType][optionCount[setType]]= RequireClass;
                            iSetOptionListValue[setType][optionCount[setType]]= value[1];
                            optionCount[setType]++;
                        }
                    }
				}
			}

			//	풀옵션인 경우.
			if ( count>=itemOption.byOptionCount-2 )
			{
#ifdef _VS2008PORTING
				for (int j=0; j<5; ++j )
#else // _VS2008PORTING
				for ( j=0; j<5; ++j )
#endif // _VS2008PORTING
				{
					option[0] = itemOption.byFullOption[j];
                    value[0]  = itemOption.byFullOptionValue[j];
                    if ( option[0]!=255 )
                    {
                        if ( option[0]<MASTERY_OPTION )
                        {
#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
							arruiSetItemTypeSequence[setType][optionCount[setType]] = 
															iCurrentSetItemTypeSequence;
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
                            bySetOptionListTmp[setType][optionCount[setType]] = option[0];
                            bySetOptionListTmp2[setType][optionCount[setType]]= RequireClass;
                            iSetOptionListValue[setType][optionCount[setType]]= value[0];
                            optionCount[setType]++;

                            if ( m_bySameSetItem!=0 ) m_bySameSetItem++;
                        }
                        else
                        {
                            if ( itemOption.byRequireClass[Class] && ExClass>=itemOption.byRequireClass[Class]-1 )
                            {
#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
								arruiSetItemTypeSequence[setType][optionCount[setType]] = 
															iCurrentSetItemTypeSequence;
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
								
                                bySetOptionListTmp[setType][optionCount[setType]] = option[0];
                                bySetOptionListTmp2[setType][optionCount[setType]]= RequireClass;
                                iSetOptionListValue[setType][optionCount[setType]]= value[0];
                                optionCount[setType]++;

                                if ( m_bySameSetItem!=0 ) m_bySameSetItem++;
                            }
                        }
                    }
				}

                Hero->ExtendState = 1;
			}
		}
	}

	// 각 옵션의 순번대로 1차원적으로 옵션을 기재 하여 나아간다.		// 세트 옵션의 추가 옵션만 해당
#ifdef _VS2008PORTING
	for (int i=0; i<2; ++i )
#else // _VS2008PORTING	
    for ( i=0; i<2; ++i )
#endif // _VS2008PORTING
    {
        for ( int j=0; j<optionCount[i]; ++j )
        {
#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
			m_mapEquippedSetItemSequence.insert( pair<BYTE, int>((i*optionCount[0])+j, arruiSetItemTypeSequence[i][j]) );
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
            m_bySetOptionList[(i*optionCount[0])+j][0] = bySetOptionListTmp[i][j];		// 옵션 요소 타입으로 어떤  Text 출력할지 	//ex) "힘 증가 "
            m_bySetOptionList[(i*optionCount[0])+j][1] = bySetOptionListTmp2[i][j];		//
            m_iSetOptionListValue[(i*optionCount[0])+j][0] = iSetOptionListValue[i][j];	// 증감 수치
            m_iSetOptionListValue[(i*optionCount[0])+j][1] = iSetOptionListValue[i][j];	// 증감 수치
        }
    }
    m_bySetOptionANum = optionCount[0];
    m_bySetOptionBNum = optionCount[1];
}


//////////////////////////////////////////////////////////////////////////
//	옵션의 설명 텍스트를 얻는다.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::getExplainText ( char* text, const BYTE option, const BYTE value, const BYTE SetIndex )
{
	switch ( option+AT_SET_OPTION_IMPROVE_STRENGTH )
	{
    case AT_SET_OPTION_IMPROVE_MAGIC_POWER:                   //  마력 증가.
    	sprintf( text, GlobalText[632], value );
        break;

	case AT_SET_OPTION_IMPROVE_STRENGTH: 				//	힘 증가
	case AT_SET_OPTION_IMPROVE_DEXTERITY: 			    //	민첩성 증가
	case AT_SET_OPTION_IMPROVE_ENERGY: 				    //	에너지 증가	                 
	case AT_SET_OPTION_IMPROVE_VITALITY: 				//	체력 증가
    case AT_SET_OPTION_IMPROVE_CHARISMA:                //  통솔력 증가.
    case AT_SET_OPTION_IMPROVE_ATTACK_MIN:              //  최소 공격력 증가.
    case AT_SET_OPTION_IMPROVE_ATTACK_MAX:              //  최대 공격력 증가.
		sprintf( text, GlobalText[950+option], value );
        break;

    case AT_SET_OPTION_IMPROVE_DAMAGE: 	                //	데미지 증가	 
	case AT_SET_OPTION_IMPROVE_ATTACKING_PERCENT: 		//	공격 성공률 증가  
	case AT_SET_OPTION_IMPROVE_DEFENCE: 			    //	방어력 증가		 
	case AT_SET_OPTION_IMPROVE_MAX_LIFE: 				//	최대 생명력 증가
	case AT_SET_OPTION_IMPROVE_MAX_MANA: 				//	최대 마나 증가
	case AT_SET_OPTION_IMPROVE_MAX_AG: 					//	최대 AG 증가		             
    case AT_SET_OPTION_IMPROVE_ADD_AG:                  //  AG 증가양 증가.
    case AT_SET_OPTION_IMPROVE_CRITICAL_DAMAGE_PERCENT: //  크리티컬 데미지 확률 증가.
    case AT_SET_OPTION_IMPROVE_CRITICAL_DAMAGE:         //  크리티컬 데미지 증가.
    case AT_SET_OPTION_IMPROVE_EXCELLENT_DAMAGE_PERCENT://  액설런트 데미지 확률 증가.
    case AT_SET_OPTION_IMPROVE_EXCELLENT_DAMAGE:        //  액설런트 데미지 증가.
    case AT_SET_OPTION_IMPROVE_SKILL_ATTACK:            //  스킬 공격력 증가.
    case AT_SET_OPTION_DOUBLE_DAMAGE:					//	더블 데미지 확률
		sprintf( text, GlobalText[949+option], value );
		break;					    					

    case AT_SET_OPTION_DISABLE_DEFENCE:
		sprintf( text, GlobalText[970], value );        //  적 방어력 무시.
		break;					    					

	case AT_SET_OPTION_TWO_HAND_SWORD_IMPROVE_DAMAGE: 	//	두손검 장비시 데미지 15% 증가	 
		sprintf( text, GlobalText[983], value );        
        break;

	case AT_SET_OPTION_IMPROVE_SHIELD_DEFENCE: 			//	방패 장비시 방어력 5% 증가		 
		sprintf( text, GlobalText[984], value );        
        break;


	case AT_SET_OPTION_IMPROVE_ATTACK_1: 				//	특정 캐릭터 공격력 증가 민첩성/60
	case AT_SET_OPTION_IMPROVE_ATTACK_2: 				//	특정 캐릭터 공격력 증가 힘/50	 
	case AT_SET_OPTION_IMPROVE_MAGIC: 					//	특정 캐릭터 마력 증가 에너지/70	 
//	case AT_SET_OPTION_IMPROVE_DEFENCE_1: 				//	특정 캐릭터 방어력 증가 레벨/20	 
//	case AT_SET_OPTION_IMPROVE_DEFENCE_2: 				//	특정 캐릭터 방어력 증가 3%		 
	case AT_SET_OPTION_IMPROVE_DEFENCE_3: 				//	특정 캐릭터 방어력 증가 민첩성/50
	case AT_SET_OPTION_IMPROVE_DEFENCE_4: 				//	특정 캐릭터 방어력 증가 체력/40	 
	case AT_SET_OPTION_FIRE_MASTERY: 					//	특정 캐릭터 불속성 추가			 
	case AT_SET_OPTION_ICE_MASTERY: 					//	특정 캐릭터 얼음속성 추가		 
	case AT_SET_OPTION_THUNDER_MASTERY: 				//	특정 캐릭터 번개속성 추가		 
	case AT_SET_OPTION_POSION_MASTERY: 					//	특정 캐릭터 독속성 추가			 
	case AT_SET_OPTION_WATER_MASTERY: 					//	특정 캐릭터 물속성 추가			 
	case AT_SET_OPTION_WIND_MASTERY: 					//	특정 캐릭터 바람속성 추가		 
	case AT_SET_OPTION_EARTH_MASTERY: 					//	특정 캐릭터 땅속성 추가			 
        sprintf ( text, GlobalText[971+(option+AT_SET_OPTION_IMPROVE_STRENGTH-AT_SET_OPTION_IMPROVE_ATTACK_2)], value );
		break;						
	}
}


//////////////////////////////////////////////////////////////////////////
//	등록된 옵션의 값을 얻는다.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::PlusSpecial ( WORD* Value, int Special )
{
	Special -= AT_SET_OPTION_IMPROVE_STRENGTH;
	int optionValue = 0;
	for ( int i=0; i<m_bySetOptionANum+m_bySetOptionBNum; ++i )
	{
		if ( m_bySetOptionList[i][0]==Special && m_bySetOptionListOnOff[i]==0 )
		{
			optionValue += m_iSetOptionListValue[i][0];
            m_bySetOptionListOnOff[i] = 1;
		}
	}

	if ( optionValue )
	{
		*Value += optionValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//	등록된 옵션의 %값을 얻는다.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::PlusSpecialPercent ( WORD* Value, int Special )
{
	Special -= AT_SET_OPTION_IMPROVE_STRENGTH;
	int optionValue = 0;
	for ( int i=0; i<m_bySetOptionANum+m_bySetOptionBNum; ++i )
	{
		if ( m_bySetOptionList[i][0]==Special && m_bySetOptionListOnOff[i]==0 )
		{
			optionValue += m_iSetOptionListValue[i][0];
            m_bySetOptionListOnOff[i] = 1;
		}
	}

	if ( optionValue )
	{
		*Value += ((*Value)*optionValue)/100;
	}
}


//////////////////////////////////////////////////////////////////////////
//	등록 옵션의 Level / ? 증가.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::PlusSpecialLevel ( WORD* Value, const WORD SrcValue, int Special )
{
	Special -= AT_SET_OPTION_IMPROVE_STRENGTH;
	int optionValue = 0;
	int count = 0;
	for ( int i=0; i<m_bySetOptionANum+m_bySetOptionBNum; ++i )
	{
		if ( m_bySetOptionList[i][0]==Special && m_bySetOptionList[i][1]!=0 && m_bySetOptionListOnOff[i]==0 )
		{
			optionValue = m_iSetOptionListValue[i][0];
            m_bySetOptionListOnOff[i] = 1;
			count++;
		}
	}

	if ( optionValue )
	{
		optionValue = SrcValue*optionValue/100;
		*Value += (optionValue*count);
	}
}


/////////////////////////////////////////////////////////////////////////
//  특수 캐릭터에 적용되는 옵션.
//////////////////////////////////////////////////////////////////////////
void    CSItemOption::PlusMastery ( int* Value, const BYTE MasteryType )
{
    int optionValue = 0;
	for ( int i=0; i<m_bySetOptionANum+m_bySetOptionBNum; ++i )
	{
		if ( m_bySetOptionList[i][0]>=MASTERY_OPTION && (m_bySetOptionList[i][0]-MASTERY_OPTION-5)==MasteryType && m_bySetOptionList[i][1]!=0 && m_bySetOptionListOnOff[i]==0 )
		{
            optionValue += m_iSetOptionListValue[i][0];
            m_bySetOptionListOnOff[i] = 1;
		}
	}

	if ( optionValue )
	{
		*Value += optionValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//	등록된 옵션의 %값을 얻는다.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::MinusSpecialPercent ( int* Value, int Special )
{
	Special -= AT_SET_OPTION_IMPROVE_STRENGTH;
	int optionValue = 0;
	for ( int i=0; i<m_bySetOptionANum+m_bySetOptionBNum; ++i )
	{
		if ( m_bySetOptionList[i][0]==Special && m_bySetOptionList[i][1]!=0 && m_bySetOptionListOnOff[i]==0 )
		{
			optionValue += m_iSetOptionListValue[i][0];
            m_bySetOptionListOnOff[i] = 1;
		}
	}

	if ( optionValue )
	{
		*Value -= *Value*optionValue/100;
	}
}


//////////////////////////////////////////////////////////////////////////
// 등록된 옵션의 값을 얻는다. 같은 종류의 세트 아이템을 위하여 이후 
// 영향을 주는 Flag를 설정 한다.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::GetSpecial ( WORD* Value, int Special )
{
	Special -= AT_SET_OPTION_IMPROVE_STRENGTH;
	int optionValue = 0;
	for ( int i=0; i<m_bySetOptionANum+m_bySetOptionBNum; ++i )
	{
		if ( m_bySetOptionList[i][0]==Special && m_bySetOptionListOnOff[i]==0 )
		{
			optionValue += m_iSetOptionListValue[i][0];
            m_bySetOptionListOnOff[i] = 1;
		}
	}

	if ( optionValue )
	{
		*Value += optionValue;
	}
}

//////////////////////////////////////////////////////////////////////////
//	등록된 옵션의 %값을 얻는다.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::GetSpecialPercent ( WORD* Value, int Special )
{
	Special -= AT_SET_OPTION_IMPROVE_STRENGTH;
	int optionValue = 0;
	for ( int i=0; i<m_bySetOptionANum+m_bySetOptionBNum; ++i )
	{
		if ( m_bySetOptionList[i][0]==Special && m_bySetOptionListOnOff[i]==0 )
		{
			optionValue += m_iSetOptionListValue[i][0];
            m_bySetOptionListOnOff[i] = 1;
		}
	}

	if ( optionValue )
	{
		*Value += *Value*optionValue/100;
	}
}


//////////////////////////////////////////////////////////////////////////
//	등록 옵션의 Level / ? 증가.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::GetSpecialLevel ( WORD* Value, const WORD SrcValue, int Special )
{
	Special -= AT_SET_OPTION_IMPROVE_STRENGTH;
	int optionValue = 0;
	int count = 0;
	for ( int i=0; i<m_bySetOptionANum+m_bySetOptionBNum; ++i )
	{
		if ( m_bySetOptionList[i][0]==Special && m_bySetOptionList[i][1]!=0 && m_bySetOptionListOnOff[i]==0 )
		{
			optionValue = m_iSetOptionListValue[i][0];
            m_bySetOptionListOnOff[i] = 1;
			count++;
		}
	}

	if ( optionValue )
	{
		optionValue = SrcValue*optionValue/100;
		*Value += (optionValue*count);
	}
}


//////////////////////////////////////////////////////////////////////////
//  세트 아이템의 기본 옵션 값을 알려준다.
//////////////////////////////////////////////////////////////////////////
int     CSItemOption::GetDefaultOptionValue ( ITEM* ip, WORD* Value )
{
    *Value = ((ip->ExtOption>>2)%0x04);

    ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];

    return p->AttType;
}


//////////////////////////////////////////////////////////////////////////
//  세트 아이템의 기본 옵션을 표시한다.
//////////////////////////////////////////////////////////////////////////
bool    CSItemOption::GetDefaultOptionText ( const ITEM* ip, char* Text )
{
    if ( ((ip->ExtOption>>2)%0x04)<=0 ) return false;

    switch ( ItemAttribute[ip->Type].AttType )
    {
    case SET_OPTION_STRENGTH :      //  힘 옵션 증가.
        sprintf ( Text, GlobalText[950], ((ip->ExtOption>>2)%0x04)*5 );
        break;

    case SET_OPTION_DEXTERITY :     //  민첩 옵션 증가.
        sprintf ( Text, GlobalText[951], ((ip->ExtOption>>2)%0x04)*5 );
        break;

    case SET_OPTION_ENERGY :        //  에너지 옵션 증가.
        sprintf ( Text, GlobalText[952], ((ip->ExtOption>>2)%0x04)*5 );
        break;

    case SET_OPTION_VITALITY :      //  체력 옵션 증가.
        sprintf ( Text, GlobalText[953], ((ip->ExtOption>>2)%0x04)*5 );
        break;

    default :
        return false;
    }
    return true;
}


#ifdef PJH_FIX_4_BUGFIX_33
bool     CSItemOption::Special_Option_Check(int Kind)
{
	int i,j;
	for(i = 0; i < 2; i++)
	{
		ITEM* item = NULL;
		item = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT + i];
		if(item == NULL || item->Type <= -1)
			continue;

		if(Kind == 0)	//파워슬레쉬 스킬.
		{
			for(j=0;j<item->SpecialNum;j++)
			{
				if(item->Special[j] == AT_SKILL_ICE_BLADE)
					return true;
			}
		}
		else
		if(Kind == 1)
		{
			for(j=0;j<item->SpecialNum;j++)
			{
				if(item->Special[j] == AT_SKILL_CROSSBOW)
					return true;
			}
		}
	}
	return false;
}
#endif //PJH_FIX_4_BUGFIX_33

//////////////////////////////////////////////////////////////////////////
//  기본 세트 능력치를 화면에 표시.
//////////////////////////////////////////////////////////////////////////
int     CSItemOption::RenderDefaultOptionText ( const ITEM* ip, int TextNum )
{
    int TNum = TextNum;
    if ( GetDefaultOptionText( ip, TextList[TNum] ) )
    {
        TextListColor[TNum] = TEXT_COLOR_BLUE;
        TNum++;

        if ( ( ip->Type>=ITEM_HELPER+8 && ip->Type<=ITEM_HELPER+9 ) || ( ip->Type>=ITEM_HELPER+12 && ip->Type<=ITEM_HELPER+13 ) || ( ip->Type>=ITEM_HELPER+21 && ip->Type<=ITEM_HELPER+27 ) )
        {
            sprintf ( TextList[TNum], GlobalText[1165] );
            TextListColor[TNum] = TEXT_COLOR_BLUE;
            TNum++;
        }
    }

    return TNum;
}


//////////////////////////////////////////////////////////////////////////
//  모든 추가 스탯을 저장한다.
//////////////////////////////////////////////////////////////////////////
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION 
void    CSItemOption::getAllAddState ( WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma )
#else // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
void    CSItemOption::getAllAddState ( WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality )
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
{
    for ( int i=EQUIPMENT_WEAPON_RIGHT; i<MAX_EQUIPMENT; ++i )
    {
		ITEM* item = &CharacterMachine->Equipment[i];
		
        if ( item->Durability<=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM
			|| ( IsRequireEquipItem( item ) == false )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM
			)
        {
            continue;
        }
		
        WORD Result = 0;
        switch ( GetDefaultOptionValue( item, &Result ) )
        {
        case SET_OPTION_STRENGTH:
            *Strength += Result*5;
            break;
			
        case SET_OPTION_DEXTERITY:
            *Dexterity += Result*5;
            break;
			
        case SET_OPTION_ENERGY:
            *Energy += Result*5;
            break;
			
        case SET_OPTION_VITALITY:
            *Vitality += Result*5;
            break;
        }
    }

	GetSpecial ( Strength,  AT_SET_OPTION_IMPROVE_STRENGTH );	    //	힘 증가
	GetSpecial ( Dexterity, AT_SET_OPTION_IMPROVE_DEXTERITY );	    //	민첩성 증가
	GetSpecial ( Energy,    AT_SET_OPTION_IMPROVE_ENERGY );        //	에너지 증가
	GetSpecial ( Vitality,  AT_SET_OPTION_IMPROVE_VITALITY );	    //	체력 증가
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION 
	GetSpecial ( Charisma,  AT_SET_OPTION_IMPROVE_CHARISMA );		//  통솔 증가
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
}



#ifdef LDS_FIX_WRONG_CALCULATEEQUIPEDITEMOPTIONVALUE
//////////////////////////////////////////////////////////////////////////
// 모든 스탯을 0으로 초기화 한뒤에 옵션 증감 수치들만 반환한다.
//////////////////////////////////////////////////////////////////////////
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	void	CSItemOption::getAllAddStateOnlyAddValue ( WORD* AddStrength, WORD* AddDexterity, WORD* AddEnergy, WORD* AddVitality, WORD* AddCharisma )
	{
		*AddStrength = *AddDexterity = *AddEnergy = *AddVitality = *AddCharisma = 0;
		memset ( m_bySetOptionListOnOff, 0, sizeof( BYTE )* 16 );
		
		getAllAddState( AddStrength, AddDexterity, AddEnergy, AddVitality, AddCharisma );
	}	
#else // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	void	CSItemOption::getAllAddStateOnlyAddValue ( WORD* AddStrength, WORD* AddDexterity, WORD* AddEnergy, WORD* AddVitality )
	{
		*AddStrength = *AddDexterity = *AddEnergy = *AddVitality = 0;
		memset ( m_bySetOptionListOnOff, 0, sizeof( BYTE )* 16 );
		
		getAllAddState( AddStrength, AddDexterity, AddEnergy, AddVitality );
	}
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION

//////////////////////////////////////////////////////////////////////////
//  모든 추가 스탯을 각 스탯별 비교 수치 와 연산된 값만 가감 하여 반환 한다.
//////////////////////////////////////////////////////////////////////////
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
void    CSItemOption::getAllAddOptionStatesbyCompare ( WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma,
													  WORD iCompareStrength, WORD iCompareDexterity, WORD iCompareEnergy, WORD iCompareVitality, WORD iCompareCharisma )
#else // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
void    CSItemOption::getAllAddOptionStatesbyCompare ( WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality,
													  WORD iCompareStrength, WORD iCompareDexterity, WORD iCompareEnergy, WORD iCompareVitality )
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
{	
    for ( int i=EQUIPMENT_WEAPON_RIGHT; i<MAX_EQUIPMENT; ++i )
    {
		ITEM* item = &CharacterMachine->Equipment[i];
		
		if( item->RequireStrength > iCompareStrength || 
			item->RequireDexterity > iCompareDexterity || 
			item->RequireEnergy > iCompareEnergy )
		{
			continue;
		}
		
        if ( item->Durability<=0 
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM
			|| ( IsRequireEquipItem( item ) == false )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM
			)
        {
            continue;
        }
		
        WORD Result = 0;
        switch ( GetDefaultOptionValue( item, &Result ) )
        {
        case SET_OPTION_STRENGTH:
            *Strength += Result*5;
            break;
			
        case SET_OPTION_DEXTERITY:
            *Dexterity += Result*5;
            break;
			
        case SET_OPTION_ENERGY:
            *Energy += Result*5;
            break;
			
        case SET_OPTION_VITALITY:
            *Vitality += Result*5;
            break;
        }
    }

	memset ( m_bySetOptionListOnOff, 0, sizeof( BYTE )* 16 );
	
	GetSpecial ( Strength,  AT_SET_OPTION_IMPROVE_STRENGTH );	    //	힘 증가
	GetSpecial ( Dexterity, AT_SET_OPTION_IMPROVE_DEXTERITY );	    //	민첩성 증가
	GetSpecial ( Energy,    AT_SET_OPTION_IMPROVE_ENERGY );			//	에너지 증가
	GetSpecial ( Vitality,  AT_SET_OPTION_IMPROVE_VITALITY );	    //	체력 증가
	
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	GetSpecial ( Charisma,  AT_SET_OPTION_IMPROVE_CHARISMA );	    //	통솔 증가
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
}
#endif // LDS_FIX_WRONG_CALCULATEEQUIPEDITEMOPTIONVALUE


//////////////////////////////////////////////////////////////////////////
//	장비창에 장착한 장비들의 세트 옵션을 체크한다.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::CheckItemSetOptions ( void )
{
	BYTE byOptionList[30] = { 0, };
    ITEM itemTmp;

    memset ( m_bySetOptionList, 0, sizeof( BYTE ) * 16 );

#ifdef LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
	for ( int i=0; i<MAX_EQUIPMENT_INDEX; ++i )
#else // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
	for ( int i=0; i<12; ++i )
#endif // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
	{	
		if ( i==EQUIPMENT_WING || i==EQUIPMENT_HELPER ) continue;

		ITEM *ip = &CharacterMachine->Equipment[i];

#ifdef LJH_FIX_ITEM_CANNOT_BE_EQUIPPED_AFTER_LOGOUT
		if( ip->Durability <= 0 ) {
			continue;
		}
#else	//LJH_FIX_ITEM_CANNOT_BE_EQUIPPED_AFTER_LOGOUT
#if defined PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM || defined YDG_FIX_SETITEM_REQUIRED_STATUS_BUG
		if( ip->Durability <= 0 || IsRequireEquipItem( ip ) == false ) {
			continue;
		}
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM
#endif	//LJH_FIX_ITEM_CANNOT_BE_EQUIPPED_AFTER_LOGOUT

        //  같은 무기 같은 세트는 세트 옵션에서 제외.
        if ( ( i==EQUIPMENT_WEAPON_LEFT || i==EQUIPMENT_RING_LEFT ) && itemTmp.Type==ip->Type && itemTmp.ExtOption==(ip->ExtOption%0x04) )
        {
            continue;
        }

		if ( ip->Type>-1 )
		{
			checkItemType( byOptionList, ip->Type, ip->ExtOption );
		}

        if ( i==EQUIPMENT_WEAPON_RIGHT || i==EQUIPMENT_RING_RIGHT )
        {
            itemTmp.Type = ip->Type;
            itemTmp.ExtOption = (ip->ExtOption%0x04);
        }
	}

	calcSetOptionList( byOptionList );

#ifdef LJH_FIX_ITEM_CANNOT_BE_EQUIPPED_AFTER_LOGOUT
	// 통솔은 증가가 빠져 있음
	// 하지만 나중에 추가 할때 추가 하는게 좋을듯.
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	getAllAddStateOnlyAddValue ( &CharacterAttribute->AddStrength,	&CharacterAttribute->AddDexterity, 
								 &CharacterAttribute->AddEnergy,	&CharacterAttribute->AddVitality, 
								 &CharacterAttribute->AddCharisma 
							   );
#else // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
    getAllAddStateOnlyAddValue ( &CharacterAttribute->AddStrength,	&CharacterAttribute->AddDexterity, 
								 &CharacterAttribute->AddEnergy,	&CharacterAttribute->AddVitality 
							   );
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION

#if defined PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM || defined LDK_FIX_USE_SECRET_ITEM_TO_SETITEM_OPTION_BUG
    //  총 추가 스탯을 계산한다
	// 통솔, 레벨을 추가 하였다.
    WORD AllStrength  = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
    WORD AllDexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
    WORD AllEnergy    = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
    WORD AllVitality  = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
	WORD AllCharisma  = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
	WORD AllLevel     = CharacterAttribute->Level;
#else //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM
    //  총 추가 스탯을 계산한다.
    WORD AllStrength = CharacterAttribute->Strength;
    WORD AllDexterity = CharacterAttribute->Dexterity;
    WORD AllEnergy = CharacterAttribute->Energy;
    WORD AllVitality = CharacterAttribute->Vitality;
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	WORD AllCharisma  =CharacterAttribute->Charisma;
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM

#else //LJH_FIX_ITEM_CANNOT_BE_EQUIPPED_AFTER_LOGOUT
#if defined PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM || defined LDK_FIX_USE_SECRET_ITEM_TO_SETITEM_OPTION_BUG
    //  총 추가 스탯을 계산한다
	// 통솔, 레벨을 추가 하였다.
    WORD AllStrength  = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
    WORD AllDexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
    WORD AllEnergy    = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
    WORD AllVitality  = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
	WORD AllCharisma  = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
	WORD AllLevel     = CharacterAttribute->Level;
#else //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM
    //  총 추가 스탯을 계산한다.
    WORD AllStrength = CharacterAttribute->Strength;
    WORD AllDexterity = CharacterAttribute->Dexterity;
    WORD AllEnergy = CharacterAttribute->Energy;
    WORD AllVitality = CharacterAttribute->Vitality;
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	WORD AllCharisma  =CharacterAttribute->Charisma;
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM

	// 통솔은 증가가 빠져 있음
	// 하지만 나중에 추가 할때 추가 하는게 좋을듯.
#ifdef LDS_ADD_CHARISMAVALUE_TOITEMOPTION
	getAllAddState ( &AllStrength, &AllDexterity, &AllEnergy, &AllVitality, &AllCharisma );
#else // LDS_ADD_CHARISMAVALUE_TOITEMOPTION
    getAllAddState ( &AllStrength, &AllDexterity, &AllEnergy, &AllVitality );
#endif // LDS_ADD_CHARISMAVALUE_TOITEMOPTION

#endif	//LJH_FIX_ITEM_CANNOT_BE_EQUIPPED_AFTER_LOGOUT

    //  다시 옵션을 검사한다.
    memset ( byOptionList, 0, sizeof( BYTE ) * 30 );
    memset ( m_bySetOptionList, 255, sizeof( BYTE ) * 16 );


#ifdef LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
#ifdef _VS2008PORTING
	for (int i=0; i<MAX_EQUIPMENT_INDEX; ++i )
#else // _VS2008PORTING
	for ( i=0; i<MAX_EQUIPMENT_INDEX; ++i )
#endif // _VS2008PORTING
#else // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
	for ( i=0; i<12; ++i )
#endif // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
	{
		if ( i==EQUIPMENT_WING || i==EQUIPMENT_HELPER ) continue;

		ITEM *ip = &CharacterMachine->Equipment[i];

        if ( ip->RequireDexterity>AllDexterity || ip->RequireEnergy>AllEnergy || ip->RequireStrength>AllStrength 
#if defined PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM || defined YDG_FIX_SETITEM_REQUIRED_STATUS_BUG
			// 통솔은 비교 안 해도 될듯 하지만 추가 될 경우 넣어 둬도 상관 없을듯.
			|| ip->RequireLevel > AllLevel || ip->RequireCharisma > AllCharisma || ip->Durability <= 0 || ( IsRequireEquipItem( ip ) == false )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_SETITEM
			)
        {
            continue;
        }

        //  같은 무기 같은 세트는 세트 옵션에서 제외.
        if ( ( i==EQUIPMENT_WEAPON_LEFT || i==EQUIPMENT_RING_LEFT ) && itemTmp.Type==ip->Type && itemTmp.ExtOption==(ip->ExtOption%0x04) )
        {
            continue;
        }

		if ( ip->Type>-1 )
		{
			checkItemType( byOptionList, ip->Type, ip->ExtOption );
		}

        if ( i==EQUIPMENT_WEAPON_RIGHT || i==EQUIPMENT_RING_RIGHT )
        {
            itemTmp.Type = ip->Type;
            itemTmp.ExtOption = (ip->ExtOption%0x04);
        }
	}

#ifdef LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
	// 장착된 아이템들의 세트 아이템만 골라내어 출력할 추가 옵션의 갯수를 설정하여 반환합니다. 
	UpdateCount_SetOptionPerEquippedSetItem( byOptionList, m_arLimitSetItemOptionCount, CharacterMachine->Equipment );
#endif // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE

	calcSetOptionList( byOptionList );
}


//////////////////////////////////////////////////////////////////////////
//	세트 아이템 처리.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::MoveSetOptionList ( const int StartX, const int StartY )
{
    int x,y,Width,Height;
	
	Width=162; Height=20; x=StartX+14; y=StartY+22;
	if ( MouseX>=x && MouseX<x+Width && MouseY>=y && MouseY<y+Height )//&& MouseLButtonPush )
	{
		m_bViewOptionList = true;

		MouseLButtonPush = false;
		MouseUpdateTime = 0;
		MouseUpdateTimeMax = 6;
	}
}


//////////////////////////////////////////////////////////////////////////
//	세트 아이템 정보를 화면에 보여준다.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::RenderSetOptionButton ( const int StartX, const int StartY )
{
    float x,y,Width,Height;
    char  Text[100];

	Width=162.f;Height=20.f; x=(float)StartX+14;y=(float)StartY+22;
	RenderBitmap(BITMAP_INTERFACE_EX+21,x,y,Width,Height,0.f,0.f,Width/256.f,Height/32.f);

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(0, 0, 0, 255);
	g_pRenderText->SetBgColor(100, 0, 0, 0);
	sprintf ( Text, "[%s]", GlobalText[989] );
    g_pRenderText->RenderText(StartX+96, (int)(y+3), Text, 0, 0, RT3_WRITE_CENTER);

	g_pRenderText->SetTextColor(0xffffffff);
	if ( m_bySetOptionANum>0 || m_bySetOptionBNum>0 )
		g_pRenderText->SetTextColor(255, 204, 25, 255);
    else
		g_pRenderText->SetTextColor(128, 128, 128, 255);
    g_pRenderText->RenderText(StartX+95, (int)(y+2), Text, 0, 0, RT3_WRITE_CENTER);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
}


//////////////////////////////////////////////////////////////////////////
//	세트 아이템 정보를 화면에 보여준다.
//////////////////////////////////////////////////////////////////////////
void	CSItemOption::RenderSetOptionList ( const int StartX, const int StartY )
{
	if ( m_bViewOptionList && ( m_bySetOptionANum>0 || m_bySetOptionBNum>0 ) )
	{
		g_pRenderText->SetTextColor(255, 255, 255, 255);
		g_pRenderText->SetBgColor(100, 0, 0, 0);

        int PosX, PosY;

        PosX = StartX+95;//+60;
        PosY = StartY+40;

        BYTE TextNum = 0;
        BYTE SkipNum = 0;
        BYTE setIndex = 0;

        sprintf ( TextList[TextNum], "\n" ); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++; SkipNum++;
        sprintf ( TextList[TextNum], "\n" ); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++; SkipNum++;
        sprintf ( TextList[TextNum], "\n" ); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++; SkipNum++;

#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
		int		iCurSetItemTypeSequence = 0, iCurSetItemType = -1;
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE

		for ( int i=0; i<m_bySetOptionANum+m_bySetOptionBNum; ++i )
		{
#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
			std::string	strCurrent;

			// For TEST
			//unsigned int uiSizeESIS = m_mapEquippedSetItemSequence.size();
			// For TEST

			m_iterESIS = m_mapEquippedSetItemSequence.find( i );

			if( m_mapEquippedSetItemSequence.end() != m_iterESIS )
			{
				iCurSetItemTypeSequence = m_iterESIS->second;
			}
			else
			{
				iCurSetItemTypeSequence = -1;
			}

			if( iCurSetItemType != iCurSetItemTypeSequence )
			{
				// For TEST
				//unsigned int uiSizeSetItemName = m_mapEquippedSetItemName.size();
				// For TEST

				iCurSetItemType = iCurSetItemTypeSequence;

				m_iterESIN = m_mapEquippedSetItemName.find( iCurSetItemTypeSequence );

				if( m_iterESIN != m_mapEquippedSetItemName.end() )
				{
					strCurrent = m_iterESIN->second;
					
					// 1089 "세트"
					sprintf ( TextList[TextNum], "%s %s", strCurrent.c_str(), GlobalText[1089] );
					TextListColor[TextNum] = 3;
					TextBold[TextNum] = true;
					TextNum++;
				}
			}
			
#else // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
            if ( i==0 && m_bySetOptionANum>0 )
            {
				// 1089 "세트"
                sprintf ( TextList[TextNum], "%s %s", m_strSetName[0], GlobalText[1089] );
       		    TextListColor[TextNum] = 3;
			    TextBold[TextNum] = true;
                TextNum++;
            }
            else if ( i==m_bySetOptionANum && m_bySetOptionBNum>0 )
            {
                setIndex = 1;

                sprintf ( TextList[TextNum], "\n" );
       		    TextListColor[TextNum] = 0;
			    TextBold[TextNum] = false;
                TextNum++; SkipNum++;

				// 1089 "세트"
                sprintf ( TextList[TextNum], "%s %s", m_strSetName[1], GlobalText[1089] );
       		    TextListColor[TextNum] = 3;
			    TextBold[TextNum] = true;
                TextNum++;
            }
            else if ( m_bySameSetItem!=0 && m_bySetOptionANum>0 && (m_bySetOptionANum-m_bySameSetItem)==i )
            {
                sprintf ( TextList[TextNum], "\n" );
       		    TextListColor[TextNum] = 0;
			    TextBold[TextNum] = false;
                TextNum++; SkipNum++;

				// 1089 "세트"
                sprintf ( TextList[TextNum], "%s %s", m_strSetName[1], GlobalText[1089] );
       		    TextListColor[TextNum] = 3;
			    TextBold[TextNum] = true;
                TextNum++;
            }
            else if ( m_bySameSetItem!=0 && m_bySetOptionBNum>0 && (m_bySetOptionBNum-m_bySameSetItem)==i )
            {
                sprintf ( TextList[TextNum], "\n" );
       		    TextListColor[TextNum] = 0;
			    TextBold[TextNum] = false;
                TextNum++; SkipNum++;

				// 1089 "세트"
                sprintf ( TextList[TextNum], "%s %s", m_strSetName[0], GlobalText[1089] );
       		    TextListColor[TextNum] = 3;
			    TextBold[TextNum] = true;
                TextNum++;
            }
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE
			getExplainText ( TextList[TextNum], m_bySetOptionList[i][0], m_iSetOptionListValue[i][0], setIndex );
            if ( m_bySetOptionList[i][0]>=AT_SET_OPTION_IMPROVE_ATTACK_1 && m_bySetOptionList[i][1]==0 )
                TextListColor[TextNum] = 2;
            else
                TextListColor[TextNum] = 1;
			TextBold[TextNum] = false;
            TextNum++;
		}

		RenderTipTextList( PosX, PosY, TextNum, 120, RT3_SORT_CENTER );
        m_bViewOptionList = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//  찾는 /NAME이 원하는 세트인지 검사한다.
//////////////////////////////////////////////////////////////////////////
void    CSItemOption::CheckRenderOptionHelper ( const char* FilterName )
{
    char Name[256];

    if ( FilterName[0]!='/' ) return;

    int Length1 = strlen ( FilterName );
    for ( int i=0; i<MAX_SET_OPTION; ++i )
    {
        ITEM_SET_OPTION& setOption = m_ItemSetOption[i];
        if ( setOption.byOptionCount<255 )
        {
            sprintf ( Name,"/%s", setOption.strSetName );

            int Length2 = strlen ( Name );

            //  검색한다.
            m_byRenderOptionList = 0;
            if ( strncmp( FilterName, Name, Length1 )==NULL && strncmp( FilterName, Name, Length2 )==NULL )
            {
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_HELP);
				g_pNewUISystem->Show(SEASON3B::INTERFACE_SETITEM_EXPLANATION);

                m_byRenderOptionList = i+1;
                return;
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//  찾아는 세트 아이템의 모든 옵션을 표시한다.
//////////////////////////////////////////////////////////////////////////
void    CSItemOption::RenderOptionHelper ( void )
{
    if ( m_byRenderOptionList<=0 ) return;

	int TextNum = 0;
    int sx = 0, sy = 0;
	ZeroMemory( TextListColor, 20 * sizeof ( int));
	for(int i=0;i<30;i++)
	{
		TextList[i][0] = NULL;
	}

    ITEM_SET_OPTION& setOption = m_ItemSetOption[m_byRenderOptionList-1];
    if ( setOption.byOptionCount>=255 )
    {
        m_byRenderOptionList = 0;
        return;
    }

    BYTE    option1 = 255;
    BYTE    option2 = 255;
    BYTE    value1  = 255;
    BYTE    value2  = 255;
	sprintf(TextList[TextNum],"\n");TextNum++;
    sprintf ( TextList[TextNum], "%s %s %s", setOption.strSetName, GlobalText[1089], GlobalText[159] );
    TextListColor[TextNum] = TEXT_COLOR_YELLOW; 
    TextNum++;

	sprintf(TextList[TextNum],"\n");TextNum++;
	sprintf(TextList[TextNum],"\n");TextNum++;
#ifdef _VS2008PORTING
	for ( int i=0; i<13; ++i )
#else // _VS2008PORTING
    for ( i=0; i<13; ++i )
#endif // _VS2008PORTING
    {
        if ( i<6 )
        {
            option1 = setOption.byStandardOption[i][0];
            option2 = setOption.byStandardOption[i][1];
            value1  = setOption.byStandardOptionValue[i][0];
            value2  = setOption.byStandardOptionValue[i][1];
        }
        else if ( i<8 )
        {
            option1 = setOption.byExtOption[i-6];
            value1  = setOption.byExtOptionValue[i-6];
        }
        else
        {
            option1 = setOption.byFullOption[i-8];
            value1  = setOption.byFullOptionValue[i-8];
        }
        if ( option1!=255 )
        {
            getExplainText ( TextList[TextNum], option1, value1, 0 );
            TextListColor[TextNum] = TEXT_COLOR_BLUE; 
            TextBold[TextNum] = false; TextNum++;
        }
        if ( option2!=255 )
        {
            getExplainText ( TextList[TextNum], option2, value2, 0 );
            TextListColor[TextNum] = TEXT_COLOR_BLUE; 
            TextBold[TextNum] = false; TextNum++;
        }
    }
	sprintf(TextList[TextNum],"\n");TextNum++;
	sprintf(TextList[TextNum],"\n");TextNum++;

	SIZE TextSize = {0, 0};	
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int Height = (int)( ((TextNum ) * TextSize.cy + TextSize.cy / 2) / g_fScreenRate_y );
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	RenderTipTextList(sx,sy,TextNum,0);
}

int CSItemOption::GetSetItmeCount( const ITEM* pselecteditem )
{
	ITEM_SET_TYPE& itemsettype = m_ItemSetType[pselecteditem->Type];
	BYTE subtype = itemsettype.byOption[(pselecteditem->ExtOption%0x04)-1];

	int setitemcount = 0;

	//여긴 반드시 수정이 필요함...
	for( int j = 0; j < MAX_ITEM; j++ )
	{
		ITEM_SET_TYPE& temptype = m_ItemSetType[j];
#ifdef PJH_ADD_SET_NEWJOB
		for(int i = 0; i < 2; i++)
#endif //PJH_ADD_SET_NEWJOB
		{
#ifdef PJH_ADD_SET_NEWJOB
			BYTE tempsubtype = temptype.byOption[i];
#else
		BYTE tempsubtype = temptype.byOption[(pselecteditem->ExtOption%0x04)-1];
#endif //PJH_ADD_SET_NEWJOB
			if( subtype == tempsubtype )
			{
				setitemcount++;
			}
		}
	}

	return setitemcount;
}

bool CSItemOption::isFullseteffect( const ITEM* pselecteditem )
{
	int mysetitemcount = 0;

	ITEM_SET_TYPE& selectedItemType = m_ItemSetType[pselecteditem->Type];
	BYTE selectedItemOption = selectedItemType.byOption[(pselecteditem->ExtOption%0x04)-1];
	ITEM_SET_OPTION& selecteditemoption = m_ItemSetOption[selectedItemOption];
#ifdef PJH_ADD_SET_NEWJOB
	int	Cmp_Buff[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
#endif //PJH_ADD_SET_NEWJOB
	
	for( int i = 0; i < MAX_EQUIPMENT; i++ )
	{
		ITEM *p = &CharacterMachine->Equipment[i];

		if( p )
		{
#ifdef PJH_ADD_SET_NEWJOB
			bool Continue_Set = false;
			for(int ipjh = 0; ipjh < mysetitemcount; ipjh++)
			{
				if(p->Type == Cmp_Buff[ipjh])
				{
					Continue_Set = true;
					break;
				}
			}
			if(Continue_Set == true)
				continue;
#endif //#ifdef PJH_ADD_SET_NEWJOB
			ITEM_SET_TYPE& myitemSType = m_ItemSetType[p->Type];
			BYTE myItemOption = myitemSType.byOption[(p->ExtOption%0x04)-1];
			ITEM_SET_OPTION& setOption = m_ItemSetOption[myItemOption];

			if ( strcmp( selecteditemoption.strSetName, setOption.strSetName ) == NULL )
			{
#ifdef PJH_ADD_SET_NEWJOB
				Cmp_Buff[mysetitemcount] = p->Type;
#endif //#ifdef PJH_ADD_SET_NEWJOB
				mysetitemcount++;
			}
		}
	}

	if( mysetitemcount == GetSetItmeCount( pselecteditem ) )
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////
//  선택한 아이템의 모든 옵션을 표시한다.
//////////////////////////////////////////////////////////////////////////
#ifdef LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY
int     CSItemOption::RenderSetOptionListInItem ( const ITEM* ip, int TextNum, bool bIsEquippedItem )
#else // LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY
int     CSItemOption::RenderSetOptionListInItem ( const ITEM* ip, int TextNum )
#endif // LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY
{
    ITEM_SET_TYPE& itemSType= m_ItemSetType[ip->Type];

    m_bySelectedItemOption = itemSType.byOption[(ip->ExtOption%0x04)-1];

    if ( m_bySelectedItemOption<=0 || m_bySelectedItemOption==255 ) return TextNum;

    int TNum = TextNum;

    ITEM_SET_OPTION& setOption = m_ItemSetOption[m_bySelectedItemOption];
    if ( setOption.byOptionCount>=255 )
    {
        m_bySelectedItemOption = 0;
        return TNum;
    }

    BYTE    option1 = 255;
    BYTE    option2 = 255;
    BYTE    value1  = 255;
    BYTE    value2  = 255;
    BYTE    count1  = 0;

    BYTE    byLimitOptionNum;
#ifdef PJH_ADD_SET_NEWJOB
	if(m_bySetOptionANum > 0)
		byLimitOptionNum = m_bySetOptionANum-m_bySameSetItem;//m_bySetOptionANum-1;
	else
		byLimitOptionNum = 0;

	if(m_bySetOptionBNum > 0)
		byLimitOptionNum += m_bySetOptionBNum-m_bySameSetItem;//m_bySetOptionANum-1;
#else
	byLimitOptionNum = m_bySetOptionANum-m_bySameSetItem;//m_bySetOptionANum-1;
#endif


	// 현재 2개 이상 착용중인 세트 아이템들에 대한 해당 세트 아이템의 Sequence Index와 출력될 추가 옵션 수
#ifdef LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE

	count1 = Search_From_EquippedSetItemNameSequence( setOption.strSetName );

	if( BYTE_MAX == count1 )
	{
		byLimitOptionNum = 0;
	}	
	else
	{
		byLimitOptionNum = abs( (m_bySetOptionANum + m_bySetOptionBNum) - m_bySameSetItem);
	}

#else  // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE	
    int     length1 = strlen ( setOption.strSetName );
    int     length2 = strlen ( m_strSetName[1] );
    int     length3 = strlen ( m_strSetName[0] );
    if ( !strncmp( setOption.strSetName, m_strSetName[1], length1 ) &&  !strncmp( m_strSetName[1], setOption.strSetName, length2 ) )
    {
        count1 = m_bySetOptionANum-m_bySameSetItem;
#ifdef PJH_ADD_SET_NEWJOB
		if(m_bySetOptionANum-m_bySameSetItem < 0)
			count1 = 0;
#else
        byLimitOptionNum = count1+m_bySameSetItem;
#endif //PJH_ADD_SET_NEWJOB
    }
    else
#ifdef PJH_ADD_SET_NEWJOB		
		if ( !strncmp( setOption.strSetName, m_strSetName[0], length1 ) && !strncmp( m_strSetName[0], setOption.strSetName, length3 ) )
#else
		if ( strncmp( setOption.strSetName, m_strSetName[0], length1 ) && strncmp( m_strSetName[0], setOption.strSetName, length3 ) )
#endif //#ifdef PJH_ADD_SET_NEWJOB
    {
#ifdef PJH_ADD_SET_NEWJOB
			if(m_bySetOptionANum > 0)
				count1 = 0;
			else
				count1 = (m_bySetOptionANum + m_bySetOptionBNum) -m_bySameSetItem;

			byLimitOptionNum = abs(m_bySetOptionANum - m_bySameSetItem);
#else
		byLimitOptionNum = 0;
#endif //PJH_ADD_SET_NEWJOB
    }
#ifdef PJH_ADD_SET_NEWJOB
	else
		byLimitOptionNum = 0;
#endif // LDS_FIX_OUTPUT_EQUIPMENTSETITEMOPTIONVALUE

	
#endif //PJH_ADD_SET_NEWJOB
	sprintf ( TextList[TNum], "\n" ); TNum += 1;
    sprintf ( TextList[TNum], "%s %s", GlobalText[1089], GlobalText[159] );
    TextListColor[TNum] = TEXT_COLOR_YELLOW; 
    TNum++;

	sprintf(TextList[TNum],"\n");TNum++;
	sprintf(TextList[TNum],"\n");TNum++;

	bool isfulloption = isFullseteffect( ip );
#ifdef PJH_ADD_SET_NEWJOB
	if( isfulloption )
	{
		byLimitOptionNum = 13;
	}
#endif //#ifdef PJH_ADD_SET_NEWJOB


#ifdef LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE

#ifdef LDS_FIX_OUTPUT_WRONG_EQUIPPEDSETITEMOPTION_BY_WRONG_SLOTINDEX
	// 선택Item 판별 위한 인덱스를 반지 인경우만 bySelectedSlotIndex로 참조 합니다.
	//  기존 ip->Type을 참조 하였으나 ip->Type 에서 반지의 경우만 모두 오른손 반지(10)로 잘
	//	못된 값이 넘어오기 때문입니다.
	BYTE byCurrentSelectedSlotIndex = ip->bySelectedSlotIndex;

	int iLimitOptionCount = m_arLimitSetItemOptionCount[byCurrentSelectedSlotIndex]-1;
#else // LDS_FIX_OUTPUT_WRONG_EQUIPPEDSETITEMOPTION_BY_WRONG_SLOTINDEX
	// 현재 세트 아이템 옵션의 출력 숫자 제한 
	int iLimitOptionCount = m_arLimitSetItemOptionCount[ip->bySelectedSlotIndex]-1;
#endif // LDS_FIX_OUTPUT_WRONG_EQUIPPEDSETITEMOPTION_BY_WRONG_SLOTINDEX

#endif // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
	

	// (세트)아이템의 모든 추가 옵션 출력. i 하나당 라인 한줄
#ifdef LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
#ifdef LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY
	for ( int i=0; i<=MAX_SETITEM_OPTIONS; ++i )	// 아이템 추가옵션 갯수 제한 수치 반영
#else // LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY
	for ( int i=0; i<=MAX_EQUIPMENT_INDEX; ++i )
#endif // LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY
#else // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
    for ( int i=0; i<13; ++i )
#endif // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE	
    {
        if ( i<6 )
        {
            option1 = setOption.byStandardOption[i][0];
            option2 = setOption.byStandardOption[i][1];
            value1  = setOption.byStandardOptionValue[i][0];
            value2  = setOption.byStandardOptionValue[i][1];
        }
        else if ( i<8 )
        {
#ifdef PJH_ADD_SET_NEWJOB
			if(((ip->ExtOption%0x04)-1) == 0)
#endif //PJH_ADD_SET_NEWJOB
			{
				option1 = setOption.byExtOption[i-6];
				value1  = setOption.byExtOptionValue[i-6];
			}
#ifdef PJH_ADD_SET_NEWJOB
			else
			{
				option2 = setOption.byExtOption[i-6];
				value2  = setOption.byExtOptionValue[i-6];
			}
#endif //PJH_ADD_SET_NEWJOB
        }
        else
        {
#ifdef PJH_ADD_SET_NEWJOB
			if(((ip->ExtOption%0x04)-1) == 0)
#endif //PJH_ADD_SET_NEWJOB
			{
				option1 = setOption.byFullOption[i-8];
				value1  = setOption.byFullOptionValue[i-8];
			}
#ifdef PJH_ADD_SET_NEWJOB
			else
			{
				option2 = setOption.byFullOption[i-8];
				value2  = setOption.byFullOptionValue[i-8];
			}
#endif //PJH_ADD_SET_NEWJOB

			if( isfulloption )
			{
				byLimitOptionNum = 13;
			}
			else
			{
				byLimitOptionNum = 255;
			}
        }

        if ( option1!=255 )
        {
            getExplainText ( TextList[TNum], option1, value1, 0 );


#ifdef LDS_FIX_OUTPUT_WRONG_EQUIPPEDSETITEMOPTION_BY_WRONG_SLOTINDEX
            if ( m_bySetOptionList[count1][0]==option1 
				&& byLimitOptionNum!=255
#ifdef LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
				&& iLimitOptionCount > i
#endif // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
#ifdef LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY	// 옵션이 모두 비활성화 출력이 되는 Flag (byLimitOptionNum = 0) 검사 추가
				&& byLimitOptionNum!=0			
#endif // LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY
#ifdef LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY
#ifdef PBG_FIX_SETITEMTOOLTIP
				&& bIsEquippedItem == false	//장착외의 아이템을 구분하자
#else //PBG_FIX_SETITEMTOOLTIP
				&& bIsEquippedItem == true // 미장착된 인벤토리에 있는 세트 아이템인지. (장착된 세트아이템만 옵션 활성화 색 설정.)
#endif //PBG_FIX_SETITEMTOOLTIP
#endif // LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY
				)
#else // LDS_FIX_OUTPUT_WRONG_EQUIPPEDSETITEMOPTION_BY_WRONG_SLOTINDEX
				
			if ( m_bySetOptionList[count1][0]==option1 
					&& i<byLimitOptionNum 
					&& byLimitOptionNum!=255
#ifdef LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
					&& iLimitOptionCount > i
#endif // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
#ifdef LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY	// 옵션이 모두 비활성화 출력이 되는 Flag (byLimitOptionNum = 0) 검사 추가
					&& byLimitOptionNum!=0
#endif // LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY
				)
#endif // LDS_FIX_OUTPUT_WRONG_EQUIPPEDSETITEMOPTION_BY_WRONG_SLOTINDEX
            {
                TextListColor[TNum] = TEXT_COLOR_BLUE; 
                count1++;
            }
            else
            {
                TextListColor[TNum] = TEXT_COLOR_GRAY; 
            }
            TextBold[TNum] = false; TNum++;
        }
        if ( option2!=255 )
        {
            getExplainText ( TextList[TNum], option2, value2, 0 );
#ifdef LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
            if ( m_bySetOptionList[count1][0]==option2 
				&& byLimitOptionNum!=255
				&& iLimitOptionCount > i
#ifdef LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY	// 옵션이 모두 비활성화 출력이 되는 Flag (byLimitOptionNum = 0) 검사 추가
				&& byLimitOptionNum!=0
#endif // LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY
				)
#else // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
			if ( m_bySetOptionList[count1][0]==option2 
				&& i<byLimitOptionNum 
				&& byLimitOptionNum!=255
#ifdef LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY	// 옵션이 모두 비활성화 출력이 되는 Flag (byLimitOptionNum = 0) 검사 추가
				&& byLimitOptionNum!=0
#endif // LDS_FIX_WRONG_QUANTITY_SETITEMOPTION_WHEN_RENDER_SETITEMTOOTIP_IN_INVENTORY
				)
#endif // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
            {
                TextListColor[TNum] = TEXT_COLOR_BLUE; 
                count1++;
            }
            else
            {
                TextListColor[TNum] = TEXT_COLOR_GRAY; 
            }
            TextBold[TNum] = false; TNum++;
        }
    }
	sprintf(TextList[TNum],"\n");TNum++;
	sprintf(TextList[TNum],"\n");TNum++;

    return TNum;
}

BYTE CSItemOption::GetSetOptionANum()
{
	return m_bySetOptionANum;
}

BYTE CSItemOption::GetSetOptionBNum()
{
	return m_bySetOptionBNum;
}

void CSItemOption::SetViewOptionList(bool bView)
{
	m_bViewOptionList = bView;
}

bool CSItemOption::IsViewOptionList()
{
	return m_bViewOptionList;
}
