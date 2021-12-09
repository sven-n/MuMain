// UnionInfo.cpp: implementation of the CUnionInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnionInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnionInfo::CUnionInfo()
{
	this->m_iUnionTimeStamp = 0;
	this->m_iRivalTimeStamp = 0;
	this->m_iMasterGuild = 0;
	memset(this->m_szMasterGuild, 0, sizeof(this->m_szMasterGuild));
}

CUnionInfo::~CUnionInfo()
{
	this->Clear(0);
}

int CUnionInfo::GetUnionMemberCount()
{
	int iCount = 0;

	this->m_Sync.Lock();
	iCount = this->m_vtUnionMember.size();
	this->m_Sync.Unlock();

	return iCount;
}

int CUnionInfo::GetRivalMemberCount()
{
	int iCount = 0;

	this->m_Sync.Lock();
	iCount = this->m_vtRivalMember.size();
	this->m_Sync.Unlock();

	return iCount;
}

BOOL CUnionInfo::IsUnionMember(int iGuildNumber)
{
	this->m_Sync.Lock();
	std::vector<int>::iterator _Itor = std::find(this->m_vtUnionMember.begin(), this->m_vtUnionMember.end(), iGuildNumber );
	
	if ( _Itor != this->m_vtUnionMember.end() )
	{
		this->m_Sync.Unlock();
		return TRUE;
	}

	this->m_Sync.Unlock();
	return FALSE;
}

BOOL CUnionInfo::IsRivalMember(int iGuildNumber)
{
	this->m_Sync.Lock();
	std::vector<int>::iterator _Itor = std::find(this->m_vtRivalMember.begin(), this->m_vtRivalMember.end(), iGuildNumber );

	if ( _Itor != this->m_vtRivalMember.end() )
	{
		this->m_Sync.Unlock();
		return TRUE;
	}

	this->m_Sync.Unlock();
	return FALSE;
}

BOOL CUnionInfo::SetUnionMaster(int iGuildNumber, char * szGuildName)
{
	this->m_Sync.Lock();
	this->m_iMasterGuild = iGuildNumber;
	memcpy(this->m_szMasterGuild, szGuildName, sizeof(this->m_szMasterGuild)-1);
	this->m_Sync.Unlock();

	return TRUE;
}

void CUnionInfo::Clear(int iRelationShipType)
{
	this->m_Sync.Lock();

	if ( iRelationShipType == 0 )
	{
		this->m_vtRivalMember.clear();
		this->m_vtUnionMember.clear();
	}
	else if ( iRelationShipType == 1 ) // Union
	{
		this->m_vtUnionMember.clear();
	}
	else if ( iRelationShipType == 2 ) // Rival
	{
		this->m_vtRivalMember.clear();
	}

	this->m_Sync.Unlock();
}
