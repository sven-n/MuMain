// ItemStack.cpp: implementation of the CItemStack class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemStack.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "Util.h"

CItemStack gItemStack;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemStack::CItemStack() // OK
{
	this->m_ItemStackInfo.clear();
}

CItemStack::~CItemStack() // OK
{

}

void CItemStack::Load(char* path) // OK
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

	this->m_ItemStackInfo.clear();

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

			ITEM_STACK_INFO info;

			info.Index = lpMemScript->GetNumber();

			info.MaxStack = lpMemScript->GetAsNumber();

			info.CreateItemIndex = lpMemScript->GetAsNumber();

			this->m_ItemStackInfo.insert(std::pair<int,ITEM_STACK_INFO>(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

int CItemStack::GetItemMaxStack(int index) // OK
{
	std::map<int,ITEM_STACK_INFO>::iterator it = this->m_ItemStackInfo.find(index);

	if(it == this->m_ItemStackInfo.end())
	{
		return 0;
	}
	else
	{
		return it->second.MaxStack;
	}
}

int CItemStack::GetCreateItemIndex(int index) // OK
{
	std::map<int,ITEM_STACK_INFO>::iterator it = this->m_ItemStackInfo.find(index);

	if(it == this->m_ItemStackInfo.end())
	{
		return -1;
	}
	else
	{
		return it->second.CreateItemIndex;
	}
}
