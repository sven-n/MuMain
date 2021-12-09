// Fruit.cpp: implementation of the CFruit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Fruit.h"
#include "DefaultClassInfo.h"
#include "Message.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "Util.h"

CFruit gFruit;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFruit::CFruit() // OK
{
	this->Init();
}

CFruit::~CFruit() // OK
{

}

void CFruit::Init() // OK
{
	int NRMaxFruitPoint = 2;
	int MGMaxFruitPoint = 2;
	int DLMaxFruitPoint = 2;

	for(int n=0;n < MAX_CHARACTER_LEVEL;n++)
	{
		if(((n+1)%10) == 0)
		{
			NRMaxFruitPoint += (((n+11)*3)/400)+2;
			MGMaxFruitPoint += (((n+11)*3)/700)+2;
			DLMaxFruitPoint += (((n+11)*3)/500)+2;
		}

		this->m_NRMaxFruitPointTable[n] = NRMaxFruitPoint;
		this->m_MGMaxFruitPointTable[n] = MGMaxFruitPoint;
		this->m_DLMaxFruitPointTable[n] = DLMaxFruitPoint;
	}
}

int CFruit::GetMaxFruitPoint(LPOBJ lpObj) // OK
{
	if(lpObj->Level < 1 || lpObj->Level > MAX_CHARACTER_LEVEL)
	{
		return 0;
	}

	if(lpObj->Class == CLASS_MG)
	{
		return this->m_MGMaxFruitPointTable[lpObj->Level-1];
	}

	if(lpObj->Class == CLASS_DL)
	{
		return this->m_DLMaxFruitPointTable[lpObj->Level-1];
	}

	return this->m_NRMaxFruitPointTable[lpObj->Level-1];
}

bool CFruit::UseFruitAddPoint(LPOBJ lpObj,int type) // OK
{
	PMSG_FRUIT_RESULT_SEND pMsg;

	pMsg.header.set(0x2C,sizeof(pMsg));

	if(lpObj->Level < 10)
	{
		pMsg.result = 2;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return 0;
	}

	if(type == 4 && lpObj->Class != CLASS_DL)
	{
		pMsg.result = 2;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return 0;
	}

	for(int n=0;n < INVENTORY_WEAR_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0)
		{
			pMsg.result = 16;
			DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
			return 0;
		}
	}

	if(lpObj->FruitAddPoint >= this->GetMaxFruitPoint(lpObj))
	{
		pMsg.result = 37;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return 0;
	}

	int* stat;

	switch(type)
	{
		case 0:
			stat = &lpObj->Strength;
			break;
		case 1:
			stat = &lpObj->Dexterity;
			break;
		case 2:
			stat = &lpObj->Vitality;
			break;
		case 3:
			stat = &lpObj->Energy;
			break;
		case 4:
			stat = &lpObj->Leadership;
			break;
		default:
			pMsg.result = 33;
			DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
			return 0;
	}

	if((*stat) >= gServerInfo.m_MaxStatPoint[lpObj->AccountLevel])
	{
		pMsg.result = 33;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return 0;
	}

	int rate = 0;

	if(lpObj->FruitAddPoint <= 10)
	{
		rate = 100;
	}
	else if((lpObj->FruitAddPoint-10) < ((this->GetMaxFruitPoint(lpObj)*10)/100))
	{
		rate = 70;
	}
	else if((lpObj->FruitAddPoint-10) < ((this->GetMaxFruitPoint(lpObj)*30)/100))
	{
		rate = 60;
	}
	else if((lpObj->FruitAddPoint-10) < ((this->GetMaxFruitPoint(lpObj)*50)/100))
	{
		rate = 50;
	}
	else if((lpObj->FruitAddPoint-10) < ((this->GetMaxFruitPoint(lpObj)*80)/100))
	{
		rate = 40;
	}
	else
	{
		rate = 30;
	}

	if((GetLargeRand()%100) < ((gServerInfo.m_FruitAddPointSuccessRate[lpObj->AccountLevel]==-1)?rate:gServerInfo.m_FruitAddPointSuccessRate[lpObj->AccountLevel]))
	{
		int amount = gServerInfo.m_FruitAddPointMin+(GetLargeRand()%((gServerInfo.m_FruitAddPointMax-gServerInfo.m_FruitAddPointMin)+1));

		amount = (((lpObj->FruitAddPoint+amount)>this->GetMaxFruitPoint(lpObj))?(this->GetMaxFruitPoint(lpObj)-lpObj->FruitAddPoint):amount);

		amount = ((((*stat)+amount)>gServerInfo.m_MaxStatPoint[lpObj->AccountLevel])?(gServerInfo.m_MaxStatPoint[lpObj->AccountLevel]-(*stat)):amount);

		(*stat) += amount;

		lpObj->FruitAddPoint += amount;

		pMsg.result = 0;

		pMsg.value = amount;

		pMsg.type = ((type<4)?(3-type):type);

		#if(GAMESERVER_EXTRA==1)
		pMsg.ViewValue = amount;
		pMsg.ViewPoint = lpObj->LevelUpPoint;
		pMsg.ViewStrength = lpObj->Strength;
		pMsg.ViewDexterity = lpObj->Dexterity;
		pMsg.ViewVitality = lpObj->Vitality;
		pMsg.ViewEnergy = lpObj->Energy;
		pMsg.ViewLeadership = lpObj->Leadership;
		#endif

		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

		gObjectManager.CharacterCalcAttribute(lpObj->Index);
	}
	else
	{
		pMsg.result = 1;

		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
	}

	return 1;
}

bool CFruit::UseFruitSubPoint(LPOBJ lpObj,int type) // OK
{
	PMSG_FRUIT_RESULT_SEND pMsg;

	pMsg.header.set(0x2C,sizeof(pMsg));

	if(lpObj->Level < 10)
	{
		pMsg.result = 5;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return 0;
	}

	if(type == 4 && lpObj->Class != CLASS_DL)
	{
		pMsg.result = 5;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return 0;
	}

	for(int n=0;n < INVENTORY_WEAR_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0)
		{
			pMsg.result = 16;
			DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
			return 0;
		}
	}

	if(lpObj->FruitSubPoint >= this->GetMaxFruitPoint(lpObj))
	{
		pMsg.result = 37;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return 0;
	}

	int* stat;

	switch(type)
	{
		case 0:
			stat = &lpObj->Strength;
			break;
		case 1:
			stat = &lpObj->Dexterity;
			break;
		case 2:
			stat = &lpObj->Vitality;
			break;
		case 3:
			stat = &lpObj->Energy;
			break;
		case 4:
			stat = &lpObj->Leadership;
			break;
		default:
			pMsg.result = 38;
			DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
			return 0;
	}

	if((*stat) <= gDefaultClassInfo.GetCharacterDefaultStat(lpObj->Class,type))
	{
		pMsg.result = 38;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return 0;
	}

	int rate = 0;

	if(lpObj->FruitSubPoint <= 10)
	{
		rate = 100;
	}
	else if((lpObj->FruitSubPoint-10) < ((this->GetMaxFruitPoint(lpObj)*10)/100))
	{
		rate = 70;
	}
	else if((lpObj->FruitSubPoint-10) < ((this->GetMaxFruitPoint(lpObj)*30)/100))
	{
		rate = 60;
	}
	else if((lpObj->FruitSubPoint-10) < ((this->GetMaxFruitPoint(lpObj)*50)/100))
	{
		rate = 50;
	}
	else if((lpObj->FruitSubPoint-10) < ((this->GetMaxFruitPoint(lpObj)*80)/100))
	{
		rate = 40;
	}
	else
	{
		rate = 30;
	}

	if((GetLargeRand()%100) < ((gServerInfo.m_FruitSubPointSuccessRate[lpObj->AccountLevel]==-1)?rate:gServerInfo.m_FruitSubPointSuccessRate[lpObj->AccountLevel]))
	{
		int amount = gServerInfo.m_FruitSubPointMin+(GetLargeRand()%((gServerInfo.m_FruitSubPointMax-gServerInfo.m_FruitSubPointMin)+1));

		amount = (((lpObj->FruitSubPoint+amount)>this->GetMaxFruitPoint(lpObj))?(this->GetMaxFruitPoint(lpObj)-lpObj->FruitSubPoint):amount);

		amount = ((((*stat)-amount)<gDefaultClassInfo.GetCharacterDefaultStat(lpObj->Class,type))?((*stat)-gDefaultClassInfo.GetCharacterDefaultStat(lpObj->Class,type)):amount);

		(*stat) -= amount;

		lpObj->LevelUpPoint += amount;

		lpObj->FruitSubPoint += amount;

		pMsg.result = 3;

		pMsg.value = amount;

		pMsg.type = ((type<4)?(3-type):type);

		#if(GAMESERVER_EXTRA==1)
		pMsg.ViewValue = amount;
		pMsg.ViewPoint = lpObj->LevelUpPoint;
		pMsg.ViewStrength = lpObj->Strength;
		pMsg.ViewDexterity = lpObj->Dexterity;
		pMsg.ViewVitality = lpObj->Vitality;
		pMsg.ViewEnergy = lpObj->Energy;
		pMsg.ViewLeadership = lpObj->Leadership;
		#endif

		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

		gObjectManager.CharacterCalcAttribute(lpObj->Index);
	}
	else
	{
		pMsg.result = 4;

		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
	}

	return 1;
}

bool CFruit::UseFruitResetPoint(LPOBJ lpObj,int type,int amount) // OK
{
	PMSG_FRUIT_RESULT_SEND pMsg;

	pMsg.header.set(0x2C,sizeof(pMsg));

	if(lpObj->Level < 10)
	{
		pMsg.result = 5;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return 0;
	}

	if(type == 4 && lpObj->Class != CLASS_DL)
	{
		pMsg.result = 5;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return 0;
	}

	for(int n=0;n < INVENTORY_WEAR_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0)
		{
			pMsg.result = 16;
			DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
			return 0;
		}
	}

	int* stat;

	switch(type)
	{
		case 0:
			stat = &lpObj->Strength;
			break;
		case 1:
			stat = &lpObj->Dexterity;
			break;
		case 2:
			stat = &lpObj->Vitality;
			break;
		case 3:
			stat = &lpObj->Energy;
			break;
		case 4:
			stat = &lpObj->Leadership;
			break;
		default:
			pMsg.result = 38;
			DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
			return 0;
	}

	if((*stat) <= gDefaultClassInfo.GetCharacterDefaultStat(lpObj->Class,type))
	{
		pMsg.result = 38;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return 0;
	}

	amount = ((((*stat)-amount)<gDefaultClassInfo.GetCharacterDefaultStat(lpObj->Class,type))?((*stat)-gDefaultClassInfo.GetCharacterDefaultStat(lpObj->Class,type)):amount);

	(*stat) -= amount;

	lpObj->LevelUpPoint += amount;

	pMsg.result = 6;

	pMsg.value = amount;

	pMsg.type = ((type<4)?(3-type):type);

	#if(GAMESERVER_EXTRA==1)
	pMsg.ViewValue = amount;
	pMsg.ViewPoint = lpObj->LevelUpPoint;
	pMsg.ViewStrength = lpObj->Strength;
	pMsg.ViewDexterity = lpObj->Dexterity;
	pMsg.ViewVitality = lpObj->Vitality;
	pMsg.ViewEnergy = lpObj->Energy;
	pMsg.ViewLeadership = lpObj->Leadership;
	#endif

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	gObjectManager.CharacterCalcAttribute(lpObj->Index);
	return 1;
}

void CFruit::GCFruitResultSend(LPOBJ lpObj,int result,int value,int type) // OK
{
	PMSG_FRUIT_RESULT_SEND pMsg;

	pMsg.header.set(0x2C,sizeof(pMsg));

	pMsg.result = result;

	pMsg.value = value;

	pMsg.type = type;

	#if(GAMESERVER_EXTRA==1)
	pMsg.ViewValue = value;
	pMsg.ViewPoint = lpObj->LevelUpPoint;
	pMsg.ViewStrength = lpObj->Strength;
	pMsg.ViewDexterity = lpObj->Dexterity;
	pMsg.ViewVitality = lpObj->Vitality;
	pMsg.ViewEnergy = lpObj->Energy;
	pMsg.ViewLeadership = lpObj->Leadership;
	#endif

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
}
