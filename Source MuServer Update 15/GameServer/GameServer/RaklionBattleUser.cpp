// Raklion.cpp: implementation of the CRaklion class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RaklionBattleUser.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRaklionBattleUser::CRaklionBattleUser() // OK
{
	this->ResetData();
}

CRaklionBattleUser::~CRaklionBattleUser() // OK
{

}

void CRaklionBattleUser::ResetData() // OK
{
	this->m_Index = -1;
	this->m_IsUse = 0;
}

void CRaklionBattleUser::SetIndex(int aIndex) // OK
{
	this->m_Index = aIndex;
	this->m_IsUse = 1;
}

BOOL CRaklionBattleUser::IsUseData() // OK
{
	return this->m_IsUse;
}

int CRaklionBattleUser::GetIndex() // OK
{
	return this->m_Index;
}
