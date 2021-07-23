// ChangeRingManager.cpp: implementation of the CChangeRingManager class.
//
//////////////////////////////////////////////////////////////////////

// common
#include "stdafx.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "ChangeRingManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChangeRingManager::CChangeRingManager()
{

}

CChangeRingManager::~CChangeRingManager()
{

}

void CChangeRingManager::LoadItemModel()
{
	AccessModel(MODEL_HELPER+10,"Data\\Item\\","Ring"  ,1);         //  변신 반지.

	// 엘리트 해골전사 반지 모델 데이타 로딩
	AccessModel(MODEL_HELPER+39,"Data\\Item\\","Ring"  ,1);         //  엘리트 해골전사 변신반지

	// 할로윈 이벤트 변신 반지 모델 데이타 로딩
	AccessModel(MODEL_HELPER+40,"Data\\Item\\","Ring"  ,1);         //  잭 오랜턴 변신반지

	// 크리스마스 이벤트 변신 반지 모델 데이타 로딩
	AccessModel(MODEL_HELPER+41,"Data\\Item\\","Ring"  ,1);         //  산타걸 변신반지

	// GM 변신 반지 모델 데이타 로딩
	AccessModel(MODEL_HELPER+42,"Data\\Item\\","Ring"  ,1);         //  GM 변신반지

#ifdef LDK_ADD_SNOWMAN_CHANGERING
	// 크리스마스 ㅋ눈사람 변신 반지 데이타 로딩
	AccessModel(MODEL_HELPER+68,"Data\\Item\\xmas\\","xmasring");			//  눈사람 변신반지
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
	AccessModel(MODEL_HELPER+76,"Data\\Item\\","PandaPetRing");
#endif //PJH_ADD_PANDA_CHANGERING
	
#ifdef YDG_ADD_SKELETON_CHANGE_RING
	AccessModel(MODEL_HELPER+122, "Data\\Item\\", "SkeletonRing");	// 스켈레톤 변신반지
#endif	// YDG_ADD_SKELETON_CHANGE_RING
}

void CChangeRingManager::LoadItemTexture()
{
	OpenTexture(MODEL_HELPER+39, "Item\\"); // 엘리트 해골전사 변신반지

	OpenTexture(MODEL_HELPER+41, "Item\\"); // 산타 변신반지

	OpenTexture(MODEL_HELPER+40, "Item\\"); // 잭 오랜턴 변신반지

	OpenTexture(MODEL_HELPER+42, "Item\\"); // GM 변신반지

#ifdef LDK_ADD_SNOWMAN_CHANGERING
	// 크리스마스 눈사람 변신 반지 데이타 로딩
	OpenTexture(MODEL_HELPER+68, "Item\\xmas\\");			//  눈사람 변신반지
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
	OpenTexture(MODEL_HELPER+76, "Item\\");			//  눈사람 변신반지
#endif //PJH_ADD_PANDA_CHANGERING

#ifdef YDG_ADD_SKELETON_CHANGE_RING
	OpenTexture(MODEL_HELPER+122, "Item\\");	// 스켈레톤 변신반지
#endif	// YDG_ADD_SKELETON_CHANGE_RING
}

bool CChangeRingManager::CheckDarkLordHair(int iType)
{
	// 해골변신반지를 착용하면 다크로드 머리 안보이게 수정
	if((iType >= MODEL_SKELETON1 && iType <= MODEL_SKELETON3) || iType == MODEL_SKELETON_PCBANG)
	{
		return true;
	}
	// 할로윈 이벤트 변신반지 착용하면 다크로드 머리 안보이게 함
	if(iType == MODEL_HALLOWEEN)
	{
		return true;
	}
	// 크리스마스 이벤트 변신반지 착용하면 다크로드 머리 안보이게 함
	if(iType == MODEL_XMAS_EVENT_CHANGE_GIRL)
	{
		return true;
	}
	// GM캐릭터인 경우(반지 착용) 다크로드 머리 안보이게 함.
	if(iType == MODEL_GM_CHARACTER)
	{
		return true;
	}
	// 환영사원 캐릭터 모델이면 다크로드 머리 안보이게 함.
	if(iType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER || iType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER)
	{
		return true;
	}

#ifdef LDK_ADD_SNOWMAN_CHANGERING
	// 눈사람 캐릭터 모델이면 다크로드 머리 안보이게 함.
	if(iType == MODEL_XMAS2008_SNOWMAN)
	{
		return true;
	}
#endif //LDK_ADD_SNOWMAN_CHANGERING

#ifdef PJH_ADD_PANDA_CHANGERING
	if(iType == MODEL_PANDA)
	{
		return true;
	}
#endif //PJH_ADD_PANDA_CHANGERING

#ifdef YDG_ADD_SKELETON_CHANGE_RING
	if (iType == MODEL_SKELETON_CHANGED)	// 스켈레톤 변신반지
	{
		return true;
	}
#endif	// YDG_ADD_SKELETON_CHANGE_RING

	return false;
}

