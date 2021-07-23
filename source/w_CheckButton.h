// w_CheckButton.h: interface for the CheckButton class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_CHECKBUTTON_H__40FF028B_A545_4658_A0AC_53CBF4004F44__INCLUDED_)
#define AFX_W_CHECKBUTTON_H__40FF028B_A545_4658_A0AC_53CBF4004F44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_UIWnd.h"

namespace ui
{
	BoostSmartPointer( CheckButton );
	class CheckButton : public UIWnd  
	{
	public:
		CheckButton( const string& uiName );
		virtual ~CheckButton();

	public:
		void Check();
		void Uncheck();
		bool IsChecked();

	protected:
		virtual void OnLButtonDown();
		virtual void OnCheck();
		virtual void OnUncheck();

	private:
		bool				m_Checked;
		
	};

	inline
	bool CheckButton::IsChecked()
	{
		return m_Checked;
	}

}

#endif // !defined(AFX_W_CHECKBUTTON_H__40FF028B_A545_4658_A0AC_53CBF4004F44__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM