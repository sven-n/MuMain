// GensSystem.cpp: implementation of the CGensSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GensSystem.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "Gate.h"
#include "Map.h"
#include "MapManager.h"
#include "Move.h"
#include "QuestWorld.h"
#include "ServerInfo.h"
#include "Util.h"
#include "Viewport.h"

CGensSystem gGensSystem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGensSystem::CGensSystem() // OK
{

}

CGensSystem::~CGensSystem() // OK
{

}

void CGensSystem::CalcGensMemberInfo(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(lpObj->GensFamily == 0)
	{
		return;
	}

	if(lpObj->GensContribution >= 10000)
	{
		if(lpObj->GensRank == 1)
		{
			lpObj->GensSymbol = 1;
		}
		else if(lpObj->GensRank >= 2 && lpObj->GensRank <= 5)
		{
			lpObj->GensSymbol = 2;
		}
		else if(lpObj->GensRank >= 6 && lpObj->GensRank <= 10)
		{
			lpObj->GensSymbol = 3;
		}
		else if(lpObj->GensRank >= 11 && lpObj->GensRank <= 30)
		{
			lpObj->GensSymbol = 4;
		}
		else if(lpObj->GensRank >= 31 && lpObj->GensRank <= 50)
		{
			lpObj->GensSymbol = 5;
		}
		else if(lpObj->GensRank >= 51 && lpObj->GensRank <= 100)
		{
			lpObj->GensSymbol = 6;
		}
		else if(lpObj->GensRank >= 101 && lpObj->GensRank <= 200)
		{
			lpObj->GensSymbol = 7;
		}
		else if(lpObj->GensRank >= 201 && lpObj->GensRank <= 300)
		{
			lpObj->GensSymbol = 8;
		}
		else
		{
			lpObj->GensSymbol = 9;
		}
	}
	else
	{
		if(lpObj->GensContribution >= 0 && lpObj->GensContribution <= 499)
		{
			lpObj->GensSymbol = 14;
		}
		if(lpObj->GensContribution >= 500 && lpObj->GensContribution <= 1499)
		{
			lpObj->GensSymbol = 13;
		}
		if(lpObj->GensContribution >= 1500 && lpObj->GensContribution <= 2999)
		{
			lpObj->GensSymbol = 12;
		}
		if(lpObj->GensContribution >= 3000 && lpObj->GensContribution <= 5999)
		{
			lpObj->GensSymbol = 11;
		}
		if(lpObj->GensContribution >= 6000 && lpObj->GensContribution <= 9999)
		{
			lpObj->GensSymbol = 10;
		}
	}

	lpObj->GensNextContribution = gGensSystemContributionTable[(lpObj->GensSymbol-1)];

	#endif
}

int CGensSystem::GetGensRewardSymbol(int GensRank) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int GensSymbol = 0;

	if(GensRank == 1)
	{
		GensSymbol = 1;
	}
	else if(GensRank >= 2 && GensRank <= 5)
	{
		GensSymbol = 2;
	}
	else if(GensRank >= 6 && GensRank <= 10)
	{
		GensSymbol = 3;
	}
	else if(GensRank >= 11 && GensRank <= 30)
	{
		GensSymbol = 4;
	}
	else if(GensRank >= 31 && GensRank <= 50)
	{
		GensSymbol = 5;
	}
	else if(GensRank >= 51 && GensRank <= 100)
	{
		GensSymbol = 6;
	}
	else if(GensRank >= 101 && GensRank <= 200)
	{
		GensSymbol = 7;
	}
	else if(GensRank >= 201 && GensRank <= 300)
	{
		GensSymbol = 8;
	}

	return GensSymbol;

	#else

	return 0;

	#endif
}