bool CChangeRingManager::CheckDarkCloak(int iClass, int iType)
{
	// 캐릭터가 마검사이고
	if(GetCharacterClass(iClass) == CLASS_DARK)
	{
		// 할로윈 이벤트 변신반지 착용하면 마검사 망토 안보이게 수정
		if(iType == MODEL_HALLOWEEN)
		{
			return true;
		}

		// 산타 변신반지 착용하면 마검사 망토 안보이게
		if(iType == MODEL_XMAS_EVENT_CHANGE_GIRL)
		{
			return true;
		}

		// GM 변신반지 착용하면 마검사 망토 안보이게
		if(iType == MODEL_GM_CHARACTER)
		{
			return true;
		}

#ifdef LDK_ADD_SNOWMAN_CHANGERING
		// 눈사람 변신반지 착용하면 마검사 망토 안보이게
	 	if(iType == MODEL_XMAS2008_SNOWMAN)
	 	{
	 		return true;
	 	}
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
	 	if(iType == MODEL_PANDA)
	 	{
	 		return true;
	 	}
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		if (iType == MODEL_SKELETON_CHANGED)	// 스켈레톤 변신반지
		{
			return true;
		}
#endif	// YDG_ADD_SKELETON_CHANGE_RING
	}

	return false;
}

bool CChangeRingManager::CheckChangeRing(short RingType)
{
	// 변신반지를 착용한 경우
	if(RingType == ITEM_HELPER+10)
	{
		return true;
	}
	// 이벤트 해골 변신 반지 착용하면 칸투르 3차로 못가게 예외 처리
	if(RingType == ITEM_HELPER+39)
	{
		return true;
	}
	// 할로윈 이벤트 호박변신반지 착용하면 칸투르 3차로 못가게 예외 처리
	if(RingType == ITEM_HELPER+40)
	{
		return true;
	}
	// 크리스마스 이벤트 산타변신반지 착용하면 칸투르 3차로 못가게 예외 처리
	if(RingType == ITEM_HELPER+41)
	{	
		return true;
	}
	// GM 변신반지 착용하면 칸투르 3차로 못가게 예외 처리
	if(RingType == ITEM_HELPER+42)
	{	
		return true;
	}

#ifdef LDK_ADD_SNOWMAN_CHANGERING
	// 눈사람변신반지 착용하면 칸투르 3차로 못가게 예외 처리
 	if(RingType == ITEM_HELPER+68)
 	{
 		return true;
 	}
#endif //LDK_ADD_SNOWMAN_CHANGERING

#ifdef PJH_ADD_PANDA_CHANGERING
 	if(RingType == ITEM_HELPER+76)
 	{
 		return true;
 	}
#endif //PJH_ADD_PANDA_CHANGERING

#ifdef YDG_ADD_SKELETON_CHANGE_RING
	if (RingType == ITEM_HELPER+122)	// 스켈레톤 변신반지
	{
		return true;
	}
#endif	// YDG_ADD_SKELETON_CHANGE_RING

	return false;
}

bool CChangeRingManager::CheckRepair(int iType)
{
	// 일반 변신반지 수리 정보 안보이게 예외 처리
	if(iType == ITEM_HELPER+10)
	{
		return true;
	}
	// 피시방 이벤트 변신반지 
	if(iType == ITEM_HELPER+39)
	{
		return true;
	}
	// 할로윈 이벤트 변신반지 수정 정보 안보이게 예외 처리
	if(iType == ITEM_HELPER+40)
	{
		return true;
	}
	// 크리스마스 이벤트 변신반지 수정 정보 안보이게 예외 처리
	if(iType == ITEM_HELPER+41)
	{
		return true;
	}
	// GM 변신반지 수정 정보 안보이게 예외 처리
	if(iType == ITEM_HELPER+42)
	{
		return true;
	}

#ifdef LDK_ADD_SNOWMAN_CHANGERING
	// 눈사람변신반지 수리 정보 안보이게 예외 처리
 	if(iType == ITEM_HELPER+68)
 	{
 		return true;
 	}
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
 	if(iType == ITEM_HELPER+76)
 	{
 		return true;
 	}
#endif //PJH_ADD_PANDA_CHANGERING

#ifdef YDG_ADD_SKELETON_CHANGE_RING
	if (iType == ITEM_HELPER+122)	// 스켈레톤 변신반지
	{
		return true;
	}
#endif	// YDG_ADD_SKELETON_CHANGE_RING

	return false;
}

