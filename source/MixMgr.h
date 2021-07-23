#ifndef _MIX_MGR_H_
#define _MIX_MGR_H_

#pragma once

//////////////////////////////////////////////////////////////////////////////////////
// MIX_RECIPE_ITEM		// 믹스 재료 아이템
// MIX_RECIPE			// 믹스 방법 (재료들의 집합)
// CMixRecipes			// 믹스 방법군 (방법들의 집합)
// CMixRecipeMgr		// 믹스 장소에 따라 믹스 방법군 관리 (방법군의 집합)
// 
// CMixItem				// 조합창 아이템
// CMixItemInventory	// 조합창 아이템들 관리 (조합창 아이템들의 집합)
//
// 1. CMixRecipeMgr는 생성되면서 mix.bmd를 로딩하여 조합군(조합 장소)에 따라 CMixRecipe에 저장합니다.
// 2. CMixRecipe는 MIX_RECIPE들로 구성되어 있으며 각 MIX_RECIPE는 조합의 방법,재료(MIX_RECIPE_ITEM) 정보를 담고 있습니다.
// 3. zzzMixInventory의 CheckMixInventoryNewCore()에서 현재 조합 인벤토리 위의 아이템을 CMixItem으로 변환하여
// CMixItemInventory에 저장합니다.
// 4. CMixItemInventory는 CMixRecipeMgr을 통해 조합창의 아이템과 조합 방법을 비교하여 올바른 조합을 찾아냅니다.
// 5. 스크립트상의 id를 서버로 전송합니다. (서버와 약속된 값)

namespace SEASON3A
{
	class CMixItem	// 조합 재료 아이템
	{
	public:
		CMixItem() { Reset(); }
		virtual ~CMixItem() { }
		CMixItem(ITEM * pItem, int iMixValue) { SetItem(pItem, iMixValue); }
		
		void Reset();
		void SetItem(ITEM * pItem, DWORD dwMixValue);	// ITEM 구조체를 비교하기 편리한 형태로 변환하며, 각종 유형 검사를 하는 함수

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
		bool IsSameItem(const CMixItem& rhs)
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		bool IsSameItem(CMixItem & rhs)
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
		{
			return (m_sType == rhs.m_sType && m_iLevel == rhs.m_iLevel &&
				(m_bCanStack || m_iDurability == rhs.m_iDurability) && m_iOption == rhs.m_iOption &&
				m_dwSpecialItem == rhs.m_dwSpecialItem);
		}
		
		bool operator==(ITEM * rhs)
		{
			return IsSameItem(CMixItem(rhs, 0));
		}

		// 조합 조건 비교용 정보
		short m_sType;
		int m_iLevel;
		int m_iOption;
		int m_iDurability;
		DWORD m_dwSpecialItem;	// 엑설런트,380아이템,세트아이템 (_SPECIAL_ITEM_RECIPE_)
		int m_iCount;		// 동일한 아이템 개수
		int m_iTestCount;	// 갯수 검사용 변수

		// 조합 확률/가격 계산용 정보
		BOOL m_bMixLuck;	// 행운옵션
		BOOL m_bIsEquipment;	// 장비인가
		BOOL m_bCanStack;	// 포갤 수 있는 아이템인가 (포션 등 내구도를 개수로 사용하는 아이템)
		BOOL m_bIsWing;		// 날개인가
		BOOL m_bIsUpgradedWing;	// 업그레이드 날개인가
		BOOL m_bIs3rdUpgradedWing;	// 3차 업그레이드 날개인가
		DWORD m_dwMixValue;	// 조합시 계산되는 가격
		BOOL m_bIsCharmItem;	// 행운의 부적인가
#ifdef PSW_ADD_MIX_CHAOSCHARM
		BOOL m_bIsChaosCharmItem; // 조합 부적인가
#endif //PSW_ADD_MIX_CHAOSCHARM
#ifdef YDG_FIX_SOCKETSPHERE_MIXRATE
		BOOL m_bIsJewelItem;	// 보석 아이템인가 (축,영,혼,창,생,조,수,석묶음)
#endif	// YDG_FIX_SOCKETSPHERE_MIXRATE

