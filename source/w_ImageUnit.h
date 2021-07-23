// w_Image.h: interface for the Image class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_IMAGE

#if !defined(AFX_W_IMAGE_H__F46E9485_8567_41A3_B315_106F60F0E227__INCLUDED_)
#define AFX_W_IMAGE_H__F46E9485_8567_41A3_B315_106F60F0E227__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZzzTexture.h"

namespace image
{
	BoostSmartPointer( ImageUnit );
	class ImageUnit
	{
	public:
		enum VERTEXPOS { eLT, eLB, eRB, eRT, ePOS_MAX };

	public:
		static ImageUnitPtr Make();
		virtual ~ImageUnit();

	public:
		const BITMAP_t& QueryImage( const int bitmapindex );
		void SetImageIndex( const int bitmapindex );
		void SetPosition( const Coord& pos );
		void SetSize( const Dim& size );
		void SetDraw( bool isdraw );
		void SetColor( const Color& color );
		void SetUVRect( const Rect& uvrect );
		void SetDrawRectLine( bool state );

		const Coord& GetPosition() const;
		const Dim& GetSize() const;
		const Color& GetColor() const;
		const Rect& GetUVRect() const;

		void Draw();
		//여기 있으면 안 좋다..따로 구분을 하던지 빼도록 하자.
		void DrawRect( const Color& color );

	private:
		void Initialize();
		void Destroy();
		ImageUnit();

	private:
		Rect			m_UvRect;
		Coord			m_Position;
		Dim				m_Size;
		Color			m_Color;
		int				m_iBitmapIndex;
		bool			m_IsDraw;
		bool			m_IsDrawRectLine;
	};

	inline
	void ImageUnit::SetDrawRectLine( bool state )
	{
		m_IsDrawRectLine = state;
	}

	inline
	const BITMAP_t& ImageUnit::QueryImage( const int bitmapindex )
	{
		return Bitmaps[m_iBitmapIndex];
	}

	inline
	void ImageUnit::SetDraw( bool isdraw )
	{
		m_IsDraw = isdraw;
	}

	inline
	void ImageUnit::SetPosition( const Coord& pos )
	{
		m_Position = pos;
	}

	inline
	void ImageUnit::SetSize( const Dim& size )
	{
		m_Size = size;
	}

	inline
	void ImageUnit::SetColor( const Color& color )
	{
		m_Color = color;
	}

	inline
	void ImageUnit::SetUVRect( const Rect& uvrect )
	{
		m_UvRect = uvrect;
	}

	inline
	const Coord& ImageUnit::GetPosition() const
	{
		return m_Position;
	}

	inline
	const Dim& ImageUnit::GetSize() const
	{
		return m_Size;
	}

	inline
	const Color& ImageUnit::GetColor() const
	{
		return m_Color;
	}

	inline
	const Rect& ImageUnit::GetUVRect() const
	{
		return m_UvRect;
	}

};

#endif // !defined(AFX_W_IMAGE_H__F46E9485_8567_41A3_B315_106F60F0E227__INCLUDED_)

#endif //NEW_USER_INTERFACE_IMAGE