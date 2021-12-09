// UnionInfo.h: interface for the CUnionInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_UNION_GUILD 100
#define MAX_UNION_MEMBER 5

class TSync
{
public:
	TSync()
	{
		this->m_nLock=0;
		InitializeCriticalSection(&this->m_cs);
	}

	~TSync()
	{
		DeleteCriticalSection(&this->m_cs);
	}

	void Lock()
	{
		EnterCriticalSection(&this->m_cs);
		InterlockedIncrement(&this->m_nLock);
	}

	void Unlock()
	{
		long nResult=InterlockedDecrement(&this->m_nLock );
		LeaveCriticalSection(&this->m_cs );
	}

private:
	CRITICAL_SECTION m_cs;	// 0
	LONG m_nLock;	// 18
};

class CUnionInfo
{
public:
	CUnionInfo();
	virtual ~CUnionInfo();
	int IsUnionMember(int iGuildNumber);
	int IsRivalMember(int iGuildNumber);
	int SetUnionMaster(int iGuildNumber, char* szGuildName);
	int GetUnionMemberCount();
	int GetRivalMemberCount();
	void Clear(int iRelationShipType);
public:
	int m_iUnionTimeStamp; // 4
	int m_iRivalTimeStamp; // 8
	int m_iMasterGuild; // C
	char m_szMasterGuild[9]; // 10
	std::vector<int> m_vtUnionMember; // 1C
	std::vector<int> m_vtRivalMember; // 2C
	TSync m_Sync; // 3C
};
