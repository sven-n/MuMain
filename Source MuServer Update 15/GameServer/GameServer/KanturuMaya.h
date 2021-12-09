// KanturuMaya.h: interface for the CKanturuMaya class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CKanturuMaya
{
public:
	CKanturuMaya();
	virtual ~CKanturuMaya();
	void Init();
	void KanturuMayaAct_IceStorm(int rate);
	void KanturuMayaAct_Hands();
	void SetMayaObjIndex(int aIndex);
private:
	int m_MayaObjIndex;
	int m_MonsterKillCount;
	int m_MayaSkillTime;
	int m_IceStormCount;
};
