// w_CompositedPanel.h: interface for the CompositedPanel class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_COMPOSITEDPANEL_H__48C4D2EA_FFE8_4CCD_A0AA_A493383B8F15__INCLUDED_)
#define AFX_W_COMPOSITEDPANEL_H__48C4D2EA_FFE8_4CCD_A0AA_A493383B8F15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_ImageUnit.h"
#include "w_UIWnd.h"

//Make로 만들지 말자.
//스크롤 바도 여길 이용해서 만들도록 해보자.
/*
    // 생성 할때 Panel 이름, Panel 타입 두개의 인자가 필요 하다.
	ui::CompositedPanelPtr shopsubbackpanel( new ui::CompositedPanel( ( format( "shopsubbackpanel%1%" ) % i ).str(), ui::ePanel_Piece_One ) );
	// 아이뒤 넣기
	shopsubbackpanel->SetID(10);
	// 그림 이미지 인덱스를 넣어 줘야 한다.
	shopsubbackpanel->SetImageIndex( BITMAP_NEWUI_SHOP_SUBBACKPANEL+5 );
	// 좌표 ( 부모의 상대 좌표 )
	shopsubbackpanel->Move( ( frame->GetSize() / 2 ) - ( Coord( 800, 600 ) / 2 ) + shopleftbackpanelpos[i] );
	// 그림 이미지의 크기
	shopsubbackpanel->Resize( shopleftbackpanelsize[i] );
    // 인풋 이벤트를 등록 할 수 있다. handler는 ShopRightModule this 포인터 이다.
	shopsubbackpanel->RegisterHandler( ui::eEvent_LButtonClick, handler, &ShopRightModule::OnExitClk );
	// 그림의 위치
	shopsubbackpanel->SetUVRect( Rect( 0, 0, 203, 256 ) );
	// 부모에 붙이기
	shopframpanel->AddChild( shopsubbackpanel );
*/

namespace ui
{
	BoostSmartPointer( CompositedPanel );
	class CompositedPanel : public UIWnd
	{
	public:
/*             ePieceFullType
    	 _____________________________
		|         |         |         |	
		|   Top   |   Top   |   Top   |
		|   left  |  Center |  Right  |
		|         |         |         |
		|_________|_________|_________|
		|         |         |         |
		|  Middle |  Middle | Middle  |
        |   left  |  Center |  Right  |
		|         |         |         |
		|_________|_________|_________|
		|         |         |         |
		| Bottom  |  Bottom | Bottom  |
	    |   left  |  Center |  Right  |
		|         |         |         |
		|_________|_________|_________|

  9장의 같은 크기 이미지로 큰 이미지를 뿌린다.
  Center같은 경우 이미지는 공유 해서 그려진다.
  용도는 배경 화면 같은 큰 이미지

*/
		enum ePieceFullType
		{
			//top
			ePiece_TopLeft,         //상위 왼쪽
			ePiece_TopCenter,       //상위 중간
			ePiece_TopRight,        //상위 오른쪽
			//middle
			ePiece_MiddleLeft,      //중간 왼쪽
			ePiece_MiddleCenter,    //중간 중간
			ePiece_MiddleRight,     //중간 오른쪽
			//bottom
			ePiece_BottomLeft,      //하위 왼쪽
			ePiece_BottomCenter,    //하위 중간
			ePiece_BottomRight,     //하위 오른쪽
			//count
			ePieceTypeCount,
		};

/*             ePieceWhThree
    	 _____________________________
		|         |         |         |	
		|   Top   |   Top   |   Top   |
		|   left  |  Center |  Right  |
		|         |         |         |
		|_________|_________|_________|


  3장의 같은 크기 이미지로 큰 이미지를 뿌린다.
  Center같은 경우 이미지는 공유 해서 그려진다.
  용도는 스크롤바, 게이지바 등등

*/

		enum ePieceWhThree
		{ 
			ePiece_wLeft,			//가로 왼쪽
			ePiece_wCenter,         //가로 중간
			ePiece_wRight,          //가로 오른쪽
			//count
			ePieceTypewCount,
		};

/*             ePieceHtThree
    			 _________
				|         |
				|   Top   |
				|   left  |
				|         |
				|_________|
				|         |
				|  Middle |
				|   left  |
				|         |
				|_________|
				|         |
				| Bottom  |
				|   left  |
				|         |
				|_________|

  3장의 같은 크기 이미지로 큰 이미지를 뿌린다.
  Center같은 경우 이미지는 공유 해서 그려진다.
  용도는 스크롤바, 게이지바 등등

*/
		enum ePieceHtThree
		{ 
			ePiece_hTop,			//세로 왼쪽
			ePiece_hCenter,			//세로 중간
			ePiece_hBottom,			//세로 오른쪽
			//count
			ePieceTypehCount,
		};

/*             ePieceFullType
    	 _____________________________
		|							  |	
		|							  |
		|							  |
		|							  |
		|							  |
		|							  |
		|							  |
        |							  |
		|							  |
		|							  |
		|							  |
		|							  |
	    |							  |
		|							  |
		|_____________________________|

  한장의 이미지를 뿌린다.
  용도는 배경 화면 같은 작은 이미지
*/
		enum ePieceOne
		{
			ePiece_oCenter,			//중간
			//count
			ePieceTypeoCount,
		};

	public:
		void SetImageIndex( const int bitmapindex );
		CompositedPanel( const string& uiName, ePanelType type = ePanel_Piece_Full );
		virtual ~CompositedPanel();

	public:
		void SetUVRect( const Rect& uvrect );

	private:
		void CalculateImageRect();
		void MakeImage( const int count );
		void Initialize( ePanelType type );
		void Destroy();

	protected:
		virtual void OnMove( const Coord& pos );
		virtual void OnResize( const Dim& dim );
		virtual void DrawImage();

	private:
		typedef vector< image::ImageUnitPtr >							Image_Vector;

	private:
		Image_Vector													m_Image;
		ePanelType														m_ePanelType;
		int																m_iSubPanelType;
		Color															m_Color;
	};
};

#endif // !defined(AFX_W_COMPOSITEDPANEL_H__48C4D2EA_FFE8_4CCD_A0AA_A493383B8F15__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM