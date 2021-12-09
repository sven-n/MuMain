// CustomDeathMessage.cpp: implementation of the CGate class.//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "CommandManager.h"
#include "CustomDeathMessage.h"
#include "Log.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "ServerInfo.h"
#include "Util.h"


CustomDeathMessage gCustomDeathMessage;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CustomDeathMessage::CustomDeathMessage() // OK
{
	this->m_CustomDeathMessage.clear();
}


CustomDeathMessage::~CustomDeathMessage() // OK
{


}


void CustomDeathMessage::Load(char* path) // OK
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


	this->m_CustomDeathMessage.clear();


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


		CUSTOMDEATHMESSAGE_INFO info;


		info.Index = lpMemScript->GetNumber();


		strcpy_s(info.Text,lpMemScript->GetAsString());


	this->m_CustomDeathMessage.insert(std::pair<int,CUSTOMDEATHMESSAGE_INFO>(info.Index,info));
	}
	}
	catch(...)
	{
	ErrorMessageBox(lpMemScript->GetLastError());
	}


	delete lpMemScript;
}


void CustomDeathMessage::GetDeathText(LPOBJ lpTarget, LPOBJ lpObj, int index) // OK
{
#if GAMESERVER_CLIENTE_UPDATE >= 4

	if(gServerInfo.m_CustomDeathMessageSwitch == 0)
	{
		return;
	}

	CUSTOMDEATHMESSAGE_INFO CustomDM;

	if(this->GetInfo(index,&CustomDM) == 0)
	{
		return;
	}

	GCChatTargetSend(lpTarget,lpObj->Index,CustomDM.Text);
#endif
}


bool CustomDeathMessage::GetInfo(int index,CUSTOMDEATHMESSAGE_INFO* lpInfo) // OK
{

	std::map<int,CUSTOMDEATHMESSAGE_INFO>::iterator it = this->m_CustomDeathMessage.find(index);

	if(it == this->m_CustomDeathMessage.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}
} 