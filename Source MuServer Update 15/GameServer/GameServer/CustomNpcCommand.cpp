//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomNpcCommand.h"
#include "CommandManager.h"
#include "Map.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "NpcTalk.h"
#include "Path.h"
#include "Util.h"

CCustomNpcCommand gCustomNpcCommand;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomNpcCommand::CCustomNpcCommand() // OK
{
	this->m_CustomNpcCommand.clear();
}

CCustomNpcCommand::~CCustomNpcCommand() // OK
{

}

void CCustomNpcCommand::Load(char* path) // OK
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

	this->m_CustomNpcCommand.clear();

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

			NPC_TYPE_INFO info;

			info.Index = lpMemScript->GetNumber();

			info.MonsterClass = lpMemScript->GetAsNumber();

			info.Map = lpMemScript->GetAsNumber();

			info.X = lpMemScript->GetAsNumber();

			info.Y = lpMemScript->GetAsNumber();

			info.Talk = lpMemScript->GetAsNumber();

			strcpy_s(info.Command,lpMemScript->GetAsString());

			this->m_CustomNpcCommand.insert(std::pair<int,NPC_TYPE_INFO>(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CCustomNpcCommand::GetNpcCommand(LPOBJ lpObj,LPOBJ lpNpc) // OK
{
	#if (GAMESERVER_CLIENTE_UPDATE >= 12)
	for(std::map<int,NPC_TYPE_INFO>::iterator it=this->m_CustomNpcCommand.begin();it != this->m_CustomNpcCommand.end();it++)
	{
		if(it->second.MonsterClass == lpNpc->Class && it->second.Map == lpNpc->Map && it->second.X == lpNpc->X && it->second.Y == lpNpc->Y)
		{
			if (it->second.Talk == 1)
			{
				CommandSelect(lpObj,it->second.Command,lpNpc->Index);
			}
			else
			{
				CommandSelect(lpObj,it->second.Command,-1);
			}
			return 1;
		}

	}
#endif
	return 0;
}