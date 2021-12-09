// SummonScroll.cpp: implementation of the CSummonScroll class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SummonScroll.h"
#include "Map.h"
#include "MemScript.h"
#include "Monster.h"
#include "RandomManager.h"
#include "Util.h"

CSummonScroll gSummonScroll;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSummonScroll::CSummonScroll() // OK
{
	#if(GAMESERVER_UPDATE>=801)

	this->m_SummonScrollInfo.clear();

	this->m_SummonScrollMonsterInfo.clear();

	#endif
}

CSummonScroll::~CSummonScroll() // OK
{

}

void CSummonScroll::Load(char* path) // OK
{
	#if(GAMESERVER_UPDATE>=801)

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

	this->m_SummonScrollInfo.clear();

	this->m_SummonScrollMonsterInfo.clear();

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

					SUMMON_SCROLL_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.ItemIndex = lpMemScript->GetAsNumber();

					this->m_SummonScrollInfo.push_back(info);
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					SUMMON_SCROLL_MONSTER_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.Group = lpMemScript->GetAsNumber();

					info.MonsterClass = lpMemScript->GetAsNumber();

					info.CreateRate = lpMemScript->GetAsNumber();

					this->m_SummonScrollMonsterInfo.push_back(info);
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

	#endif
}

bool CSummonScroll::CheckSummonScroll(int ItemIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	for(std::vector<SUMMON_SCROLL_INFO>::iterator it=this->m_SummonScrollInfo.begin();it != this->m_SummonScrollInfo.end();it++)
	{
		if(it->ItemIndex == ItemIndex)
		{
			return 1;
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CSummonScroll::GetSummonScrollInfo(int ItemIndex,SUMMON_SCROLL_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	for(std::vector<SUMMON_SCROLL_INFO>::iterator it=this->m_SummonScrollInfo.begin();it != this->m_SummonScrollInfo.end();it++)
	{
		if(it->ItemIndex == ItemIndex)
		{
			(*lpInfo) = (*it);
			return 1;
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CSummonScroll::CreateSummonScrollMonster(int aIndex,int ItemIndex,int map,int x,int y) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	SUMMON_SCROLL_INFO SummonScrollInfo;

	if(this->GetSummonScrollInfo(ItemIndex,&SummonScrollInfo) == 0)
	{
		return 0;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0 || gMap[lpObj->Map].CheckAttr(x,y,1) != 0)
	{
		return 0;
	}

	if(DS_MAP_RANGE(lpObj->Map) != 0 || BC_MAP_RANGE(lpObj->Map) != 0 || CC_MAP_RANGE(lpObj->Map) != 0 || IT_MAP_RANGE(lpObj->Map) != 0 || DA_MAP_RANGE(lpObj->Map) != 0 || DG_MAP_RANGE(lpObj->Map) != 0 || IG_MAP_RANGE(lpObj->Map) != 0)
	{
		return 0;
	}

	SUMMON_SCROLL_MONSTER_INFO* lpSummonScrollInfo = 0;

	CRandomManager RandomManager[MAX_SUMMON_SCROLL_MONSTER_GROUP];

	for(std::vector<SUMMON_SCROLL_MONSTER_INFO>::iterator it=this->m_SummonScrollMonsterInfo.begin();it != this->m_SummonScrollMonsterInfo.end();it++)
	{
		if(it->Index == SummonScrollInfo.Index)
		{
			RandomManager[it->Group].AddElement((int)(&(*it)),it->CreateRate);
		}
	}

	for(int n=0;n < MAX_SUMMON_SCROLL_MONSTER_GROUP;n++)
	{
		if(RandomManager[n].GetRandomElement((int*)&lpSummonScrollInfo) != 0)
		{
			int index = gObjAddMonster(map);

			if(OBJECT_RANGE(index) == 0)
			{
				continue;
			}

			LPOBJ lpMonster = &gObj[index];

			int px = x;
			int py = y;

			if(gObjGetRandomFreeLocation(map,&px,&py,3,3,50) == 0)
			{
				continue;
			}

			lpMonster->PosNum = -1;
			lpMonster->X = px;
			lpMonster->Y = py;
			lpMonster->TX = px;
			lpMonster->TY = py;
			lpMonster->OldX = px;
			lpMonster->OldY = py;
			lpMonster->StartX = px;
			lpMonster->StartY = py;
			lpMonster->Dir = 1;
			lpMonster->Map = map;
			lpMonster->MonsterDeleteTime = GetTickCount()+1800000;

			if(gObjSetMonster(index,lpSummonScrollInfo->MonsterClass) == 0)
			{
				gObjDel(index);
				continue;
			}
		}
	}

	return 1;

	#else

	return 0;

	#endif
}
