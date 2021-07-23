#pragma once

#include "w_coord.h"

#ifdef NEW_USER_INTERFACE_UTIL

namespace util
{
#pragma pack( push, 1 )
template < typename TYPE = short >
struct _Rectangle
{
    union
    {
        struct
        {
            _Coordinates<TYPE> tl;
            _Coordinates<TYPE> br;
        };

        struct  
        {
            TYPE left;
            TYPE top;
            TYPE right;
            TYPE bottom;
        };
    };


    void            Reset() { left = top = right = bottom = (TYPE)0; }
    void            Nomalize();
    bool            IsIncluded( const _Coordinates<TYPE>& coord ) const;
    bool            IsIntersect( const _Rectangle<TYPE>& rect ) const;
    TYPE            GetHeight() const;
    TYPE            GetWidth() const;
    _Coordinates<TYPE>  GetSize() const { return _Coordinates<TYPE>( GetWidth(), GetHeight() ); }
    _Coordinates<TYPE>  GetCenterCoord() const;

    void            InflateRect( TYPE x, TYPE y );
    void            InflateRect( const _Coordinates<TYPE>& dim );
    void            InflateRect( const _Rectangle<TYPE>& rect );
    void            InflateRect( TYPE l, TYPE t, TYPE r, TYPE b );

    void            DeflateRect( TYPE x, TYPE y );
    void            DeflateRect( const _Coordinates<TYPE>& dim );
    void            DeflateRect( const _Rectangle<TYPE>& rect );
    void            DeflateRect( TYPE l, TYPE t, TYPE r, TYPE b );    

    _Rectangle();
    _Rectangle( const _Rectangle<TYPE>& rhs );
    _Rectangle( const TYPE left, const TYPE top, const TYPE right, const TYPE bottom );
    _Rectangle( const _Coordinates<TYPE>& tl, const _Coordinates<TYPE>& br );

    _Rectangle<TYPE>&           operator  = ( const _Rectangle<TYPE>& rhs );
    _Rectangle<TYPE>&           operator  = ( const RECT& rect );
    _Rectangle<TYPE>&           operator  = ( const RECT* pRect );

    void            operator += ( const TYPE rhs );
    void            operator += ( const _Coordinates<TYPE>& rhs );

    void            operator -= ( const TYPE rhs );
    void            operator -= ( const _Coordinates<TYPE>& rhs );

    bool            operator == ( const _Rectangle<TYPE>& rhs );
    bool            operator != ( const _Rectangle<TYPE>& rhs );

    void            operator &= ( const _Rectangle<TYPE>& rhs );
    void            operator |= ( const _Rectangle<TYPE>& rhs );

    _Rectangle<TYPE>            operator  & ( const _Rectangle<TYPE>& rhs ) const;
    _Rectangle<TYPE>            operator  | ( const _Rectangle<TYPE>& rhs ) const;

    _Rectangle<TYPE>            operator  + ( const TYPE rhs ) const;
    _Rectangle<TYPE>            operator  + ( const _Coordinates<TYPE>& rhs ) const;

    _Rectangle<TYPE>            operator  - ( const TYPE rhs ) const;
    _Rectangle<TYPE>            operator  - ( const _Coordinates<TYPE>& rhs ) const;

    friend _Rectangle<TYPE>     operator  + ( const TYPE lhs, const _Rectangle<TYPE>& rhs );
    friend _Rectangle<TYPE>     operator  + ( const _Coordinates<TYPE>& lhs, const _Rectangle<TYPE>& rhs );

};

#pragma pack( pop )

struct _RectangleSHORT : public _Rectangle<short>
{
public:
    _RectangleSHORT() {}
    _RectangleSHORT( const RECT& r );
    _RectangleSHORT( const _Rectangle<short>& rhs ) : _Rectangle<short>( rhs ) {}
    _RectangleSHORT( const short left, const short top, const short right, const short bottom );
    _RectangleSHORT( const _Coordinates<short>& tl, const _Coordinates<short>& br );
    _RectangleSHORT& operator = ( const _Rectangle<short>& rect );
    _RectangleSHORT& operator = ( const _Rectangle<float>& rectf );
                     operator RECT() const;

