// KanturuMonsterMng.cpp: implementation of the CKanturuMonsterMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KanturuMonsterMng.h"
#include "Map.h"
#include "MemScript.h"
#include "Monster.h"
#include "User.h"
#include "Util.h"

CKanturuMonsterMng gKanturuMonsterMng;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKanturuMonsterMng::CKanturuMonsterMng() // OK
{
	this->ResetLoadData();

	this->ResetRegenMonsterObjData();
}

CKanturuMonsterMng::~CKanturuMonsterMng() // OK
{

}

void CKanturuMonsterMng::ResetLoadData() // OK
{
	this->m_MaxMonsterCount = 0;

	for(int n=0;n < MAX_KANTURU_MONSTER;n++)
	{
		memset(&this->m_SetBaseInfo[n],-1,sizeof(this->m_SetBaseInfo[n]));
	}

	this->m_MayaObjIndex = -1;

	this->m_FileDataLoad = 0;
}

void CKanturuMonsterMng::Load(char* path) // OK
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

	this->ResetLoadData();

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

					KANTURU_MONSTER_SETBASE_INFO info;

					memset(&info,0,sizeof(info));

					info.Group = lpMemScript->GetNumber();

					info.Type = lpMemScript->GetAsNumber();

					info.Map = lpMemScript->GetAsNumber();

					info.Dis = lpMemScript->GetAsNumber();

					info.X = lpMemScript->GetAsNumber();

					info.Y = lpMemScript->GetAsNumber();

					info.Dir = lpMemScript->GetAsNumber();

					this->SetMonsterSetBaseInfo(info);
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

	this->m_FileDataLoad = 1;

	delete lpMemScript;
}

void CKanturuMonsterMng::SetMonsterSetBaseInfo(KANTURU_MONSTER_SETBASE_INFO info) // OK
{
	if(this->m_MaxMonsterCount < 0 || this->m_MaxMonsterCount >= MAX_KANTURU_MONSTER)
	{
		return;
	}

	this->m_SetBaseInfo[this->m_MaxMonsterCount++] = info;
}

void CKanturuMonsterMng::MonsterDie(int aIndex) // OK
{
	for(int n=0;n < this->m_KanturuMonster.GetCount();n++)
	{
		if(this->m_KanturuMonster.m_ObjIndex[n] == aIndex)
		{
			this->m_AliveMonsterCount--;
			return;
		}
	}
}

void CKanturuMonsterMng::ResetRegenMonsterObjData() // OK
{
	if(OBJECT_RANGE(this->m_MayaObjIndex) != 0)
	{
		gObjDel(this->m_MayaObjIndex);
		this->m_MayaObjIndex = -1;
	}

	for(int n=0;n < this->m_KanturuMonster.GetCount();n++)
	{
		if(OBJECT_RANGE(this->m_KanturuMonster.m_ObjIndex[n]) != 0)
		{
			gObjDel(this->m_KanturuMonster.m_ObjIndex[n]);
		}
	}

	this->m_KanturuMonster.Reset();

	this->m_MaxRegenMonsterCount = 0;

	this->m_AliveMonsterCount = 0;
}

int CKanturuMonsterMng::SetKanturuMonster(int GroupNumber) // OK
{
	int RegenMonsterCount = 0;

	for(int n=0;n < MAX_KANTURU_MONSTER;n++)
	{
		if(this->m_SetBaseInfo[n].Group != GroupNumber || this->m_SetBaseInfo[n].Type == 0)
		{
			continue;
		}

		int index = gObjAddMonster(this->m_SetBaseInfo[n].Map);

		if(OBJECT_RANGE(index) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[index];

		lpObj->PosNum = -1;
		lpObj->X = this->m_SetBaseInfo[n].X;
		lpObj->Y = this->m_SetBaseInfo[n].Y;
		lpObj->Map = this->m_SetBaseInfo[n].Map;

		if(this->GetPosition(n,lpObj->Map,&lpObj->X,&lpObj->Y) == 0)
		{
			gObjDel(index);
			continue;
		}

		lpObj->TX = lpObj->X;
		lpObj->TY = lpObj->Y;
		lpObj->OldX = lpObj->X;
		lpObj->OldY = lpObj->Y;
		lpObj->StartX = (BYTE)lpObj->X;
		lpObj->StartY = (BYTE)lpObj->Y;
		lpObj->DieRegen = 0;
		lpObj->MoveRange = this->m_SetBaseInfo[n].Dis;

		if(this->m_SetBaseInfo[n].Dir == 0xFF)
		{
			lpObj->Dir = GetLargeRand()%8;
		}
		else
		{
			lpObj->Dir = this->m_SetBaseInfo[n].Dir;
		}

		if(gObjSetMonster(index,this->m_SetBaseInfo[n].Type) == 0)
		{
			gObjDel(index);
			continue;
		}

		if(this->m_SetBaseInfo[n].Type == 364)
		{
			this->m_MayaObjIndex = index;
			continue;
		}
		else
		{
			RegenMonsterCount++;
			this->m_KanturuMonster.AddObj(index);
			this->m_MaxMonsterCount++;
			this->m_AliveMonsterCount++;
			continue;
		}
	}

	return RegenMonsterCount;
}

int CKanturuMonsterMng::GetPosition(int index,short map,short* x,short* y) // OK
{
	if(index < 0 || index >= MAX_KANTURU_MONSTER)
	{
		return 0;
	}

	if(gMap[map].CheckAttr(this->m_SetBaseInfo[index].X,this->m_SetBaseInfo[index].Y,1) == 0 && gMap[map].CheckAttr(this->m_SetBaseInfo[index].X,this->m_SetBaseInfo[index].Y,4) == 0 && gMap[map].CheckAttr(this->m_SetBaseInfo[index].X,this->m_SetBaseInfo[index].Y,8) == 0)
	{
		(*x) = this->m_SetBaseInfo[index].X;
		(*y) = this->m_SetBaseInfo[index].Y;
		return 1;
	}

	for(int n=0;n < 100;n++)
	{
		int px = (this->m_SetBaseInfo[index].X-3)+(GetLargeRand()%7);
		int py = (this->m_SetBaseInfo[index].Y-3)+(GetLargeRand()%7);

		if(gMap[map].CheckAttr(px,py,1) == 0 && gMap[map].CheckAttr(px,py,4) == 0 && gMap[map].CheckAttr(px,py,8) == 0)
		{
			(*x) = px;
			(*y) = py;
			return 1;
		}
	}

	return 0;
}

int CKanturuMonsterMng::GetMayaObjIndex() // OK
{
	return this->m_MayaObjIndex;
}

int CKanturuMonsterMng::GetAliveMonsterCount() // OK
{
	return this->m_AliveMonsterCount;
}

int CKanturuMonsterMng::IsExistAliveMonster() // OK
{
	return ((this->m_AliveMonsterCount<=0)?0:1);
}