		// 기타 정보
		BOOL m_b380AddedItem;	// 380 옵션이 이미 추가된 아이템 여부
		BOOL m_bFenrirAddedItem;	// 펜릴 옵션 추가가 된 아이템 여부
		WORD m_wHarmonyOption;		// 조화의 보석 옵션 (환원 가격 산출용)
		WORD m_wHarmonyOptionLevel;	// 조화의 보석 옵션 레벨 (환원 가격 산출용)

#ifdef ADD_SOCKET_MIX
		// 소켓아이템 정보 (소켓아이템일때만)
		BYTE m_bySocketCount;					// 소켓 개수 (0~MAX_SOCKETS), 0: 소켓 아이템 아님
		BYTE m_bySocketSeedID[MAX_SOCKETS];		// 시드 고유번호 (0~49), -1: 빈 소켓
		BYTE m_bySocketSphereLv[MAX_SOCKETS];	// 스피어 레벨 (1~5)
		// 시드시피어 아이템 정보 (시드스피어일때만)
		BYTE m_bySeedSphereID;					// 시드스피어의 시드 고유번호 (0~49), -1: 시드스피어 아님
#endif	// ADD_SOCKET_MIX
	};

	class CMixItemInventory	// 조합 인벤토리에 올려진 재료 아이템 관리 클래스
	{
	public:
		CMixItemInventory() { Reset(); }
		virtual ~CMixItemInventory() { Reset(); }
		void Reset() { m_iNumMixItems = 0; }

		int AddItem(ITEM * pItem);
		int GetNumMixItems() { return m_iNumMixItems; }
		CMixItem * GetMixItems() { return m_MixItems; }
		
	protected:
		DWORD EvaluateMixItemValue(ITEM * pItem);

	protected:
		CMixItem m_MixItems[32];
		int m_iNumMixItems;
	};

	enum _SPECIAL_ITEM_RECIPE_
	{
		RCP_SP_EXCELLENT = 1,
		RCP_SP_ADD380ITEM = 2,
		RCP_SP_SETITEM = 4,
		RCP_SP_HARMONY = 8,		// 조화의 보석 제련 아이템 여부
#ifdef ADD_SOCKET_MIX
		RCP_SP_SOCKETITEM = 16,
#endif	// ADD_SOCKET_MIX
	};

	typedef struct _MIX_RECIPE_ITEM		// 조합 재료: 스크립트(툴)와 연계되어있음 -> 설정 방식은 툴의 소스 참조
	{
		short m_sTypeMin;
		short m_sTypeMax;
		int m_iLevelMin;
		int m_iLevelMax;
		int m_iOptionMin;
		int m_iOptionMax;
		int m_iDurabilityMin;
		int m_iDurabilityMax;
		int m_iCountMin;
		int m_iCountMax;		// 동일한 아이템 개수
		DWORD m_dwSpecialItem;	// 엑설런트,380아이템,세트아이템 (_SPECIAL_ITEM_RECIPE_)
	} MIX_RECIPE_ITEM;

	enum _MIX_TYPES_
	{
		MIXTYPE_GOBLIN_NORMAL,		// 0. 고블린 일반 조합
		MIXTYPE_GOBLIN_CHAOSITEM,	// 1. 카오스 무기 아이템조합
		MIXTYPE_GOBLIN_ADD380,		// 2. 380아이템 옵션 추가
		MIXTYPE_CASTLE_SENIOR,		// 3. 원로원 성주 조합
		MIXTYPE_TRAINER,			// 4. 조련사
		MIXTYPE_OSBOURNE,			// 5. 오스본 제련석
		MIXTYPE_JERRIDON,			// 6. 제리든 조화의보석 아이템 환원
		MIXTYPE_ELPIS,				// 7. 엘피스 조화의보석
#ifdef ADD_GLOBAL_MIX_MAR08
		MIXTYPE_CHAOS_CARD,			// 8. 해외 카오스카드 조합
		MIXTYPE_CHERRYBLOSSOM,		// 9. 해외 벚꽃가지 조합
#endif	// ADD_GLOBAL_MIX_MAR08
#ifdef ADD_SOCKET_MIX
		MIXTYPE_EXTRACT_SEED,		// 10. 시드 추출
		MIXTYPE_SEED_SPHERE,		// 11. 시드 스피어 합성
		MIXTYPE_ATTACH_SOCKET,		// 12. 소켓 아이템 장착
		MIXTYPE_DETACH_SOCKET,		// 13. 소켓 아이템 해체
#endif	// ADD_SOCKET_MIX
		MAX_MIX_TYPES
	};
	#define MAX_MIX_SOURCES 8	// 최대 재료 가지 수
	#define MAX_MIX_NAMES 3		// 조합 이름 텍스트 조합 수 (%s %s %s)
	#define MAX_MIX_DESCRIPTIONS 3	// 조합 설명줄 수 (%s\n%s\n...)
	#define MAX_MIX_RATE_TOKEN 32	// txt에서 읽어들일 확률 공식의 토큰 수