    _RectangleSHORT&           operator  = ( const RECT& rect )     { return (_RectangleSHORT&)_Rectangle<short>::operator=( rect ); }
    _RectangleSHORT&           operator  = ( const RECT* pRect )    { return (_RectangleSHORT&)_Rectangle<short>::operator=( pRect ); }
};

struct _RectangleFLOAT : public _Rectangle<float>
{
public:
    _RectangleFLOAT();
    _RectangleFLOAT( const _Rectangle<float>& rhs ) : _Rectangle<float>( rhs ) {}
    _RectangleFLOAT( const _RectangleSHORT& rect );
    _RectangleFLOAT( const float left, const float top, const float right, const float bottom );
    _RectangleFLOAT( const _Coordinates<float>& tl, const _Coordinates<float>& br );
    _RectangleFLOAT( const RECT& rect );
    _RectangleFLOAT& operator = ( const _RectangleSHORT& rect );
    _RectangleFLOAT& operator = ( const _Rectangle<float>& rectf );

    _RectangleFLOAT&           operator  = ( const RECT& rect )     { return (_RectangleFLOAT&)_Rectangle<float>::operator=( rect ); }
    _RectangleFLOAT&           operator  = ( const RECT* pRect )    { return (_RectangleFLOAT&)_Rectangle<float>::operator=( pRect ); }
};

template< typename TYPE >
inline
_Rectangle<TYPE>::_Rectangle()
{
	left    = 0;
    top     = 0;
    right   = 0;
    bottom  = 0;
}

template< typename TYPE >
inline
_Rectangle<TYPE>::_Rectangle( const _Rectangle<TYPE>& rhs )
{
    left    = rhs.left;
    top     = rhs.top;
    right   = rhs.right;
    bottom  = rhs.bottom;
}

template< typename TYPE >
inline
_Rectangle<TYPE>::_Rectangle( const TYPE left, const TYPE top, const TYPE right, const TYPE bottom )
{
    this->left      = left;
    this->top       = top;
    this->right     = right;
    this->bottom    = bottom;
}


template< typename TYPE >
inline
_Rectangle<TYPE>::_Rectangle( const _Coordinates<TYPE>& tl, const _Coordinates<TYPE>& br )
{
    left    = tl.x;
    top     = tl.y;
    right   = br.x;
    bottom  = br.y;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::Nomalize()
{
    TYPE temp;
    if( left > right ) { temp = left; left = right; right = temp; }
    if( top > bottom ) { temp = top; top = bottom; bottom = temp; }
}

template< typename TYPE >
inline
bool _Rectangle<TYPE>::IsIncluded( const _Coordinates<TYPE>& coord ) const
{
    if( coord.x < left )
        return false;
    if( coord.y < top )
        return false;

    if( coord.x >= right )
        return false;

    if( coord.y >= bottom )
        return false;

    return true;
}

template< typename TYPE >
inline
bool _Rectangle<TYPE>::IsIntersect( const _Rectangle<TYPE>& rect ) const
{
    if( left >= rect.right || right <= rect.left ||
        top >= rect.bottom || bottom <= rect.top )
        return false;
    return true;
}

template< typename TYPE >
inline
TYPE _Rectangle<TYPE>::GetHeight() const
{
    return ( bottom - top );
}

template< typename TYPE >
inline
TYPE _Rectangle<TYPE>::GetWidth() const
{
    return ( right - left );
}

template< typename TYPE >
inline
_Coordinates<TYPE> _Rectangle<TYPE>::GetCenterCoord() const
{
    return _Coordinates<TYPE>( ( right - left ) / (TYPE)2, ( bottom - top ) / (TYPE)2 );
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::InflateRect( TYPE x, TYPE y )
{
    left    -= x;
    top     -= y;
    right   += x;
    bottom  += y;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::InflateRect( const _Coordinates<TYPE>& dim )
{
    left    -= dim.x;
    top     -= dim.y;
    right   += dim.x;
    bottom  += dim.y;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::InflateRect( const _Rectangle<TYPE>& rect )
{
    left    -= rect.left;
    top     -= rect.top;
    right   += rect.right;
    bottom  += rect.bottom;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::InflateRect( TYPE l, TYPE t, TYPE r, TYPE b )
{
    left    -= l;
    top     -= t;
    right   += r;
    bottom  += b;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::DeflateRect( TYPE x, TYPE y )
{
    left    += x;
    top     += y;
    right   -= x;
    bottom  -= y;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::DeflateRect( const _Coordinates<TYPE>& dim )
{
    left    += dim.x;
    top     += dim.y;
    right   -= dim.x;
    bottom  -= dim.y;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::DeflateRect( const _Rectangle<TYPE>& rect )
{
    left    += rect.left;
    top     += rect.top;
    right   -= rect.right;
    bottom  -= rect.bottom;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::DeflateRect( TYPE l, TYPE t, TYPE r, TYPE b )
{
    left    += l;
    top     += t;
    right   -= r;
    bottom  -= b;
}


template< typename TYPE >
inline
_Rectangle<TYPE>& _Rectangle<TYPE>::operator  = ( const _Rectangle<TYPE>& rhs )
{
    left    = rhs.left;
    top     = rhs.top;
    right   = rhs.right;
    bottom  = rhs.bottom;
    return *this;
}

template< typename TYPE >
inline
_Rectangle<TYPE>& _Rectangle<TYPE>::operator  = ( const RECT& rect )
{
    left    = (TYPE)rect.left;
    top     = (TYPE)rect.top;
    right   = (TYPE)rect.right;
    bottom  = (TYPE)rect.bottom;
    return *this;
}

template< typename TYPE >
inline
_Rectangle<TYPE>& _Rectangle<TYPE>::operator  = ( const RECT* pRect )
{
    left    = (TYPE)pRect->left;
    top     = (TYPE)pRect->top;
    right   = (TYPE)pRect->right;
    bottom  = (TYPE)pRect->bottom;
    return *this;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::operator += ( const TYPE rhs )
{
    left    += rhs;
    top     += rhs;
    right   += rhs;
    bottom  += rhs;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::operator += ( const _Coordinates<TYPE>& rhs )
{   
    left    += rhs.x;
    top     += rhs.y;
    right   += rhs.x;
    bottom  += rhs.y;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::operator -= ( const TYPE rhs )
{
    left    -= rhs;
    top     -= rhs;
    right   -= rhs;
    bottom  -= rhs;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::operator -= ( const _Coordinates<TYPE>& rhs )
{   
    left    -= rhs.x;
    top     -= rhs.y;
    right   -= rhs.x;
    bottom  -= rhs.y;
}

template< typename TYPE >
inline
bool _Rectangle<TYPE>::operator == ( const _Rectangle<TYPE>& rhs )
{
    return ( left == rhs.left && top == rhs.top && right == rhs.right && bottom == rhs.bottom ) ? true : false;
}

template< typename TYPE >
inline
bool _Rectangle<TYPE>::operator != ( const _Rectangle<TYPE>& rhs )
{
    return ( operator == ( rhs ) ) ? false : true;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::operator &= ( const _Rectangle<TYPE>& rhs )
{
    left    = ( left   > rhs.left   ) ? left : rhs.left;
    top     = ( top    > rhs.top    ) ? top  : rhs.top;
    right   = ( right  > rhs.right  ) ? rhs.right  : right;
    bottom  = ( bottom > rhs.bottom ) ? rhs.bottom : bottom;
}

template< typename TYPE >
inline
void _Rectangle<TYPE>::operator |= ( const _Rectangle<TYPE>& rhs )
{
    left    = ( left   < rhs.left   ) ? left : rhs.left;
    top     = ( top    < rhs.top    ) ? top  : rhs.top;
    right   = ( right  < rhs.right  ) ? rhs.right  : right;
    bottom  = ( bottom < rhs.bottom ) ? rhs.bottom : bottom;
}

template< typename TYPE >
inline
_Rectangle<TYPE> _Rectangle<TYPE>::operator  & ( const _Rectangle<TYPE>& rhs ) const
{
    _Rectangle<TYPE> rect;
    rect = *this;
    rect &= rhs;
    return rect;
}

template< typename TYPE >
inline
_Rectangle<TYPE> _Rectangle<TYPE>::operator  | ( const _Rectangle<TYPE>& rhs ) const
{
    _Rectangle<TYPE> rect;
    rect = *this;
    rect |= rhs;
    return rect;
}

template< typename TYPE >
inline
_Rectangle<TYPE> _Rectangle<TYPE>::operator  + ( const TYPE rhs ) const
{
    _Rectangle<TYPE> rect;
    rect = *this;
    rect += rhs;
    return rect;
}

template< typename TYPE >
inline
_Rectangle<TYPE> _Rectangle<TYPE>::operator  + ( const _Coordinates<TYPE>& rhs ) const
{
    _Rectangle<TYPE> rect;
    rect = *this;
    rect += rhs;
    return rect;
}

template< typename TYPE >
inline
_Rectangle<TYPE> _Rectangle<TYPE>::operator  - ( const TYPE rhs ) const
{
    _Rectangle<TYPE> rect;
    rect = *this;
    rect -= rhs;
    return rect;
}

template< typename TYPE >
inline
_Rectangle<TYPE> _Rectangle<TYPE>::operator  - ( const _Coordinates<TYPE>& rhs ) const
{
    _Rectangle<TYPE> rect;
    rect = *this;
    rect -= rhs;
    return rect;
}


template< typename TYPE >
inline
_Rectangle<TYPE> operator  + ( const TYPE lhs, const _Rectangle<TYPE>& rhs )
{
    _Rectangle<TYPE> rect;
    rect = rhs;
    rect += lhs;
    return rect;
}

template< typename TYPE >
inline
_Rectangle<TYPE> operator  + ( const _Coordinates<TYPE>& lhs, const _Rectangle<TYPE>& rhs )
{
    _Rectangle<TYPE> rect;
    rect = rhs;
    rect += lhs;
    return rect;
}

inline
_RectangleSHORT::_RectangleSHORT( const _Coordinates<short>& tl, const _Coordinates<short>& br )
{
    left    = tl.x;
    top     = tl.y;
    right   = br.x;
    bottom  = br.y;
}

inline
_RectangleSHORT& _RectangleSHORT::operator = ( const _Rectangle<short>& rect )
{
    left    = rect.left;
    top     = rect.top;
    right   = rect.right;
    bottom  = rect.bottom;
    return *this;
}

inline
_RectangleSHORT& _RectangleSHORT::operator = ( const _Rectangle<float>& rectf )
{
    left    = (short)floor( rectf.left + 0.5f );
    top     = (short)floor( rectf.top + 0.5f );
    right   = (short)floor( rectf.right + 0.5f );
    bottom  = (short)floor( rectf.bottom + 0.5f );
    return *this;
}

inline
_RectangleSHORT::_RectangleSHORT( const short left, const short top, const short right, const short bottom )
{
    this->left      = left;
    this->top       = top;
    this->right     = right;
    this->bottom    = bottom;
}

inline
_RectangleSHORT::_RectangleSHORT( const RECT& r )
{
    left    = static_cast<short>( r.left );
    top     = static_cast<short>( r.top );
    right   = static_cast<short>( r.right );
    bottom  = static_cast<short>( r.bottom );
}

inline
_RectangleSHORT::operator RECT() const
{
    RECT r;
    r.left      = left;
    r.top       = top;
    r.right     = right;
    r.bottom    = bottom;
    return r;
}

inline
_RectangleFLOAT::_RectangleFLOAT()
{
	left    = 0.0f;
    top     = 0.0f;
    right   = 0.0f;
    bottom  = 0.0f;
}

inline
_RectangleFLOAT::_RectangleFLOAT( const float left, const float top, const float right, const float bottom )
{
    this->left      = left;
    this->top       = top;
    this->right     = right;
    this->bottom    = bottom;
}

inline
_RectangleFLOAT::_RectangleFLOAT( const _Coordinates<float>& tl, const _Coordinates<float>& br )
{
    left    = tl.x;
    top     = tl.y;
    right   = br.x;
    bottom  = br.y;
}

inline
_RectangleFLOAT::_RectangleFLOAT( const RECT& rect )
{
    left    = static_cast<float>(rect.left);
    top     = static_cast<float>(rect.top);
    right   = static_cast<float>(rect.right);
    bottom  = static_cast<float>(rect.bottom);
}

inline
_RectangleFLOAT& _RectangleFLOAT::operator = ( const _Rectangle<float>& rectf )
{
    left   = rectf.left;
    top    = rectf.top;
    right  = rectf.right;
    bottom = rectf.bottom;
    return *this;
}

inline
_RectangleFLOAT::_RectangleFLOAT( const _RectangleSHORT& rect )
{
    left   = static_cast<float>(rect.left);
    top    = static_cast<float>(rect.top);
    right  = static_cast<float>(rect.right);
    bottom = static_cast<float>(rect.bottom);
}

inline
_RectangleFLOAT& _RectangleFLOAT::operator = ( const _RectangleSHORT& rect )
{
    left   = static_cast<float>(rect.left);
    top    = static_cast<float>(rect.top);
    right  = static_cast<float>(rect.right);
    bottom = static_cast<float>(rect.bottom);
    return *this;
}

typedef _RectangleSHORT Rect;
typedef _RectangleFLOAT RectF;

};

#endif //NEW_USER_INTERFACE_UTIL