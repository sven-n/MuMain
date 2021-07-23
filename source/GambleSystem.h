// GambleSystem.h: interface for the GembleSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEMBLESYSTEM_H__366551E9_005D_44AC_844B_B78AC928455B__INCLUDED_)
#define AFX_GEMBLESYSTEM_H__366551E9_005D_44AC_844B_B78AC928455B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef LDK_ADD_GAMBLE_SYSTEM

typedef struct _buyItemInfo
{
	int ItemIndex;
	DWORD ItemCost;

	_buyItemInfo()
	{
		ItemIndex = 0;
		ItemCost = 0;
	}
}BUYITEMINFO, *LPBUYITEMINFO;

class GambleSystem
{
private:
	bool m_isGambleShop;	//겜블상점 모드.
	BYTE m_byBuyItemPos;	//구입할 아이템 위치.
	
	BUYITEMINFO	m_itemInfo;		// 구입 아이템 정보

public:
	static GambleSystem& Instance();

	virtual ~GambleSystem();

	void Init();

	void SetGambleShop(bool isGambleshop = true)	{ m_isGambleShop = isGambleshop; }	// 겜블 여부 세팅.
	bool IsGambleShop()								{ return m_isGambleShop; }			// 겜블상점인가?

	// 구입 아이템 정보
	void SetBuyItemInfo(int index, DWORD cost)	{ m_itemInfo.ItemIndex = index; m_itemInfo.ItemCost = cost; }
	LPBUYITEMINFO GetBuyItemInfo()				{ return &m_itemInfo; }

private:
	GambleSystem() { Init(); }

};

#endif //LDK_ADD_GAMBLE_SYSTEM

#endif // !defined(AFX_GEMBLESYSTEM_H__366551E9_005D_44AC_844B_B78AC928455B__INCLUDED_)
