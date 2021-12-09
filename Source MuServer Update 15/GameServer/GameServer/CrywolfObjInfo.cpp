// CrywolfObjInfo.cpp: implementation of the CCrywolfObjInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CrywolfObjInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCrywolfObjInfo::CCrywolfObjInfo() // OK
{
	this->Reset();
}

void CCrywolfObjInfo::Reset() // OK
{
	this->m_ObjCount = 0;

	for(int n=0;n < MAX_CRYWOLF_OBJ_INFO;n++)
	{
		this->m_ObjIndex[n] = -1;
	}
}

bool CCrywolfObjInfo::AddObj(int aIndex) // OK
{
	if(this->m_ObjCount < 0 || this->m_ObjCount >= MAX_CRYWOLF_OBJ_INFO)
	{
		return 0;
	}

	this->m_ObjIndex[this->m_ObjCount++] = aIndex;
	return 1;
}
