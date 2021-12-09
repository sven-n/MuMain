// Move.cpp: implementation of the CMove class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Move.h"
#include "CastleSiege.h"
#include "CustomArena.h"
#include "DefaultClassInfo.h"
#include "DoubleGoer.h"
#include "EffectManager.h"
#include "Gate.h"
#include "GensSystem.h"
#include "IllusionTemple.h"
#include "ImperialGuardian.h"
#include "ItemManager.h"
#include "Map.h"
#include "MapManager.h"
#include "MemScript.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "SkillManager.h"
#include "Util.h"

CMove gMove;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMove::CMove() // OK
{
	this->m_MoveInfo.clear();
}

CMove::~CMove() // OK
{

}

void CMove::Load(char* path) // OK
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

	this->m_MoveInfo.clear();

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

			MOVE_INFO info;

			info.Index = lpMemScript->GetNumber();

			strcpy_s(info.Name,lpMemScript->GetAsString());

			info.Money = lpMemScript->GetAsNumber();

			info.MinLevel = lpMemScript->GetAsNumber();

			info.MaxLevel = lpMemScript->GetAsNumber();

			info.MinReset = lpMemScript->GetAsNumber();

			info.MaxReset = lpMemScript->GetAsNumber();

			info.AccountLevel = lpMemScript->GetAsNumber();

			info.Gate = lpMemScript->GetAsNumber();

			this->m_MoveInfo.insert(std::pair<int,MOVE_INFO>(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CMove::GetInfo(int index,MOVE_INFO* lpInfo) // OK
{
	std::map<int,MOVE_INFO>::iterator it = this->m_MoveInfo.find(index);

	if(it == this->m_MoveInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}
}

bool CMove::GetInfoByName(char* name,MOVE_INFO* lpInfo) // OK
{
	for(std::map<int,MOVE_INFO>::iterator it=this->m_MoveInfo.begin();it != this->m_MoveInfo.end();it++)
	{
		if(_stricmp(it->second.Name,name) == 0)
		{
			(*lpInfo) = it->second;
			return 1;
		}
	}

	return 0;
}

void CMove::Move(LPOBJ lpObj,int index) // OK
{
	MOVE_INFO MoveInfo;

	if(this->GetInfo(index,&MoveInfo) == 0)
	{
		return;
	}

	if(MoveInfo.MinLevel != -1 && lpObj->Level < gGate.GetMoveLevel(lpObj,gGate.GetGateMap(MoveInfo.Gate),MoveInfo.MinLevel))
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(224),gGate.GetMoveLevel(lpObj,gGate.GetGateMap(MoveInfo.Gate),MoveInfo.MinLevel));
		return;
	}

	if(MoveInfo.MaxLevel != -1 && lpObj->Level > MoveInfo.MaxLevel)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(230),MoveInfo.MaxLevel);
		return;
	}

	if(MoveInfo.MinReset != -1 && lpObj->Reset < MoveInfo.MinReset)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(231),MoveInfo.MinReset);
		return;
	}

	if(MoveInfo.MaxReset != -1 && lpObj->Reset > MoveInfo.MaxReset)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(232),MoveInfo.MaxReset);
		return;
	}

	if(lpObj->Money < ((DWORD)MoveInfo.Money))
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(225),MoveInfo.Money);
		return;
	}

	if(lpObj->AccountLevel < MoveInfo.AccountLevel)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(226));
		return;
	}

	if(gServerInfo.m_PKLimitFree == 0 && lpObj->PKLevel >= 5)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(227));
		return;
	}

	if(lpObj->Interface.use != 0 || lpObj->Teleport != 0 || lpObj->DieRegen != 0 || lpObj->PShopOpen != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(226));
		return;
	}

	if(gGate.GetGateMap(MoveInfo.Gate) == MAP_ATLANS && (lpObj->Inventory[8].IsItem() != 0 && (lpObj->Inventory[8].m_Index == GET_ITEM(13,2) || lpObj->Inventory[8].m_Index == GET_ITEM(13,3)))) // Uniria,Dinorant
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(274));
		return;
	}

	if((gGate.GetGateMap(MoveInfo.Gate) == MAP_ICARUS || gGate.GetGateMap(MoveInfo.Gate) == MAP_KANTURU3) && (lpObj->Inventory[7].IsItem() == 0 && lpObj->Inventory[8].m_Index != GET_ITEM(13,3) && lpObj->Inventory[8].m_Index != GET_ITEM(13,37))) // Dinorant,Fenrir
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(228));
		return;
	}

	if(lpObj->GensFamily == GENS_FAMILY_NONE && gMapManager.GetMapGensBattle(gGate.GetGateMap(MoveInfo.Gate)) != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(229));
		return;
	}

	if(CA_MAP_RANGE(gGate.GetGateMap(MoveInfo.Gate)) == 0 || gCustomArena.CheckEnterEnabled(lpObj,MoveInfo.Gate) != 0)
	{
		if(gObjMoveGate(lpObj->Index,MoveInfo.Gate) != 0)
		{
			lpObj->PShopRedrawAbs = 1;
			lpObj->Money -= MoveInfo.Money;
			GCMoneySend(lpObj->Index,lpObj->Money);
		}
	}
}

