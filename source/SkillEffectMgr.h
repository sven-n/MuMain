// SkillEffectMgr.h: interface for the CSkillEffectMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKILLEFFECTMGR_H__F6B84A39_B629_44F1_80C7_1CCD996154D6__INCLUDED_)
#define AFX_SKILLEFFECTMGR_H__F6B84A39_B629_44F1_80C7_1CCD996154D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS

class CSkillEffectMgr  
{
public:
	CSkillEffectMgr();
	virtual ~CSkillEffectMgr();

	int GetSize() { return MAX_SKILL_EFFECTS; }
	OBJECT * GetEffect(int iIndex);

	BOOL IsSkillEffect(int Type,vec3_t Position,vec3_t Angle,vec3_t Light,int SubType=0,OBJECT *Target=NULL,short PKKey=-1,
				  WORD SkillIndex=0,WORD Skill=0,WORD SkillSerialNum=0, float Scale = 0.0f, short int sTargetIndex = -1);
	OBJECT * CreateEffect();

	bool DeleteEffect(int Type,OBJECT *Owner, int iSubType = -1);
	void DeleteEffect( int efftype );
	void DeleteAllEffects();

	bool SearchEffect(int iType, OBJECT* pOwner, int iSubType = -1);
	BOOL FindSameEffectOfSameOwner( int iType, OBJECT *pOwner);

	void MoveEffects();

protected:
	OBJECT m_SkillEffects[MAX_SKILL_EFFECTS];		// 스킬 판정에 사용하는 이펙트들 (블러드어택 등)
};

extern CSkillEffectMgr g_SkillEffects;

#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS

#endif // !defined(AFX_SKILLEFFECTMGR_H__F6B84A39_B629_44F1_80C7_1CCD996154D6__INCLUDED_)