void CGensSystem::NpcGensDuprianSteward(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_QUEST_WORLD;
	lpObj->Interface.state = 0;
	lpObj->QuestWorldMonsterClass = lpNpc->Class;

	PMSG_QUEST_WORLD_NPC_TALK_SEND pMsg;

	pMsg.header.set(0xF9,0x01,sizeof(pMsg));

	pMsg.MonsterClass = lpNpc->Class;

	if(lpObj->GensFamily == GENS_FAMILY_VARNERT)
	{
		pMsg.contribution = lpObj->GensContribution;
	}
	else
	{
		pMsg.contribution = 0;
	}

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::NpcGensVarnertSteward(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_QUEST_WORLD;
	lpObj->Interface.state = 0;
	lpObj->QuestWorldMonsterClass = lpNpc->Class;

	PMSG_QUEST_WORLD_NPC_TALK_SEND pMsg;

	pMsg.header.set(0xF9,0x01,sizeof(pMsg));

	pMsg.MonsterClass = lpNpc->Class;

	if(lpObj->GensFamily == GENS_FAMILY_DUPRIAN)
	{
		pMsg.contribution = lpObj->GensContribution;
	}
	else
	{
		pMsg.contribution = 0;
	}

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

bool CGensSystem::VerifyGensVictimList(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	GENS_SYSTEM_VICTIM_LIST* lpVictimList = 0;

	if(this->SearchGensVictimList(lpObj,lpTarget,&lpVictimList) == 0)
	{
		this->InsertGensVictimList(lpObj,lpTarget);
		return 1;
	}

	if((GetTickCount()-lpVictimList->Time) < ((DWORD)(gServerInfo.m_GensSystemContributionFloodTime*1000)))
	{
		lpVictimList->KillCount++;
		return 0;
	}
	else
	{
		lpVictimList->Set(lpObj->Name);
		return 1;
	}

	#else

	return 0;

	#endif
}

bool CGensSystem::InsertGensVictimList(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_GENS_SYSTEM_VICTIM;n++)
	{
		if(lpTarget->GensVictimList[n].Time == 0 || (GetTickCount()-lpTarget->GensVictimList[n].Time) > ((DWORD)(gServerInfo.m_GensSystemContributionFloodTime*1000)))
		{
			lpTarget->GensVictimList[n].Set(lpObj->Name);
			return 1;
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CGensSystem::SearchGensVictimList(LPOBJ lpObj,LPOBJ lpTarget,GENS_SYSTEM_VICTIM_LIST** lpVictimList) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_GENS_SYSTEM_VICTIM;n++)
	{
		if(lpTarget->GensVictimList[n].Time != 0 && strcmp(lpTarget->GensVictimList[n].Name,lpObj->Name) == 0)
		{
			(*lpVictimList) = &lpTarget->GensVictimList[n];
			return 1;
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

void CGensSystem::UserDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(this->CheckGens(lpObj,lpTarget) == 0)
	{
		return;
	}

	if(this->VerifyGensVictimList(lpObj,lpTarget) == 0)
	{
		return;
	}

	int ContributionIncrease = gServerInfo.m_GensSystemKillerContributionIncrease;

	int ContributionDecrease = gServerInfo.m_GensSystemVictimContributionDecrease;

	if(lpTarget->Level >= lpObj->Level)
	{
		switch((((lpTarget->Level-lpObj->Level)-1)/10))
		{
			case 0:
				ContributionIncrease -= 0;
				ContributionDecrease -= 0;
				break;
			case 1:
				ContributionIncrease -= 2;
				ContributionDecrease -= 0;
				break;
			case 2:
				ContributionIncrease -= 2;
				ContributionDecrease -= 0;
				break;
			case 3:
				ContributionIncrease -= 3;
				ContributionDecrease -= 1;
				break;
			case 4:
				ContributionIncrease -= 3;
				ContributionDecrease -= 1;
				break;
			default:
				ContributionIncrease -= 4;
				ContributionDecrease -= 2;
				break;
		}
	}
	else
	{
		switch((((lpObj->Level-lpTarget->Level)-1)/10))
		{
			case 0:
				ContributionIncrease += 0;
				ContributionDecrease += 0;
				break;
			case 1:
				ContributionIncrease += 1;
				ContributionDecrease += 0;
				break;
			case 2:
				ContributionIncrease += 1;
				ContributionDecrease += 0;
				break;
			case 3:
				ContributionIncrease += 1;
				ContributionDecrease += 0;
				break;
			case 4:
				ContributionIncrease += 1;
				ContributionDecrease += 0;
				break;
			default:
				ContributionIncrease += 2;
				ContributionDecrease += 0;
				break;
		}
	}

	if(lpTarget->GensSymbol >= lpObj->GensSymbol)
	{
		switch((lpTarget->GensSymbol-lpObj->GensSymbol))
		{
			case 0:
				ContributionIncrease += ((lpTarget->GensSymbol>=9)?0:0);
				break;
			case 1:
				ContributionIncrease += ((lpTarget->GensSymbol>=9)?1:3);
				break;
			case 2:
				ContributionIncrease += ((lpTarget->GensSymbol>=9)?2:4);
				break;
			default:
				ContributionIncrease += ((lpTarget->GensSymbol>=9)?3:5);
				break;
		}
	}

	ContributionDecrease = ((ContributionDecrease<gServerInfo.m_GensSystemVictimMinContributionDecrease)?gServerInfo.m_GensSystemVictimMinContributionDecrease:ContributionDecrease);

	ContributionDecrease = ((ContributionDecrease>gServerInfo.m_GensSystemVictimMaxContributionDecrease)?gServerInfo.m_GensSystemVictimMaxContributionDecrease:ContributionDecrease);

	lpObj->GensContribution -= ((ContributionDecrease>lpObj->GensContribution)?lpObj->GensContribution:ContributionDecrease);

	this->GDGensSystemUpdateSend(lpObj->Index);

	ContributionIncrease = ((ContributionIncrease<gServerInfo.m_GensSystemKillerMinContributionIncrease)?gServerInfo.m_GensSystemKillerMinContributionIncrease:ContributionIncrease);

	ContributionIncrease = ((ContributionIncrease>gServerInfo.m_GensSystemKillerMaxContributionIncrease)?gServerInfo.m_GensSystemKillerMaxContributionIncrease:ContributionIncrease);

	lpTarget->GensContribution += ContributionIncrease;

	this->GDGensSystemUpdateSend(lpTarget->Index);

	#endif
}

bool CGensSystem::CheckGens(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(lpObj->Type != OBJECT_USER || lpTarget->Type != OBJECT_USER)
	{
		return 0;
	}

	if(gMapManager.GetMapGensBattle(lpObj->Map) == 0 || gMapManager.GetMapGensBattle(lpTarget->Map) == 0)
	{
		return 0;
	}

	if(lpObj->GensFamily == GENS_FAMILY_NONE || lpTarget->GensFamily == GENS_FAMILY_NONE)
	{
		return 0;
	}

	if(lpObj->GensFamily != lpTarget->GensFamily)
	{
		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

BYTE CGensSystem::GetGensMoveIndex(BYTE* GensMoveIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	BYTE count = 0;

	MOVE_INFO MoveInfo;

	for(int n=0;n < MAX_MOVE;n++)
	{
		if(gMove.GetInfo(n,&MoveInfo) != 0)
		{
			if(gMapManager.GetMapGensBattle(gGate.GetGateMap(MoveInfo.Gate)) != 0)
			{
				GensMoveIndex[count++] = n;
			}
		}
	}

	return count;

	#else

	return 0;

	#endif
}

BYTE CGensSystem::GetGensBattleMap(BYTE* GensBattleMap) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	BYTE count = 0;

	for(int n=0;n < MAX_MAP;n++)
	{
		if(gMapManager.GetMapNonPK(n) == 0)
		{
			if(gMapManager.GetMapGensBattle(n) != 0)
			{
				GensBattleMap[count++] = n;
			}
		}
	}

	return count;

	#else

	return 0;

	#endif
}

void CGensSystem::CGGensSystemInsertRecv(PMSG_GENS_SYSTEM_INSERT_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_GensSystemSwitch == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_QUEST_WORLD)
	{
		return;
	}

	if(lpObj->GensFamily != GENS_FAMILY_NONE)
	{
		this->GCGensSystemInsertSend(aIndex,1,GENS_FAMILY_NONE);
		return;
	}

	if(lpObj->Level < gServerInfo.m_GensSystemInsertMinLevel)
	{
		this->GCGensSystemInsertSend(aIndex,3,GENS_FAMILY_NONE);
		return;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && lpObj->GuildNumber != 0)
	{
		this->GCGensSystemInsertSend(aIndex,5,GENS_FAMILY_NONE);
		return;
	}

	if(gServerInfo.m_GensSystemPartyLock != 0 && OBJECT_RANGE(lpObj->PartyNumber) != 0)
	{
		this->GCGensSystemInsertSend(aIndex,6,GENS_FAMILY_NONE);
		return;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && (lpObj->UseGuildMatching != 0 || lpObj->UseGuildMatchingJoin != 0))
	{
		this->GCGensSystemInsertSend(aIndex,8,GENS_FAMILY_NONE);
		return;
	}

	if(gServerInfo.m_GensSystemPartyLock != 0 && (lpObj->UsePartyMatching != 0 || lpObj->UsePartyMatchingJoin != 0))
	{
		this->GCGensSystemInsertSend(aIndex,8,GENS_FAMILY_NONE);
		return;
	}

	this->GDGensSystemInsertSend(aIndex,lpMsg->GensFamily);

	#endif
}

void CGensSystem::CGGensSystemDeleteRecv(PMSG_GENS_SYSTEM_DELETE_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_GensSystemSwitch == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_QUEST_WORLD)
	{
		return;
	}

	if(lpObj->GensFamily == GENS_FAMILY_NONE)
	{
		this->GCGensSystemDeleteSend(aIndex,1);
		return;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && lpObj->GuildNumber != 0)
	{
		this->GCGensSystemDeleteSend(aIndex,2);
		return;
	}

	if(gServerInfo.m_GensSystemPartyLock != 0 && OBJECT_RANGE(lpObj->PartyNumber) != 0)
	{
		this->GCGensSystemDeleteSend(aIndex,2);
		return;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && (lpObj->UseGuildMatching != 0 || lpObj->UseGuildMatchingJoin != 0))
	{
		this->GCGensSystemDeleteSend(aIndex,4);
		return;
	}

	if(gServerInfo.m_GensSystemPartyLock != 0 && (lpObj->UsePartyMatching != 0 || lpObj->UsePartyMatchingJoin != 0))
	{
		this->GCGensSystemDeleteSend(aIndex,4);
		return;
	}

	if((lpObj->GensFamily == GENS_FAMILY_VARNERT && lpObj->QuestWorldMonsterClass != 543) || (lpObj->GensFamily == GENS_FAMILY_DUPRIAN && lpObj->QuestWorldMonsterClass != 544))
	{
		this->GCGensSystemDeleteSend(aIndex,3);
		return;
	}

	this->GDGensSystemDeleteSend(aIndex,lpObj->GensFamily);

	#endif
}

void CGensSystem::CGGensSystemRewardRecv(PMSG_GENS_SYSTEM_REWARD_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_GensSystemSwitch == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_QUEST_WORLD)
	{
		return;
	}

	SYSTEMTIME time;

	GetSystemTime(&time);

	if(lpObj->GensFamily == GENS_FAMILY_NONE || lpObj->GensRank > 8)
	{
		this->GCGensSystemRewardSend(aIndex,2);
		return;
	}

	if(time.wDay < gServerInfo.m_GensSystemStartRewardDay || time.wDay > gServerInfo.m_GensSystemFinalRewardDay)
	{
		this->GCGensSystemRewardSend(aIndex,1);
		return;
	}

	if((lpObj->GensFamily == GENS_FAMILY_VARNERT && lpObj->QuestWorldMonsterClass != 543) || (lpObj->GensFamily == GENS_FAMILY_DUPRIAN && lpObj->QuestWorldMonsterClass != 544))
	{
		this->GCGensSystemRewardSend(aIndex,5);
		return;
	}

	this->GDGensSystemRewardSend(aIndex);

	#endif
}

void CGensSystem::CGGensSystemMemberRecv(PMSG_GENS_SYSTEM_MEMBER_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_GensSystemSwitch == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GCGensSystemMemberSend(aIndex);

	#endif
}

void CGensSystem::GCGensSystemInsertSend(int aIndex,BYTE result,BYTE GensFamily) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	PMSG_GENS_SYSTEM_INSERT_SEND pMsg;

	pMsg.header.set(0xF8,0x02,sizeof(pMsg));

	pMsg.result = result;

	pMsg.GensFamily = GensFamily;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GCGensSystemDeleteSend(int aIndex,BYTE result) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	PMSG_GENS_SYSTEM_DELETE_SEND pMsg;

	pMsg.header.set(0xF8,0x04,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GCGensSystemMemberSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	LPOBJ lpObj = &gObj[aIndex];

	PMSG_GENS_SYSTEM_MEMBER_SEND pMsg;

	pMsg.header.set(0xF8,0x07,sizeof(pMsg));

	pMsg.GensFamily = lpObj->GensFamily;

	pMsg.GensRank = ((lpObj->GensContribution>=10000)?lpObj->GensRank:0);

	pMsg.GensSymbol = lpObj->GensSymbol;

	pMsg.GensContribution = lpObj->GensContribution;

	pMsg.GensNextContribution = lpObj->GensNextContribution;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GCGensSystemDisableMoveSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	LPOBJ lpObj = &gObj[aIndex];

	PMSG_GENS_SYSTEM_DISABLE_MOVE_SEND pMsg;

	pMsg.header.set(0xF8,0x08,sizeof(pMsg));

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GCGensSystemRewardSend(int aIndex,BYTE result) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	PMSG_GENS_SYSTEM_REWARD_SEND pMsg;

	pMsg.header.set(0xF8,0x0A,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::DGGensSystemInsertRecv(SDHP_GENS_SYSTEM_INSERT_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGensSystemInsertRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpMsg->result != 0)
	{
		return;
	}

	lpObj->GensFamily = lpMsg->GensFamily;

	lpObj->GensRank = lpMsg->GensRank;

	lpObj->GensContribution = lpMsg->GensContribution;

	this->CalcGensMemberInfo(lpObj);

	gViewport.GCViewportSimpleGensSystemSend(lpObj);

	this->GCGensSystemInsertSend(lpObj->Index,lpMsg->result,lpMsg->GensFamily);

	#endif
}

void CGensSystem::DGGensSystemDeleteRecv(SDHP_GENS_SYSTEM_DELETE_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGensSystemDeleteRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpMsg->result != 0)
	{
		return;
	}

	lpObj->GensFamily = 0;

	lpObj->GensRank = 0;

	lpObj->GensSymbol = 0;

	lpObj->GensContribution = 0;

	lpObj->GensNextContribution = 0;

	gViewport.GCViewportSimpleGensSystemSend(lpObj);

	this->GCGensSystemDeleteSend(lpObj->Index,lpMsg->result);

	#endif
}

void CGensSystem::DGGensSystemMemberRecv(SDHP_GENS_SYSTEM_MEMBER_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGensSystemMemberRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpMsg->result != 0)
	{
		return;
	}

	lpObj->LoadGens = 1;

	lpObj->GensFamily = lpMsg->GensFamily;

	lpObj->GensRank = lpMsg->GensRank;

	lpObj->GensContribution = lpMsg->GensContribution;

	this->CalcGensMemberInfo(lpObj);

	gViewport.GCViewportSimpleGensSystemSend(lpObj);

	#endif
}

void CGensSystem::DGGensSystemUpdateRecv(SDHP_GENS_SYSTEM_UPDATE_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGensSystemUpdateRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpMsg->result != 0)
	{
		return;
	}

	if(lpObj->LoadGens == 0)
	{
		return;
	}

	lpObj->GensFamily = lpMsg->GensFamily;

	lpObj->GensRank = lpMsg->GensRank;

	lpObj->GensContribution = lpMsg->GensContribution;

	this->CalcGensMemberInfo(lpObj);

	gViewport.GCViewportSimpleGensSystemSend(lpObj);

	#endif
}

