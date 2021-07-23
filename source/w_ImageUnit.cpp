// w_Image.cpp: implementation of the Image class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_ImageUnit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_IMAGE

#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"

using namespace image;

/*
OpenJpeg(FileName,BITMAP_MAPTILE,GL_NEAREST,GL_REPEAT);
GL_REPEAT를 해줘야 한다.
*/

namespace
{
	void ChangeUVCoordinates( Rect& UvRect, DimF& ImgSize, const float SizePercent, vector<CoordF>& vertexpos )
	{
		const CoordF percentPos  = CoordF( static_cast<float>(UvRect.left), static_cast<float>(UvRect.top) ) / CoordF( ImgSize.x, ImgSize.y ) * SizePercent;
		const DimF   percentSize = DimF( static_cast<float>(UvRect.right), static_cast<float>(UvRect.bottom) ) / DimF( ImgSize ) * SizePercent;

		vertexpos.push_back( CoordF( percentPos.x, percentPos.y ) );
		vertexpos.push_back( CoordF( percentPos.x, percentSize.y ) );
		vertexpos.push_back( CoordF( percentSize.x, percentSize.y ) );
		vertexpos.push_back( CoordF( percentSize.x, percentPos.y ) );
	}

	void ChangeUVCoordinates( DimF& RectSize, DimF& ImgSize, const float SizePercent, vector<CoordF>& vertexpos )
	{
		const DimF percent = DimF( RectSize ) / DimF( ImgSize ) * SizePercent;

		if( percent != DimF( 1.f, 1.f ) )
		{
			vertexpos.push_back( CoordF( 0.0f, 0.0f ) );
			vertexpos.push_back( CoordF( 0.0f, percent.y ) );
			vertexpos.push_back( CoordF( percent.x, percent.y ) );
			vertexpos.push_back( CoordF( percent.x, 0.0f ) );
		}
		else
		{
			vertexpos.push_back( CoordF( 0.0f, 0.0f ) );
			vertexpos.push_back( CoordF( 0.0f, 1.0f ) );
			vertexpos.push_back( CoordF( 1.0f, 1.0f ) );
			vertexpos.push_back( CoordF( 1.0f, 0.0f ) );
		}
	}

	void ChangePoisionCoordinates( const Coord& pos, const Dim& size, vector<CoordF>& vertexpos )
	{
		Coord oldPos   = Coord( pos.x, TheShell().GetWindowSize().y - pos.y );
		Dim   oldsSize = size;

		vertexpos.push_back( CoordF(oldPos.x, oldPos.y) );
		vertexpos.push_back( CoordF(oldPos.x, oldPos.y - oldsSize.y) );
		vertexpos.push_back( CoordF(oldPos.x + oldsSize.x, oldPos.y - oldsSize.y) );
		vertexpos.push_back( CoordF(oldPos.x + oldsSize.x, oldPos.y) );
	}
};

ImageUnitPtr ImageUnit::Make()
{
	ImageUnitPtr img( new ImageUnit() );
	img->Initialize();
	return img;
}

ImageUnit::ImageUnit() : m_iBitmapIndex( -1 ), m_IsDraw( true ), m_IsDrawRectLine( false )
{

}

ImageUnit::~ImageUnit()
{
	Destroy();
}

void ImageUnit::Initialize()
{

}

void ImageUnit::Destroy()
{

}

void ImageUnit::SetImageIndex( const int bitmapindex )
{
	assert( bitmapindex <= BITMAP_EFFECT_TEXTURE_END );
	m_iBitmapIndex = bitmapindex;

	SetSize( Dim( static_cast<int>(Bitmaps[m_iBitmapIndex].Width), 
				  static_cast<int>(Bitmaps[m_iBitmapIndex].Height) 
				) 
		   );
}

//이부분은 전부 opengl로 다 빼자.
void ImageUnit::Draw()
{
	//index가 없으면 Draw를 하지 않는다.
	if( m_iBitmapIndex == -1 ) return;

	if( m_IsDraw == false ) return;

	vector<CoordF>				POScoordf;
	vector<CoordF>				UVcoordf;
	
	if( m_UvRect != Rect( 0, 0, 0, 0 ) )
	{
		ChangeUVCoordinates( m_UvRect, DimF( static_cast<float>(Bitmaps[m_iBitmapIndex].Width), 
											 static_cast<float>(Bitmaps[m_iBitmapIndex].Height) ), 1.0f, UVcoordf 
						   );
	}
	else
	{
		ChangeUVCoordinates( DimF( m_Size.x, m_Size.y ), DimF( static_cast<float>(Bitmaps[m_iBitmapIndex].Width), 
															   static_cast<float>(Bitmaps[m_iBitmapIndex].Height) 
															 ), 1.0f, UVcoordf );
	}

	ChangePoisionCoordinates( m_Position, m_Size, POScoordf );

	EnableAlphaTest();

	OPENGL_BINDTEXTURE( GL_TEXTURE_2D, Bitmaps[m_iBitmapIndex].TextureNumber );
	OPENGL_BEGIN( GL_TRIANGLE_FAN );
	OPENGL_COLOR4UB( m_Color );

	for( int i = 0; i < POScoordf.size(); ++i )
	{
		OPENGL_TEXCOORD2F( UVcoordf[i] );
		OPENGL_VERTEX2F( POScoordf[i] );
	}

	OPENGL_END();
}

void ImageUnit::DrawRect( const Color& color )
{
	int i = 0;

	EnableAlphaTest();

	DisableTexture();

	vector<CoordF>				POScoordf;
	ChangePoisionCoordinates( m_Position, m_Size, POScoordf );

	OPENGL_COLOR4UB( color );
	OPENGL_BEGIN( GL_TRIANGLE_FAN );
	for(i=0;i<POScoordf.size();i++)
	{
		OPENGL_VERTEX2F( POScoordf[i] );
	}
	
	OPENGL_END();

	if( m_IsDrawRectLine )
	{
		Coord Position   = Coord( m_Position.x, TheShell().GetWindowSize().y - m_Position.y );
		Dim   Size       = m_Size;
		Color color      = 0x77ffffff;

		Position = Coord( Position.x - 1, Position.y + 1 );
		Size     = Dim( Size.x + 2, Size.y + 2 );

		DRAWLINE( Coord( Position.x, Position.y ), Coord( Position.x, Position.y - Size.y ), color );
		DRAWLINE( Coord( Position.x, Position.y - Size.y ), Coord( Position.x + Size.x, Position.y - Size.y ), color );
		DRAWLINE( Coord( Position.x + Size.x, Position.y - Size.y ), Coord( Position.x + Size.x, Position.y ), color );
		DRAWLINE( Coord( Position.x + Size.x, Position.y ), Coord( Position.x, Position.y ), color );

		Position = Coord( Position.x - 2, Position.y + 2 );
		Size     = Dim( Size.x + 4, Size.y + 4 );

		DRAWLINE( Coord( Position.x, Position.y ), Coord(Position.x, Position.y - Size.y), color );
		DRAWLINE( Coord(Position.x, Position.y - Size.y), Coord(Position.x + Size.x, Position.y - Size.y), color );
		DRAWLINE( Coord(Position.x + Size.x, Position.y - Size.y), Coord(Position.x + Size.x, Position.y), color );
		DRAWLINE( Coord(Position.x + Size.x, Position.y), Coord( Position.x, Position.y ), color );
	}
}

#endif //NEW_USER_INTERFACE_IMAGE