	enum _MIXRATE_OPS
	{
		// 연산자
		MRCP_NUMBER = 0,
		MRCP_ADD,	// +
		MRCP_SUB,	// -
		MRCP_MUL,	// *
		MRCP_DIV,	// /
		MRCP_LP,	// (
		MRCP_RP,	// )
		MRCP_INT,	// INT()

		// 값
		MRCP_MAXRATE = 32,	// 스크립트의 최대 확률
		MRCP_ITEM,			// 조합창 아이템 가격 총합
		MRCP_WING,			// 조합창 날개 가격 총합
		MRCP_EXCELLENT,		// 조합창 엑템 가격 총합
		MRCP_EQUIP,			// 조합창 무기+방어구 가격 총합
		MRCP_SET,			// 조합창 세트아이템 가격 총합
		MRCP_LEVEL1,		// 조합 조건 첫번째 아이템의 레벨 (1~13)
#ifdef YDG_FIX_SOCKETSPHERE_MIXRATE
		MRCP_NONJEWELITEM,	// 보석가격을 제외한 조합창 아이템 가격 총합 (축,영,혼,창,생,조,수,석묶음)
#endif	/ /YDG_FIX_SOCKETSPHERE_MIXRATE

		// 조건 값
		MRCP_LUCKOPT = 64	// 아이템의 행운 옵션 보너스 값이 있으면 25, 없으면 0
	};

	typedef struct _MIXRATE_TOKEN
	{
		enum _MIXRATE_OPS op;
		float value;
	} MIXRATE_TOKEN;

	typedef struct _MIX_RECIPE	// 조합 방법: 스크립트(툴)와 연계되어있음 -> 수정시 툴도 같이 고쳐줘야함
	{
		int m_iMixIndex;	// 조합 일련번호
		int m_iMixID;		// 서버로 보낼 조합 ID
		int m_iMixName[MAX_MIX_NAMES];
		int m_iMixDesc[MAX_MIX_DESCRIPTIONS];
		int m_iMixAdvice[MAX_MIX_DESCRIPTIONS];
		int m_iWidth;
		int m_iHeight;
		int m_iRequiredLevel;
		BYTE m_bRequiredZenType;
		DWORD m_dwRequiredZen;
		int m_iNumRateData;
		MIXRATE_TOKEN m_RateToken[MAX_MIX_RATE_TOKEN];
		int m_iSuccessRate;
		BYTE m_bMixOption;
		BYTE m_bCharmOption;
#ifdef PSW_ADD_MIX_CHAOSCHARM
		BYTE m_bChaosCharmOption;
#endif //PSW_ADD_MIX_CHAOSCHARM
		MIX_RECIPE_ITEM m_MixSources[MAX_MIX_SOURCES];
		int m_iNumMixSoruces;
	} MIX_RECIPE;

	enum _MIX_SOURCE_STATUS { MIX_SOURCE_ERROR, MIX_SOURCE_NO, MIX_SOURCE_PARTIALLY, MIX_SOURCE_YES };

	class CMixRecipes	// 조합 방법 관리 클래스
	{
	public:
		CMixRecipes() { Reset(); }
		virtual ~CMixRecipes() { Reset(); }

		void Reset();
		void AddRecipe(MIX_RECIPE * pMixRecipe);
		void ClearCheckRecipeResult();	// 아이템 인벤 검사결과 초기화
		int CheckRecipe(int iNumMixItems, CMixItem * pMixItems);	// 아이템 인벤 정보와 조합 방법을 비교하여 결과 반환
		int CheckRecipeSimilarity(int iNumMixItems, CMixItem * pMixItems);	// 아이템 조합 유사도 검사

