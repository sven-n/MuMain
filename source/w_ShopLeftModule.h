// w_ShopLeftModule.h: interface for the ShopLeftModule class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#if !defined(AFX_W_SHOPLEFTMODULE_H__1DFB3AA0_DD38_4F67_92FF_4A0A4B6217A9__INCLUDED_)
#define AFX_W_SHOPLEFTMODULE_H__1DFB3AA0_DD38_4F67_92FF_4A0A4B6217A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_UIType.h"
#include "w_UIWnd.h"
#include "w_RadioButton.h"
#include "w_CustomRadioButton.h"
#include "w_CompositedPanel.h"
#include "w_Panel.h"

BoostSmartPointer( ShopLeftModule );
class ShopLeftModule : public Module, public ShopServerProxyHandler 
{
public:
	struct ShopCategoryData 
	{
		BoostWeak_Ptr(ui::Panel)		s_BackPanel;
		BoostWeak_Ptr(ShopLeftModule)   s_handler;
		list<SHOPCATEGORY>				s_ItemDatas;
		Coord							s_IntervalSize;
		int								s_IsCheck;

		ShopCategoryData() : s_IsCheck( 1 ){}
	};

	BoostSmartPointer( ShopCategoryList );
	class ShopCategoryList
	{
	public:
		static ShopCategoryListPtr	Make( ShopCategoryData& data );
		virtual ~ShopCategoryList();

	private:
		void Initialize( ShopCategoryData& data );
		void Destroy();
		ShopCategoryList( ShopCategoryData& data );
	};	

public:
	static ShopLeftModulePtr Make( Module::eType type );
	virtual ~ShopLeftModule();
	
public:
	void OnLowCategoryClk( ui::UIWnd& wnd );

	void OnBuyClk( ui::UIWnd& wnd );
	void OnPresentClk( ui::UIWnd& wnd );
	void OnCheckboxClk( ui::UIWnd& wnd );
	
public:
	virtual void OnUpDateShopItemInfo( const eCashShopResult result );

private:
	void UpdateItemPanel( list<SHOPCATEGORY>& categorydata );
	void Initialize( BoostWeak_Ptr(ShopLeftModule) handler );
	void Destroy();
	ShopLeftModule( Module::eType type );

private:
	BoostWeak_Ptr(ShopLeftModule)			m_handler;
	ui::PanelPtr							m_CategoryBackPanel;
	ShopCategoryListPtr						m_ShopCategorys;
};

#endif // !defined(AFX_W_SHOPLEFTMODULE_H__1DFB3AA0_DD38_4F67_92FF_4A0A4B6217A9__INCLUDED_)

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM
