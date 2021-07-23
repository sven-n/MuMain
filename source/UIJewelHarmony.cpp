// UIRefinery.cpp: implementation of the UIRefinery class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "UIManager.h"
#include "UIJewelHarmony.h"
#include "wsclientinline.h"

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#ifdef USE_HARMONYJEWELOPTION_BMD
	#define HARMONYJEWELOPTION_DATA_FILE string("Data\\Local\\"+g_strSelectedML+"\\JewelOfHarmonyOptiontest_"+g_strSelectedML+".bmd").c_str()
#else //USE_HARMONYJEWELOPION_BMD
	#define HARMONYJEWELOPTION_DATA_FILE string("Data\\Local\\"+g_strSelectedML+"\\JewelOfHarmonyOption_"+g_strSelectedML+".bmd").c_str()
#endif //USE_HARMONYJEWELOPION_BMD

#ifdef USE_NOTSMELTING_BMD
	#define NOTSMELTING_DATA_FILE string("Data\\Local\\"+g_strSelectedML+"\\JewelOfHarmonySmelttest_"+g_strSelectedML+".bmd").c_str()
#else //USE_NOTSMELTING_BMD
	#define NOTSMELTING_DATA_FILE string("Data\\Local\\"+g_strSelectedML+"\\JewelOfHarmonySmelt_"+g_strSelectedML+".bmd").c_str()
#endif //USE_NOTSMELTING_BMD

#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#ifdef USE_HARMONYJEWELOPTION_BMD
	#define HARMONYJEWELOPTION_DATA_FILE "Data\\Local\\JewelOfHarmonyOptiontest.bmd"
#else //USE_HARMONYJEWELOPION_BMD
	#define HARMONYJEWELOPTION_DATA_FILE "Data\\Local\\JewelOfHarmonyOption.bmd"
#endif //USE_HARMONYJEWELOPION_BMD

#ifdef USE_NOTSMELTING_BMD
	#define NOTSMELTING_DATA_FILE "Data\\Local\\JewelOfHarmonySmelttest.bmd"
#else //USE_NOTSMELTING_BMD
	#define NOTSMELTING_DATA_FILE "Data\\Local\\JewelOfHarmonySmelt.bmd"
#endif //USE_NOTSMELTING_BMD
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern float		g_fScreenRate_x;
extern float		g_fScreenRate_y;

