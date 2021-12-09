// HackSkillCheck.cpp: implementation of the CHackSkillCheck class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "HackSkillCheck.h"
#include "GameMain.h"
#include "GensSystem.h"
#include "Log.h"
#include "Map.h"
#include "MapManager.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "ServerInfo.h"
#include "Util.h"

CHackSkillCheck gHackSkillCheck;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHackSkillCheck::CHackSkillCheck() // OK
{
	this->m_HackSkillInfo.clear();
}

CHackSkillCheck::~CHackSkillCheck() // OK
{

}

void CHackSkillCheck::Load(char* path) // OK
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

	this->m_HackSkillInfo.clear();

	int count = 0;

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

			HACKSKILL_INFO info;

			info.Index = count;

			info.Skill = lpMemScript->GetNumber();

			info.MinSpeed = lpMemScript->GetAsNumber();

			info.MaxSpeed = lpMemScript->GetAsNumber();

			info.Delay = lpMemScript->GetAsNumber();

			info.MaxCount = lpMemScript->GetAsNumber();

			this->m_HackSkillInfo.insert(std::pair<int,HACKSKILL_INFO>(info.Index,info));

			count++;
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CHackSkillCheck::GetInfo(int index,HACKSKILL_INFO* lpInfo) // OK
{
	std::map<int,HACKSKILL_INFO>::iterator it = this->m_HackSkillInfo.find(index);

	if(it == this->m_HackSkillInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}
}

bool CHackSkillCheck::GetInfoBySpeed(int Skill,int Speed, HACKSKILL_INFO* lpInfo) // OK
{
	for(std::map<int,HACKSKILL_INFO>::iterator it=this->m_HackSkillInfo.begin();it != this->m_HackSkillInfo.end();it++)
	{
		if(it->second.Skill == Skill && Speed >= it->second.MinSpeed && (Speed <= it->second.MaxSpeed || it->second.MaxSpeed == -1))
		{
			(*lpInfo) = it->second;
			return 1;
		}
	}

	return 0;
}

bool CHackSkillCheck::CheckSpeedHack(LPOBJ lpObj,int skill) // OK
{
	if (gServerInfo.m_SpeedHackSkillEnable != 1)
	{
		return 0;
	}

	HACKSKILL_INFO HackSkillInfo;

	if(this->GetInfoBySpeed(skill,lpObj->PhysiSpeed,&HackSkillInfo) == 0)
	{
		return 0;
	}

	if(lpObj->SpeedHackSkill != skill)
	{
		lpObj->SpeedHackSkill = skill;
		lpObj->SpeedHackCount = 0;
		lpObj->SpeedHackDelay = GetTickCount();
		lpObj->SpeedHackDialog = 0;
	}

	if(lpObj->SpeedHackCount > HackSkillInfo.MaxCount)
	{

		if( lpObj->SpeedHackDialog == 0)
		{
			LogAdd(LOG_RED,"[SpeedHackDetected][%s][%s] Skill: %d",lpObj->Account,lpObj->Name,skill);

			if (gServerInfo.m_SpeedHackSkillDialog == 1)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(843));	
			}
			lpObj->SpeedHackDialog = 1;
		}

		if (gServerInfo.m_SpeedHackSkillType == 1)
		{
			GCCloseClientSend(lpObj->Index,0);
			return 1;
		}
		else if (gServerInfo.m_SpeedHackSkillType == 2)
		{
			return 1;
		}
		else if (gServerInfo.m_SpeedHackSkillType == 3)
		{
			if( GetTickCount() > (lpObj->SpeedHackDelay + HackSkillInfo.Delay + (DWORD)gServerInfo.m_SpeedHackSkillPenalty))
			{
				lpObj->SpeedHackCount = 0;
				lpObj->SpeedHackDelay = GetTickCount();
				lpObj->SpeedHackDialog = 0;
			}
			return 1;
		}
		else if (gServerInfo.m_SpeedHackSkillType == 4)
		{
			SDHP_COMMAND_BLOC_SEND pMsg;

			pMsg.header.set(0x0F,0x06,sizeof(pMsg)); 

			pMsg.index = lpObj->Index;

			pMsg.days = 30;

			memcpy(pMsg.namebloc,lpObj->Account,sizeof(pMsg.namebloc));

			gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

			gObjUserKill(lpObj->Index);
			return 1;
		}
		else
		{
			if( GetTickCount() > (lpObj->SpeedHackDelay+HackSkillInfo.Delay))
			{
				lpObj->SpeedHackCount = 0;
				lpObj->SpeedHackDelay = GetTickCount();
			}
		}
		return 0;
	}

	if( GetTickCount() < (lpObj->SpeedHackDelay+HackSkillInfo.Delay))
	{
		lpObj->SpeedHackCount++;
		return 0;
	}
	else
	{
		if (gServerInfo.m_SpeedHackSkillDebug == 1)
		{
			LogAdd(LOG_RED,"[SpeedHackTeste] Skill: %d | AttackSpeed: %d | Count: %d",skill,lpObj->PhysiSpeed,lpObj->SpeedHackCount);
		}
	}

	lpObj->SpeedHackCount = 0;
	lpObj->SpeedHackDelay = GetTickCount();

	return 0;
}