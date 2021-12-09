// CustomQuest.cpp: implementation of the CCustomQuest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomStartItem.h"
#include "DSProtocol.h"
#include "EffectManager.h"
#include "GameMain.h"
#include "ItemManager.h"
#include "Log.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "ServerInfo.h"
#include "SocketItemType.h"
#include "Util.h"


CCustomStartItem gCustomStartItem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomStartItem::CCustomStartItem() // OK
{
	this->Init();
}

CCustomStartItem::~CCustomStartItem() // OK
{

}

void CCustomStartItem::Init() // OK
{
	this->m_CustomStartItemItemInfo.clear();
	this->m_CustomStartItemBuffInfo.clear();
	this->m_CustomStartItemCoinInfo.clear();
}

void CCustomStartItem::Load(char* path) // OK
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

	this->Init();

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

					STARTITEM_ITEM_INFO info;

					info.Class = lpMemScript->GetNumber();

					info.ItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetAsNumber(),lpMemScript->GetAsNumber()));

					info.ItemLevel = lpMemScript->GetAsNumber();

					info.ItemDurability = lpMemScript->GetAsNumber();

					info.ItemOption1 = lpMemScript->GetAsNumber();

					info.ItemOption2 = lpMemScript->GetAsNumber();

					info.ItemOption3 = lpMemScript->GetAsNumber();

					info.ItemNewOption = lpMemScript->GetAsNumber();

					info.AncOption = lpMemScript->GetAsNumber();

					info.JOH	= lpMemScript->GetAsNumber();

					info.OpEx = lpMemScript->GetAsNumber();

					info.Socket1 = lpMemScript->GetAsNumber();

					info.Socket2 = lpMemScript->GetAsNumber();

					info.Socket3 = lpMemScript->GetAsNumber();

					info.Socket4 = lpMemScript->GetAsNumber();

					info.Socket5 = lpMemScript->GetAsNumber();

					info.Duration = lpMemScript->GetAsNumber();

					this->m_CustomStartItemItemInfo.push_back(info);
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					STARTITEM_BUFF_INFO info;

					info.Class = lpMemScript->GetNumber();

					info.EffectID = lpMemScript->GetAsNumber();

					info.Power1 = lpMemScript->GetAsNumber();

					info.Power2 = lpMemScript->GetAsNumber();

					info.Time = lpMemScript->GetAsNumber();

					this->m_CustomStartItemBuffInfo.push_back(info);
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					STARTITEM_COIN_INFO info;

					info.Class = lpMemScript->GetNumber();

					info.Coin1 = lpMemScript->GetAsNumber();

					info.Coin2 = lpMemScript->GetAsNumber();

					info.Coin3 = lpMemScript->GetAsNumber();

					this->m_CustomStartItemCoinInfo.push_back(info);
				}
				else
				{
					break;
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CCustomStartItem::SetStartItem(LPOBJ lpObj) // OK
{
	this->InsertItem(lpObj);
	this->InsertBuff(lpObj);
	//this->InsertCoin(lpObj);
	this->DGStartItemSaveSend(lpObj);
}

void CCustomStartItem::InsertItem(LPOBJ lpObj) // OK
{

	for(int n=0;n < INVENTORY_WEAR_SIZE;n++)
	{
		for(std::vector<STARTITEM_ITEM_INFO>::iterator it=this->m_CustomStartItemItemInfo.begin();it != this->m_CustomStartItemItemInfo.end();it++)
		{
			if (it->Class != lpObj->Class)
			{
				continue;
			}
			if (it->Slot != n)
			{
				continue;
			}


			BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

			if (gSocketItemType.CheckSocketItemType(it->ItemIndex) == 1)
			{
				int qtd = gSocketItemType.GetSocketItemMaxSocket(it->ItemIndex);

				ItemSocketOption[0] = (BYTE)((qtd > 0)?((it->Socket1 != 255)?it->Socket1:255):255);
				ItemSocketOption[1] = (BYTE)((qtd > 1)?((it->Socket2 != 255)?it->Socket2:255):255);
				ItemSocketOption[2] = (BYTE)((qtd > 2)?((it->Socket3 != 255)?it->Socket3:255):255);
				ItemSocketOption[3] = (BYTE)((qtd > 3)?((it->Socket4 != 255)?it->Socket4:255):255);
				ItemSocketOption[4] = (BYTE)((qtd > 4)?((it->Socket5 != 255)?it->Socket5:255):255);
				//this->m_Item[slot].m_SocketOptionBonus = gSocketItemOption.GetSocketItemBonusOption(this->m_Item);
			}

			#if(GAMESERVER_UPDATE>=603)
					GDCreateItemSend(lpObj->Index,0xEB,0,0,it->ItemIndex,it->ItemLevel,it->ItemDurability,it->ItemOption1,it->ItemOption2,it->ItemOption3,-1,it->ItemNewOption,it->AncOption,it->JOH,it->OpEx,ItemSocketOption,0xFF,((it->Duration>0)?((DWORD)time(0)+it->Duration):0));
			#else
					GDCreateItemSend(lpObj->Index,0xEB,0,0,it->ItemIndex,it->ItemLevel,it->ItemDurability,it->ItemOption1,it->ItemOption2,it->ItemOption3,-1,it->ItemNewOption,it->AncOption,it->JOH,it->OpEx,ItemSocketOption,0xFF,0);
			#endif
		}		
	}
}

void CCustomStartItem::InsertBuff(LPOBJ lpObj) // OK
{
	for(std::vector<STARTITEM_BUFF_INFO>::iterator it=this->m_CustomStartItemBuffInfo.begin();it != this->m_CustomStartItemBuffInfo.end();it++)
	{
		if (it->Class != lpObj->Class)
		{
			continue;
		}

		EFFECT_INFO* lpInfo = gEffectManager.GetInfo(it->EffectID);

		if(lpInfo == 0)
		{
			continue;
		}

		gEffectManager.AddEffect(lpObj,1,it->EffectID,(lpInfo->Type==2)?(int)(time(0)+(it->Time*60)):it->Time*60,it->Power1,it->Power2,0,0);

	}
}

void CCustomStartItem::InsertCoin(LPOBJ lpObj) // OK
{
	for(std::vector<STARTITEM_COIN_INFO>::iterator it=this->m_CustomStartItemCoinInfo.begin();it != this->m_CustomStartItemCoinInfo.end();it++)
	{
		if (it->Class != lpObj->Class)
		{
			continue;
		}

		GDSetCoinSend(lpObj->Index, (it->Coin1 < 0)?0:it->Coin1, (it->Coin2 < 0)?0:it->Coin2, (it->Coin3 < 0)?0:it->Coin3,"StartItem");
	}
}

void CCustomStartItem::DGStartItemSaveSend(LPOBJ lpObj) // OK
{
	SDHP_STARTITEM_SAVE_SEND pMsg;

	pMsg.header.set(0xF7,0x03,sizeof(pMsg));

	pMsg.index = lpObj->Index;

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg)); 
}