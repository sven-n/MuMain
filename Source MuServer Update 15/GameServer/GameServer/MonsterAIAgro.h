// MonsterAIAgro.h: interface for the CMonsterAIAgro class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_MONSTER_AI_AGRO 20

class CMonsterAIAgroInfo
{
public:
	CMonsterAIAgroInfo() // OK
	{
		this->Reset();
	}

	void Reset() // OK
	{
		this->m_UserIndex = -1;
		this->m_AgroValue = -1;
	}

	int GetUserIndex() // OK
	{
		return this->m_UserIndex;
	}

	int GetAgroValue() // OK
	{
		return this->m_AgroValue;
	}

	void SetAgro(int aIndex,int value) // OK
	{
		this->m_UserIndex = aIndex;
		this->m_AgroValue = value;
	}

	int IncAgro(int value) // OK
	{
		this->m_AgroValue += value;
		return this->m_AgroValue;
	}

	int DecAgro(int value) // OK
	{
		this->m_AgroValue -= value;
		return this->m_AgroValue;
	}
private:
	int m_UserIndex;
	int m_AgroValue;
};

class CMonsterAIAgro
{
public:
	CMonsterAIAgro();
	virtual ~CMonsterAIAgro();
	void ResetAll();
	bool SetAgro(int aIndex,int value);
	bool DelAgro(int aIndex);
	int GetAgro(int aIndex);
	int IncAgro(int aIndex,int value);
	int DecAgro(int aIndex,int value);
	void DecAllAgro(int value);
	int GetMaxAgroUserIndex(int aIndex);
private:
	CMonsterAIAgroInfo m_Agro[MAX_MONSTER_AI_AGRO];
};
