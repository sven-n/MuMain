// w_CheckButton.cpp: implementation of the CheckButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_CheckButton.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace ui;

CheckButton::CheckButton( const string& uiName ) : UIWnd( uiName ), m_Checked( false )
{

}

CheckButton::~CheckButton()
{

}

void CheckButton::Check()
{
	m_Checked = true;
	OnCheck();
}

void CheckButton::Uncheck()
{
	m_Checked = false;
	OnUncheck();
}

void CheckButton::OnLButtonDown()
{
	UIWnd::OnLButtonDown();

    if( IsChecked() )
        Uncheck();
    else
        Check();

}

void CheckButton::OnCheck()
{
	NotifyToHandler( eEvent_Check );
}

void CheckButton::OnUncheck()
{
	NotifyToHandler( eEvent_Uncheck );
}

#endif //NEW_USER_INTERFACE_UISYSTEM