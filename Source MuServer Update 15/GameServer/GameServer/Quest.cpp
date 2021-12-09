// Quest.cpp: implementation of the CQuest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Quest.h"
#include "GameMain.h"
#include "MemScript.h"
#include "QuestObjective.h"
#include "QuestReward.h"
#include "Util.h"

CQuest gQuest;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQuest::CQuest() // OK
{
	this->m_count = 0;
}

CQuest::~CQuest() // OK
{

}

void CQuest::Load(char* path) // OK
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

	this->m_count = 0;

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

			QUEST_INFO info;

			memset(&info,0,sizeof(info));

			info.Index = lpMemScript->GetNumber();

			info.StartType = lpMemScript->GetAsNumber();

			info.MonsterClass = lpMemScript->GetAsNumber();

			info.CurrentState = lpMemScript->GetAsNumber();

			info.RequireIndex = lpMemScript->GetAsNumber();

			info.RequireState = lpMemScript->GetAsNumber();

			info.RequireMinLevel = lpMemScript->GetAsNumber();

			info.RequireMaxLevel = lpMemScript->GetAsNumber();

			for(int n=0;n < MAX_CLASS;n++){info.RequireClass[n] = lpMemScript->GetAsNumber();}

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CQuest::SetInfo(QUEST_INFO info) // OK
{
	if(this->m_count < 0 || this->m_count >= MAX_QUEST)
	{
		return;
	}

	this->m_QuestInfo[this->m_count++] = info;
}

QUEST_INFO* CQuest::GetInfo(int index) // OK
{
	if(index < 0 || index >= this->m_count)
	{
		return 0;
	}

	return &this->m_QuestInfo[index];
}

QUEST_INFO* CQuest::GetInfoByIndex(LPOBJ lpObj,int QuestIndex) // OK
{
	for(int n=0;n < this->m_count;n++)
	{
		QUEST_INFO* lpInfo = this->GetInfo(n);

		if(lpInfo == 0)
		{
			continue;
		}

		if(lpInfo->Index != QuestIndex)
		{
			continue;
		}

		if(this->CheckQuestRequisite(lpObj,lpInfo) == 0)
		{
			continue;
		}

		return lpInfo;
	}

	return 0;
}

bool CQuest::AddQuestList(LPOBJ lpObj,int QuestIndex,int QuestState) // OK
{
	if(QuestIndex < 0 || QuestIndex >= MAX_QUEST_LIST)
	{
		return 0;
	}

	lpObj->Quest[QuestIndex/4] = (lpObj->Quest[QuestIndex/4] & gQuestBitMask[(QuestIndex%4)*2]) | ((QuestState & 3) << ((QuestIndex%4)*2));

	return 1;
}

BYTE CQuest::GetQuestList(LPOBJ lpObj,int QuestIndex) // OK
{
	if(QuestIndex < 0 || QuestIndex >= MAX_QUEST_LIST)
	{
		return 0;
	}

	return lpObj->Quest[QuestIndex/4];
}

bool CQuest::CheckQuestRequisite(LPOBJ lpObj,QUEST_INFO* lpInfo) // OK
{
	if(this->CheckQuestListState(lpObj,lpInfo->Index,lpInfo->CurrentState) == 0)
	{
		return 0;
	}

	if(lpInfo->RequireIndex != -1 && this->CheckQuestListState(lpObj,lpInfo->RequireIndex,lpInfo->RequireState) == 0)
	{
		return 0;
	}

	if(lpInfo->RequireMinLevel != -1 && lpInfo->RequireMinLevel > lpObj->Level)
	{
		return 0;
	}

	if(lpInfo->RequireMaxLevel != -1 && lpInfo->RequireMaxLevel < lpObj->Level)
	{
		return 0;
	}

	if(lpInfo->RequireClass[lpObj->Class] == 0 || lpInfo->RequireClass[lpObj->Class] > (lpObj->ChangeUp+1))
	{
		return 0;
	}

	return 1;
}

bool CQuest::CheckQuestListState(LPOBJ lpObj,int QuestIndex,int QuestState) // OK
{
	if(QuestIndex < 0 || QuestIndex >= MAX_QUEST_LIST)
	{
		return 0;
	}

	if(((lpObj->Quest[QuestIndex/4] >> ((QuestIndex%4)*2)) & 3) == QuestState)
	{
		return 1;
	}

	return 0;
}

long CQuest::GetQuestRewardLevelUpPoint(LPOBJ lpObj) // OK
{
	int point = 0;

	for(int n=0;n < MAX_QUEST_LIST;n++)
	{
		if(this->CheckQuestListState(lpObj,n,QUEST_FINISH) != 0)
		{
			point += gQuestReward.GetQuestRewardPoint(lpObj,n);
		}
	}

	return point;
}

