// Command.cpp: implementation of the CCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Command.h"
#include "MemScript.h"
#include "Util.h"

CCommand gCommand;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommand::CCommand() // OK
{
	memset(this->m_DefaultCommand,0,sizeof(this->m_DefaultCommand));

	this->m_CommandInfo.clear();
}

CCommand::~CCommand() // OK
{

}

void CCommand::Load(char* path) // OK
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

	this->m_CommandInfo.clear();

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

			COMMAND_LIST info;

			info.Index = lpMemScript->GetNumber();

			strcpy_s(info.Command,lpMemScript->GetAsString());

			info.Enable[0] = lpMemScript->GetAsNumber();

			info.Enable[1] = lpMemScript->GetAsNumber();

			info.Enable[2] = lpMemScript->GetAsNumber();

			info.Enable[3] = lpMemScript->GetAsNumber();

			info.Money[0] = lpMemScript->GetAsNumber();

			info.Money[1] = lpMemScript->GetAsNumber();

			info.Money[2] = lpMemScript->GetAsNumber();

			info.Money[3] = lpMemScript->GetAsNumber();

			info.MinLevel[0] = lpMemScript->GetAsNumber();

			info.MinLevel[1] = lpMemScript->GetAsNumber();

			info.MinLevel[2] = lpMemScript->GetAsNumber();

			info.MinLevel[3] = lpMemScript->GetAsNumber();

			info.MaxLevel[0] = lpMemScript->GetAsNumber();

			info.MaxLevel[1] = lpMemScript->GetAsNumber();

			info.MaxLevel[2] = lpMemScript->GetAsNumber();

			info.MaxLevel[3] = lpMemScript->GetAsNumber();

			info.MinReset[0] = lpMemScript->GetAsNumber();

			info.MinReset[1] = lpMemScript->GetAsNumber();

			info.MinReset[2] = lpMemScript->GetAsNumber();

			info.MinReset[3] = lpMemScript->GetAsNumber();

			info.MaxReset[0] = lpMemScript->GetAsNumber();

			info.MaxReset[1] = lpMemScript->GetAsNumber();

			info.MaxReset[2] = lpMemScript->GetAsNumber();

			info.MaxReset[3] = lpMemScript->GetAsNumber();

			info.Delay = lpMemScript->GetAsNumber();

			info.GameMaster = lpMemScript->GetAsNumber();

			info.Coin1 = lpMemScript->GetAsNumber();

			info.Coin2 = lpMemScript->GetAsNumber();

			info.Coin3 = lpMemScript->GetAsNumber();

			this->m_CommandInfo.insert(std::pair<int,COMMAND_LIST>(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

char* CCommand::GetCommand(int index) // OK
{
	std::map<int,COMMAND_LIST>::iterator it = this->m_CommandInfo.find(index);

	if(it == this->m_CommandInfo.end())
	{
		wsprintf(this->m_DefaultCommand,"Could not find message %d!",index);
		return this->m_DefaultCommand;
	}
	else
	{
		return it->second.Command;
	}
}

bool CCommand::GetInfo(int index,COMMAND_LIST* lpInfo) // OK
{
	std::map<int,COMMAND_LIST>::iterator it = this->m_CommandInfo.find(index);

	if(it == this->m_CommandInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}
}

bool CCommand::GetInfoByName(char* label,COMMAND_LIST* lpInfo) // OK
{
	for(std::map<int,COMMAND_LIST>::iterator it=this->m_CommandInfo.begin();it != this->m_CommandInfo.end();it++)
	{
		if(_stricmp(it->second.Command,label) == 0)
		{
			(*lpInfo) = it->second;
			return 1;
		}
	}
	return 0;
}
