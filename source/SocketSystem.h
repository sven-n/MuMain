#if !defined(_SOCKET_SYSTEM_H_)
#define _SOCKET_SYSTEM_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef SOCKET_SYSTEM

namespace SEASON4A
{
	enum _SOCKET_OPTION_TYPE
	{
		SOT_SOCKET_ITEM_OPTIONS,		// 일반 소켓 옵션
		SOT_MIX_SET_BONUS_OPTIONS,		// 보너스 조합 옵션
		SOT_EQUIP_SET_BONUS_OPTIONS,	// 세트 옵션
		MAX_SOCKET_OPTION_TYPES
	};
	
	const int MAX_SOCKET_OPTION = 50;
	const int MAX_SOCKET_OPTION_NAME_LENGTH = 64;
	const int MAX_SPHERE_LEVEL = 5;
#ifdef YDG_FIX_SOCKETITEM_SELLPRICE_BUG
	const int MAX_SOCKET_TYPES = 6;
#endif	// YDG_FIX_SOCKETITEM_SELLPRICE_BUG

	enum _SOCKET_OPTION_CATEGORY
	{
		SOC_NULL,
		SOC_IMPROVE_ATTACK,		// 공격 강화
		SOC_IMPROVE_DEFENSE,	// 방어 강화
		SOC_IMPROVE_WEAPON,		// 무기 강화
		SOC_IMPROVE_ARMOR,		// 방어구 강화
		SOC_IMPROVE_BATTLE,		// 전투 강화
		SOC_IMPROVE_STATUS,		// 스탯 강화
		SOC_UNIQUE_OPTION,		// 유니크 옵션
		MAX_SOCKET_OPTION_CATEGORY
	};

	enum _SOCKET_OPTIONS
	{
		SOPT_ATTACK_N_MAGIC_DAMAGE_BONUS_BY_LEVEL = 0,	// "(레벨별)공격력/마력증가"
		SOPT_ATTACK_SPEED_BONUS,				// "공격속도증가"
		SOPT_ATTACT_N_MAGIC_DAMAGE_MAX_BONUS,	// "최대공격력/마력증가"
		SOPT_ATTACK_N_MAGIC_DAMAGE_MIN_BONUS,	// "최소공격력/마력증가"
		SOPT_ATTACK_N_MAGIC_DAMAGE_BONUS,		// "공격력/마력증가"
#ifdef YDG_FIX_SOCKET_BALANCE_PATCH
		SOPT_DECREASE_AG_USE,					// "AG소모량감소"
#else	// YDG_FIX_SOCKET_BALANCE_PATCH
		SOPT_SKILL_DAMAGE_BONUS,				// "스킬공격력증가"
		SOPT_ATTACK_RATE_BONUS,					// "공격성공율증가"
		SOPT_PVP_ATTACK_RATE_BONUS,				// "대인공격성공율증가"
		SOPT_MAGIC_POWER_BONUS,					// "마력증가"
#endif	// YDG_FIX_SOCKET_BALANCE_PATCH

		SOPT_DEFENCE_RATE_BONUS = 10,			// "방어성공율증가"
		SOPT_DEFENCE_BONUS,						// "방어력증가"
		SOPT_SHIELD_DEFENCE_BONUS,				// "방패방어력증가"
		SOPT_DECREASE_DAMAGE,					// "데미지감소"
		SOPT_REFLECT_DAMAGE,					// "데미지반사"

		SOPT_MONSTER_DEATH_LIFE_BONUS = 16,		// "몬스터사망시생명증가"
		SOPT_MONSTER_DEATH_MANA_BONUS,			// "몬스터사망시마나증가"
#ifdef YDG_FIX_SOCKET_BALANCE_PATCH
		SOPT_SKILL_DAMAGE_BONUS,				// "스킬공격력증가"
		SOPT_ATTACK_RATE_BONUS,					// "공격성공율증가"
		SOPT_INCREASE_ITEM_DURABILITY,			// "아이템내구도강화"
#else	// YDG_FIX_SOCKET_BALANCE_PATCH
		SOPT_SD_USE_RATE_BONUS,					// "SD감소율상승" -> PvP시 피 대신 SD가 깎이는 비율을 늘여준다
		SOPT_IGNORE_SD_RATE_BONUS,				// "공격시SD무시확률상승"
#endif	// YDG_FIX_SOCKET_BALANCE_PATCH

		SOPT_LIFE_REGENERATION_BONUS = 21,		// "생명자동회복증가"
		SOPT_MAX_LIFE_BONUS,					// "최대생명증가"
		SOPT_MAX_MANA_BONUS,					// "최대마나증가"
		SOPT_MANA_REGENERATION_BONUS,			// "마나자동회복증가"
		SOPT_MAX_AG_BONUS,						// "최대AG증가"
		SOPT_AG_REGENERATION_BONUS,				// "AG값증가"
#ifndef YDG_FIX_SOCKET_BALANCE_PATCH	// 정리시 삭제해야함
		SOPT_MONSTER_DEATH_ZEN_BONUS,			// "몬스터사망시젠증가"
#endif	// YDG_FIX_SOCKET_BALANCE_PATCH

