// CustomJewel.cpp: implementation of the CCustomJewel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomMix.h"
#include "CustomWing.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "SetItemType.h"
#include "SocketItemType.h"
#include "Util.h"

CCustomMix gCustomMix;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomMix::CCustomMix() // OK
{
	this->Init();
}

CCustomMix::~CCustomMix() // OK
{

}

void CCustomMix::Init() // OK
{
	this->m_CustomMixInfo.clear();
	this->m_CustomMixItemInfo.clear();
	this->m_CustomMixRewardInfo.clear();
}

void CCustomMix::Load(char* path) // OK
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

	this->Init();

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

					CUSTOM_MIX_INFO info;

					info.Index = lpMemScript->GetNumber();

					info.MixMoney = lpMemScript->GetAsNumber();

					info.SuccessRate[0] = lpMemScript->GetAsNumber();

					info.SuccessRate[1] = lpMemScript->GetAsNumber();

					info.SuccessRate[2] = lpMemScript->GetAsNumber();

					info.SuccessRate[3] = lpMemScript->GetAsNumber();

					this->m_CustomMixInfo.push_back(info);
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_MIX_ITEM_INFO info;

					info.Index = lpMemScript->GetNumber();

					info.Group = lpMemScript->GetAsNumber();

					info.Count = lpMemScript->GetAsNumber();

					info.ItemIndex = lpMemScript->GetAsNumber();

					info.Level = lpMemScript->GetAsNumber();

					info.Option1 = lpMemScript->GetAsNumber();

					info.Option2 = lpMemScript->GetAsNumber();

					info.Option3 = lpMemScript->GetAsNumber();

					info.NewOption = lpMemScript->GetAsNumber();

					info.SetOption = lpMemScript->GetAsNumber();

					this->m_CustomMixItemInfo.push_back(info);
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_MIX_REWARD_INFO info;

					info.Index = lpMemScript->GetNumber();

					info.Group = lpMemScript->GetAsNumber();

					info.BagSpecial = lpMemScript->GetAsNumber();

					this->m_CustomMixRewardInfo.push_back(info);
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

bool CCustomMix::CheckCustomMix(int index) // OK
{
	for(std::vector<CUSTOM_MIX_INFO>::iterator it=this->m_CustomMixInfo.begin();it != this->m_CustomMixInfo.end();it++)
	{
		if(it->Index == index)
		{
			return 1;
		}
	}
	return 0; 
}

int CCustomMix::GetCustomMixSuccessRate(int index,int AccountLevel) // OK
{
	for(std::vector<CUSTOM_MIX_INFO>::iterator it=this->m_CustomMixInfo.begin();it != this->m_CustomMixInfo.end();it++)
	{
		if(it->Index == index)
		{
			return it->SuccessRate[AccountLevel];
		}
	}
	return 0; 
}

int CCustomMix::GetCustomMixMoney(int index) // OK
{
	for(std::vector<CUSTOM_MIX_INFO>::iterator it=this->m_CustomMixInfo.begin();it != this->m_CustomMixInfo.end();it++)
	{
		if(it->Index == index)
		{
			return it->MixMoney;
		}
	}
	return 0; 
}

int CCustomMix::GetCustomMixReward(int index) // OK
{
	for(std::vector<CUSTOM_MIX_REWARD_INFO>::iterator it=this->m_CustomMixRewardInfo.begin();it != this->m_CustomMixRewardInfo.end();it++)
	{
		if(it->Index == index)
		{
			return it->BagSpecial;
		}
	}
	return 0; 
}

bool CCustomMix::CheckItem(LPOBJ lpObj,int index) // OK 
{
	int ItemOK;
	int Qtd;

	for(std::vector<CUSTOM_MIX_ITEM_INFO>::iterator it=this->m_CustomMixItemInfo.begin();it != this->m_CustomMixItemInfo.end();it++)
	{
		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != index)
		{
			continue;
		}

		ItemOK	= 0;
		Qtd		= 0;

		for(int n=0;n < CHAOS_BOX_SIZE;n++)
		{
			if(lpObj->ChaosBox[n].IsItem() == 0)
			{
				continue;
			}

			if( lpObj->ChaosBox[n].m_Index == it->ItemIndex)
			{
				ItemOK	= 0;
				if (it->Level >= 0)
				{
					if(lpObj->ChaosBox[n].m_Level == it->Level)
					{
						ItemOK++;
					}
				}
				else 
				{
					ItemOK++;
				}

				if (it->Option1 >= 0 )
				{
					if (lpObj->ChaosBox[n].m_Option1 == it->Option1)
					{
						ItemOK++;
					}
				}
				else 
				{
					ItemOK++;
				}

				if (it->Option2 >= 0)
				{
					if (lpObj->ChaosBox[n].m_Option2 == it->Option2)
					{
						ItemOK++;
					}
				}
				else 
				{
					ItemOK++;
				}

				if (it->Option3 >= 0)
				{
					if (lpObj->ChaosBox[n].m_Option3 == it->Option3)
					{
						ItemOK++;
					}
				}
				else 
				{
					ItemOK++;
				}

				if (it->NewOption >= 0)
				{
					if (GetNewOptionCount(lpObj->ChaosBox[n].m_NewOption) >= it->NewOption)
					{
						ItemOK++;
					}
				}
				else 
				{
					ItemOK++;
				}

				if (it->SetOption >= 0)
				{
					if (lpObj->ChaosBox[n].IsSetItem())
					{
						ItemOK++;
					}
				}
				else 
				{
					ItemOK++;
				}

				if (ItemOK >= 6)
				{
					Qtd++;
				}

			}
		}

		if (Qtd < it->Count)
		{
			return 0;
		}
	}
	return 1;
}