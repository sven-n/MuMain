// Filter.cpp: implementation of the CFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FilterRename.h"
#include "MemScript.h"
#include "Util.h"

CFilterRename gFilterRename;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFilterRename::CFilterRename() // OK
{
	this->m_count = 0;
}

CFilterRename::~CFilterRename() // OK
{

}

void CFilterRename::Load(char* path) // OK
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

			FILTERRENAME_INFO info;

			memset(&info,0,sizeof(info));

			strcpy_s(info.label,lpMemScript->GetString());

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CFilterRename::SetInfo(FILTERRENAME_INFO info) // OK
{
	if(this->m_count < 0 || this->m_count >= MAX_FILTERRENAME_SYNTAX)
	{
		return;
	}

	this->m_FilterInfo[this->m_count++] = info;
}

bool CFilterRename::CheckFilter(char* text) // OK
{
#if (GAMESERVER_CLIENTE_UPDATE >= 9)
	for(int n=0;n < this->m_count;n++)
	{
		char* temp = text;

		while(true)
		{
			temp = strstr(temp,this->m_FilterInfo[n].label);

			if(temp == 0)
			{
				break;
			}
			else
			{
				return 1;
			}
		}
	}
#endif

	return 0;
}