		BOOL IsMixSource(ITEM * pItem);	// 이 아이템이 조합 재료로 쓸수 있는 아이템인가
		MIX_RECIPE * GetCurRecipe();	// 현재 조합 얻기
		int GetCurMixID();	// 서버에 전달할 조합 ID 얻기
		BOOL GetCurRecipeName(unicode::t_char * pszNameOut, int iNameLine);	// 조합 이름 얻기
		BOOL GetCurRecipeDesc(unicode::t_char * pszDescOut, int iDescLine);	// 조합 도움말 얻기
		MIX_RECIPE * GetMostSimilarRecipe();	// 유사 조합 얻기
		BOOL GetMostSimilarRecipeName(unicode::t_char * pszNameOut, int iNameLine);	// 유사 조합 이름 얻기
		BOOL GetRecipeAdvice(unicode::t_char * pszAdviceOut, int iAdivceLine);	// 유사 조합의 도움말 얻기
		int GetSourceName(int iItemNum, unicode::t_char * pszNameOut, int iNumMixItems, CMixItem * pMixItems);	// 유사 조합 재료 이름 얻기
		BOOL IsReadyToMix() { return (m_iCurMixIndex > 0); }	// 조합 가능 상태인가
		int GetSuccessRate() { return m_iSuccessRate; }	// 성공율 얻기
		DWORD GetReqiredZen() { return m_dwRequiredZen; }	// 요구젠 얻기
#ifdef ADD_SOCKET_MIX
		int GetFirstItemSocketCount() { return m_byFirstItemSocketCount; }
		int GetFirstItemSocketSeedID(int iIndex) { if (iIndex >= m_byFirstItemSocketCount) return SOCKET_EMPTY; else return m_byFirstItemSocketSeedID[iIndex]; }
		int GetFirstItemSocketShpereLv(int iIndex) { if (iIndex >= m_byFirstItemSocketCount) return 0; else return m_byFirstItemSocketSphereLv[iIndex]; }
#endif	// ADD_SOCKET_MIX

		void CalcCharmBonusRate(int iNumMixItems, CMixItem * pMixItems);	// 행운의 부적아이템 확률 증가 총합 계산
#ifdef PSW_ADD_MIX_CHAOSCHARM
		void CalcChaosCharmCount(int iNumMixItems, CMixItem * pMixItems);
#endif //PSW_ADD_MIX_CHAOSCHARM

#ifdef LJH_MOD_CANNOT_USE_CHARMITEM_AND_CHAOSCHARMITEM_SIMULTANEOUSLY
		WORD GetTotalChaosCharmCount()	{ return m_wTotalChaosCharmCount; }
		WORD GetTotalCharmCount()		{ return m_wTotalCharmBonus; }
#endif //LJH_MOD_CANNOT_USE_CHARMITEM_AND_CHAOSCHARMITEM_SIMULTANEOUSLY

	protected:
		bool IsOptionItem(MIX_RECIPE_ITEM & rItem) { return (rItem.m_iCountMin == 0); }	// 옵션(안넣어도 되는) 아이템인가
		BOOL CheckRecipeSub(std::vector<MIX_RECIPE *>::iterator iter, int iNumMixItems, CMixItem * pMixItems);
		int CheckRecipeSimilaritySub(std::vector<MIX_RECIPE *>::iterator iter, int iNumMixItems, CMixItem * pMixItems);	// 유사도 비교
		bool CheckItem(MIX_RECIPE_ITEM & rItem, CMixItem & rSource);	// 같은 아이템인지 비교
		void EvaluateMixItems(int iNumMixItems, CMixItem * pMixItems);
		void CalcMixRate(int iNumMixItems, CMixItem * pMixItems);
		void CalcMixReqZen(int iNumMixItems, CMixItem * pMixItems);
		BOOL GetRecipeName(MIX_RECIPE * pRecipe, unicode::t_char * pszNameOut, int iNameLine, BOOL bSimilarRecipe);	// 주어진 조합법의 이름 얻기

