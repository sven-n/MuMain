// w_ShopServerProxy.h: interface for the ShopServerProxy class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_PROXY

#if !defined(AFX_W_SHOPSERVERPROXY_H__4DE81AC1_61AC_4BFD_A71C_80D07EEAD841__INCLUDED_)
#define AFX_W_SHOPSERVERPROXY_H__4DE81AC1_61AC_4BFD_A71C_80D07EEAD841__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_ServerProxy.h"

//Handler에 Virtual 함수만 추가 하자.
//순수 가장 함수로 만들지 말자.--;;;
struct ShopServerProxyHandler 
{
	virtual ~ShopServerProxyHandler() = 0 {}

	//in-out
	virtual void OnShopIn( const eShopType shoptype ) {}
	virtual void OnShopOunt( const eShopType shoptype ) {}

	//update
	virtual void OnChangeShopType( const eShopType type ) {}
	virtual void OnChangeCashPoint( const DWORD cashpoint ) {}
	virtual void OnUpDateShopItemInfo( const eCashShopResult result ) {}
	virtual void OnChangeEquipmentItem( const CASHSHOP_ITEMLIST& item ) {}

	//result
	virtual void OnCashShopPurchaseResult() {}
};

BoostSmartPointer( ShopServerProxy );
class ShopServerProxy : public MESSAGEBOXHANDLER, public LocalServerProxy<ShopServerProxyHandler>
{
public:
	static ShopServerProxyPtr Make( BoostWeak_Ptr(ServerProxy) proxy );
	void SetMessageBox( int id );
	void SetDisconnectServer( bool isdisconnect = true );
	void SetClientShopOut();
	void SetShopInit();
	virtual ~ShopServerProxy();

public:
	const eShopType         GetShopType() const;
	const eHighCategoryType GetHighCategoryType() const;
	const eLowCategoryType  GetLowCategoryType() const;
	const DWORD				GetCashPoint() const;
	const DWORD				GetLowCategoryPageCount() const;

public:
	//부분유료화 상점이 디폴트임
	void SetShopIn( const eShopType shoptype = eShop_PartCharge );
	void SetShopOut( const eShopType shoptype = eShop_PartCharge );
	void ServerFailResultMakeMessageBox( eCashShopResult result );

public://cart item
	const bool IsShopMyCartItem( WORD itemcode );
	const bool InsertShopMyCart( CASHSHOP_ITEMLIST& insertmyitem );
	const bool DeleteShopMyCart( CASHSHOP_ITEMLIST& deletemyitem );
	const bool GetShopMyCart( ShopCashShopItem_List& outcategoryitems );
	void ShopReset( bool isMyCartClear = true );

public://shop item
	const bool IsCategoryShopItem();
	const bool InsertCategoryShopItem( ShopCashShopItem_List& insertcategoryitems );
	const bool GetCategoryShopItem( ShopCashShopItem_List& outcategoryitems );

public:
	void ChangeShopType( const eShopType type );
	void ChangeHighCategorClk( const eHighCategoryType type );
	void ChangeLowCategorClk( const eLowCategoryType type );
	void ChangeLowCategorPageClk( WORD pagecount );
	void ChangeCheckBtClk( CASHSHOP_ITEMLIST& item );
	void ChangeEquipmentItem( const CASHSHOP_ITEMLIST& item );
	void CashShopPurchase( const DWORD itemcode );

public:
	void SendRequestShopInOut( bool isOpen );
	void RecevieShopInOut( BYTE* ReceiveBuffer );
	void SendRequestShopCashPoint();
	void RecevieShopCashPoint( BYTE* ReceiveBuffer );
	void SendRequestShopItemlist();
	void RecevieShopItemlist( BYTE* ReceiveBuffer );
	void SendRequestShopItemPurchase( int itemcode );
	void RecevieShopItemPurchase( BYTE* ReceiveBuffer );

public:
	const bool IsShopInOut();
	const bool IsDisconnectServer();
	const bool IsShopInOutPacket();

protected:
	virtual void OnMessageBox_OK( int id );

private:
	void Initialize( BoostWeak_Ptr(ShopServerProxy) proxy );
	void Destroy();
	ShopServerProxy();

private:
	ShopChashShopItem_Map				m_ShopItems;
	ChashShopMyCartItem_Map				m_ShopMyCartItems;

private:
	eHighCategoryType					m_HighCategoryType;
	eLowCategoryType					m_LowCategoryType;
	eShopType							m_ShopType;
	DWORD								m_CashPoint;
	WORD								m_CategoryPage;
	bool								m_IsShopIn;
	bool								m_IsShopInPacket;
	bool								m_IsDisconnect;
	BoostWeak_Ptr(ShopServerProxy)      m_handler;

};

inline
const bool ShopServerProxy::IsShopInOutPacket()
{
	if( m_IsShopInPacket )
	{
		return true;
	}
	else
	{
		return IsShopInOut();
	}
}

inline
const DWORD	ShopServerProxy::GetCashPoint() const
{
	return m_CashPoint;
}

inline
const bool ShopServerProxy::IsShopInOut()
{
	return m_IsShopIn;
}

inline
const eShopType ShopServerProxy::GetShopType() const
{
	return m_ShopType;
}

inline
const eHighCategoryType ShopServerProxy::GetHighCategoryType() const
{
	return m_HighCategoryType;
}

inline
const eLowCategoryType  ShopServerProxy::GetLowCategoryType() const
{
	return m_LowCategoryType;
}

inline
const DWORD ShopServerProxy::GetLowCategoryPageCount() const
{
	return m_CategoryPage;
}

inline
void ShopServerProxy::SetDisconnectServer( bool isdisconnect )
{
	m_IsDisconnect = isdisconnect;
}

inline
const bool ShopServerProxy::IsDisconnectServer()
{
	return m_IsDisconnect;
}

#endif // !defined(AFX_W_SHOPSERVERPROXY_H__4DE81AC1_61AC_4BFD_A71C_80D07EEAD841__INCLUDED_)

#endif //NEW_USER_INTERFACE_PROXY