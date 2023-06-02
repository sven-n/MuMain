//////////////////////////////////////////////////////////////////////////
//  - 싱글톤 -
//
//
//////////////////////////////////////////////////////////////////////////
#ifndef __SINGLETON_H__
#define __SINGLETON_H__

/*+++++++++++++++++++++++++++++++++++++
    CLASS.
+++++++++++++++++++++++++++++++++++++*/
template <typename T>
class Singleton
{
    static T* _Singleton;

public:
    Singleton(void)
    {
        if (_Singleton == 0)
        {
            int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
            _Singleton = (T*)((int)this + offset);
        }
    }

    virtual ~Singleton(void) {  /*assert( _Singleton );*/  _Singleton = 0; }

    static T& GetSingleton(void) {  /*assert( _Singleton );*/  return (*_Singleton); }
    static T* GetSingletonPtr(void) { return (_Singleton); }
    static bool IsInitialized(void) { return _Singleton ? true : false; }

    //여기 부분은 좀 생각을 해보자..
    //new로 만들어서 넣으면...delete를 해줘야 하는데...
    //할려면 boost로 만들어진 data만 넣도록 하자.
    //static void RegisterSingleton ( T* p ) { _Singleton = p; }
};

template <typename T> T* Singleton <T>::_Singleton = 0;

#endif