		// 조합옵션 조건 검사용
		BOOL IsChaosItem(CMixItem & rSource);	// 카오스무기인가
		BOOL IsChaosJewel(CMixItem & rSource);	// 혼석인가
		BOOL Is380AddedItem(CMixItem & rSource);	// 380 추가옵션이 이미 들어있는 아이템인가
		BOOL IsFenrirAddedItem(CMixItem & rSource);	// 펜릴 추가옵션이 이미 들어있는 아이템인가
		BOOL IsUpgradableItem(CMixItem & rSource);	// 레벨업 가능한 아이템인가(무기+날개+망토)
		BOOL IsSourceOfRefiningStone(CMixItem & rSource);	// 제련석의 재료인가
		BOOL IsCharmItem(CMixItem & rSource);	// 행운의 부적인가
#ifdef PSW_ADD_MIX_CHAOSCHARM
		BOOL IsChaosCharmItem(CMixItem & rSource);
#endif //PSW_ADD_MIX_CHAOSCHARM
#ifdef YDG_FIX_SOCKETSPHERE_MIXRATE
		BOOL IsJewelItem(CMixItem & rSource);	// 보석아이템인가 (축,영,혼,창,생,조,수,석묶음)
#endif	// YDG_FIX_SOCKETSPHERE_MIXRATE
#ifdef YDG_FIX_SOCKET_ATTACH_CONDITION
		BOOL IsSourceOfAttachSeedSphereToWeapon(CMixItem & rSource);	// 무기에 장착할 수 있는 시드스피어인가
		BOOL IsSourceOfAttachSeedSphereToArmor(CMixItem & rSource);		// 방어구에 장착할 수 있는 시드스피어인가
#endif	// YDG_FIX_SOCKET_ATTACH_CONDITION

		// 성공률 계산
		float MixrateAddSub();
		float MixrateMulDiv();
		float MixrateFactor();
	protected:
		std::vector<MIX_RECIPE *> m_Recipes;
		int m_iCurMixIndex;
		int m_iMostSimilarMixIndex;

		int m_iSuccessRate;
		DWORD m_dwRequiredZen;

		BOOL m_bFindMixLuckItem;		// 조합 인벤상에 행운 아이템이 있는가
		DWORD m_dwTotalItemValue;		// 총 아이템 가격
		DWORD m_dwExcellentItemValue;	// 총 엑템 가격
		DWORD m_dwEquipmentItemValue;	// 총 무기/방어구 가격
		DWORD m_dwWingItemValue;		// 총 날개 가격
		DWORD m_dwSetItemValue;			// 총 세트 아이템 가격
		DWORD m_iFirstItemLevel;		// 첫번째 재료의 레벨
		int m_iFirstItemType;			// 첫번째 재료의 타입(380용)
#ifdef YDG_FIX_SOCKETSPHERE_MIXRATE
		DWORD m_dwTotalNonJewelItemValue;	// 보석을 제외한 총 아이템 가격
#endif	// YDG_FIX_SOCKETSPHERE_MIXRATE
#ifdef ADD_SOCKET_MIX
		// 첫번째 재료의 소켓 정보
		BYTE m_byFirstItemSocketCount;					// 소켓 개수 (0~MAX_SOCKETS), 0: 소켓 아이템 아님
		BYTE m_byFirstItemSocketSeedID[MAX_SOCKETS];	// 시드 고유번호 (0~49), -1: 빈 소켓
		BYTE m_byFirstItemSocketSphereLv[MAX_SOCKETS];	// 스피어 레벨 (1~5)
#endif	// ADD_SOCKET_MIX
		WORD m_wTotalCharmBonus;		// 총 행운의 부적 증가 확률
#ifdef PSW_ADD_MIX_CHAOSCHARM
		WORD m_wTotalChaosCharmCount;
#endif //PSW_ADD_MIX_CHAOSCHARM

		// 유사도 검사후 아이템 목록 출력시 인벤토리에 해당 아이템이 있으면 색을 바꿔주기 위한 시험 변수
		int m_iMixSourceTest[MAX_MIX_SOURCES];
		int m_iMostSimilarMixSourceTest[MAX_MIX_SOURCES];	// 유사 조합법 재료중 부족한 수

		// 성공률 계산용
		int m_iMixRateIter;		// 성공률 계산용 변수 (iter)
		MIXRATE_TOKEN * m_pMixRates;
	};

