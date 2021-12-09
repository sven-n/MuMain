// AllowableIpList.cpp: implementation of the CAllowableIpList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AllowableIpList.h"
#include "MemScript.h"
#include "Util.h"

CAllowableIpList gAllowableIpList;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAllowableIpList::CAllowableIpList() // OK
{
	this->m_AllowableIpInfo.clear();
}

CAllowableIpList::~CAllowableIpList() // OK
{

}

void CAllowableIpList::Load(char* path) // OK
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

	this->m_AllowableIpInfo.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ALLOWABLE_IP_INFO info;

					strcpy_s(info.IpAddr,lpMemScript->GetString());

					this->m_AllowableIpInfo.insert(std::pair<std::string,ALLOWABLE_IP_INFO>(std::string(info.IpAddr),info));
				}
				else
				{
					break;
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CAllowableIpList::CheckAllowableIp(char* ip) // OK
{
	std::map<std::string,ALLOWABLE_IP_INFO>::iterator it = this->m_AllowableIpInfo.find(std::string(ip));

	if(it == this->m_AllowableIpInfo.end())
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
