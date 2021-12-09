// MemoryAllocator.h: interface for the CMemoryAllocator class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CriticalSection.h"
#include "MemoryAllocatorInfo.h"

class CMemoryAllocator
{
public:
	CMemoryAllocator();
	virtual ~CMemoryAllocator();
	bool GetMemoryAllocatorFree(int* index,int MinIndex,int MaxIndex,DWORD MinTime);
	bool GetMemoryAllocatorInfo(CMemoryAllocatorInfo* lpMemoryAllocatorInfo,int index);
	void InsertMemoryAllocatorInfo(CMemoryAllocatorInfo MemoryAllocatorInfo);
	void RemoveMemoryAllocatorInfo(CMemoryAllocatorInfo MemoryAllocatorInfo);
	void BindMemoryAllocatorInfo(int index,CMemoryAllocatorInfo MemoryAllocatorInfo);
public:
	CCriticalSection m_critical;
	CMemoryAllocatorInfo m_TempMemoryAllocatorInfo;
	CMemoryAllocatorInfo m_MemoryAllocatorInfo[MAX_OBJECT];
};

extern CMemoryAllocator gMemoryAllocator;