	class CMixRecipeMgr
	{
	public:
		CMixRecipeMgr():m_iMixType(0),m_bIsMixInit(TRUE)
		{
#ifdef ADD_SOCKET_MIX
			m_iMixSubType = 0;
#endif	// ADD_SOCKET_MIX
			m_btPlusChaosRate = 0;
#ifdef USE_MIXTEST_BMD
			OpenRecipeFile("Data\\Local\\Mixtest.bmd");
#else	// USE_MIXTEST_BMD
			OpenRecipeFile("Data\\Local\\Mix.bmd");
#endif	// USE_MIXTEST_BMD
		}
		virtual ~CMixRecipeMgr() {}

		void SetMixType(int iMixType) { m_iMixType = iMixType; }	// 조합창 상태 설정-0:일반조합,1:카오스아이템,2:380옵션추가
		int GetMixInventoryType();	// 조합창 상태 얻기-0:일반조합,1:카오스아이템,2:380옵션추가
		int GetMixInventoryEquipmentIndex();	// 현재 조합창 인덱스 얻기 (SendRequestEquipmentItem로 서버에 보낼)

		void ResetMixItemInventory();
		void AddItemToMixItemInventory(ITEM * pItem);
		void CheckMixInventory();

#ifdef ADD_SOCKET_MIX
		void SetMixSubType(int iMixSubType) { m_iMixSubType = iMixSubType; }
		int GetMixSubType() { return m_iMixSubType; }
#endif	// ADD_SOCKET_MIX

		BOOL IsMixSource(ITEM * pItem)	// 이 아이템이 조합 재료로 쓸수 있는 아이템인가
		{
			m_MixRecipe[GetMixInventoryType()].CalcCharmBonusRate(m_MixItemInventory.GetNumMixItems(), m_MixItemInventory.GetMixItems());
#ifdef PSW_ADD_MIX_CHAOSCHARM
			m_MixRecipe[GetMixInventoryType()].CalcChaosCharmCount(m_MixItemInventory.GetNumMixItems(), m_MixItemInventory.GetMixItems());
#endif //PSW_ADD_MIX_CHAOSCHARM
			return m_MixRecipe[GetMixInventoryType()].IsMixSource(pItem);
		}
		int CheckRecipe(int iNumMixItems, CMixItem * pMixItems)	// 조합 검사
		{
			return m_MixRecipe[GetMixInventoryType()].CheckRecipe(iNumMixItems, pMixItems);
		}
		int CheckRecipeSimilarity(int iNumMixItems, CMixItem * pMixItems)	// 유사 조합 검사
		{
			return m_MixRecipe[GetMixInventoryType()].CheckRecipeSimilarity(iNumMixItems, pMixItems);
		}
		BOOL IsReadyToMix()	// 조합 가능 상태인가
		{
			return m_MixRecipe[GetMixInventoryType()].IsReadyToMix();
		}
		MIX_RECIPE * GetCurRecipe()	// 현재 조합 얻기
		{
			return m_MixRecipe[GetMixInventoryType()].GetCurRecipe();
		}
		int GetSuccessRate()	// 성공율 얻기
		{
			return m_MixRecipe[GetMixInventoryType()].GetSuccessRate();
		}
		int GetReqiredZen()	// 요구젠 얻기
		{
			return m_MixRecipe[GetMixInventoryType()].GetReqiredZen();
		}
		int GetCurMixID()	// 서버에 전달할 조합 ID 얻기
		{
			return m_MixRecipe[GetMixInventoryType()].GetCurMixID();
		}
		BOOL GetCurRecipeName(unicode::t_char * pszNameOut, int iNameLine)	// 조합 이름 얻기
		{
			return m_MixRecipe[GetMixInventoryType()].GetCurRecipeName(pszNameOut, iNameLine);
		}
		BOOL GetCurRecipeDesc(unicode::t_char * pszDescOut, int iDescLine)	// 조합 도움말 얻기
		{
			return m_MixRecipe[GetMixInventoryType()].GetCurRecipeDesc(pszDescOut, iDescLine);
		}
		MIX_RECIPE * GetMostSimilarRecipe()	// 유사 조합 얻기
		{
			return m_MixRecipe[GetMixInventoryType()].GetMostSimilarRecipe();
		}
		BOOL GetMostSimilarRecipeName(unicode::t_char * pszNameOut, int iNameLine)	// 유사 조합 이름 얻기
		{
			return m_MixRecipe[GetMixInventoryType()].GetMostSimilarRecipeName(pszNameOut, iNameLine);
		}
		BOOL GetRecipeAdvice(unicode::t_char * pszAdviceOut, int iAdivceLine)	// 유사 조합의 도움말 얻기
		{
			return m_MixRecipe[GetMixInventoryType()].GetRecipeAdvice(pszAdviceOut, iAdivceLine);
		}
		int GetSourceName(int iItemNum, unicode::t_char * pszNameOut)	// 유사 조합 재료 이름 얻기
		{
			return m_MixRecipe[GetMixInventoryType()].GetSourceName(iItemNum, pszNameOut, m_MixItemInventory.GetNumMixItems(),
				m_MixItemInventory.GetMixItems());
		}
		void ClearCheckRecipeResult()	// 조합 검사 결과 초기화
		{
			m_MixRecipe[GetMixInventoryType()].ClearCheckRecipeResult();
			SetMixType(0);
		}
		BOOL IsMixInit() { return m_bIsMixInit; }

