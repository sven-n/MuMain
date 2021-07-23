// w_ShopUIInfo.h: interface for the ShopUIInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_BUILDER

#if !defined(AFX_W_SHOPUIINFO_H__7FE2742E_BCF7_4DAE_8493_F897AD7C7D3D__INCLUDED_)
#define AFX_W_SHOPUIINFO_H__7FE2742E_BCF7_4DAE_8493_F897AD7C7D3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

BoostSmartPointer( ShopUIInfo );
class ShopUIInfo  
{
public:
	static ShopUIInfoPtr Make();
	virtual ~ShopUIInfo();

public:
	void QueryHighShopCategory( eShopType shoptype, ShopCategory_List& outdata );
	void QueryLowShopCategory( eShopType shoptype, eHighCategoryType highcategory, ShopCategory_List& outdata );

private:
	void Initialize();
	void Destroy();
	void LoadFromFile( const string& fileName );
	ShopUIInfo();
		
private:
	ShopCategory_List					m_ShopInfoCharge;
	ShopCategory_List					m_ShopInfoNoCharge;

	ShopCategory_Map					m_ShopInfoChargeMap;
	ShopCategory_Map					m_ShopInfoNoChargeMap;
};

#endif // !defined(AFX_W_SHOPUIINFO_H__7FE2742E_BCF7_4DAE_8493_F897AD7C7D3D__INCLUDED_)

#endif //NEW_USER_INTERFACE_BUILDER