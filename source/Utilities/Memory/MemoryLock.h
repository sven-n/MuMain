// MemoryLock.h: interface for the CMemoryLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORYLOCK_H__ED135F22_F857_4DDE_9846_52935BCE88FD__INCLUDED_)
#define AFX_MEMORYLOCK_H__ED135F22_F857_4DDE_9846_52935BCE88FD__INCLUDED_

#pragma once

#ifdef _DEBUG
#define ML_NOCORRUPT	// 이상없이 잘 돌아가는지 체크해 보고 싶으면 이곳을 활성화한다.
#endif //_DEBUG

#ifdef _DEBUG
#define GO_DEBUG	_asm { int 3 }
#else
#define GO_DEBUG	{}
#endif

#include "HashTable.h"

#define SIZE_XOR_MEMORYLOCK	(16)

class CMemoryLock : public CHashTable <BYTE*, void*>
{
public:
	CMemoryLock();
	virtual ~CMemoryLock();
	void Clear();

	virtual void Destroy();

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

protected:
	void *m_pDumpMemory;
	DWORD m_dwHashKey;
	BYTE **m_ppDataTableAlloc;
	void **m_ppKeyTableAlloc;
	void ResetDumpMemory()
	{
		return;
	}
	void RegenerateHashKey()
	{
		return;
	}
public:
	virtual DWORD Hash(void *pKey)
	{
		return 0;
	}
	void Reposition()	// 테이블 상의 위치를 바꿔준다 ( 어떤 것도 Lock 되지 않은 상태에서만 가능)
	{
		return;
	}
};


#endif // !defined(AFX_MEMORYLOCK_H__ED135F22_F857_4DDE_9846_52935BCE88FD__INCLUDED_)
