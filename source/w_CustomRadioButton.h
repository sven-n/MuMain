// w_CustomRadioButton.h: interface for the CustomRadioButton class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_CUSTOMRADIOBUTTON_H__D500FC92_128D_4EA7_BB1F_A132873BA2E1__INCLUDED_)
#define AFX_W_CUSTOMRADIOBUTTON_H__D500FC92_128D_4EA7_BB1F_A132873BA2E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_RadioButton.h"
#include "w_ImageUnit.h"
#include "w_UIWnd.h"

/*
  // 그룹으로 생성 하자
  ui::RadioButton::GroupPtr groupRadioButton( new ui::RadioButton::Group );

  for( int i = 0; i < 3; ++i ) 
  {
	// 생성 할때 radioButton 이름이 필요 하다.
	CustomRadioButtonPtr radioButton( new CustomRadioButton( ( format( "radioButton%1%" ) % i ).str() ) );
	// 그림 이미지 인덱스를 넣어 줘야 한다.
	// 그림은 두장의 이미지를 필요로 한다.
	radioButton->SetImageIndex( BITMAP_NEWUI_SHOP_SUBBACKPANEL+5 );
	// 좌표 ( 부모의 상대 좌표 )
	radioButton->Move( ( frame->GetSize() / 2 ) - ( Coord( 800, 600 ) / 2 ) + shopleftbackpanelpos[i] );
	// 그림 이미지의 크기
	radioButton->Resize( Dim( 10, 10 ) );
	// 그룹에 등록 하기
	radioButton->RegisterGroup( groupRadioButton, radioButton );
	// 부모에 붙이기
	shopframpanel->AddChild( radioButton );

     // 그룹에 등록 한 버튼중 어떤것을 활성화 할꺼냐 이다
	 if( i == 0 ) {
		radioButton->BasicChangeFrame();
	 }
  }
*/

namespace ui
{
	BoostSmartPointer( CustomRadioButton );
	class CustomRadioButton : public RadioButton
	{
	public:
		enum RadioButtonState
		{ 
			eRadioButton_Up,
			eRadioButton_Down,

			eRadioButton_StateCount
		};

	public:
		CustomRadioButton( const string& name );
		virtual ~CustomRadioButton(void);

		void MakeImage( const int bitmapindex );
		void SetImageIndex( const int bitmapindex );
		void SetRadioButtonName( const string& buttonname, const Coord& pos, BYTE type );
		
		void BasicChangeFrame();
		void UpChangeFrame();

	protected:
		virtual void DrawImage();
		virtual void OnMove( const Coord& pos );
		virtual void OnResize( const Dim& dim );

		virtual void OnSelect();
		virtual void OnDeselect();

		virtual void OnMouseOver();
		virtual void OnMouseOut();

	private:
		void Initialize();
		void Destroy();
		void ChangeFrame( int index );

	private:
		struct RadioButtonImage 
		{
			image::ImageUnitPtr				s_Image;

			RadioButtonImage()
			{ 
				s_Image = image::ImageUnit::Make(); 
			}

			~RadioButtonImage()
			{
				if( s_Image ) s_Image.reset(); 
			}

			void SetImageIndex( const int bitmapindex )
			{
				if( s_Image ) s_Image->SetImageIndex( bitmapindex );
			}

			void SetPosition( const Coord& pos )
			{
				if( s_Image ) s_Image->SetPosition( pos );
			}
			
			void SetSize( const Dim& size )
			{
				if( s_Image ) s_Image->SetSize( size );
			}

			void SetDraw( bool isdraw )
			{
				if( s_Image ) s_Image->SetDraw( isdraw );
			}
			
			const Dim& GetSize()
			{
				assert(s_Image);
				return s_Image->GetSize();
			}

			void Draw()
			{
				if( s_Image ) s_Image->Draw();
			}
		};

	private:
		vector< BoostSmart_Ptr(RadioButtonImage) >	m_RadioButtonImage;
	};

};

#endif // !defined(AFX_W_CUSTOMRADIOBUTTON_H__D500FC92_128D_4EA7_BB1F_A132873BA2E1__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM
