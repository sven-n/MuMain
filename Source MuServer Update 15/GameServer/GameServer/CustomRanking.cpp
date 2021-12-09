#include "stdafx.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "Util.h"
#include "Message.h"
#include "User.h"
#include "Path.h"
#include "ServerInfo.h"
#include "MemScript.h"
#include "CustomRanking.h"
#include "Notice.h"

CCustomRanking gCustomRanking;

void CCustomRanking::Load(char* path)
{

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

	for(int n=0;n < MAX_RANK;n++)
	{
		this->r_Data[n];
	}

	this->m_count = 0;

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					int index = lpMemScript->GetNumber();

					strcpy_s(this->r_Data[index].Name,lpMemScript->GetAsString());

					strcpy_s(this->r_Data[index].Col1,lpMemScript->GetAsString());

					strcpy_s(this->r_Data[index].Col2,lpMemScript->GetAsString());

					this->m_count++;
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;

	//LogAdd(LOG_BLUE, "[ServerInfo] CustomRanking Info loaded successfully");
}

void CCustomRanking::GCReqRankingCount(int Index,PMSG_CUSTOM_RANKING_COUNT_RECV* lpMsg)
{

#if (GAMESERVER_CLIENTE_UPDATE >= 6)

	if(gServerInfo.m_CustomRankingSwitch == 0)
	{
		return;
	}

	PMSG_CUSTOM_RANKING_COUNT_SEND pMsg;

    pMsg.header.set(0xF3,0xE7, sizeof(pMsg)); 

	pMsg.count = (this->m_count > MAX_RANK) ? MAX_RANK : this->m_count;

	DataSend(Index, (LPBYTE)&pMsg, pMsg.header.size);

#endif

}

void CCustomRanking::GCReqRanking(int Index,PMSG_CUSTOM_RANKING_RECV* lpMsg)
{

#if (GAMESERVER_CLIENTE_UPDATE >= 6)

	if(gServerInfo.m_CustomRankingSwitch == 0)
	{
		return;
	}

	if(gObjIsConnected(Index) == false)
	{
		return;
	}

	if (this->m_count <= 0 )
	{
		return;
	}

	if (lpMsg->type < 0 || lpMsg->type >= this->m_count)
	{
		return;
	}

	if (this->m_count == 0)
	{
		return;
	}

	SDHP_CUSTOM_RANKING_SEND pMsg;

    pMsg.header.set(0xF4, sizeof(pMsg)); 

	pMsg.index = Index;

	pMsg.type = lpMsg->type;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

#endif
}

void CCustomRanking::GDCustomRankingRecv(SDHP_CUSTOM_RANKING_RECV* lpMsg)
{
#if (GAMESERVER_CLIENTE_UPDATE >= 6)

	if( lpMsg == NULL )
	{
		return;
	}

	BYTE send[4096];

	PMSG_CUSTOM_RANKING_SEND pMsg;

	pMsg.header.set(0xF3,0xE6,0);

	int size = sizeof(pMsg);

	memcpy(pMsg.rankname, this->r_Data[lpMsg->type].Name, sizeof(pMsg.rankname));
	memcpy(pMsg.col1, this->r_Data[lpMsg->type].Col1, sizeof(pMsg.col1));
	memcpy(pMsg.col2, this->r_Data[lpMsg->type].Col2, sizeof(pMsg.col2));

	pMsg.count = 0;

	CUSTOM_RANKING_DATA info;

	for(int n=0;n < lpMsg->count;n++)
	{
		CUSTOM_RANKING_DATA* lpInfo = (CUSTOM_RANKING_DATA*)(((BYTE*)lpMsg)+sizeof(SDHP_CUSTOM_RANKING_RECV)+(sizeof(CUSTOM_RANKING_DATA)*n));
		memcpy(info.szName, lpInfo->szName, sizeof(info.szName));
	    info.valor = lpInfo->valor;
		pMsg.count++;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(lpMsg->index,send,size);

#endif
}