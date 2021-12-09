// GambleSystem.h: interface for the GembleSystem class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEMBLESYSTEM_H__366551E9_005D_44AC_844B_B78AC928455B__INCLUDED_)
#define AFX_GEMBLESYSTEM_H__366551E9_005D_44AC_844B_B78AC928455B__INCLUDED_

#pragma once

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
	bool m_isGambleShop;
	BYTE m_byBuyItemPos;
	
	BUYITEMINFO	m_itemInfo;

public:
	static GambleSystem& Instance();

	virtual ~GambleSystem();

	void Init();

	void SetGambleShop(bool isGambleshop = true)	{ m_isGambleShop = isGambleshop; }
	bool IsGambleShop()								{ return m_isGambleShop; }

	void SetBuyItemInfo(int index, DWORD cost)	{ m_itemInfo.ItemIndex = index; m_itemInfo.ItemCost = cost; }
	LPBUYITEMINFO GetBuyItemInfo()				{ return &m_itemInfo; }

private:
	GambleSystem() { Init(); }

};

#endif //LDK_ADD_GAMBLE_SYSTEM

#endif // !defined(AFX_GEMBLESYSTEM_H__366551E9_005D_44AC_844B_B78AC928455B__INCLUDED_)
