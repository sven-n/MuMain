// CustomButton.h: interface for the CustomBotton class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_CUSTOMBUTTON_H__EB9D7B0A_C257_4C5A_B309_E0E12F8DB262__INCLUDED_)
#define AFX_W_CUSTOMBUTTON_H__EB9D7B0A_C257_4C5A_B309_E0E12F8DB262__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_ImageUnit.h"
#include "w_UIWnd.h"

namespace ui
{
/*
  // 생성 할때 Button 이름 필요 하다.
  // Button의 이름은 아이뒤 라고 생각 하면 된다.
  ui::CustomButtonPtr exitbutton( new ui::CustomButton( "exitbutton" ) );
  // 아이뒤 넣기
  exitbutton->SetID(10);
  // 그림 이미지 인덱스를 넣어 줘야 한다.
  exitbutton->MakeImage( BITMAP_NEWUI_SHOP_RIGHTBT+2 );
  // 좌표 ( 부모의 상대 좌표 )
  exitbutton->Move( Coord( 14, 224 + 240 + 34 ) );
  // 그림 이미지의 크기
  exitbutton->Resize( Dim( 34, 30 ) );
  // 인풋 이벤트를 등록 할 수 있다. handler는 ShopRightModule this 포인터 이다.
  exitbutton->RegisterHandler( ui::eEvent_LButtonClick, handler, &ShopRightModule::OnExitClk );
  // 부모에 붙이기
  shoprightFrame.AddChild( exitbutton );
*/	
	BoostSmartPointer( CustomButton );
	class CustomButton : public UIWnd
	{
	public:
		enum ButtonState
		{ 
			eButton_Over,
			eButton_Up,
			eButton_Down,

			eButton_StateCount
		};
		
	public:
		void MakeImage( const int bitmapindex );
		void SetImageIndex( const int bitmapindex );
		void SetCustomButtonName( const string& buttonname, const Coord& pos, BYTE type );

		CustomButton( const string& uiName );
		virtual ~CustomButton();

	protected:
		virtual void DrawImage();
		virtual void OnMove( const Coord& prevPos );
		virtual void OnResize( const Dim& dim );
		virtual void OnMouseOver();
		virtual void OnMouseOut();
		virtual void OnLButtonDown();
		virtual void OnLButtonUp();

	private:
		void Initialize();
		void Destroy();
		void ChangeFrame( int index );

	private:
		struct ButtonImage 
		{
			image::ImageUnitPtr	s_Image;

			ButtonImage() { 
				s_Image = image::ImageUnit::Make(); 
			}

			~ButtonImage() {
				if( s_Image ) s_Image.reset(); 
			}

			void SetImageIndex( const int bitmapindex ) {
				if( s_Image ) s_Image->SetImageIndex( bitmapindex );
			}

			void SetPosition( const Coord& pos ) {
				if( s_Image ) s_Image->SetPosition( pos );
			}
			
			void SetSize( const Dim& size ) {
				if( s_Image ) s_Image->SetSize( size );
			}

			void SetDraw( bool isdraw ) {
				if( s_Image ) s_Image->SetDraw( isdraw );
			}

			void Draw() {
				if( s_Image ) s_Image->Draw();
			}
		};

		vector< BoostSmart_Ptr(ButtonImage) >					 m_ButtonImage;
	};

};

#endif // !defined(AFX_W_CUSTOMBUTTON_H__EB9D7B0A_C257_4C5A_B309_E0E12F8DB262__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM