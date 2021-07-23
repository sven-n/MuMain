// w_CustomCheckButton.h: interface for the CustomCheckButton class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_CUSTOMCHECKBUTTON_H__1FA4153D_1F62_42A5_91F1_87BF0708D476__INCLUDED_)
#define AFX_W_CUSTOMCHECKBUTTON_H__1FA4153D_1F62_42A5_91F1_87BF0708D476__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_CheckButton.h"
#include "w_ImageUnit.h"
#include "w_UIWnd.h"

/*
// 생성 할때 checkButton 이름이 필요 하다.
CustomCheckButtonPtr checkButton( new CustomCheckButton( "checkButton" ) );
// 그림 이미지 인덱스를 넣어 줘야 한다.
// 그림은 두장의 이미지를 필요로 한다.
checkButton->SetImageIndex( BITMAP_NEWUI_SHOP_SUBBACKPANEL+5 );
// 좌표 ( 부모의 상대 좌표 )
checkButton->Move( ( frame->GetSize() / 2 ) - ( Coord( 800, 600 ) / 2 ) + shopleftbackpanelpos[i] );
// 그림 이미지의 크기
checkButton->Resize( Dim( 10, 10 ) );
// 부모에 붙이기
shopframpanel->AddChild( checkButton );
*/

namespace ui
{
	BoostSmartPointer( CustomCheckButton );
	class CustomCheckButton : public CheckButton
	{
	public:
		enum CheckButtonState
		{
			eCheckButton_UnCheck,
			eCheckButton_Check,
			
			eCheckButton_StateCount,
		};

		UIMAKERDECLARE;

	public:
		CustomCheckButton( const string& uiName );
		virtual ~CustomCheckButton();

		void MakeImage( const int bitmapindex );
		void SetImageIndex( const int bitmapindex );

	protected:
		virtual void OnMouseOver();
		virtual void OnMouseOut();
		virtual void OnMove( const Coord& prevPos );
		virtual void OnResize( const Dim& dim );
		virtual void OnCheck();
		virtual void OnUncheck();
		virtual void DrawImage();

	private:
		void Initialize();
		void Destroy();
		void ChangeFrame( int index );

	private:
		typedef vector< image::ImageUnitPtr >					 Image_Vector;
		Image_Vector											 m_Image;
	};

};
#endif // !defined(AFX_W_CUSTOMCHECKBUTTON_H__1FA4153D_1F62_42A5_91F1_87BF0708D476__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM