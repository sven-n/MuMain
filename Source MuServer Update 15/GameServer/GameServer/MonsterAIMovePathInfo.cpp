// MonsterAIMovePathInfo.cpp: implementation of the CMonsterAIMovePathInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIMovePathInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIMovePathInfo::CMonsterAIMovePathInfo() // OK
{
	this->Reset();
}

void CMonsterAIMovePathInfo::Reset() // OK
{
	this->m_Type = -1;
	this->m_MapNumber = -1;
	this->m_PathX = -1;
	this->m_PathY = -1;
}