bool CChangeRingManager::CheckMoveMap(short sLeftRingType, short sRightRingType)
{
	if(sLeftRingType == ITEM_HELPER+10 || sRightRingType == ITEM_HELPER+10)
	{
		return true;
	}
	// 엘리트 해골전사 변신반지 착용하면 맵 이름 빨갛게 처리
	if(sLeftRingType == ITEM_HELPER+39 || sRightRingType == ITEM_HELPER+39)
	{
		return true;
	}
	//CSK : 할로윈 이벤트 변신반지 착용하면 맵 이름 빨갛게 처리
	if(sLeftRingType == ITEM_HELPER+40 || sRightRingType == ITEM_HELPER+40)
	{
		return true;
	}
	// 크리스마스 이벤트 변신반지 착용하면 맵 이름 빨갛게 처리
	if(sLeftRingType == ITEM_HELPER+41 || sRightRingType == ITEM_HELPER+41)
	{
		return true;
	}
	// GM 변신반지 착용하면 맵 이름 빨갛게 처리
	if(sLeftRingType == ITEM_HELPER+42 || sRightRingType == ITEM_HELPER+42)
	{
		return true;
	}

#ifdef LDK_ADD_SNOWMAN_CHANGERING
	// 눈사람변신반지 착용하면 맵 이름 빨갛게 처리
 	if(sLeftRingType == ITEM_HELPER+68 || sRightRingType == ITEM_HELPER+68)
 	{
 		return true;
 	}
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
 	if(sLeftRingType == ITEM_HELPER+76 || sRightRingType == ITEM_HELPER+76)
 	{
 		return true;
 	}
#endif //PJH_ADD_PANDA_CHANGERING

#ifdef YDG_ADD_SKELETON_CHANGE_RING
	if (sLeftRingType == ITEM_HELPER+122 || sRightRingType == ITEM_HELPER+122)	// 스켈레톤 변신반지
	{
		return true;
	}
#endif	// YDG_ADD_SKELETON_CHANGE_RING

	return false;
}

#ifdef KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING
bool CChangeRingManager::CheckBanMoveIcarusMap(short sLeftRingType, short sRightRingType)
{
	if(sLeftRingType == ITEM_HELPER+10 || sRightRingType == ITEM_HELPER+10)
	{
		return true;
	}
	// 엘리트 해골전사 변신반지 착용하면 맵 이름 빨갛게 처리
	if(sLeftRingType == ITEM_HELPER+39 || sRightRingType == ITEM_HELPER+39)
	{
		return true;
	}
	//CSK : 할로윈 이벤트 변신반지 착용하면 맵 이름 빨갛게 처리
	if(sLeftRingType == ITEM_HELPER+40 || sRightRingType == ITEM_HELPER+40)
	{
		return true;
	}
	// 크리스마스 이벤트 변신반지 착용하면 맵 이름 빨갛게 처리
	if(sLeftRingType == ITEM_HELPER+41 || sRightRingType == ITEM_HELPER+41)
	{
		return true;
	}
	// GM 변신반지 착용하면 맵 이름 빨갛게 처리
	if(sLeftRingType == ITEM_HELPER+42 || sRightRingType == ITEM_HELPER+42)
	{
		return true;
	}
	
#ifdef LDK_ADD_SNOWMAN_CHANGERING
	// 눈사람변신반지 착용하면 맵 이름 빨갛게 처리
	if(sLeftRingType == ITEM_HELPER+68 || sRightRingType == ITEM_HELPER+68)
	{
		return true;
	}
#endif //LDK_ADD_SNOWMAN_CHANGERING

#ifndef YDG_FIX_MOVE_ICARUS_EQUIPED_SKELETON_CHANGE_RING	// 정리할때 삭제할부분
#ifdef YDG_ADD_SKELETON_CHANGE_RING
	if (sLeftRingType == ITEM_HELPER+122 || sRightRingType == ITEM_HELPER+122)	// 스켈레톤 변신반지
	{
		return true;
	}
#endif	// YDG_ADD_SKELETON_CHANGE_RING
#endif	// YDG_FIX_MOVE_ICARUS_EQUIPED_SKELETON_CHANGE_RING	// 정리할때 삭제할부분

	return false;
}
#endif // KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING

