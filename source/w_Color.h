#pragma once

#ifdef NEW_USER_INTERFACE_UTIL

namespace util
{
	class Color
	{
	public:
		Color(void);
		~Color(void);
		Color( unsigned int );
		Color( unsigned char alpha
			 , unsigned char red
			 , unsigned char blue
			 , unsigned char green );

		Color( float alpha
			 , float red
			 , float blue
			 , float green );

		Color& operator = ( const Color& color );

		operator unsigned int() const;

		unsigned char Alpha() const;
		unsigned char Red() const;
		unsigned char Blue() const;
		unsigned char Green() const;

		void        SetAlpha( unsigned char value );
		void        SetRed( unsigned char value );
		void        SetBlue( unsigned char value );
		void        SetGreen( unsigned char value );

		DWORD		GetColor()
		{
			return static_cast<DWORD>( Red()|(Green()<<8)|(Blue()<<16) );
		}

		const Color& operator+=( const Color& color )
		{
			SetAlpha( Alpha() + color.Alpha() );
			SetRed( Red() + color.Red() );
			SetGreen( Green() + color.Green() );
			SetBlue( Blue() + color.Blue() );
			return *this;
		}
		Color operator+( const Color& color ) const
		{
			Color s;
			s = *this;

			s.SetAlpha( s.Alpha() + color.Alpha() );
			s.SetRed( s.Red() + color.Red() );
			s.SetGreen( s.Green() + color.Green() );
			s.SetBlue( s.Blue() + color.Blue() );

			return s;
		}
		Color operator-( const Color& color ) const
		{
			Color s;
			s = *this;
			s.SetAlpha( s.Alpha() - color.Alpha() );
			s.SetRed( s.Red() - color.Red() );
			s.SetGreen( s.Green() - color.Green() );
			s.SetBlue( s.Blue() - color.Blue() );
			return s;
		}

		Color operator*( const float f ) const
		{
			Color s;
			s = *this;
			s.SetAlpha( (unsigned char)( s.Alpha() * f ) );
			s.SetRed( (unsigned char)( s.Red() * f ) );
			s.SetGreen( (unsigned char)( s.Green() * f ) );
			s.SetBlue( (unsigned char)( s.Blue() * f ) );
			return s;
		}

		Color operator/( const float f ) const
		{
			Color s;
			s = *this;
			s.SetAlpha( (unsigned char)( s.Alpha() / f ) );
			s.SetRed( (unsigned char)( s.Red() / f ) );
			s.SetGreen( (unsigned char)( s.Green() / f ) );
			s.SetBlue( (unsigned char)( s.Blue() / f ) );
			return s;
		}

	private:
		unsigned int    m_Color;

	};

inline
Color::Color(void)
{
    m_Color = 0xffffffff;
}

inline
Color::~Color(void)
{
}

inline
Color::Color( unsigned int color )
{
    m_Color = color;
}

inline
Color::Color( unsigned char alpha
             , unsigned char red
             , unsigned char blue
             , unsigned char green )
{
    m_Color = 0xffffffff;
    SetAlpha( alpha );
    SetRed( red );
    SetBlue( blue );
    SetGreen( green );
}

inline
Color::Color( float alpha
	 , float red
	 , float blue
	 , float green )
{
	m_Color = 0xffffffff;
	SetAlpha( static_cast<unsigned char>((float)(0xff) * alpha) );
    SetRed( static_cast<unsigned char>((float)(0xff) * red) );
    SetBlue( static_cast<unsigned char>((float)(0xff) * blue) );
    SetGreen( static_cast<unsigned char>((float)(0xff) * green) );
}

inline
Color& Color::operator = ( const Color& color )
{
    m_Color = color.m_Color;
    return *this;
}

inline
Color::operator unsigned int() const
{
    return m_Color;
}

inline
unsigned char Color::Alpha() const
{
    return (m_Color >> 24) & 0xff;
}

inline
unsigned char Color::Red() const
{
    return (m_Color >> 16) & 0xff;
}

inline
unsigned char Color::Blue() const
{
    return m_Color & 0xff;
}

inline
unsigned char Color::Green() const
{
    return ( m_Color >> 8 ) & 0xff;
}

inline
void Color::SetAlpha( unsigned char value )
{
	m_Color &= ~( 0xff << 24 ) ; 
	m_Color |= (value << 24);
}

inline
void Color::SetRed( unsigned char value )
{
	m_Color &= ~(0xff << 16);
	m_Color |= (value << 16);
}

inline
void Color::SetBlue( unsigned char value )
{
	m_Color &= ~(0xff<<8);
	m_Color |= (value<<8);
}

inline
void Color::SetGreen( unsigned char value )
{
	m_Color &= ~(0xff );
	m_Color |= (value);
}

};

#endif //NEW_USER_INTERFACE_UTIL