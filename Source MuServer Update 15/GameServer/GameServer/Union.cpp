// Union.cpp: implementation of the CUnion class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Union.h"

CUnion gUnionManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnion::CUnion()
{

}

CUnion::~CUnion()
{

}

int CUnion::DelAllUnion()
{
	this->m_Sync.Lock();
	std::map<int, CUnionInfo*>::iterator _PosItor;

	for ( _PosItor = this->m_mpUnion.begin() ; _PosItor != this->m_mpUnion.end() ; ++_PosItor )
	{
		delete _PosItor->second;
	}

	this->m_mpUnion.clear();
	this->m_Sync.Unlock();
	return -1;
}

int CUnion::GetGuildRelationShip(int iGuildNumber, int iTargetGuildNumber)
{
	CUnionInfo * pUnionInfo = this->SearchUnion(iGuildNumber);

	if ( pUnionInfo == NULL )
	{
		return 0;
	}

	if ( pUnionInfo->IsUnionMember(iTargetGuildNumber) != FALSE )
	{
		return 1;
	}

	if ( pUnionInfo->IsRivalMember(iTargetGuildNumber) != FALSE )
	{
		return 2;
	}
	
	return 0;
}

int CUnion::GetGuildRelationShipCount(int iGuildNumber, int iRelationShipType)
{
	CUnionInfo * pUnionInfo = this->SearchUnion(iGuildNumber);

	if ( pUnionInfo == NULL )
	{
		return 0;
	}

	if ( iRelationShipType == 1 )
	{
		return pUnionInfo->GetUnionMemberCount();
	}
	else if ( iRelationShipType == 2 )
	{
		return pUnionInfo->GetRivalMemberCount();
	}
	return 0;
}

CUnionInfo * CUnion::SearchUnion(int iMasterGuildNumber)
{
	this->m_Sync.Lock();

	std::map<int, CUnionInfo *>::iterator _Itor = this->m_mpUnion.find(iMasterGuildNumber);

	if ( _Itor != this->m_mpUnion.end() )
	{
		this->m_Sync.Unlock();
		return (*(_Itor)).second;
	}

	this->m_Sync.Unlock();
	return NULL;
}

BOOL CUnion::AddUnion(int iMasterGuildNumber, char* szUnionMasterGuildName)
{
	CUnionInfo * pUnionInfo = this->SearchUnion(iMasterGuildNumber);

	if ( pUnionInfo == NULL )
	{
		this->m_Sync.Lock();
		pUnionInfo = new CUnionInfo;
		pUnionInfo->SetUnionMaster(iMasterGuildNumber, szUnionMasterGuildName);
		this->m_mpUnion[iMasterGuildNumber] = pUnionInfo;
		this->m_Sync.Unlock();
		return TRUE;
	}

	this->m_Sync.Lock();
	pUnionInfo->SetUnionMaster(iMasterGuildNumber, szUnionMasterGuildName);
	this->m_Sync.Unlock();
	return FALSE;
}

int CUnion::DelUnion(int iMasterGuildNumber)
{
	CUnionInfo * pUnionInfo = this->SearchUnion(iMasterGuildNumber);

	if ( pUnionInfo != NULL )
	{
		this->m_Sync.Lock();
		this->m_mpUnion.erase(iMasterGuildNumber);
		delete pUnionInfo;
		this->m_Sync.Unlock();
		return TRUE;
	}

	return -1;
}

int CUnion::SetGuildUnionMemberList(int iMasterGuildNumber, int iCount, int const * iList)
{
	CUnionInfo * pUnionInfo = this->SearchUnion(iMasterGuildNumber);

	if ( pUnionInfo != NULL )
	{
		pUnionInfo->Clear(1);
		this->m_Sync.Lock();

		for ( int i=0;i<iCount;i++)
		{
			pUnionInfo->m_vtUnionMember.push_back(iList[i]);
		}

		this->m_Sync.Unlock();
		return TRUE;
	}

	return -1;
}

int CUnion::SetGuildRivalMemberList(int iMasterGuildNumber, int iCount, int const * iList)
{
	CUnionInfo * pUnionInfo = this->SearchUnion(iMasterGuildNumber);

	if ( pUnionInfo != NULL )
	{
		pUnionInfo->Clear(2);
		this->m_Sync.Lock();

		for ( int i=0;i<iCount;i++)
		{
			pUnionInfo->m_vtRivalMember.push_back(iList[i]);
		}

		this->m_Sync.Unlock();
		return TRUE;
	}

	return -1;
}

int CUnion::GetGuildUnionMemberList(int iMasterGuildNumber, int & iCount, int * iList)
{
	CUnionInfo * pUnionInfo = this->SearchUnion(iMasterGuildNumber);

	if ( pUnionInfo != NULL )
	{
		this->m_Sync.Lock();

		std::vector<int>::iterator iB = pUnionInfo->m_vtUnionMember.begin();
		std::vector<int>::iterator iE = pUnionInfo->m_vtUnionMember.end();
		
		for ( ; iB != iE ; iB++ )
		{
			iList[iCount] = *iB;
			iCount++;
		}

		this->m_Sync.Unlock();

		if ( iCount > 0 &&  iCount < 100 )
		{
			return TRUE;
		}
	}

	return -1;
}

BOOL CUnion::GetUnionName(int iMasterGuildNumber, char* szMasterGuildName)
{
	CUnionInfo * pUnionInfo = this->SearchUnion(iMasterGuildNumber);

	if ( pUnionInfo != NULL )
	{
		this->m_Sync.Lock();
		memcpy(szMasterGuildName, pUnionInfo->m_szMasterGuild, sizeof(pUnionInfo->m_szMasterGuild)-1);
		this->m_Sync.Unlock();
		return TRUE;
	}

	return -1;
}
