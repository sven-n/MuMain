// GameMaster.cpp: implementation of the CGameMaster class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameMaster.h"
#include "MemScript.h"
#include "Util.h"

CGameMaster gGameMaster;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGameMaster::CGameMaster() // OK
{
	this->m_count = 0;
}

CGameMaster::~CGameMaster() // OK
{

}

void CGameMaster::Load(char* path) // OK
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

			GAME_MASTER_INFO info;

			memset(&info,0,sizeof(info));

			strcpy_s(info.Account,lpMemScript->GetString());

			strcpy_s(info.Name,lpMemScript->GetAsString());

			info.Level = lpMemScript->GetAsNumber();

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CGameMaster::SetInfo(GAME_MASTER_INFO info) // OK
{
	if(this->m_count < 0 || this->m_count >= MAX_GAME_MASTER)
	{
		return;
	}

	this->m_GameMasterInfo[this->m_count++] = info;
}

void CGameMaster::SetGameMasterLevel(LPOBJ lpObj,int level) // OK
{
	for(int n=0;n < this->m_count;n++)
	{
		if(strcmp(this->m_GameMasterInfo[n].Account,lpObj->Account) == 0 && strcmp(this->m_GameMasterInfo[n].Name,lpObj->Name) == 0)
		{
			return;
		}
	}

	GAME_MASTER_INFO info;

	memcpy(info.Account,lpObj->Account,sizeof(info.Account));

	memcpy(info.Name,lpObj->Name,sizeof(info.Name));

	info.Level = level;

	this->SetInfo(info);
}

bool CGameMaster::CheckGameMasterLevel(LPOBJ lpObj,int level) // OK
{
	for(int n=0;n < this->m_count;n++)
	{
		if(strcmp(this->m_GameMasterInfo[n].Account,lpObj->Account) == 0 && strcmp(this->m_GameMasterInfo[n].Name,lpObj->Name) == 0 && this->m_GameMasterInfo[n].Level >= level)
		{
			return 1;
		}
	}

	return 0;
}
