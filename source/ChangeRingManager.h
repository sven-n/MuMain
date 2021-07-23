// ChangeRingManager.h: interface for the CChangeRingManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANGERINGMANAGER_H__ABB6CFCF_B65B_48BB_B54B_73685EE2E30A__INCLUDED_)
#define AFX_CHANGERINGMANAGER_H__ABB6CFCF_B65B_48BB_B54B_73685EE2E30A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CChangeRingManager  
{
public:
	CChangeRingManager();
	virtual ~CChangeRingManager();
	static CChangeRingManager* GetInstance() { static CChangeRingManager s_Instance; return &s_Instance; }
	
	// 변신반지 모델 데이타 로딩하는 함수
	void LoadItemModel();

	// 변신반지 모델 텍스쳐 로딩하는 함수
	void LoadItemTexture();

	// 다크로드 머리 안보이게 예외처리 해주는 함수
	bool CheckDarkLordHair(int iType);

	// 마검사 망토 안보이게 예외처리 해주는 함수
	bool CheckDarkCloak(int iClass, int iType);

	// 칸투르3차로 못가게 예외처리
	bool CheckChangeRing(short RingType);

	// 수리안되게 예외처리
	bool CheckRepair(int iType);

	// 변신반지 착용하면 이동못하는 맵 빨갛게 예외처리 해주는 함수
	bool CheckMoveMap(short sLeftRingType, short sRightRingType);
	
	// 변신반지 착용시 이카루스로 이동못하게 하는 함수 (이카루스만 해당)
#ifdef KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING
	bool CheckBanMoveIcarusMap(short sLeftRingType, short sRightRingType);
#endif // KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING

};

#define g_ChangeRingMgr CChangeRingManager::GetInstance()

#endif // !defined(AFX_CHANGERINGMANAGER_H__ABB6CFCF_B65B_48BB_B54B_73685EE2E30A__INCLUDED_)
