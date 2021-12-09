// CustomItem.cpp: implementation of the CCustomItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CashShop.h"
#include "CustomBuyVip.h"
#include "DSProtocol.h"
#include "JSProtocol.h"
#include "Util.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "Util.h"

CCustomBuyVip gCustomBuyVip;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomBuyVip::CCustomBuyVip() // OK
{
	this->Init();
}

CCustomBuyVip::~CCustomBuyVip() // OK
{

}

void CCustomBuyVip::Init() // OK
{
	for(int n=0;n < MAX_CUSTOM_BUYVIP;n++)
	{
		this->m_CustomBuyVipInfo[n].Index = -1;
	}
}

void CCustomBuyVip::Load(char* path) // OK
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

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			CUSTOM_BUYVIP_INFO info;

			memset(&info,0,sizeof(info));

			info.Index = lpMemScript->GetNumber();

			info.Exp = lpMemScript->GetAsNumber();

			info.Drop = lpMemScript->GetAsNumber();

			info.Days = lpMemScript->GetAsNumber();

			info.Coin1 = lpMemScript->GetAsNumber();

			info.Coin2 = lpMemScript->GetAsNumber();

			info.Coin3 = lpMemScript->GetAsNumber();

			strcpy_s(info.VipName,lpMemScript->GetAsString());

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		printf(lpMemScript->GetLastError());
	}

	delete lpMemScript;

	//LogAdd(LOG_BLUE, "[ServerInfo] CustomBuyVip Info loaded successfully");
}

void CCustomBuyVip::SetInfo(CUSTOM_BUYVIP_INFO info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_CUSTOM_BUYVIP)
	{
		return;
	}

	this->m_CustomBuyVipInfo[info.Index] = info;
}

CUSTOM_BUYVIP_INFO* CCustomBuyVip::GetInfo(int index) // OK
{
	if(index < 0 || index >= MAX_CUSTOM_BUYVIP)
	{
		return 0;
	}

	if(this->m_CustomBuyVipInfo[index].Index != index)
	{
		return 0;
	}

	return &this->m_CustomBuyVipInfo[index];
}

void CCustomBuyVip::BuyVip(int aIndex,BUYPREMIUM_REQ* lpMsg)
{
#if (GAMESERVER_CLIENTE_UPDATE >= 6)
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if (lpMsg->VipType < 1 || lpMsg->VipType > 3)
	{
		return;
	}

	if (lpMsg->VipType < lpObj->AccountLevel)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(759));
		return;
	}

	lpObj->BuyVip = lpMsg->VipType;
	gCashShop.CGCashShopPointRecv(lpObj->Index);
#endif
}

void CCustomBuyVip::BuyVipDone(LPOBJ lpObj)
{
#if (GAMESERVER_CLIENTE_UPDATE >= 6)

	if(gObjIsConnectedGP(lpObj->Index) == 0)
	{
		return;
	}

	if (lpObj->BuyVip < 1 || lpObj->BuyVip > 3)
	{
		return;
	}

	if (lpObj->BuyVip < lpObj->AccountLevel)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(759));
		return;
	}

	CUSTOM_BUYVIP_INFO* VipInfo = this->GetInfo(lpObj->BuyVip-1);

	if (lpObj->Coin1 < VipInfo->Coin1 || lpObj->Coin2 < VipInfo->Coin2 || lpObj->Coin3 < VipInfo->Coin3)
	{
		lpObj->BuyVip = 0;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(757));
		return;
	}

	LogAdd(LOG_EVENT,"[%s][%s] BuyVip (Level: %d)",lpObj->Account,lpObj->Name,lpObj->BuyVip);

	GDSetCoinSend(lpObj->Index, -(VipInfo->Coin1), -(VipInfo->Coin2), -(VipInfo->Coin3),"BuyVip");

	GJAccountLevelSaveSend(lpObj->Index,lpObj->BuyVip,VipInfo->Days*86400);
	GJAccountLevelSend(lpObj->Index);

	gCashShop.CGCashShopPointRecv(lpObj->Index);

	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(758),VipInfo->VipName);

	lpObj->BuyVip = 0;

#endif
}
