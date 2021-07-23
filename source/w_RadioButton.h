// w_RadioButton.h: interface for the RadioButton class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_RADIOBUTTON_H__F32A9AB8_1878_4686_A8D6_030AA8E3AE52__INCLUDED_)
#define AFX_W_RADIOBUTTON_H__F32A9AB8_1878_4686_A8D6_030AA8E3AE52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_UIWnd.h"

namespace ui
{
	BoostSmartPointer( RadioButton );
	class RadioButton : public UIWnd 
	{
	public:
		BoostSmartPointer( Group );
		class Group
		{
		public:
			Group();
			~Group();
			void    RegisterSelectedRadioButton( BoostWeak_Ptr(RadioButton) btn );

		private:
			BoostWeak_Ptr(RadioButton) m_SelectedBtn;
		};

		RadioButton( const string& name );
		virtual ~RadioButton(void);
		void    RegisterGroup( BoostSmart_Ptr(Group) group, BoostWeak_Ptr(RadioButton) groupHandler );
		bool    IsSelected() const;
		void    Select();
		void    Deselect();
		void    Lock();
		void    UnLock();

	protected:
		virtual void    OnLButtonDown();
		virtual void    OnSelect();
		virtual void    OnDeselect();

	private:
		BoostSmart_Ptr(Group)        m_Group;
		BoostWeak_Ptr(RadioButton)   m_GroupHandler;

		bool                    m_Selected;
		bool					m_LockKey;

	};
};

#endif // !defined(AFX_W_RADIOBUTTON_H__F32A9AB8_1878_4686_A8D6_030AA8E3AE52__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM