// w_InventoryBox.cpp: implementation of the InventoryBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"

#include "w_InventoryBox.h"

#include "w_ToolTipPanel.h"

#include "NewUIInventoryCtrl.h"
#include "NewUIMyInventory.h"
#include "NewUISystem.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

using namespace ui;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

InventoryBox::InventoryBox( const string& name ) : UIWnd( name )
{
	RegisterDrawEvent( eDraw_Image );
	RegisterDrawEvent( eDraw_Mesh );
	RegisterDrawEvent( eDraw_Rect );
}

InventoryBox::~InventoryBox()
{
	Destroy();
	
}

void InventoryBox::Initialize()
{
	/*
	if( !m_ToolTipPanel )
	{
		ToolTipPanelPtr tooltip( new ToolTipPanel( "tooltip" ) );
		AddChild( tooltip );

		m_ToolTipPanel = tooltip;
	}
	*/
}

void InventoryBox::Destroy()
{

}

void InventoryBox::MakeImage( const int bitmapindex, const Dim& count )
{
	SetInventoryBoxCount( count );
	SetImageIndex( bitmapindex );
}

void InventoryBox::SetImageIndex( const int bitmapindex )
{
	for( int i = 0; i < m_InventoryBoxCount.y; ++i )
	{
		for( int j = 0; j < m_InventoryBoxCount.x; ++j )
		{
			BoostSmart_Ptr(InventoryBox::InventoryBoxImage) Imageunit( new InventoryBox::InventoryBoxImage );
			Imageunit->SetImageIndex( bitmapindex );
	
			m_Image.push_back( Imageunit );
		}
	}
}

void InventoryBox::SetItem( const int inventorytype )
{
	switch( inventorytype )
	{
	case 0: m_InventoryItem = Inventory; break;
	}
}

void InventoryBox::CalculateInventoryBoxCoord()
{
	Coord curpos = GetScreenPosition();

	for( int i = 0; i < m_InventoryBoxCount.y; ++i )
	{
		for( int j = 0; j < m_InventoryBoxCount.x; ++j )
		{
			int index = ( i * m_InventoryBoxCount.x ) + j;

			Dim cursize = m_Image[index]->GetSize();

			Coord pos = curpos + Coord( j * cursize.x, i * cursize.y ); 

			m_Image[index]->SetPosition( pos );
		}
	}
}

void InventoryBox::CalculateInventoryBoxDim()
{
	for( int i = 0; i < m_InventoryBoxCount.y; ++i )
	{
		for( int j = 0; j < m_InventoryBoxCount.x; ++j )
		{
			int index = ( i * m_InventoryBoxCount.x ) + j;
			m_Image[index]->SetSize( m_Size );
		}
	}

	m_Size = m_Size * m_InventoryBoxCount;
}

void InventoryBox::OnMove( const Coord& prevPos )
{
	UIWnd::OnMove( prevPos );
	CalculateInventoryBoxCoord();
}

void InventoryBox::OnResize( const Dim& prevSize )
{
	UIWnd::OnResize( prevSize );

	CalculateInventoryBoxDim();
	CalculateInventoryBoxCoord();
}

void InventoryBox::Process( int delta )
{
	g_pMyInventory->UpdateShop( GetScreenPosition() );
/*
	//OTL 진짜 이방법 밖에 없나..ㅜㅜ
	//죄송합니다..ㅜㅜ
	Coord pos = GetScreenPosition();

	pos.x += 3;
	pos.y += 3;

	bool success = false;

	ClearInventoryColor(Inventory,COLUMN_INVENTORY,ROW_INVENTORY);
	MoveInventoryPushNew( pos.x, pos.y,Inventory,COLUMN_INVENTORY,ROW_INVENTORY,false );

	if( PickItem.Number > 0 )//pick가 되었을 경우
	{
		success = MoveInventoryPopCheckNew( pos.x, pos.y, Inventory, COLUMN_INVENTORY, ROW_INVENTORY, 0 );

		if( MouseLButtonPush )
		{
			MouseLButtonPush = false;
			MouseUpdateTime = 0;
     		MouseUpdateTimeMax = 6;
		}
	}
*/
}

const Color& InventoryBox::QueryInventoryBoxColor( const ITEM *p )
{
	switch( p->Color )
	{
	case 0:
		m_Color = Color( 1.f, 1.f, 1.f, 1.f );   //  보통 상태.
		break;
	case 1:
		m_Color = Color( 1.f, 0.8f,0.8f,0.8f );
		break;
	case 2:
		m_Color = Color( 1.f, 0.6f, 0.7f, 1.f );
		break;
	case 3:
		m_Color = Color( 1.f, 1.f, 0.2f, 0.1f );  //  사용 불가능.
		break;
	case 4:
		m_Color = Color( 1.f, 0.5f, 1.f, 0.6f );
		break;
    case 5:
		m_Color = Color( 0.8f, 0.7f, 0.f, 1.f );  //  내구력 경고. 50%
        break;
    case 6:
		m_Color = Color( 0.8f, 0.5f, 0.f, 1.f );  //  내구력 경고. 70%
        break;
    case 7:
		m_Color = Color( 0.8f, 0.3f, 0.3f, 1.f ); //  내구력 경고. 80%
        break;
    case 8:
		m_Color = Color( 1.0f, 0.f, 0.f, 1.f );   //  내구력 경고. 100%
        break;
    case 99:
		m_Color = Color( 1.0f, 1.f, 0.2f, 0.1f ); //  거래 창에서 사용.
        break;
	}

	return m_Color;
}

