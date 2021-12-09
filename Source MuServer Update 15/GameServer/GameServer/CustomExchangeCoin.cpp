// CustomMove.cpp: implementation of the CCustomMove class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "CustomExchangeCoin.h"
#include "DSProtocol.h"
#include "GensSystem.h"
#include "Log.h"
#include "Map.h"
#include "MapManager.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "Util.h"

CCustomExchangeCoin gCustomExchangeCoin;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomExchangeCoin::CCustomExchangeCoin() // OK
{
	this->m_CustomExchangeCoinInfo.clear();
}

CCustomExchangeCoin::~CCustomExchangeCoin() // OK
{

}

void CCustomExchangeCoin::Load(char* path) // OK
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

	this->m_CustomExchangeCoinInfo.clear();

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

			CUSTOMEXCHANGECOIN_INFO info;

			info.Index = lpMemScript->GetNumber();

			strcpy_s(info.Name,lpMemScript->GetAsString());

			info.RequireCoin1 = lpMemScript->GetAsNumber();

			info.RequireCoin2 = lpMemScript->GetAsNumber();

			info.RequireCoin3 = lpMemScript->GetAsNumber();

			info.RewardCoin1 = lpMemScript->GetAsNumber();

			info.RewardCoin2 = lpMemScript->GetAsNumber();

			info.RewardCoin3 = lpMemScript->GetAsNumber();

			this->m_CustomExchangeCoinInfo.insert(std::pair<int,CUSTOMEXCHANGECOIN_INFO>(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CCustomExchangeCoin::GetInfo(int index,CUSTOMEXCHANGECOIN_INFO* lpInfo) // OK
{
	std::map<int,CUSTOMEXCHANGECOIN_INFO>::iterator it = this->m_CustomExchangeCoinInfo.find(index);

	if(it == this->m_CustomExchangeCoinInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}
}

bool CCustomExchangeCoin::GetInfoByName(LPOBJ lpObj, char* message, int Npc) // OK
{
	char command[32] = {0};

	memset(command,0,sizeof(command));

	gCommandManager.GetString(message,command,sizeof(command),0);

	for(std::map<int,CUSTOMEXCHANGECOIN_INFO>::iterator it=this->m_CustomExchangeCoinInfo.begin();it != this->m_CustomExchangeCoinInfo.end();it++)
	{
		if(_stricmp(it->second.Name,command) == 0)
		{
			if (it->second.RequireCoin1 > 0 && it->second.RequireCoin1 > lpObj->Coin1)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(877), it->second.RequireCoin1);
				if (Npc >= 0)
				{
					GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(877), it->second.RequireCoin1);
				}
				return 1;
			}
			if (it->second.RequireCoin2 > 0 && it->second.RequireCoin2 > lpObj->Coin2)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(878), it->second.RequireCoin2);
				if (Npc >= 0)
				{
					GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(878), it->second.RequireCoin2);
				}
				return 1;
			}
			if (it->second.RequireCoin3 > 0 && it->second.RequireCoin3 > lpObj->Coin3)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(879), it->second.RequireCoin3);
				if (Npc >= 0)
				{
					GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(879), it->second.RequireCoin3);
				}
				return 1;
			}

			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(880));
			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(880));
			}

			int Coin1 = (((it->second.RewardCoin1 < 0)?0:it->second.RewardCoin1) - ((it->second.RequireCoin1 < 0)?0:it->second.RequireCoin1));
			int Coin2 = (((it->second.RewardCoin2 < 0)?0:it->second.RewardCoin2) - ((it->second.RequireCoin2 < 0)?0:it->second.RequireCoin2));
			int Coin3 = (((it->second.RewardCoin3 < 0)?0:it->second.RewardCoin3) - ((it->second.RequireCoin3 < 0)?0:it->second.RequireCoin3));

			GDSetCoinSend(lpObj->Index, Coin1, Coin2, Coin3,"ExchangeCoin");
			return 1;
		}
	}

	return 0;
}