// ItemAddOptioninfo.cpp: implementation of the ItemAddOptioninfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "UIManager.h"

#include "ItemAddOptioninfo.h"
#include "wsclientinline.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef USE_ITEMADDOPTION_BMD
#define ITEMADDOPTION_DATA_FILE "Data\\Local\\ItemAddOptiontest.bmd"
#else //USE_ITEMADDOPTION_BMD
#define ITEMADDOPTION_DATA_FILE "Data\\Local\\ItemAddOption.bmd"
#endif //USE_ITEMADDOPTION_BMD

ItemAddOptioninfo* ItemAddOptioninfo::MakeInfo()
{
	ItemAddOptioninfo* option = new ItemAddOptioninfo();
	return option;
}

ItemAddOptioninfo::ItemAddOptioninfo()
{

#ifdef INFO_BUILDER
	//string a = TheGolbalText( 1 );
#endif //INFO_BUILDER

	bool Result = true;

	Result = OpenItemAddOptionInfoFile( ITEMADDOPTION_DATA_FILE );

	if( !Result )
	{
		char szMessage[256];
		::sprintf(szMessage, "%s file not found.\r\n", ITEMADDOPTION_DATA_FILE);
		g_ErrorReport.Write(szMessage);
		::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
		::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
	}
}

ItemAddOptioninfo::~ItemAddOptioninfo()
{

}

const bool ItemAddOptioninfo::OpenItemAddOptionInfoFile( const string& filename )
{
	FILE *fp = ::fopen( filename.c_str(), "rb" );
	if ( fp != NULL )
	{
		int nSize = sizeof(ITEM_ADD_OPTION) * MAX_ITEM;

		::fread(m_ItemAddOption, nSize, 1, fp);
		::BuxConvert((BYTE*)m_ItemAddOption, nSize);
		::fclose(fp);

		return true;
	}
	return false;
}

/*
// 380 아이템 옵션 추가 - 이민정 2006.7.11
2184 "대인공격성공률 상승 +%d"
2185 "대인공격 추가 데미지 +%d"
2186 "대인방어성공률상승 +%d"
2187 "대인방어력 +%d"
2188 "최대 HP 상승 +%d"
2189 "최대 SD 상승 +%d"
2190 "SD 자동회복"
2191 "SD 회복량 증가 +%d%"
2192 "옵션 추가"
2193 "아이템 옵션 추가 조합"
2194 "380 아이템에 옵션 추가"
2195 "%s시"
2196 "반드시 아이템의 레벨이 4이상 및"
2197 "옵션이 4 이상이어야 조합이 가능합니다."
*/

//#define 1  대인공격성공률 상승
//#define 2  대인공격 추가 데미지 +%d
//#define 3  대인방어성공률상승
//#define 4  대인방어력 +%d
//#define 5  최대 HP 상승
//#define 6  최대 SD 상승
//#define 7  SD 자동회복
//#define 8  SD 회복량 증가

void ItemAddOptioninfo::GetItemAddOtioninfoText( std::vector<string>& outtextlist, int type )
{
	int optiontype = 0;
	int optionvalue = 0;

	for( int i = 0; i < 2; ++i )
	{
		string text;
		char TempText[100];

		if( i == 0 )
		{
			optiontype  = m_ItemAddOption[type].m_byOption1;
			optionvalue = m_ItemAddOption[type].m_byValue1;
		}
		else 
		{
			optiontype  = m_ItemAddOption[type].m_byOption2;
			optionvalue = m_ItemAddOption[type].m_byValue2;
		}

 		switch( optiontype )
		{
		case 1: sprintf( TempText, GlobalText[2184], optionvalue );
			break;
		case 2: sprintf( TempText, GlobalText[2185], optionvalue );
			break;
		case 3: sprintf( TempText, GlobalText[2186], optionvalue );
			break;
		case 4: sprintf( TempText, GlobalText[2187], optionvalue );
			break;
		case 5: sprintf( TempText, GlobalText[2188], optionvalue );
			break;
		case 6: sprintf( TempText, GlobalText[2189], optionvalue );
			break;
		case 7: sprintf( TempText, GlobalText[2190] );
			break;
		case 8: sprintf( TempText, GlobalText[2191], optionvalue );
			break;
		}

		text = TempText;
		outtextlist.push_back( text );
	}
}