void CMove::CGTeleportRecv(PMSG_TELEPORT_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(gEffectManager.CheckStunEffect(lpObj) != 0)
	{
		return;
	}

	if(lpMsg->gate == 0)
	{
		if(gObjCheckTeleportArea(aIndex,lpMsg->x,lpMsg->y) == 0)
		{
			this->GCTeleportSend(lpObj->Index,0,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y,lpObj->Dir);
			return;
		}

		#if(GAMESERVER_TYPE==1)

		if(lpObj->Map == MAP_CASTLE_SIEGE && gCastleSiege.CheckTeleportMagicAxisY(lpObj->Y,lpMsg->x,lpMsg->y) == 0)
		{
			lpMsg->y = (BYTE)lpObj->Y;
		}

		#endif

		CSkill* lpSkill = gSkillManager.GetSkill(lpObj,SKILL_TELEPORT);

		if(lpSkill != 0)
		{
			if(IT_MAP_RANGE(lpObj->Map) != 0 && gIllusionTemple.GetState(GET_IT_LEVEL(lpObj->Map)) != IT_STATE_START)
			{
				lpMsg->x = (BYTE)lpObj->X;
				lpMsg->y = (BYTE)lpObj->Y;
			}

			#if(GAMESERVER_UPDATE>=501)

			if(DG_MAP_RANGE(lpObj->Map) != 0 && gDoubleGoer.GetState() != DG_STATE_START)
			{
				lpMsg->x = (BYTE)lpObj->X;
				lpMsg->y = (BYTE)lpObj->Y;
			}

			#endif

			if(gSkillManager.CheckSkillMana(lpObj,lpSkill->m_index) == 0 || gSkillManager.CheckSkillBP(lpObj,lpSkill->m_index) == 0)
			{
				return;
			}

			gSkillManager.GCSkillAttackSend(lpObj,lpSkill->m_index,aIndex,1);

			gObjTeleportMagicUse(aIndex,lpMsg->x,lpMsg->y);

			lpObj->Mana -= (gSkillManager.GetSkillMana(lpSkill->m_index)*lpObj->MPConsumptionRate)/100;

			lpObj->BP -= (gSkillManager.GetSkillBP(lpSkill->m_index)*lpObj->BPConsumptionRate)/100;

			GCManaSend(lpObj->Index,0xFF,(int)lpObj->Mana,lpObj->BP);
		}
	}
	else
	{
		if(gGate.IsInGate(lpObj,lpMsg->gate) == 0)
		{
			gObjClearViewport(lpObj);

			this->GCTeleportSend(lpObj->Index,lpMsg->gate,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y,lpObj->Dir);

			gObjectManager.CharacterUpdateMapEffect(lpObj);
		}
		else
		{
			#if(GAMESERVER_UPDATE>=501)

			if(IG_MAP_RANGE(lpObj->Map) == 0)
			{
				gObjMoveGate(aIndex,lpMsg->gate);
			}
			else
			{
				gImperialGuardian.MoveGate(lpObj,lpMsg->gate);
			}

			#else

			gObjMoveGate(aIndex,lpMsg->gate);

			#endif
		}
	}
}

void CMove::CGTeleportMoveRecv(PMSG_TELEPORT_MOVE_RECV* lpMsg,int aIndex) // OK
{
	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->Move(&gObj[aIndex],lpMsg->number);
}

void CMove::GCTeleportSend(int aIndex,int gate,BYTE map,BYTE x,BYTE y,BYTE dir) // OK
{
	PMSG_TELEPORT_SEND pMsg;

	pMsg.header.set(0x1C,sizeof(pMsg));

	pMsg.gate = ((gate>0)?1:gate);
	pMsg.map = map;
	pMsg.x = x;
	pMsg.y = y;
	pMsg.dir = dir;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}
