// w_ShopMiddleModule.h: interface for the ShopMiddleModule class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#if !defined(AFX_W_SHOPMIDDLEMODULE_H__2BDCF710_75E7_4ACA_B3DD_CDC6AABF0567__INCLUDED_)
#define AFX_W_SHOPMIDDLEMODULE_H__2BDCF710_75E7_4ACA_B3DD_CDC6AABF0567__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_UIType.h"
#include "w_UIWnd.h"
#include "w_CompositedPanel.h"
#include "w_CustomButton.h"
#include "w_CustomCheckButton.h"
#include "w_Panel.h"
#include "w_Text.h"
#include "w_ItemModuleView.h"

BoostSmartPointer( ShopMiddleModule );
class ShopMiddleModule : public Module, 
						 public ShopServerProxyHandler,
						 public MESSAGEBOXHANDLER
{
public:
	struct ShopItemListData 
	{
		BoostWeak_Ptr(ui::Panel)		 s_BackPanel;
		BoostWeak_Ptr(ShopMiddleModule)  s_handler;
		ShopCashShopItem_List    	     s_ItemDatas;
		Coord						     s_IntervalSize;
		bool						     s_IsCheck;
		WORD							 s_page;

		ShopItemListData() : s_IsCheck( false ), s_page( 0 ){}
	};

	BoostSmartPointer(ShopItemList);
	class ShopItemList
	{
	public:
		static ShopItemListPtr	Make( ShopItemListData& data );
		virtual ~ShopItemList();

	public:
		CASHSHOP_ITEMLIST& GetPageItem( CASHSHOP_ITEMLIST& csitem, const WORD itemcode );

	private:
		void Initialize( ShopItemListData& data );
		void Destroy();
		ShopItemList();

	private:
		ChashShopMyCartItem_Map					m_ShopPageItems;
	};

public:
	static ShopMiddleModulePtr Make( Module::eType type );
	virtual ~ShopMiddleModule();
	
public:
	void OnBuyClk( ui::UIWnd& wnd );
	void OnPresentClk( ui::UIWnd& wnd );
	void OnCheckboxClk( ui::UIWnd& wnd );
	void OnPageClk( ui::UIWnd& wnd );
	void OnEquipmentItemClk( ui::UIWnd& wnd );

protected:
	virtual void OnUpDateShopItemInfo( const eCashShopResult result );
	virtual void OnCashShopPurchaseResult();

protected:
	virtual void OnMessageBox_OK( int id );
    virtual void OnMessageBox_CANCEL( int id );

private:
	void UpdateItemPanel( eHighCategoryType type = eHighCategory_NewItem );
	void Initialize( BoostWeak_Ptr(ShopMiddleModule) handler );
	void Destroy();
	ShopMiddleModule( Module::eType type );

private:
	ShopItemListPtr						m_ShoopItems;

private:
	ui::PanelPtr						m_ItemBackPanel;
	ui::TextPtr							m_CurPageText;
	BoostWeak_Ptr(ShopMiddleModule)		m_handler;
	Dim								    m_FrameSize;
	//messageBox를 출력후 결과 처리하면 꼭 -1로 리셋을 해주자...-.-;;;
	int									m_BackItemCode;
};

#endif // !defined(AFX_W_SHOPMIDDLEMODULE_H__2BDCF710_75E7_4ACA_B3DD_CDC6AABF0567__INCLUDED_)

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM
