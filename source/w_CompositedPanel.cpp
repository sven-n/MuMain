// w_CompositedPanel.cpp: implementation of the CompositedPanel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "w_CompositedPanel.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace ui;

CompositedPanel::CompositedPanel( const string& uiName, ePanelType type ) : UIWnd( uiName ), m_ePanelType( type ), m_iSubPanelType( 0 )
{
	Initialize( type );
	RegisterDrawEvent( eDraw_Image );
}

CompositedPanel::~CompositedPanel()
{
	Destroy();
}
			
void CompositedPanel::Initialize( ePanelType type )
{
	switch( type )
	{
	case ePanel_Piece_Full:			MakeImage( static_cast<int>( ePieceTypeCount ) );  break;
	case ePanel_Piece_Width_Three:  MakeImage( static_cast<int>( ePieceTypewCount ) ); break;
	case ePanel_Piece_Height_Three: MakeImage( static_cast<int>( ePieceTypehCount ) ); break;
	case ePanel_Piece_One:			MakeImage( static_cast<int>( ePieceTypeoCount ) ); break;
	}

	CalculateImageRect();
}

void CompositedPanel::Destroy()
{
	for( int i = m_iSubPanelType - 1; i > -1; --i )
	{
		m_Image[i].reset();
	}
}

void CompositedPanel::SetImageIndex( const int bitmapindex )
{
	//첫번째 인덱스를 넣어준다.
	for ( int i = 0; i < m_Image.size(); ++i )
	{
		m_Image[i]->SetImageIndex( bitmapindex + i );
	}
}

void CompositedPanel::MakeImage( const int count )
{
	m_iSubPanelType = count;

	for( int i = 0; i < count; ++i )
	{
		image::ImageUnitPtr Img = image::ImageUnit::Make();
		m_Image.push_back( Img );
	}
}

void CompositedPanel::SetUVRect( const Rect& uvrect )
{
	//한장 일때만 하자.
	if( m_ePanelType == ePanel_Piece_One )
	{
		for ( int i = 0; i < m_Image.size(); ++i )
		{
			m_Image[i]->SetUVRect( uvrect );
		}
	}
	else
	{
		assert(0);
	}
}

void CompositedPanel::CalculateImageRect()
{
	const Coord& screenPos = GetScreenPosition();

	if( m_ePanelType == ePanel_Piece_Full )/*ePanel_Piece_Full*/
	{
		//pos
        m_Image[ePiece_TopLeft]->SetPosition( screenPos );
        m_Image[ePiece_TopCenter]->SetPosition( screenPos + Coord( m_Image[ePiece_TopLeft]->GetSize().x, 0 ) );
        m_Image[ePiece_TopRight]->SetPosition( screenPos + Coord( m_Size.x - m_Image[ePiece_TopRight]->GetSize().x, 0 ) );
        m_Image[ePiece_MiddleLeft]->SetPosition( screenPos + Coord( 0, m_Image[ePiece_TopLeft]->GetSize().y ) );
        m_Image[ePiece_MiddleCenter]->SetPosition( screenPos + Coord( m_Image[ePiece_MiddleLeft]->GetSize().x, m_Image[ePiece_TopCenter]->GetSize().y ) );
        m_Image[ePiece_MiddleRight]->SetPosition( screenPos + Coord( m_Size.x - m_Image[ePiece_MiddleRight]->GetSize().x, m_Image[ePiece_TopRight]->GetSize().y ) );
        m_Image[ePiece_BottomLeft]->SetPosition( screenPos + Coord( 0, m_Size.y - m_Image[ePiece_BottomLeft]->GetSize().y ) );
        m_Image[ePiece_BottomCenter]->SetPosition( screenPos + Coord( m_Image[ePiece_BottomLeft]->GetSize().x, m_Size.y - m_Image[ePiece_BottomCenter]->GetSize().y ) );
        m_Image[ePiece_BottomRight]->SetPosition( screenPos + Coord( m_Size.x - m_Image[ePiece_BottomRight]->GetSize().x, m_Size.y - m_Image[ePiece_BottomRight]->GetSize().y ) );
		//size
        m_Image[ePiece_TopCenter]->SetSize( Dim( m_Size.x - m_Image[ePiece_TopLeft]->GetSize().x - m_Image[ePiece_TopRight]->GetSize().x, m_Image[ePiece_TopCenter]->GetSize().y ) );
        m_Image[ePiece_MiddleCenter]->SetSize( Dim( m_Size.x - m_Image[ePiece_MiddleLeft]->GetSize().x - m_Image[ePiece_MiddleRight]->GetSize().x, m_Size.y - m_Image[ePiece_TopCenter]->GetSize().y - m_Image[ePiece_BottomCenter]->GetSize().y ) );
        m_Image[ePiece_BottomCenter]->SetSize( Dim( m_Size.x - m_Image[ePiece_BottomLeft]->GetSize().x - m_Image[ePiece_BottomRight]->GetSize().x, m_Image[ePiece_BottomCenter]->GetSize().y ) );
        m_Image[ePiece_MiddleLeft]->SetSize( Dim( m_Image[ePiece_MiddleLeft]->GetSize().x, m_Size.y - m_Image[ePiece_TopLeft]->GetSize().y - m_Image[ePiece_BottomLeft]->GetSize().y ) );
        m_Image[ePiece_MiddleRight]->SetSize( Dim( m_Image[ePiece_MiddleRight]->GetSize().x, m_Size.y - m_Image[ePiece_TopRight]->GetSize().y - m_Image[ePiece_BottomRight]->GetSize().y ) );
	}
	else if( m_ePanelType == ePanel_Piece_Width_Three )/*ePanel_Piece_Width_Three*/
	{
        const Dim pieceSize( m_Image[ePiece_wLeft]->GetSize() );
		//pos
        m_Image[ePiece_wLeft]->SetPosition( screenPos );
        m_Image[ePiece_wCenter]->SetPosition( screenPos + Coord( pieceSize.x, 0 ) );
        m_Image[ePiece_wRight]->SetPosition( screenPos + Coord( m_Size.x - pieceSize.x, 0 ) );
		//size
        m_Image[ePiece_wCenter]->SetSize( Dim( m_Size.x - pieceSize.x * 2, pieceSize.y ) );
        m_Image[ePiece_wRight]->SetSize( pieceSize );
        if( m_Size.x <= pieceSize.x * 2 )
        {
            m_Image[ePiece_wCenter]->SetSize( Dim( 0, 0 ) );

            if( m_Size.x <= pieceSize.x )
            {
                m_Image[ePiece_wRight]->SetSize( Dim( 0, 0 ) );
            }

            if( m_Size.x == 0 )
            {
                m_Image[ePiece_wLeft]->SetSize( Dim( 0, 0 ) );
            }
        }
	}
	else if( m_ePanelType == ePanel_Piece_Height_Three )/*ePanel_Piece_Height_Three*/
	{
        const Dim pieceSize( m_Image[ePiece_hTop]->GetSize() );
		//pos
        m_Image[ePiece_hTop]->SetPosition( screenPos );
        m_Image[ePiece_hCenter]->SetPosition( screenPos + Coord( 0, pieceSize.y ) );
        m_Image[ePiece_hBottom]->SetPosition( screenPos + Coord( 0, m_Size.y - pieceSize.y ) );
		//size
        m_Image[ePiece_hCenter]->SetSize( Dim( pieceSize.x, m_Size.y - pieceSize.y * 2 ) );
	}
	else/*ePanel_Piece_One*/
	{
		m_Image[ePiece_oCenter]->SetPosition( screenPos );
		m_Image[ePiece_oCenter]->SetSize( m_Size );
	}
}

void CompositedPanel::OnMove( const Coord& pos )
{
	UIWnd::OnMove( pos );
	CalculateImageRect();
}

void CompositedPanel::OnResize( const Dim& dim )
{
	UIWnd::OnResize( dim );
	CalculateImageRect();
}

void CompositedPanel::DrawImage()
{
	for ( int i = 0; i < m_Image.size(); ++i )
	{
		m_Image[i]->Draw();
	}
}

#endif //NEW_USER_INTERFACE_UISYSTEM