// w_ShopItemInfo.h: interface for the ShopItem class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_BUILDER

#if !defined(AFX_W_SHOPITEMINFO_H__861D6852_5155_4F61_92F3_F1957F3DB763__INCLUDED_)
#define AFX_W_SHOPITEMINFO_H__861D6852_5155_4F61_92F3_F1957F3DB763__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

BoostSmartPointer( ShopItemInfo );
class ShopItemInfo
{
public:
	static ShopItemInfoPtr Make();
	virtual ~ShopItemInfo();

public:
	void LoadFromFile( const string& fileName );
	SHOPCATEGORYITEM& QueryShopCategoryItem( SHOPCATEGORYITEM& scitem, const eShopType type, const int itemtype, const int itemindex );

private:
	void Initialize();
	void Destroy();
	ShopItemInfo();

private:
	typedef map< DWORD, ShopCategoryItem_Map >				ShopItemInfo_Map;
	ShopItemInfo_Map										m_ShopInfoChargeItem;
	ShopItemInfo_Map										m_ShopInfoNoChargeItem;
};

#endif // !defined(AFX_W_SHOPITEMINFO_H__861D6852_5155_4F61_92F3_F1957F3DB763__INCLUDED_)

#endif //NEW_USER_INTERFACE_BUILDER