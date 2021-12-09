// ChangeRingManager.h: interface for the CChangeRingManager class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANGERINGMANAGER_H__ABB6CFCF_B65B_48BB_B54B_73685EE2E30A__INCLUDED_)
#define AFX_CHANGERINGMANAGER_H__ABB6CFCF_B65B_48BB_B54B_73685EE2E30A__INCLUDED_

#pragma once

class CChangeRingManager  
{
public:
	CChangeRingManager();
	virtual ~CChangeRingManager();
	static CChangeRingManager* GetInstance() { static CChangeRingManager s_Instance; return &s_Instance; }
		void LoadItemModel();
	void LoadItemTexture();
	bool CheckDarkLordHair(int iType);
	bool CheckDarkCloak(int iClass, int iType);
	bool CheckChangeRing(short RingType);
	bool CheckRepair(int iType);
	bool CheckMoveMap(short sLeftRingType, short sRightRingType);
	bool CheckBanMoveIcarusMap(short sLeftRingType, short sRightRingType);
};

#define g_ChangeRingMgr CChangeRingManager::GetInstance()

#endif // !defined(AFX_CHANGERINGMANAGER_H__ABB6CFCF_B65B_48BB_B54B_73685EE2E30A__INCLUDED_)