		SOPT_EXCELLENT_DAMAGE_BONUS = 29,		// "엑설런트데미지증가"
		SOPT_EXCELLENT_DAMAGE_RATE_BONUS,		// "엑설런트데미지확률증가"
		SOPT_CRITICAL_DAMAGE_BONUS,				// "크리티컬데미지증가"
		SOPT_CRITICAL_DAMAGE_RATE_BONUS,		// "크리티컬데미지확률증가"

		SOPT_STRENGTH_BONUS = 34,				// "힘증가"
		SOPT_DEXTERITY_BONUS,					// "민첩증가"
		SOPT_VITALITY_BONUS,					// "체력증가"
		SOPT_ENERGY_BONUS,						// "에너지증가"
		SOPT_REQUIRED_STENGTH_BONUS,			// "필요힘감소"
		SOPT_REQUIRED_DEXTERITY_BONUS,			// "필요민첩감소"

		SOPT_UNIQUE01 = 41,						// "유니크옵션1" (미사용)
		SOPT_UNIQUE02,							// "유니크옵션2" (미사용)
	};

	enum _SOCKET_BONUS_OPTIONS
	{
		SBOPT_ATTACK_DAMAGE_BONUS = 0,			// "공격력 상승"
		SBOPT_SKILL_DAMAGE_BONUS,				// "스킬 공격력 상승"
		SBOPT_MAGIC_POWER_BONUS,				// "공격력/마력 상승"
		SBOPT_SKILL_DAMAGE_BONUS_2,				// "스킬 공격력 상승"
		SBOPT_DEFENCE_BONUS,					// "방어력 상승"
		SBOPT_MAX_LIFE_BONUS,					// "최대 생명 증가"
	};

	typedef struct _SOCKET_OPTION_INFO
	{
		int m_iOptionID;
		int m_iOptionCategory;
		char m_szOptionName[MAX_SOCKET_OPTION_NAME_LENGTH];
		char m_bOptionType;
#ifdef YDG_FIX_SCRIPT_LEVEL_VALUE
		int m_iOptionValue[5];		// 옵션 수치 값 (스피어 레벨별)
#else	// YDG_FIX_SCRIPT_LEVEL_VALUE
		int m_iOptionValue;
#endif	// YDG_FIX_SCRIPT_LEVEL_VALUE
		BYTE m_bySocketCheckInfo[6];
	} SOCKET_OPTION_INFO;

// 	typedef struct _SOCKET_OPTION_INFO_UNICODE
// 	{
// 		int m_iOptionID;
// 		int m_iOptionCategory;
// 		wchar_t m_szOptionName[MAX_SOCKET_OPTION_NAME_LENGTH];
// 		wchar_t m_bOptionType;
// #ifdef YDG_FIX_SCRIPT_LEVEL_VALUE
// 		int m_iOptionValue[5];		// 옵션 수치 값 (스피어 레벨별)
// #else	// YDG_FIX_SCRIPT_LEVEL_VALUE
// 		int m_iOptionValue;
// #endif	// YDG_FIX_SCRIPT_LEVEL_VALUE
// 		BYTE m_bySocketCheckInfo[6];
// 	} SOCKET_OPTION_INFO_UNICODE;

	typedef struct _SOCKET_OPTION_STATUS_BONUS
	{
		int m_iAttackDamageMinBonus;	// 최소 공격력
		int m_iAttackDamageMaxBonus;	// 최대 공격력
		int m_iAttackRateBonus;			// 공격 성공율
		int m_iSkillAttackDamageBonus;	// 스킬 공격력
		int m_iAttackSpeedBonus;		// 공격속도
#ifndef YDG_FIX_SOCKET_BALANCE_PATCH
		int m_iPvPAttackRateBonus;		// 대인 공격 성공율
#endif	// YDG_FIX_SOCKET_BALANCE_PATCH
#ifdef YDG_FIX_SOCKET_MISSING_MAGIC_POWER_BONUS
		int m_iMagicPowerMinBonus;		// 최소 마력
		int m_iMagicPowerMaxBonus;		// 최대 마력
#else	// YDG_FIX_SOCKET_MISSING_MAGIC_POWER_BONUS
		int m_iMagicPowerBonus;			// 마력
#endif	// YDG_FIX_SOCKET_MISSING_MAGIC_POWER_BONUS

		int m_iDefenceBonus;			// 방어력
#ifdef YDG_FIX_SOCKET_BALANCE_PATCH
		float m_fDefenceRateBonus;		// 방어성공율
#else	// YDG_FIX_SOCKET_BALANCE_PATCH
		int m_iDefenceRateBonus;		// 방어율
#endif	// YDG_FIX_SOCKET_BALANCE_PATCH
		int m_iShieldDefenceBonus;		// 방패 방어력

		int m_iStrengthBonus;			// 힘
		int m_iDexterityBonus;			// 민첩
		int m_iVitalityBonus;			// 체력
		int m_iEnergyBonus;				// 에너지
	} SOCKET_OPTION_STATUS_BONUS;

