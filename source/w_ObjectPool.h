// w_ObjectPool.h: interface for the ObjectPool class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UTIL

#if !defined(AFX_W_OBJECTPOOL_H__1CC67B72_8B0A_4E66_875A_B31EE4B0342D__INCLUDED_)
#define AFX_W_OBJECTPOOL_H__1CC67B72_8B0A_4E66_875A_B31EE4B0342D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "boost/pool/object_pool.hpp"

#define NOT_THREAD_OBJECTPOOL

namespace util
{
	template<typename T>
	class ObjectPool  
	{
	public:
        struct _Pool
        {
            object_pool<T>   objectPool;
        };

    private:
#ifdef NOT_THREAD_OBJECTPOOL
        static _Pool								s_Pool;        
#else //NOT_THREAD_OBJECTPOOL
        static __declspec( thread ) object_pool<T>  s_Pool;
#endif //NOT_THREAD_OBJECTPOOL
        

    public:
        static void* operator new( size_t size )
        {
            assert( sizeof( T ) == size );
            void* p = s_Pool.objectPool.malloc();
            return p;
        }

        static void operator delete( void* rawMemory )
        {
            s_Pool.objectPool.free( reinterpret_cast<T*>(rawMemory) );
        }

	    ObjectPool()
        {
        }

        virtual ~ObjectPool() = 0{}
    };
};

#define MAKE_OBJECT_POOL( TYPE ) \
    util::ObjectPool<TYPE>::_Pool  util::ObjectPool<TYPE>::s_Pool;

#endif // !defined(AFX_W_OBJECTPOOL_H__1CC67B72_8B0A_4E66_875A_B31EE4B0342D__INCLUDED_)

#endif //NEW_USER_INTERFACE_UTIL