namespace 
{
	int GetTextLines( const char* inText, char *outText, int maxLine, int lineSize )
	{
		int iLine = 0;
		const char *lpLineStart = inText;
		char *lpDst = outText;
		const char* lpSpace = NULL;
		int iMbclen = 0;
		for ( const char *lpSeek = inText; *lpSeek; lpSeek += iMbclen, lpDst += iMbclen)
		{
			iMbclen = _mbclen( ( unsigned char*)lpSeek);
			if ( iMbclen + ( int)( lpSeek - lpLineStart) >= lineSize)
			{	// 꽉 찼다. 다음 줄로
				// 최근 공백이 가까우면 공백까지
				if ( lpSpace && ( int)( lpSeek - lpSpace) < min( 10, lineSize / 2))
				{
					lpDst -= ( lpSeek - lpSpace - 1);
					lpSeek = lpSpace + 1;
				}

				lpLineStart = lpSeek;
				*lpDst = '\0';
				if ( iLine >= maxLine - 1)
				{	// 모든 줄을 다 썼다.
					break;
				}
				++iLine;
				lpDst = outText + iLine * lineSize;
				lpSpace = NULL;
			}

			// 한 글자 복사
			memcpy( lpDst, lpSeek, iMbclen);
			if ( *lpSeek == ' ')
			{
				lpSpace = lpSeek;
			}
		}
		// 끝
		*lpDst = '\0';

		return ( iLine + 1);
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////// JewelHarmony info ///////////////////////////
//////////////////////////////////////////////////////////////////////

JewelHarmonyInfo* JewelHarmonyInfo::MakeInfo()
{
	JewelHarmonyInfo* info = new JewelHarmonyInfo;
	return info;
}

JewelHarmonyInfo::JewelHarmonyInfo()
{
	bool Result = true;
	//원래는 인포 빌더를 만들어서 사용하자..--;;
	if( !OpenJewelHarmonyInfoFile( HARMONYJEWELOPTION_DATA_FILE ) )
	{
		Result = false;
	}

	if( !Result )
	{
		char szMessage[256];
		::sprintf(szMessage, "%s file not found.\r\n", "JewelOfHarmonyOption.bmd && JewelOfHarmonySmelt.bmd");
		g_ErrorReport.Write(szMessage);
		::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
		::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
	}
}

JewelHarmonyInfo::~JewelHarmonyInfo()
{

}

//텍스트 파일을 열어둔다
const bool JewelHarmonyInfo::OpenJewelHarmonyInfoFile( const string& filename )
{
	FILE *fp = ::fopen( filename.c_str(), "rb" );
	if ( fp != NULL )
	{
		int nSize = sizeof(HARMONYJEWELOPTION) * MAXHARMONYJEWELOPTIONTYPE * MAXHARMONYJEWELOPTIONINDEX;

		::fread(m_OptionData, nSize, 1, fp);
		::BuxConvert((BYTE*)m_OptionData, nSize);
		::fclose(fp);

		return true;
	}
	return false;
}

//아이템의 장비가 무기, 지팡이, 방어구 인지를 검사 한다
const StrengthenItem JewelHarmonyInfo::GetItemType( int type )
{
	StrengthenItem itemtype = SI_None;

	if( ITEM_SWORD <= type && ITEM_STAFF > type )
	{
		itemtype = SI_Weapon;
		return itemtype;
	}
	else if( ITEM_STAFF <= type && ITEM_SHIELD > type )
	{
		itemtype = SI_Staff;
		return itemtype;
	}
	else if( ITEM_SHIELD <= type && ITEM_WING > type )
	{
		itemtype = SI_Defense;
		return itemtype;
	}

	return itemtype;
}

//강화 옵션의 여부를 검사 한다...
const bool JewelHarmonyInfo::IsHarmonyJewelOption( int type, int option )
{
	bool isSuccess = false;

	switch(type)
	{
	case SI_Weapon:
		{
			if( 0 <= option && ( MAXHARMONYJEWELOPTIONINDEX + 1 ) > option )
			{
				isSuccess = true;
			}
		}
		break;
	case SI_Staff:
		{
			if( 0 <= option && ( MAXHARMONYJEWELOPTIONINDEX + 1 ) - 2 > option )
			{
				isSuccess = true;
			}
		}
		break;
	case SI_Defense:
		{
			if( 0 <= option && ( MAXHARMONYJEWELOPTIONINDEX + 1 ) - 2 > option )
			{
				isSuccess = true;
			}
		}
		break;
	}

	return isSuccess;
}

//강화 옵션을 선택해서 옵션을 담아 보내 준다
//index에 따라서 멀 검사 할껀지를 결정한다
// index 0 = 필요 능력치 감소량 검사
// index 1 = 공격력  검사, 마력
// index 2 = 방어력, 최대 AG 상승, 최대 Hp 상승
void JewelHarmonyInfo::GetStrengthenCapability( StrengthenCapability* pitemSC, const ITEM* pitem, const int index )
{
	if( pitem->Jewel_Of_Harmony_Option != 0 )
	{
		StrengthenItem type = GetItemType( static_cast<int>(pitem->Type) );

		if( type < SI_None )
		{
			if( IsHarmonyJewelOption( type, pitem->Jewel_Of_Harmony_Option ) )
			{
				HARMONYJEWELOPTION harmonyjewel = GetHarmonyJewelOptionInfo( type, pitem->Jewel_Of_Harmony_Option );

				if( index == 0 ) //필요한 힘, 민첩 감소량...체크
				{	
					if( type == SI_Weapon )
					{
						pitemSC->SI_isNB = true;

						if( pitem->Jewel_Of_Harmony_Option == 3 ) //필요 힘 감소
						{
							pitemSC->SI_NB.SI_force = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
						else if( pitem->Jewel_Of_Harmony_Option == 4 ) //필요 민첩 감소
						{
							pitemSC->SI_NB.SI_activity = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
					}
					else if( type == SI_Staff )
					{
						pitemSC->SI_isNB = true;

						if( pitem->Jewel_Of_Harmony_Option == 2 ) //필요 힘 감소
						{
							pitemSC->SI_NB.SI_force = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
						else if( pitem->Jewel_Of_Harmony_Option == 3 ) //필요 민첩 감소
						{
							pitemSC->SI_NB.SI_activity = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
					}
				}
				//여기다가 검사 할 조건들을 만들어 주고 구조체에 추가 해서 리턴 하자...

				else if( index == 1 )
				{
					if( type == SI_Weapon )
					{
						pitemSC->SI_isSP = true;

						if( pitem->Jewel_Of_Harmony_Option == 1 ) //최소 공격력 상승
						{
							pitemSC->SI_SP.SI_minattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
						else if( pitem->Jewel_Of_Harmony_Option == 2 ) //최대 공격력 상승
						{
							pitemSC->SI_SP.SI_maxattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
						else if( pitem->Jewel_Of_Harmony_Option == 5 ) //최소, 최대 공격력 상승
						{
							pitemSC->SI_SP.SI_minattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
							pitemSC->SI_SP.SI_maxattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
						else if( pitem->Jewel_Of_Harmony_Option == 7 ) //스킬 공격력
						{
							pitemSC->SI_SP.SI_skillattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
						else if( pitem->Jewel_Of_Harmony_Option == 8 ) //대인 공격율
						{
							pitemSC->SI_SP.SI_attackpowerRate = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
					}
					else if( type == SI_Staff )
					{
						pitemSC->SI_isSP = true;

						if( pitem->Jewel_Of_Harmony_Option == 1 ) //최소 공격력 상승
						{
							pitemSC->SI_SP.SI_magicalpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
						else if( pitem->Jewel_Of_Harmony_Option == 4 ) //스킬 공격력
						{
							pitemSC->SI_SP.SI_skillattackpower = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
						else if( pitem->Jewel_Of_Harmony_Option == 7 ) //대인 공격율
						{
							pitemSC->SI_SP.SI_attackpowerRate = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
					}
				}

				else if( index == 2 )// 방어력 증가, 최대 AG상승, 최대 Hp상승
				{
					if( SI_Defense == type )
					{
						pitemSC->SI_isSD = true;

						if( pitem->Jewel_Of_Harmony_Option == 1 )
						{
							pitemSC->SI_SD.SI_defense = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
						else if( pitem->Jewel_Of_Harmony_Option == 2 )
						{
							pitemSC->SI_SD.SI_AG = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
						else if( pitem->Jewel_Of_Harmony_Option == 3 )
						{
							pitemSC->SI_SD.SI_HP = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
						else if( pitem->Jewel_Of_Harmony_Option == 6 ) //대인 방어율
						{
							pitemSC->SI_SD.SI_defenseRate = harmonyjewel.HarmonyJewelLevel[pitem->Jewel_Of_Harmony_OptionLevel];
						}
					}
				}
			}
		}
	}
}
