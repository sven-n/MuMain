// w_ShopRightModule.h: interface for the ShopRightModule class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#if !defined(AFX_W_SHOPRIGHTMODULE_H__3AD1393A_F6C6_4C45_B3AD_01545CA8F243__INCLUDED_)
#define AFX_W_SHOPRIGHTMODULE_H__3AD1393A_F6C6_4C45_B3AD_01545CA8F243__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_CompositedPanel.h"
#include "w_ModelView.h"
#include "w_Panel.h"
#include "w_Text.h"

BoostSmartPointer( ShopRightModule );
class ShopRightModule : public Module, 
						public ShopServerProxyHandler,
						public MESSAGEBOXHANDLER
{
public:
	static ShopRightModulePtr Make( Module::eType type );
	virtual ~ShopRightModule();

public:
	void OnResetClk( ui::UIWnd& wnd );
	void OnCashClk( ui::UIWnd& wnd );
	void OnExitClk( ui::UIWnd& wnd );

private:
	void Initialize( BoostWeak_Ptr(ShopRightModule) handler );
	void Destroy();
	ShopRightModule( Module::eType type );
	
protected:
	virtual void OnChangeEquipmentItem( const CASHSHOP_ITEMLIST& item );
	virtual void OnChangeCashPoint( const DWORD cashpoint );

protected:
	virtual void OnMessageBox_OK( int id );
	
private:
	BoostWeak_Ptr(ShopRightModule)  m_handler;
	ui::PanelPtr			        m_PreviewPanel;
	ui::ModelViewPtr				m_ModelView;
	ui::TextPtr						m_Price;

};

#endif // !defined(AFX_W_SHOPRIGHTMODULE_H__3AD1393A_F6C6_4C45_B3AD_01545CA8F243__INCLUDED_)

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM