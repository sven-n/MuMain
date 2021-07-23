#pragma once

#include <math.h>

#ifdef NEW_USER_INTERFACE_UTIL

namespace util
{
#pragma pack( push, 1 )
template< typename TYPE = short >
struct _Coordinates
{
    TYPE x;
    TYPE y;

    _Coordinates();
    _Coordinates( const _Coordinates<TYPE>& rhs )   { x = rhs.x; y = rhs.y; }
    _Coordinates( TYPE xval, TYPE yval )            { x = xval; y = yval; }

    void            Reset() { x = y = (TYPE)0; }

    TYPE                    SquareMagnitude() const;
    TYPE                    Magnitude() const;

    _Coordinates<TYPE>&     operator  = ( const _Coordinates<TYPE>& rhs );
    void                    operator += ( const TYPE rhs );
    void                    operator += ( const _Coordinates<TYPE>& rhs );
    void                    operator -= ( const TYPE rhs );
    void                    operator -= ( const _Coordinates<TYPE>& rhs );
    void                    operator *= ( const TYPE rhs );
    void                    operator *= ( const _Coordinates<TYPE>& rhs );
    void                    operator /= ( const TYPE rhs );
    void                    operator /= ( const _Coordinates<TYPE>& rhs );
    void                    operator %= ( const TYPE rhs );
    void                    operator %= ( const _Coordinates<TYPE>& rhs );
    bool                    operator == ( const _Coordinates<TYPE>& rhs ) const;
    bool                    operator != ( const _Coordinates<TYPE>& rhs ) const;

    _Coordinates<TYPE>      operator  + ( const TYPE rhs ) const;
    _Coordinates<TYPE>      operator  + ( const _Coordinates<TYPE>& rhs ) const;
    _Coordinates<TYPE>      operator  - ( const TYPE rhs ) const;
    _Coordinates<TYPE>      operator  - ( const _Coordinates<TYPE>& rhs ) const;
    _Coordinates<TYPE>      operator  * ( const TYPE rhs ) const;
    _Coordinates<TYPE>      operator  * ( const _Coordinates<TYPE>& rhs ) const;
    _Coordinates<TYPE>      operator  / ( const TYPE rhs ) const;
    _Coordinates<TYPE>      operator  / ( const _Coordinates<TYPE>& rhs ) const;

    friend _Coordinates<TYPE>  operator + ( const TYPE lhs, const _Coordinates<TYPE>& rhs );
    friend _Coordinates<TYPE>  operator * ( const TYPE lhs, const _Coordinates<TYPE>& rhs );
};
#pragma pack( pop )

struct _CoordinatesSHORT : public _Coordinates<short>
{
public:
    _CoordinatesSHORT(){}
    _CoordinatesSHORT( const int value );
    _CoordinatesSHORT( const short x, const short y );
    _CoordinatesSHORT( const _Coordinates<short>& rhs ) : _Coordinates<short>( rhs ) {}
    _CoordinatesSHORT( const _Coordinates<float>& rhs ) { x = (short)rhs.x; y = (short)rhs.y; }
    _CoordinatesSHORT& operator = ( const _Coordinates<short>& coord );
    _CoordinatesSHORT& operator = ( const _Coordinates<float>& coordf );
};

struct _CoordinatesFLOAT : public _Coordinates<float>
{
public:
    _CoordinatesFLOAT(){}
    _CoordinatesFLOAT( const float x, const float y );
    _CoordinatesFLOAT( const _Coordinates<float>& rhs ) : _Coordinates<float>( rhs ) {}
    _CoordinatesFLOAT( const _Coordinates<short>& rhs ) { x = (float)rhs.x; y = (float)rhs.y; }
    _CoordinatesFLOAT& operator = ( const _Coordinates<float>& coordf );
    _CoordinatesFLOAT& operator = ( const _Coordinates<short>& coord );
};

template< typename TYPE >
inline
_Coordinates<TYPE>::_Coordinates()
: x( 0 ), y( 0 )
{
}

template< typename TYPE >
inline
TYPE _Coordinates<TYPE>::SquareMagnitude() const
{
    return x * x + y * y;
}

template< typename TYPE >
inline
TYPE _Coordinates<TYPE>::Magnitude() const
{
    return sqrt( x * x + y * y );
}

template< typename TYPE >
inline
_Coordinates<TYPE>& _Coordinates<TYPE>::operator  = ( const _Coordinates& rhs )
{
    x = rhs.x;
    y = rhs.y;
    return *this;
}

template< typename TYPE >
inline
void _Coordinates<TYPE>::operator += ( const TYPE rhs )
{
    x += rhs;
    y += rhs;
}

template< typename TYPE >
inline
void _Coordinates<TYPE>::operator -= ( const TYPE rhs )
{
    x -= rhs;
    y -= rhs;
}

template< typename TYPE >
inline
void _Coordinates<TYPE>::operator *= ( const TYPE rhs )
{
    x *= rhs; 
    y *= rhs;
}

template< typename TYPE >
inline
void _Coordinates<TYPE>::operator /= ( const TYPE rhs )
{
    //_ASSERT( rhs );
    x /= rhs;
    y /= rhs;
}

template< typename TYPE >
inline
void _Coordinates<TYPE>::operator %= ( const TYPE rhs )
{
    //_ASSERT( rhs );
    x %= rhs;
    y %= rhs;
}

template< typename TYPE >
inline
void _Coordinates<TYPE>::operator += ( const _Coordinates<TYPE>& rhs )
{
    x += rhs.x;
    y += rhs.y;
}

template< typename TYPE >
inline
void _Coordinates<TYPE>::operator -= ( const _Coordinates<TYPE>& rhs )
{
    x -= rhs.x;
    y -= rhs.y;
}

template< typename TYPE >
inline
void _Coordinates<TYPE>::operator *= ( const _Coordinates<TYPE>& rhs )
{
    x *= rhs.x; 
    y *= rhs.y;
}

template< typename TYPE >
inline
void _Coordinates<TYPE>::operator /= ( const _Coordinates<TYPE>& rhs )
{
    //_ASSERT( rhs.x && rhs.y );
    x /= rhs.x;
    y /= rhs.y;
}

template< typename TYPE >
inline
void _Coordinates<TYPE>::operator %= ( const _Coordinates<TYPE>& rhs )
{
    //_ASSERT( rhs.x && rhs.y );
    x %= rhs.x;
    y %= rhs.y;
}

template< typename TYPE >
inline
bool _Coordinates<TYPE>::operator == ( const _Coordinates<TYPE>& rhs ) const
{
    return ( x == rhs.x && y == rhs.y ) ? true : false;
}

template< typename TYPE >
inline
bool _Coordinates<TYPE>::operator != ( const _Coordinates<TYPE>& rhs ) const
{
    return operator==( rhs ) ? false : true;
}

template< typename TYPE >
inline
_Coordinates<TYPE> _Coordinates<TYPE>::operator + ( const _Coordinates<TYPE>& rhs ) const
{
    _Coordinates<TYPE> temp;
    temp = *this;
    temp += rhs;
    return temp;
}

template< typename TYPE >
inline
_Coordinates<TYPE> _Coordinates<TYPE>::operator + ( const TYPE rhs ) const
{
    _Coordinates<TYPE> temp;
    temp = *this;
    temp += rhs;
    return temp;
}

template< typename TYPE >
inline
_Coordinates<TYPE> _Coordinates<TYPE>::operator - ( const _Coordinates<TYPE>& rhs ) const
{
    _Coordinates<TYPE> temp;
    temp = *this;
    temp -= rhs;
    return temp;
}

template< typename TYPE >
inline
_Coordinates<TYPE> _Coordinates<TYPE>::operator - ( const TYPE rhs ) const
{
    _Coordinates<TYPE> temp;
    temp = *this;
    temp -= rhs;
    return temp;
}

template< typename TYPE >
inline
_Coordinates<TYPE> _Coordinates<TYPE>::operator * ( const _Coordinates<TYPE>& rhs ) const
{
    _Coordinates<TYPE> temp;
    temp = *this;
    temp *= rhs;
    return temp;
}

template< typename TYPE >
inline
_Coordinates<TYPE> _Coordinates<TYPE>::operator * ( const TYPE rhs ) const
{
    _Coordinates<TYPE> temp;
    temp = *this;
    temp *= rhs;
    return temp;
}

template< typename TYPE >
inline
_Coordinates<TYPE> _Coordinates<TYPE>::operator / ( const _Coordinates<TYPE>& rhs ) const
{
    _Coordinates<TYPE> temp;
    temp = *this;
    temp /= rhs;
    return temp;
}

template< typename TYPE >
inline
_Coordinates<TYPE> _Coordinates<TYPE>::operator / ( const TYPE rhs ) const
{
    _Coordinates<TYPE> temp;
    temp = *this;
    temp /= rhs;
    return temp;
}

template< typename TYPE >
inline
_Coordinates<TYPE>  operator + ( const TYPE lhs, const _Coordinates<TYPE>& rhs )
{
    _Coordinates<TYPE> temp;
    temp = rhs;
    temp += lhs;
    return temp;
}

template< typename TYPE >
inline
_Coordinates<TYPE>  operator * ( const TYPE lhs, const _Coordinates<TYPE>& rhs )
{
    _Coordinates<TYPE> temp;
    temp = rhs;
    temp *= lhs;
    return temp;
}

inline
_CoordinatesSHORT::_CoordinatesSHORT( const short x, const short y )
{
    this->x = x;
    this->y = y;
}

inline
_CoordinatesSHORT::_CoordinatesSHORT( const int value )
{
    x = value >> 16;
    y = value & 0xffff;
}

inline
_CoordinatesSHORT& _CoordinatesSHORT::operator = ( const _Coordinates<short>& coord )
{
    x = coord.x;
    y = coord.y;
    return *this;
}

inline
_CoordinatesSHORT& _CoordinatesSHORT::operator = ( const _Coordinates<float>& coordf )
{
    x = (short)floor( coordf.x + 0.5f );
    y = (short)floor( coordf.y + 0.5f );
    return *this;
}

inline
_CoordinatesFLOAT::_CoordinatesFLOAT( const float x, const float y )
{
    this->x = x;
    this->y = y;
}


inline
_CoordinatesFLOAT& _CoordinatesFLOAT::operator = ( const _Coordinates<float>& coordf )
{
    x = coordf.x;
    y = coordf.y;
    return *this;
}

inline
_CoordinatesFLOAT& _CoordinatesFLOAT::operator = ( const _Coordinates<short>& coord )
{
    x = (float)coord.x;
    y = (float)coord.y;
    return *this;
}

typedef _CoordinatesSHORT Coord;
typedef _CoordinatesFLOAT CoordF;

typedef _CoordinatesSHORT Dim;
typedef _CoordinatesFLOAT DimF;

};



#endif //NEW_USER_INTERFACE_UTIL

