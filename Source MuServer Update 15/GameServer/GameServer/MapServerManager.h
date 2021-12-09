// MapServerManager.h: interface for the CMapServerManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Map.h"

#define MAX_MAP_GROUPS 20
#define MAX_MAP_SUBGROUPS 20
#define MAX_SVR_PER_GROUP 400

class CMapServerData
{
public:
	CMapServerData() // OK
	{
		this->Clear(1);
	}

	virtual ~CMapServerData() // OK
	{

	}

	void Clear(int value) // OK
	{
		this->m_used = 0;
		this->m_MapServerGroup = -1;
		this->m_ServerCode = -1;

		memset(this->m_IpAddr,0,sizeof(this->m_IpAddr));

		this->m_port = 0;

		for(int n=0;n < MAX_MAP;n++)
		{
			switch(value)
			{
				case -1:
					this->m_MapMove[n] = -2;
					break;
				case 0:
					this->m_MapMove[n] = -1;
					break;
				default:
					this->m_MapMove[n] = -3;
					break;
			}
		}
	}
public:
	int m_used;
	BYTE m_MapServerGroup;
	short m_ServerCode;
	char m_IpAddr[16];
	WORD m_port;
	short m_MapMove[MAX_MAP];
};

class CMapServerManager
{
public:
	CMapServerManager();
	virtual ~CMapServerManager();
	void Clear();
	void Load(char* path);
	int GetMapServerGroup();
	bool GetMapServerData(WORD ServerCode,char* IpAddress,WORD* ServerPort);
	bool CheckMapServer(int map);
	short CheckMapServerMove(int aIndex,int map,short ServerCode);
public:
	bool m_MapDataLoadOk;
	CMapServerData m_MapServer[MAX_MAP_GROUPS][MAX_MAP_SUBGROUPS];
	int m_MapServerGroup[MAX_MAP_GROUPS];
	std::map<int,CMapServerData*> m_MapServerCode;
	CRITICAL_SECTION m_critical;
	CMapServerData* m_MapServerData;
};

extern CMapServerManager gMapServerManager;
