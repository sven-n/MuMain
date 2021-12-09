// MonsterAIMovePath.cpp: implementation of the CMonsterAIMovePath class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIMovePath.h"
#include "MemScript.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIMovePath::CMonsterAIMovePath() // OK
{
	this->DelAllAIMonsterMovePath();
}

CMonsterAIMovePath::~CMonsterAIMovePath() // OK
{

}

void CMonsterAIMovePath::LoadData(char* path) // OK
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

	this->DelAllAIMonsterMovePath();

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
				if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CMonsterAIMovePathInfo info;

					info.m_Type = lpMemScript->GetNumber();

					info.m_MapNumber = lpMemScript->GetAsNumber();

					info.m_PathX = lpMemScript->GetAsNumber();

					info.m_PathY = lpMemScript->GetAsNumber();

					if(this->m_MonsterAIMovePathInfoCount >= 0 && this->m_MonsterAIMovePathInfoCount < MAX_MONSTER_AI_MOVE_PATH_INFO)
					{
						this->m_MonsterAIMovePathInfoArray[this->m_MonsterAIMovePathInfoCount++] = info;
					}
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

	this->m_DataLoad = 1;

	delete lpMemScript;
}

void CMonsterAIMovePath::DelAllAIMonsterMovePath() // OK
{
	this->m_DataLoad = 0;

	for(int n=0;n < MAX_MONSTER_AI_MOVE_PATH_INFO;n++)
	{
		this->m_MonsterAIMovePathInfoArray[n].Reset();
	}

	this->m_MonsterAIMovePathInfoCount = 0;
}

void CMonsterAIMovePath::GetMonsterNearestPath(int sx,int sy,int tx,int ty,int* ox,int* oy,int NearestDistance) // OK
{
	for(int n=0;n < this->m_MonsterAIMovePathInfoCount;n++)
	{
		if(sqrt(pow(((float)sx-(float)this->m_MonsterAIMovePathInfoArray[n].m_PathX),2)+pow(((float)sy-(float)this->m_MonsterAIMovePathInfoArray[n].m_PathY),2)) < 10)
		{
			if(sqrt(pow(((float)tx-(float)this->m_MonsterAIMovePathInfoArray[n].m_PathX),2)+pow(((float)ty-(float)this->m_MonsterAIMovePathInfoArray[n].m_PathY),2)) < NearestDistance)
			{
				NearestDistance = (int)sqrt(pow(((float)tx-(float)this->m_MonsterAIMovePathInfoArray[n].m_PathX),2)+pow(((float)ty-(float)this->m_MonsterAIMovePathInfoArray[n].m_PathY),2));
				(*ox) = this->m_MonsterAIMovePathInfoArray[n].m_PathX;
				(*oy) = this->m_MonsterAIMovePathInfoArray[n].m_PathY;
			}
		}
	}
}