	// 소켓 정보 관리 클래스 (소켓 아이템 옵션 등)
	class CSocketItemMgr
	{
	public:
		CSocketItemMgr();
		virtual ~CSocketItemMgr();

		BOOL IsSocketItem(const ITEM* pItem);	// 소켓 아이템인가
		BOOL IsSocketItem(const OBJECT* pObject);	// 소켓 아이템 오브젝트인가
		int GetSeedShpereSeedID(const ITEM* pItem);	// 시드스피어이면 시드 ID를 반환한다. 시드스피어가 아니면 SOCKET_EMPTY 반환
		int GetSocketCategory(int iSeedID);		// 시드 ID가 어떤 속성인지 알려준다
		
		// 소켓 아이템 툴팁 관련
		int AttachToolTipForSocketItem(const ITEM* pItem, int iTextNum);	// 아이템 툴팁에 소켓 옵션 목록을 뿌려준다.
		int AttachToolTipForSeedSphereItem(const ITEM* pItem, int iTextNum);	// 시드, 스피어, 시드스피어 아이템 툴팁을 생성준다.

		// 소켓 아이템 세트 관련
		void CheckSocketSetOption();		// 장비의 소켓 세트를 검사 (장비 전체 세트옵션)
		BOOL IsSocketSetOptionEnabled();	// 소켓 세트옵션이 있나
		void RenderToolTipForSocketSetOption(int iPos_x, int iPos_y);	// 소켓 세트 옵션 목록을 뿌려준다.
		
		// 소켓 옵션 텍스트 생성
		void CreateSocketOptionText(char * pszOptionText, int iSeedID, int iSphereLv);

		// 소켓 아이템 가격 보너스 계산
#ifdef KJW_FIX_SOCKET_BONUS_BIT_OPERATION
		__int64 CalcSocketBonusItemValue(const ITEM * pItem, __int64 iOrgGold);
#else // KJW_FIX_SOCKET_BONUS_BIT_OPERATION
		int CalcSocketBonusItemValue(const ITEM * pItem, int iOrgGold);
#endif // KJW_FIX_SOCKET_BONUS_BIT_OPERATION

		// 소켓아이템 옵션값을 구한다
		int GetSocketOptionValue(const ITEM * pItem, int iSocketIndex);

		void CalcSocketStatusBonus();		// 장비의 스테이터스 보너스를 검사 (m_StatusBonus 생성)
		SOCKET_OPTION_STATUS_BONUS m_StatusBonus; 

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		void OpenSocketItemScript(const unicode::t_char * szFileName);	// socketitem.bmd를 읽어들인다.
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

	protected:
		BOOL IsSocketItem(int iItemType);	// 소켓 아이템인가
#ifndef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		void OpenSocketItemScript(const unicode::t_char * szFileName);	// socketitem.bmd를 읽어들인다.
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#ifdef YDG_FIX_SCRIPT_LEVEL_VALUE
		void CalcSocketOptionValueText(char * pszOptionValueText, int iOptionType, float fOptionValue);	// 소켓 옵션 텍스트 계산
		int CalcSocketOptionValue(int iOptionType, float fOptionValue);	// 소켓 옵션 값 계산
#else	// YDG_FIX_SCRIPT_LEVEL_VALUE
		void CalcSocketOptionValueText(char * pszOptionValueText, int iOptionType, float fOptionValue, float fSphereValue);	// 소켓 옵션 텍스트 계산
		int CalcSocketOptionValue(int iOptionType, float fOptionValue, float fSphereValue);	// 소켓 옵션 값 계산
#endif	// YDG_FIX_SCRIPT_LEVEL_VALUE

	protected:
		SOCKET_OPTION_INFO m_SocketOptionInfo[MAX_SOCKET_OPTION_TYPES][MAX_SOCKET_OPTION];
		int m_iNumEquitSetBonusOptions;		// 검사할 소켓장비 세트옵션 개수

		std::deque<DWORD> m_EquipSetBonusList;	// 캐릭터의 소켓장비 세트옵션 ID 목록
#ifndef YDG_FIX_SCRIPT_LEVEL_VALUE
		float m_fSphereValues[5];
#endif	// YDG_FIX_SCRIPT_LEVEL_VALUE
	};
}

extern SEASON4A::CSocketItemMgr g_SocketItemMgr;
#endif	// SOCKET_SYSTEM
#endif // !defined(_SOCKET_SYSTEM_H_)