void InventoryBox::DrawImage()
{
	g_pMyInventory->RenderShop( GetScreenPosition() );
/*
	//인벤토리 아이템 박스
	Coord curpos = GetScreenPosition();

	for( int i = 0; i < m_InventoryBoxCount.y; ++i )
	{
		for( int j = 0; j < m_InventoryBoxCount.x; ++j )
		{
			int index = ( i * m_InventoryBoxCount.x ) + j;
			ITEM *p = &m_InventoryItem[index];

			if(p->Type != -1)
			{
				m_Image[index]->ChangeFrame( eInventoryBox_Up );
				m_Image[index]->Draw( QueryInventoryBoxColor( p ) );
			}
			else
            {
				m_Image[index]->ChangeFrame( eInventoryBox_Down );
				m_Image[index]->Draw( QueryInventoryBoxColor( p ) );
            }
		}
	}
*/
}

void InventoryBox::DrawMesh()
{
	g_pMyInventory->RenderShop3D( GetScreenPosition() );
/*
	//OTL 진짜 이방법 밖에 없나..ㅜㅜ
	//죄송합니다..ㅜㅜ
	Coord pos = GetScreenPosition();

	//아이템
	if( Dim(800, 600) == TheShell().GetWindowSize() )	
		RenderInventory3DNew( pos.x-2,pos.y-1, Inventory,COLUMN_INVENTORY,ROW_INVENTORY,0 );
	else if( Dim(1024, 768) == TheShell().GetWindowSize() )
		RenderInventory3DNew( pos.x-3,pos.y, Inventory,COLUMN_INVENTORY,ROW_INVENTORY,0 );
	else if( Dim(1280, 1024) == TheShell().GetWindowSize() )
		RenderInventory3DNew( pos.x-5,pos.y, Inventory,COLUMN_INVENTORY,ROW_INVENTORY,0 );

	//아이템 위의 숫자
	BeginBitmap();
	EnableAlphaTest();
	RenderInventory3DNew( pos.x-10 ,pos.y, Inventory,COLUMN_INVENTORY,ROW_INVENTORY,1);
	EndBitmap();
		
	if(PickItem.Number>0 && 0==AskYesOrNo && g_iPersonalShopMsgType!=2)
	{
		//피크 되었을때 피크된 3d 아이템
		BeginDrawMesh();
		RenderPickItem3DNew();
	}
*/
}

void InventoryBox::DrawRect()
{
	Coord pos = GetScreenPosition();

	SEASON3B::CNewUIInventoryCtrl* myInventory = g_pMyInventory->GetInventoryCtrl();

	if( myInventory ) 
	{
		ITEM* pTooltipItem = myInventory->GetTooltipItem();

		if( pTooltipItem )
		{
			ToolTipPanelPtr tooltip( new ToolTipPanel( "tooltip" ) );
			ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pTooltipItem->Type];
			int iTargetX = pos.x + pTooltipItem->x*20 + pItemAttr->Width*20/2;
			int iTargetY = pos.y + pTooltipItem->y*20;

			vector<string>	data;
			vector<DWORD>	colordata;
			string			tempstring;
			
			RenderItemInfo( 0, 0, pTooltipItem, false, false );						
			
			for ( int i = 0; i < ITEMINFOCOUNT; ++i )
			{
				tempstring = TextList[i];
				if( tempstring.size() > 0 && TextList[i][0] != '\0' && TextList[i][0] != '\n' )
				{
					data.push_back( tempstring );
					colordata.push_back( TextListColor[i] );
				}
			}
			
			tooltip->SetInfo( data, colordata );
			tooltip->Move( Coord( iTargetX, iTargetY ) );
			tooltip->SetAngleStat( true );
			tooltip->DrawRect();
		}
	}
/*
	Coord pos = GetScreenPosition();

	float tempscale = 21.5f;

	for( int i = 0; i < m_InventoryBoxCount.y; ++i )
	{
		for( int j = 0; j < m_InventoryBoxCount.x; ++j )
		{
			float sx  = pos.x+j*tempscale;
			float sy  = pos.y+i*tempscale;
			int index = ( i * m_InventoryBoxCount.x ) + j;
			ITEM *p = &m_InventoryItem[index];

			if(p->Type != -1)
			{
				float Width  = (float)ItemAttribute[p->Type].Width  * tempscale;
				float Height = (float)ItemAttribute[p->Type].Height * tempscale;

         		if(p->Number > 0)
				{
					int ChangeMousePos_x = MouseX*WindowWidth/640;
					int ChangeMousePos_y = MouseY*WindowHeight/480;

					if((PickItem.Number==0) && ChangeMousePos_x>=sx && ChangeMousePos_x<sx+Width && ChangeMousePos_y>=sy && ChangeMousePos_y<sy+Height)
					{
						vector<string>	data;
						vector<DWORD>	colordata;
						string			tempstring;

						RenderItemInfo( 0, 0, p, false, false );						

						for ( int i = 0; i < ITEMINFOCOUNT; ++i )
						{
							tempstring = TextList[i];
							if( tempstring.size() > 0 && TextList[i][0] != '\0' && TextList[i][0] != '\n' )
							{
								data.push_back( tempstring );
								colordata.push_back( TextListColor[i] );
							}
						}

						ToolTipPanelPtr tooltip( new ToolTipPanel( "tooltip" ) );
						tooltip->SetInfo( data, colordata );
						tooltip->Move( Coord( sx+30, sy+10 ) );
						tooltip->SetAngleStat( true );
						tooltip->DrawRect();
					}
				}
			}
		}
	}
*/
}

#endif //NEW_USER_INTERFACE_UISYSTEM