void CGensSystem::DGGensSystemRewardRecv(SDHP_GENS_SYSTEM_REWARD_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGGensSystemRewardRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpMsg->result != 0)
	{
		this->GCGensSystemRewardSend(lpObj->Index,2);
		return;
	}

	if(lpMsg->GensRewardStatus != 0)
	{
		this->GCGensSystemRewardSend(lpObj->Index,4);
		return;
	}

	if(gItemManager.GetInventoryEmptySlotCount(lpObj) < gGensSystemRewardAmountTable[gGensSystem.GetGensRewardSymbol(lpMsg->GensRank)])
	{
		this->GCGensSystemRewardSend(lpObj->Index,3);
		return;
	}

	for(int n=0;n < gGensSystemRewardAmountTable[gGensSystem.GetGensRewardSymbol(lpMsg->GensRank)];n++)
	{
		GDCreateItemSend(lpObj->Index,0xEB,0,0,(GET_ITEM(14,141)+((gGensSystem.GetGensRewardSymbol(lpMsg->GensRank)-1)/2)),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
	}

	this->GCGensSystemRewardSend(lpObj->Index,0);

	this->GDGensSystemRewardSaveSend(lpObj->Index,lpMsg->GensRank,1);

	#endif
}

void CGensSystem::DGGensSystemCreateRecv(SDHP_GENS_SYSTEM_CREATE_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			if(gObj[n].GensFamily == lpMsg->GensFamily)
			{
				if((lpMsg->GensRankStart == 0xFFFFFFFF || ((DWORD)gObj[n].GensRank) >= lpMsg->GensRankStart) && (lpMsg->GensRankFinal == 0xFFFFFFFF || ((DWORD)gObj[n].GensRank) <= lpMsg->GensRankFinal))
				{
					this->GDGensSystemMemberSend(n);
				}
			}
		}
	}

	#endif
}

