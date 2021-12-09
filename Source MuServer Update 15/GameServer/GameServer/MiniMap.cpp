// MiniMap.cpp: implementation of the CMiniMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MiniMap.h"
#include "MemScript.h"
#include "Party.h"
#include "Util.h"

CMiniMap gMiniMap;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMiniMap::CMiniMap() // OK
{
	#if(GAMESERVER_UPDATE>=802)

	this->m_MiniMapInfo.clear();

	#endif
}

CMiniMap::~CMiniMap() // OK
{

}

void CMiniMap::Load(char* path) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->m_MiniMapInfo.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			MINI_MAP_INFO info;

			memset(&info,0,sizeof(info));

			info.Index = lpMemScript->GetNumber();

			info.Group = lpMemScript->GetAsNumber();

			info.Type = lpMemScript->GetAsNumber();

			info.X = lpMemScript->GetAsNumber();

			info.Y = lpMemScript->GetAsNumber();

			strcpy_s(info.Text,lpMemScript->GetAsString());

			this->m_MiniMapInfo.push_back(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;

	#endif
}

void CMiniMap::CGMiniMapStartPartyInfoRecv(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	lpObj->MiniMapState = 1;

	if(lpObj->MiniMapValue != lpObj->Map)
	{
		this->GCMiniMapInfoSend(aIndex);

		lpObj->MiniMapValue = lpObj->Map;
	}

	#endif
}

void CMiniMap::CGMiniMapClosePartyInfoRecv(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	lpObj->MiniMapState = 0;

	#endif
}

void CMiniMap::CGMiniMapInfoRecv(PMSG_MINI_MAP_INFO_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	#endif
}

void CMiniMap::GCMiniMapPartyInfoSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if(gObj[aIndex].MiniMapState == 0)
	{
		return;
	}

	int index = gObj[aIndex].PartyNumber;

	if(index == -1)
	{
		return;
	}

	BYTE send[256];

	PMSG_MINI_MAP_PARTY_INFO_SEND pMsg;

	pMsg.header.set(0xE7,0x01,0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	PMSG_MINI_MAP_PARTY_INFO info;

	for(int n=0;n < MAX_PARTY_USER;n++)
	{
		if(OBJECT_RANGE(gParty.m_PartyInfo[index].Index[n]) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[gParty.m_PartyInfo[index].Index[n]];

		memcpy(info.name,lpObj->Name,sizeof(info.name));

		info.map = lpObj->Map;

		info.x = (BYTE)lpObj->X;

		info.y = (BYTE)lpObj->Y;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	pMsg.header.size = size;

	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(aIndex,send,size);

	#endif
}

void CMiniMap::GCMiniMapInfoSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	int count = 0;

	for(std::vector<MINI_MAP_INFO>::iterator it=this->m_MiniMapInfo.begin();it != this->m_MiniMapInfo.end();it++)
	{
		if(it->Index != gObj[aIndex].Map)
		{
			continue;
		}

		PMSG_MINI_MAP_INFO_SEND pMsg;

		pMsg.header.set(0xE7,0x03,sizeof(pMsg));

		pMsg.index = count++;

		pMsg.group = it->Group;

		pMsg.type = it->Type;

		pMsg.flag = 0;

		pMsg.x = it->X;

		pMsg.y = it->Y;

		memcpy(pMsg.text,it->Text,sizeof(pMsg.text));

		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
	}

	#endif
}
