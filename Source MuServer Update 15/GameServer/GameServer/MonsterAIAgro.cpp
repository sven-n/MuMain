// MonsterAIAgro.cpp: implementation of the CMonsterAIAgro class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIAgro.h"
#include "Map.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIAgro::CMonsterAIAgro() // OK
{
	this->ResetAll();
}

CMonsterAIAgro::~CMonsterAIAgro() // OK
{

}

void CMonsterAIAgro::ResetAll() // OK
{
	for(int n=0;n < MAX_MONSTER_AI_AGRO;n++)
	{
		this->m_Agro[n].Reset();
	}
}

bool CMonsterAIAgro::SetAgro(int aIndex,int value) // OK
{
	for(int n=0;n < MAX_MONSTER_AI_AGRO;n++)
	{
		if(OBJECT_RANGE(this->m_Agro[n].GetUserIndex()) == 0)
		{
			this->m_Agro[n].SetAgro(aIndex,value);
			return 1;
		}
	}

	return 0;
}

bool CMonsterAIAgro::DelAgro(int aIndex) // OK
{
	for(int n=0;n < MAX_MONSTER_AI_AGRO;n++)
	{
		if(this->m_Agro[n].GetUserIndex() == aIndex)
		{
			this->m_Agro[n].Reset();
			return 1;
		}
	}

	return 0;
}

int CMonsterAIAgro::GetAgro(int aIndex) // OK
{
	for(int n=0;n < MAX_MONSTER_AI_AGRO;n++)
	{
		if(this->m_Agro[n].GetUserIndex() == aIndex)
		{
			return this->m_Agro[n].GetAgroValue();
		}
	}

	return -1;
}

int CMonsterAIAgro::IncAgro(int aIndex,int value) // OK
{
	for(int n=0;n < MAX_MONSTER_AI_AGRO;n++)
	{
		if(this->m_Agro[n].GetUserIndex() == aIndex)
		{
			return this->m_Agro[n].IncAgro(value);
		}
	}

	return -1;
}

int CMonsterAIAgro::DecAgro(int aIndex,int value) // OK
{
	for(int n=0;n < MAX_MONSTER_AI_AGRO;n++)
	{
		if(this->m_Agro[n].GetUserIndex() == aIndex)
		{
			return this->m_Agro[n].DecAgro(value);
		}
	}

	return -1;
}

void CMonsterAIAgro::DecAllAgro(int value) // OK
{
	for(int n=0;n < MAX_MONSTER_AI_AGRO;n++)
	{
		if(OBJECT_RANGE(this->m_Agro[n].GetUserIndex()) == 0)
		{
			this->m_Agro[n].Reset();
		}
		else
		{
			this->m_Agro[n].DecAgro(value);
		}
	}
}

int CMonsterAIAgro::GetMaxAgroUserIndex(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	int MaxAgroUserIndex = -1;
	int MaxAgroUserValue = -1;

	for(int n=0;n < MAX_MONSTER_AI_AGRO;n++)
	{
		if(OBJECT_RANGE(this->m_Agro[n].GetUserIndex()) == 0)
		{
			continue;
		}

		LPOBJ lpTarget = &gObj[this->m_Agro[n].GetUserIndex()];

		if(gObjIsConnected(lpTarget->Index) == 0)
		{
			this->m_Agro[n].Reset();
			continue;
		}

		if(lpTarget->Live == 0)
		{
			this->m_Agro[n].Reset();
			continue;
		}

		if(lpTarget->X < (lpObj->X-7) || lpTarget->X > (lpObj->X+7) || lpTarget->Y < (lpObj->Y-7) || lpTarget->Y > (lpObj->Y+7))
		{
			continue;
		}

		if(gMap[lpTarget->Map].CheckAttr(lpTarget->X,lpTarget->Y,1) != 0)
		{
			continue;
		}

		if(this->m_Agro[n].GetAgroValue() > MaxAgroUserValue)
		{
			MaxAgroUserIndex = this->m_Agro[n].GetUserIndex();
			MaxAgroUserValue = this->m_Agro[n].GetAgroValue();
		}
	}

	return MaxAgroUserIndex;
}