bool CQuest::NpcTalk(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	for(int n=0;n < this->m_count;n++)
	{
		QUEST_INFO* lpInfo = this->GetInfo(n);

		if(lpInfo == 0)
		{
			continue;
		}

		if(lpInfo->StartType != 0)
		{
			continue;
		}

		if(lpInfo->MonsterClass != lpNpc->Class)
		{
			continue;
		}

		if(this->CheckQuestRequisite(lpObj,lpInfo) == 0)
		{
			continue;
		}

		lpObj->Interface.use = 1;
		lpObj->Interface.type = INTERFACE_QUEST;
		lpObj->Interface.state = 0;

		this->GCQuestStateSend(lpObj->Index,lpInfo->Index);

		this->GCQuestKillCountSend(lpObj->Index,lpInfo->Index);

		return 1;
	}

	return 0;
}

void CQuest::CGQuestInfoRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	this->GCQuestInfoSend(aIndex);
}

void CQuest::CGQuestStateRecv(PMSG_QUEST_STATE_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	QUEST_INFO* lpInfo = this->GetInfoByIndex(lpObj,lpMsg->QuestIndex);

	if(lpInfo == 0)
	{
		return;
	}

	if(gQuestObjective.CheckQuestObjective(lpObj,lpInfo->Index) == 0)
	{
		this->GCQuestResultSend(aIndex,lpInfo->Index,0xFF,this->GetQuestList(lpObj,lpInfo->Index));
		return;
	}

	if(lpInfo->CurrentState == QUEST_NORMAL)
	{
		gQuestObjective.RemoveQuestObjective(lpObj,lpInfo->Index);
		gQuestReward.InsertQuestReward(lpObj,lpInfo->Index);
		this->AddQuestList(lpObj,lpInfo->Index,QUEST_ACCEPT);
		gQuestObjective.InitQuestObjectiveKillCount(lpObj,lpInfo->Index);
		this->GCQuestResultSend(aIndex,lpInfo->Index,0x00,this->GetQuestList(lpObj,lpInfo->Index));
		return;
	}

	if(lpInfo->CurrentState == QUEST_ACCEPT)
	{
		gQuestObjective.RemoveQuestObjective(lpObj,lpInfo->Index);
		gQuestReward.InsertQuestReward(lpObj,lpInfo->Index);
		this->AddQuestList(lpObj,lpInfo->Index,QUEST_FINISH);
		gQuestObjective.InitQuestObjectiveKillCount(lpObj,lpInfo->Index);
		this->GCQuestResultSend(aIndex,lpInfo->Index,0x00,this->GetQuestList(lpObj,lpInfo->Index));
		return;
	}

	if(lpInfo->CurrentState == QUEST_FINISH)
	{
		gQuestObjective.RemoveQuestObjective(lpObj,lpInfo->Index);
		gQuestReward.InsertQuestReward(lpObj,lpInfo->Index);
		this->AddQuestList(lpObj,lpInfo->Index,QUEST_FINISH);
		gQuestObjective.InitQuestObjectiveKillCount(lpObj,lpInfo->Index);
		this->GCQuestResultSend(aIndex,lpInfo->Index,0xFF,this->GetQuestList(lpObj,lpInfo->Index));
		return;
	}

	if(lpInfo->CurrentState == QUEST_CANCEL)
	{
		gQuestObjective.RemoveQuestObjective(lpObj,lpInfo->Index);
		gQuestReward.InsertQuestReward(lpObj,lpInfo->Index);
		this->AddQuestList(lpObj,lpInfo->Index,QUEST_ACCEPT);
		gQuestObjective.InitQuestObjectiveKillCount(lpObj,lpInfo->Index);
		this->GCQuestResultSend(aIndex,lpInfo->Index,0x00,this->GetQuestList(lpObj,lpInfo->Index));
		return;
	}
}

void CQuest::CGQuestNpcWarewolfRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use != 0)
	{
		return;
	}

	if(lpObj->X < 57 || lpObj->X > 67 || lpObj->Y < 234 || lpObj->Y > 244)
	{
		return;
	}

	if(this->CheckQuestListState(lpObj,5,QUEST_ACCEPT) != 0 || this->CheckQuestListState(lpObj,5,QUEST_FINISH) != 0)
	{
		gObjMoveGate(aIndex,256);
	}
}

void CQuest::CGQuestNpcKeeperRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use != 0)
	{
		return;
	}

	if(lpObj->X < 114 || lpObj->X > 124 || lpObj->Y < 163 || lpObj->Y > 173)
	{
		return;
	}

	if(this->CheckQuestListState(lpObj,6,QUEST_ACCEPT) != 0 || this->CheckQuestListState(lpObj,6,QUEST_FINISH) != 0)
	{
		gObjMoveGate(aIndex,257);
	}
}

void CQuest::GCQuestInfoSend(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->SendQuestInfo != 0)
	{
		return;
	}

	PMSG_QUEST_INFO_SEND pMsg;

	pMsg.header.set(0xA0,sizeof(pMsg));

	pMsg.count = MAX_QUEST_LIST;
	
	memcpy(pMsg.QuestInfo,lpObj->Quest,sizeof(pMsg.QuestInfo));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	lpObj->SendQuestInfo = 1;

}

void CQuest::GCQuestStateSend(int aIndex,int QuestIndex) // OK
{
	this->GCQuestInfoSend(aIndex);

	PMSG_QUEST_STATE_SEND pMsg;

	pMsg.header.set(0xA1,sizeof(pMsg));

	pMsg.QuestIndex = QuestIndex;

	pMsg.QuestState = this->GetQuestList(&gObj[aIndex],QuestIndex);

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CQuest::GCQuestResultSend(int aIndex,int QuestIndex,int QuestResult,int QuestState) // OK
{
	PMSG_QUEST_RESULT_SEND pMsg;

	pMsg.header.set(0xA2,sizeof(pMsg));

	pMsg.QuestIndex = QuestIndex;

	pMsg.QuestResult = QuestResult;

	pMsg.QuestState = QuestState;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CQuest::GCQuestRewardSend(int aIndex,int QuestReward,int QuestAmount) // OK
{
	this->GCQuestInfoSend(aIndex);

	PMSG_QUEST_REWARD_SEND pMsg;

	pMsg.header.set(0xA3,sizeof(pMsg));

	pMsg.index[0] = SET_NUMBERHB(aIndex);

	pMsg.index[1] = SET_NUMBERLB(aIndex);

	pMsg.QuestReward = QuestReward;

	pMsg.QuestAmount = QuestAmount;

	#if(GAMESERVER_EXTRA==1)
	pMsg.ViewPoint = gObj[aIndex].LevelUpPoint;
	#endif

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	MsgSendV2(&gObj[aIndex],(BYTE*)&pMsg,pMsg.header.size);
}

void CQuest::GCQuestKillCountSend(int aIndex,int QuestIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->QuestKillCountIndex != QuestIndex)
	{
		return;
	}

	if(this->CheckQuestListState(lpObj,QuestIndex,QUEST_ACCEPT) == 0)
	{
		return;
	}

	PMSG_QUEST_KILL_COUNT_SEND pMsg;

	pMsg.header.set(0xA4,sizeof(pMsg));

	pMsg.QuestResult = 1;

	pMsg.QuestIndex = QuestIndex;

	memcpy(pMsg.QuestKillCount,lpObj->QuestKillCount,sizeof(pMsg.QuestKillCount));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CQuest::DGQuestKillCountRecv(SDHP_QUEST_KILL_COUNT_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGQuestKillCountRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpObj->LoadQuestKillCount != 0)
	{
		return;
	}

	lpObj->LoadQuestKillCount = 1;

	lpObj->QuestKillCountIndex = lpMsg->QuestIndex;

	for(int n=0;n < MAX_QUEST_KILL_COUNT;n++)
	{
		lpObj->QuestKillCount[n].MonsterClass = lpMsg->MonsterClass[n];
		lpObj->QuestKillCount[n].KillCount = lpMsg->KillCount[n];
	}
}

void CQuest::GDQuestKillCountSend(int aIndex) // OK
{
	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	if(gObj[aIndex].LoadQuestKillCount != 0)
	{
		return;
	}

	SDHP_QUEST_KILL_COUNT_SEND pMsg;

	pMsg.header.set(0x0C,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void CQuest::GDQuestKillCountSaveSend(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->LoadQuestKillCount == 0)
	{
		return;
	}

	SDHP_QUEST_KILL_COUNT_SAVE_SEND pMsg;

	pMsg.header.set(0x0C,0x30,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.QuestIndex = lpObj->QuestKillCountIndex;

	for(int n=0;n < MAX_QUEST_KILL_COUNT;n++)
	{
		pMsg.MonsterClass[n] = lpObj->QuestKillCount[n].MonsterClass;
		pMsg.KillCount[n] = lpObj->QuestKillCount[n].KillCount;
	}

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}
