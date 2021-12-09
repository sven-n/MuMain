// CustomCommandDescription.cpp: implementation of the CCustomCommandDescription class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "CustomCommandDescription.h"
#include "Log.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "Util.h"

CCustomCommandDescription gCustomCommandDescription;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomCommandDescription::CCustomCommandDescription() // OK
{
	this->m_CustomCommandDescriptionInfo.clear();
}

CCustomCommandDescription::~CCustomCommandDescription() // OK
{

}

void CCustomCommandDescription::Load(char* path) // OK
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

	this->m_CustomCommandDescriptionInfo.clear();

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

			CUSTOMCOMMANDDESCRIPTION_INFO info;

			info.Index = lpMemScript->GetNumber();

			strcpy_s(info.Commmand,lpMemScript->GetAsString());

			strcpy_s(info.Description,lpMemScript->GetAsString());

			this->m_CustomCommandDescriptionInfo.insert(std::pair<int,CUSTOMCOMMANDDESCRIPTION_INFO>(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CCustomCommandDescription::GetInfo(int index,CUSTOMCOMMANDDESCRIPTION_INFO* lpInfo) // OK
{
	std::map<int,CUSTOMCOMMANDDESCRIPTION_INFO>::iterator it = this->m_CustomCommandDescriptionInfo.find(index);

	if(it == this->m_CustomCommandDescriptionInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}
}

bool CCustomCommandDescription::GetInfoByName(LPOBJ lpObj, char* message) // OK
{
#if GAMESERVER_CLIENTE_UPDATE >= 7
	char command[32] = {0};

	memset(command,0,sizeof(command));

	gCommandManager.GetString(message,command,sizeof(command),0);

	for(std::map<int,CUSTOMCOMMANDDESCRIPTION_INFO>::iterator it=this->m_CustomCommandDescriptionInfo.begin();it != this->m_CustomCommandDescriptionInfo.end();it++)
	{
		if(_stricmp(it->second.Commmand,command) == 0)
		{
			gNotice.GCNoticeSend(lpObj->Index,0,0,0,0,0,0,it->second.Description,lpObj->Name);
			return 1;
		}
	}
#endif
	return 0;
}
