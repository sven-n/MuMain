// w_RadioButton.cpp: implementation of the RadioButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_RadioButton.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

using namespace ui;

RadioButton::Group::Group()
{

}
RadioButton::Group::~Group()
{

}
void RadioButton::Group::RegisterSelectedRadioButton( BoostWeak_Ptr(RadioButton) btn )
{
	if( !m_SelectedBtn.expired() )
	{
		m_SelectedBtn.lock()->Deselect();
	}
	m_SelectedBtn = btn;
}

RadioButton::RadioButton( const string& name ) : UIWnd( name ), m_Selected( false )
{
}

RadioButton::~RadioButton(void)
{
}

void RadioButton::RegisterGroup( BoostSmart_Ptr(Group) group, BoostWeak_Ptr(RadioButton) groupHandler )
{
	m_Group         = group;
	m_GroupHandler  = groupHandler;
	m_LockKey       = false;
}

bool RadioButton::IsSelected() const
{
	return m_Selected;
}

void RadioButton::Select()
{
	m_Selected = true;

	if( m_Group )
	{
		m_Group->RegisterSelectedRadioButton( m_GroupHandler );
	}

	OnSelect();
}

void RadioButton::Deselect()
{
	m_Selected = false;
	OnDeselect();
}

void RadioButton::OnSelect()
{
	NotifyToHandler( eEvent_Select );
}

void RadioButton::OnDeselect()
{
	NotifyToHandler( eEvent_Deselect );
}

void RadioButton::Lock()
{
	m_LockKey = true;
}

void RadioButton::UnLock()
{
	m_LockKey = false;
}

void RadioButton::OnLButtonDown()
{
	if( !m_LockKey )
	{
		UIWnd::OnLButtonDown();
		Select();
	}
}

#endif //NEW_USER_INTERFACE_UISYSTEM