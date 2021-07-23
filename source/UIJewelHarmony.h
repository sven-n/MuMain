// UIRefinery.h: interface for the UIRefinery class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIREFINERY_H__29D29488_91BA_4EB6_AEE3_F936A9C872EE__INCLUDED_)
#define AFX_UIREFINERY_H__29D29488_91BA_4EB6_AEE3_F936A9C872EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
//////////////////////// JewelHarmony info ///////////////////////////
//////////////////////////////////////////////////////////////////////

//타입 (무기, 지팡이, 방어구)
#define MAXHARMONYJEWELOPTIONTYPE 3

//강화 아이템 옵션
// 타입 0 = 10 무기
// 타입 1 = 8 지팡이
// 타입 2 = 8 방어구
#define MAXHARMONYJEWELOPTIONINDEX 10

//#define IS_BUTTON_SORT

struct HarmonyJewelOption
{
	//강화 옵션 타입
	int OptionType;
	//강화 옵션 이름
	char Name[60];
	//최소 레벨
	int Minlevel;
	//레벨에 따른 데이타
	int HarmonyJewelLevel[14];
	//환원에 필요한 젠
	int Zen[14];

	HarmonyJewelOption() : OptionType( -1 ), Minlevel( -1 )
	{
		for( int i = 0; i < 14; ++i )
		{
			HarmonyJewelLevel[i] = -1;
			Zen[i] = -1;
		}
	}
};

//필요 힘 민첩 감소 강화
struct NaturalAbility
{
	//필요 힘 감소
	int SI_force;
	//필요 민첩 감소
	int SI_activity;

	NaturalAbility() : SI_force( 0 ), SI_activity( 0 ){}
};

//공격력 강화
struct StrikingPower
{
	//최하 공격력
	int SI_minattackpower;
	//최대 공격력
	int SI_maxattackpower;
	//최대 마력
	int SI_magicalpower;
	//공격율 상승
	int SI_attackpowerRate;
	//스킬 공격력 상승
	int SI_skillattackpower;

	StrikingPower() : SI_minattackpower(0), 
					  SI_maxattackpower(0), 
					  SI_magicalpower(0),
					  SI_attackpowerRate(0),
					  SI_skillattackpower(0){}
};

//방어구 강화
struct StrengthenDefense  
{
	//강화된 방어량
	int SI_defense;
	//강화된 AG량
	int SI_AG;
	//강화된 HP량
	int SI_HP;
	//방어율 상승
	int SI_defenseRate;

	StrengthenDefense() : SI_defense( 0 ), SI_AG( 0 ), SI_HP( 0 ), SI_defenseRate( 0 ){}
};

struct StrengthenCapability 
{
	//필요 힘 민첩 감소 강화
	NaturalAbility SI_NB;
	bool		   SI_isNB;

	//공격력 강화
	StrikingPower  SI_SP;
	bool           SI_isSP;

	//방어구 강화
	StrengthenDefense SI_SD;
	bool			  SI_isSD;

	StrengthenCapability() : SI_isNB( false ), SI_isSP( false ), SI_isSD( false ){}
};

enum StrengthenItem
{ 
	SI_Weapon = 0, 
	SI_Staff, 
	SI_Defense,
	SI_None,
};

typedef HarmonyJewelOption HARMONYJEWELOPTION;

//조화 시스템 및 정보
class JewelHarmonyInfo
{
public:
	virtual ~JewelHarmonyInfo();
	static JewelHarmonyInfo* MakeInfo();

public:
	const StrengthenItem		GetItemType( int type );
	const HARMONYJEWELOPTION&   GetHarmonyJewelOptionInfo( int type, int option );
	void						GetStrengthenCapability( StrengthenCapability* pitemSC, const ITEM* pitem, const int index );

public:
	const bool IsHarmonyJewelOption( int type, int option );
	
private:
	JewelHarmonyInfo();
	const bool OpenJewelHarmonyInfoFile( const string& filename );

private:
	HARMONYJEWELOPTION		m_OptionData[MAXHARMONYJEWELOPTIONTYPE][MAXHARMONYJEWELOPTIONINDEX];
};

inline
const HARMONYJEWELOPTION& JewelHarmonyInfo::GetHarmonyJewelOptionInfo( int type, int option )
{
	return m_OptionData[type][option-1];//-1은 option 값이 1부터 시작함
}

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
extern string g_strSelectedML;
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#endif // !defined(AFX_UIREFINERY_H__29D29488_91BA_4EB6_AEE3_F936A9C872EE__INCLUDED_)
