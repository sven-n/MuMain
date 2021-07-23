// MemoryLock.h: interface for the CMemoryLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORYLOCK_H__ED135F22_F857_4DDE_9846_52935BCE88FD__INCLUDED_)
#define AFX_MEMORYLOCK_H__ED135F22_F857_4DDE_9846_52935BCE88FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifdef _DEBUG
#define ML_NOCORRUPT	// 이상없이 잘 돌아가는지 체크해 보고 싶으면 이곳을 활성화한다.
#endif //_DEBUG

#ifdef _DEBUG
#define GO_DEBUG	_asm { int 3 }
#else
#define GO_DEBUG	{}
#endif


#include "HashTable.h"


#define SIZE_XOR_MEMORYLOCK	( 16)


class CMemoryLock : public CHashTable <BYTE*, void*>
{
public:
	CMemoryLock();
	virtual ~CMemoryLock();
	void Clear( void);

	virtual void Destroy( void);

protected:
	HWND m_hWnd;	// 오류 메시지를 보낼 윈도우 핸들
	UINT m_uiMessage;	// 윈도우에 보낼 메시지
#ifdef _DEBUG
	int m_iUsedCount;
#endif

public:
	static BYTE s_byXor[SIZE_XOR_MEMORYLOCK];

public:
	void Init( HWND hWnd, UINT uiMessage);

	template <class T>
	void Lock( T &Data)
	{
		return;
	}

	template <class T>
	void UnLock( T &Data)
	{	// 저장, 원래 값 리턴
		return;
	}

	template <class T>
	T Get( T &Data)
	{
		return Data;
	}

	template <class T>
	void Release( T &Data)
	{
		return;
	}
	
protected:
	template <class T>
	void MakeConvertedDataAndCorrupt( BYTE *pbyDst, T &Data)
	{
		return;
	}

	template <class T>
	void RestoreConvertedData( T &Data, BYTE *pbySrc)
	{
		return;
	}

	// 테이블상의 위치가 이동하도록 하기 위한 부분
protected:
	void *m_pDumpMemory;	// new 할때 위치를 더 유동적으로 하기 위한 부분
	DWORD m_dwHashKey;	// 해쉬키가 일정해도 되지만 이동을 위해 변수로 수정
	BYTE **m_ppDataTableAlloc;	// 할당때의 포인터 - 이것을 쓰는 이유는 new 를 계속 새로 하다 보면 일정한 몇개의 주소가 돌아가면서 new 되기 때문에 그것을 막고자 다른 곳에서 new 해서 random 으로 약간 주소를 변경하기 위함이다.
	void **m_ppKeyTableAlloc;	// 할당때의 포인터
	void ResetDumpMemory( void)
	{
		return;
	}
	void RegenerateHashKey( void)
	{
		return;
	}
public:
	virtual DWORD Hash( void *pKey)
	{
		return 0;
	}
	void Reposition( void)	// 테이블 상의 위치를 바꿔준다 ( 어떤 것도 Lock 되지 않은 상태에서만 가능)
	{
		return;
	}
};


#endif // !defined(AFX_MEMORYLOCK_H__ED135F22_F857_4DDE_9846_52935BCE88FD__INCLUDED_)
