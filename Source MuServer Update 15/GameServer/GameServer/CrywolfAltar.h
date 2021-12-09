// CrywolfAltar.h: interface for the CCrywolfAltar class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_CRYWOLF_ALTAR 5
#define MAX_CRYWOLF_ALTAR_CONTRACT 2

class CCrywolfAltarInfo
{
public:
	CCrywolfAltarInfo() // OK
	{
		this->Reset();
	}

	void Reset() // OK
	{
		this->m_AltarState = 0;
		this->m_UserIndex = -1;
		this->m_AltarIndex = -1;
		this->m_ContractCount = 0;
		this->m_AppliedContractTime = 0;
		this->m_ValidContractTime = 0;
		this->m_LastValidContractTime = 0;
	}

	int GetRemainContractCount() // OK
	{
		return (((MAX_CRYWOLF_ALTAR_CONTRACT-this->m_ContractCount)<0)?0:(MAX_CRYWOLF_ALTAR_CONTRACT-this->m_ContractCount));
	}

	void SetAltarState(int state) // OK
	{
		this->m_AltarState = state;
	}

	int GetAltarState() // OK
	{
		return this->m_AltarState;
	}

	void SetAltarIndex(int AltarIndex) // OK
	{
		this->m_AltarIndex = AltarIndex;
	}
public:
	int m_AltarState;
	int m_AltarIndex;
	int m_UserIndex;
	int m_ContractCount;
	int m_AppliedContractTime;
	int m_ValidContractTime;
	int m_LastValidContractTime;
};

class CCrywolfAltar
{
public:
	CCrywolfAltar();
	virtual ~CCrywolfAltar();
	void CrywolfAltarAct(int aIndex);
	void ResetAllAltar();
	void ResetAltar(int Class);
	void ResetAltarUserIndex(int AltarObjIndex,int Class);
	void SetAllAltarObjectIndex();
	void SetAllAltarViewState(int AltarState);
	void SetAltarObjIndex(int Class,int ObjIndex);
	bool SetAltarUserIndex(int AltarObjIndex,int Class,int UserIndex);
	void SetAltarViewState(int Class,int AltarState);
	void SetAltarValidContract(int AltarObjIndex,int Class);
	int GetAltarState(int Class);
	int GetAltarNumber(int Class);
	int GetAltarUserIndex(int Class);
	int GetAltarRemainContractCount(int Class);
	int GetPriestHPSum();
	int GetPriestMaxHPSum();
	int GetContractedAltarCount();
private:
	CCrywolfAltarInfo m_AltarInfo[MAX_CRYWOLF_ALTAR];
};

extern CCrywolfAltar gCrywolfAltar;