		void SetPlusChaosRate(BYTE btPlusChaosRate) { m_btPlusChaosRate = btPlusChaosRate; }
		BYTE GetPlusChaosRate() { return m_btPlusChaosRate; }
#ifdef ADD_SOCKET_MIX
		int GetFirstItemSocketCount() { return m_MixRecipe[GetMixInventoryType()].GetFirstItemSocketCount(); }
		int GetFirstItemSocketSeedID(int iIndex) { if (iIndex >= GetFirstItemSocketCount()) return SOCKET_EMPTY; else return m_MixRecipe[GetMixInventoryType()].GetFirstItemSocketSeedID(iIndex); }
		int GetFirstItemSocketShpereLv(int iIndex) { if (iIndex >= GetFirstItemSocketCount()) return 0; else return m_MixRecipe[GetMixInventoryType()].GetFirstItemSocketShpereLv(iIndex); }
		int GetSeedSphereID(int iOrder);	// 재료 시드 스피어의 ID를 얻는다. 더이상 없으면SOCKET_EMPTY 반환 (SOCKET_EMPTY이 나올때까지 값을 얻어낼 수 있다)
#endif	// ADD_SOCKET_MIX
#ifdef LJH_MOD_CANNOT_USE_CHARMITEM_AND_CHAOSCHARMITEM_SIMULTANEOUSLY
		WORD GetTotalChaosCharmCount()
		{ 
			m_MixRecipe[GetMixInventoryType()].CalcChaosCharmCount(m_MixItemInventory.GetNumMixItems(), m_MixItemInventory.GetMixItems());
			return m_MixRecipe[GetMixInventoryType()].GetTotalChaosCharmCount(); 
		}
		WORD GetTotalCharmCount()
		{ 
			m_MixRecipe[GetMixInventoryType()].CalcCharmBonusRate(m_MixItemInventory.GetNumMixItems(), m_MixItemInventory.GetMixItems());
			return m_MixRecipe[GetMixInventoryType()].GetTotalCharmCount(); 
		}
#endif //LJH_MOD_CANNOT_USE_CHARMITEM_AND_CHAOSCHARMITEM_SIMULTANEOUSLY

	protected:
		void OpenRecipeFile(const unicode::t_char * szFileName);	// mix.bmd를 읽어들인다.

	protected:
		CMixRecipes m_MixRecipe[MAX_MIX_TYPES];
		CMixItemInventory m_MixItemInventory;
		int m_iMixType;	// 조합창 상태-0:일반조합,1:카오스아이템,2:380옵션추가
#ifdef ADD_SOCKET_MIX
		int m_iMixSubType;	// 조합창 서브타입(소켓아이템 소켓 번호)
#endif	// ADD_SOCKET_MIX
		BOOL m_bIsMixInit;	// 조합 초기 상태인가 (아이템 하나도 올리지 않은 상태)
		BYTE m_btPlusChaosRate;	// 크라이울프 성공 서버에서의 확률 보너스 (서버로부터 받음)
	};
}

extern SEASON3A::CMixRecipeMgr g_MixRecipeMgr;

#endif	// _MIX_MGR_H_