void CGensSystem::GDGensSystemInsertSend(int aIndex,BYTE GensFamily) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_GENS_SYSTEM_INSERT_SEND pMsg;

	pMsg.header.set(0x11,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.GensFamily = GensFamily;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GDGensSystemDeleteSend(int aIndex,BYTE GensFamily) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_GENS_SYSTEM_DELETE_SEND pMsg;

	pMsg.header.set(0x11,0x01,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.GensFamily = GensFamily;

	pMsg.GensFamily = gObj[aIndex].GensFamily;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GDGensSystemMemberSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_GENS_SYSTEM_MEMBER_SEND pMsg;

	pMsg.header.set(0x11,0x02,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GDGensSystemUpdateSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	if(gObj[aIndex].LoadGens == 0)
	{
		return;
	}

	SDHP_GENS_SYSTEM_UPDATE_SEND pMsg;

	pMsg.header.set(0x11,0x03,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.GensFamily = gObj[aIndex].GensFamily;

	pMsg.GensRank = gObj[aIndex].GensRank;

	pMsg.GensContribution = gObj[aIndex].GensContribution;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GDGensSystemRewardSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_GENS_SYSTEM_REWARD_SEND pMsg;

	pMsg.header.set(0x11,0x04,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.GensFamily = gObj[aIndex].GensFamily;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CGensSystem::GDGensSystemRewardSaveSend(int aIndex,DWORD GensRank,DWORD GensRewardStatus) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	LPOBJ lpObj = &gObj[aIndex];

	SDHP_GENS_SYSTEM_REWARD_SAVE_SEND pMsg;

	pMsg.header.set(0x11,0x30,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.GensFamily = lpObj->GensFamily;

	pMsg.GensRank = GensRank;

	pMsg.GensRewardStatus = GensRewardStatus;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}
