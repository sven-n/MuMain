// Builder.cpp: implementation of the Builder class.
//
//////////////////////////////////////////////////////////////////////

//main header
#include "stdafx.h"

//info header
#include "ExplanationInfo.h"
#include "ItemInfo.h"
#include "SkillInfo.h"
#include "SlideInfo.h"
#include "DialogInfo.h"
#include "CreditInfo.h"
#include "FilterInfo.h"
#include "FilterNameInfo.h"
#include "MovereqInfo.h"
#include "QuestInfo.h"

#include "Builder.h"

#ifdef INFO_BUILDER

using namespace info;

static Builder infobuilder;

namespace
{
	const int TEXTFILENAMECOUNT                                    = eInfo_File_Count;
	const int TEXTFILENAMELENGTH                                   = 64;
	const char TEXTFILEPOSITION[TEXTFILENAMELENGTH]                = "data\\local\\";
	const char TEXTFILETESTSERVER[TEXTFILENAMELENGTH]              = "test";
	const char TEXTFILEBMD[TEXTFILENAMELENGTH]                     = ".bmd";
	const char TEXTFILETXT[TEXTFILENAMELENGTH]                     = ".txt";
	const char TEXTFILENAME[TEXTFILENAMECOUNT][TEXTFILENAMELENGTH] = 
	{
		"text",
		"item", 
		"skill", 
		"slide", 
		"dialog", 
		"credit", 
		"filter", 
		"filtername", 
		"monsterskill", 
		"movereq", 
		"quest", 
		"itemsettype",
		"itemsetoption", 
		"npcname", 
		"jewelofharmonyoption", 
		"jewelofharmonysmelt", 
		"itemaddoption"
	};
}

Builder::Builder() : m_isTestServer( false )
{
	try {
		InitBuilder();
    }
    catch( ... ) {
        MessageBox( NULL, "InitBuilder Error", "Builder", MB_OK );
		Clear();
        throw;
    }
}

Builder::~Builder()
{
	Clear();
}

void Builder::Clear()
{
	for( int i = eInfo_File_Count - 1; i > -1; --i )
	{
		InfoMAP::iterator iter = m_InfoMap.find( (InfoTextType)i );
		if( iter != m_InfoMap.end() )
		{
			SAFE_DELETE((*iter).second);
			//m_InfoMap.erase(iter);
		}
	}

	m_InfoMap.clear();
}

void Builder::InitBuilder()
{
#ifdef _TEST_SERVER
	m_isTestServer = true;
#endif //_TEST_SERVER

#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
#ifdef _LANGUAGE_KOR
	sprintf( m_Language, "(%s)", "Kor" );
#elif _LANGUAGE_ENG
	sprintf( m_Language, "(%s)", "Eng" );
#elif _LANGUAGE_TAI
	sprintf( m_Language, "(%s)", "Tai" );
#elif _LANGUAGE_CHS
	sprintf( m_Language, "(%s)", "Chs" );
#elif _LANGUAGE_JPN
	sprintf( m_Language, "(%s)", "Jpn" );
#elif _LANGUAGE_PHI
	sprintf( m_Language, "(%s)", "Phi" );
#elif _LANGUAGE_VIE
	sprintf( m_Language, "(%s)", "Vie" );
#endif
#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
	switch( SELECTED_LANGUAGE )
	{
	case LANGUAGE_KOREAN:		sprintf( m_Language, "(%s)", "Kor" ); break;
	case LANGUAGE_ENGLISH:		sprintf( m_Language, "(%s)", "Eng" ); break;
	case LANGUAGE_TAIWANESE:	sprintf( m_Language, "(%s)", "Tai" ); break;
	case LANGUAGE_CHINESE:		sprintf( m_Language, "(%s)", "Chs" ); break;
	case LANGUAGE_JAPANESE:		sprintf( m_Language, "(%s)", "Jpn" ); break;
	case LANGUAGE_PHILIPPINES:	sprintf( m_Language, "(%s)", "Phi" ); break;
	case LANGUAGE_VIETNAMESE:	sprintf( m_Language, "(%s)", "Vie" ); break;
	}
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE

	for ( int i = eInfo_Text_File; i < eInfo_File_Count; ++i )
	{
		InfoTextType type = (InfoTextType)i;
		m_InfoMap.insert( std::make_pair( type, MakeInfo( type ) ) );
	}
}

InfoFile& Builder::QueryInfo( InfoTextType type )
{
	InfoMAP::iterator iter = m_InfoMap.find( type );

	if( iter != m_InfoMap.end() ) {
		return *(((*iter).second).get());
	}
	else {
		assert( 0 );
		BoostSmart_Ptr(InfoFile) info = MakeInfo( type );
		m_InfoMap.insert( std::make_pair( type, info ) );
		return *(info.get());
	}
}

BoostSmart_Ptr(InfoFile) Builder::MakeInfo( InfoTextType type )
{
	BoostSmart_Ptr(InfoFile) info;

	char TextFileName[512];

	sprintf( TextFileName, "%s%s%s%s%s", //경로, 파일명, 테스트, 나라별, 확장명
		                   TEXTFILEPOSITION, 
						   TEXTFILENAME[type], 
						   m_isTestServer ? TEXTFILETESTSERVER : "",
						   type == eInfo_NpcName_File ? m_Language : "",
						   type == eInfo_NpcName_File ? TEXTFILETXT : TEXTFILEBMD
		   );

	switch( type )
	{
	case eInfo_Text_File:					info  = ExplanationInfo::MakeInfo( TextFileName);	 break;
	case eInfo_Item_File:					info  = ItemInfo::MakeInfo( TextFileName );			 break;
	case eInfo_Skill_File:					info  = SkillInfo::MakeInfo( TextFileName );		 break;
	case eInfo_Slide_File:					info  = SlideInfo::MakeInfo( TextFileName );		 break;
	case eInfo_Dialog_File:					info  = DialogInfo::MakeInfo( TextFileName );		 break;
	case eInfo_Credit_File:					info  = CreditInfo::MakeInfo( TextFileName );		 break;
	case eInfo_Filter_File:					info  = FilterInfo::MakeInfo( TextFileName );	     break;
	case eInfo_FilterName_File:				info  = FilterNameInfo::MakeInfo( TextFileName );	 break;
	case eInfo_MonsterSkill_File:			info  = SkillInfo::MakeInfo( TextFileName );         break;
	case eInfo_Movereq_File:				info  = MovereqInfo::MakeInfo( TextFileName );       break;
	case eInfo_Quest_File:					info  = QuestInfo::MakeInfo( TextFileName );		 break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case eInfo_ItemSetType_File:			info  = QuestInfo::MakeInfo( TextFileName );		 break;
	case eInfo_ItemSetOption_File:			info  = QuestInfo::MakeInfo( TextFileName );		 break;
	case eInfo_NpcName_File:				info  = QuestInfo::MakeInfo( TextFileName );		 break;
	case eInfo_JewelOfHarmonyOption_File:	info  = QuestInfo::MakeInfo( TextFileName );		 break;
	case eInfo_JewelOfHarmonySmelt_File:	info  = QuestInfo::MakeInfo( TextFileName );		 break;
	case eInfo_ItemAddOption_File:			info  = QuestInfo::MakeInfo( TextFileName );         break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}

	if( info && !info->isopenfile() ) {
		assert( 0 );
	}
	return info;
}

#endif //INFO_BUILDER