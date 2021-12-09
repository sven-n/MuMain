// MemoryLock.cpp: implementation of the CMemoryLock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemoryLock.h"


#define ML_TABLE_SIZE	( 1024)


BYTE CMemoryLock::s_byXor[SIZE_XOR_MEMORYLOCK] =
{ 0x93, 0xd2, 0xb5, 0x7e, 0xa3, 0xcf, 0x39, 0x6d,
	0xeb, 0x1a, 0x7d, 0x47, 0x29, 0xe3, 0xfb, 0x49};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemoryLock::CMemoryLock()
{
	return;
}

CMemoryLock::~CMemoryLock()
{
	return;
}

void CMemoryLock::Clear( void)
{
	return;
}

void CMemoryLock::Destroy( void)
{
	return;
}

void CMemoryLock::Init( HWND hWnd, UINT uiMessage)
{
	return;
}
