// SkillEffectMgr.cpp: implementation of the CSkillEffectMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkillEffectMgr.h"
#include "ZzzLodTerrain.h"
#include "CDirection.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"

#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS

CSkillEffectMgr g_SkillEffects;

void MoveEffect( OBJECT *o, int iIndex);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkillEffectMgr::CSkillEffectMgr()
{
	DeleteAllEffects();
}

CSkillEffectMgr::~CSkillEffectMgr()
{

}

OBJECT * CSkillEffectMgr::GetEffect(int iIndex)
{
	assert(iIndex < MAX_SKILL_EFFECTS);

	return m_SkillEffects + iIndex;
}

BOOL CSkillEffectMgr::IsSkillEffect(int Type,vec3_t Position,vec3_t Angle,
				  vec3_t Light,int SubType,OBJECT *Owner,short PKKey,WORD SkillIndex,WORD Skill,WORD SkillSerialNum,
				  float Scale, short int sTargetIndex )
{
	// 플레이어 스킬만
	if (Owner != &Hero->Object)
		return FALSE;

	switch (Type)
	{
	case BITMAP_BOSS_LASER:		// 아쿠아플래쉬 (법사)
	case MODEL_SKILL_BLAST:		// 블래스트 (법사)
	case MODEL_DARK_SCREAM:		// 파이어스크림 (다크로드)
	case BITMAP_SWORD_FORCE:	// 블러드어택 (마검사)
		return TRUE;
	case MODEL_SKILL_INFERNO:	// 인페르노 (법사)
		if(SubType < 2) return TRUE;
		break;
	case MODEL_CIRCLE:			// 헬파이어 (법사)
		if (SubType == 0) return TRUE;
		break;
	case BITMAP_FLAME:			// 불기둥 (법사)
		if ( SubType==0 ) return TRUE;
		break;
	case MODEL_STORM:			// 회오리 (법사)
		if ( SubType==0 ) return TRUE;
		break;

//     case MODEL_ARROW_DOUBLE:
// 		if(SubType==1) return TRUE;
// 		break;
// 	case MODEL_ARROW:
// 		if( SubType!=3 && SubType!=4 ) return TRUE;
// 		break;
// 	case MODEL_ARROW_BEST_CROSSBOW :
// 	case MODEL_ARROW_STEEL:
// 	case MODEL_ARROW_THUNDER:
// 	case MODEL_ARROW_LASER:
// 	case MODEL_ARROW_V:
// 	case MODEL_ARROW_SAW:
// 	case MODEL_ARROW_NATURE:
// 	case MODEL_ARROW_WING:
//     case MODEL_LACEARROW:
// 	case MODEL_ARROW_SPARK:
// 	case MODEL_ARROW_RING:
// 	case MODEL_ARROW_BOMB:
// 	case MODEL_ARROW_DARKSTINGER:
// 	case MODEL_ARROW_GAMBLE:
// 	case MODEL_ARROW_DRILL:
// 		return TRUE;
	}
	return FALSE;
}

OBJECT * CSkillEffectMgr::CreateEffect()
{
#ifdef _VS2008PORTING
	for(int icntEffect=0;icntEffect<MAX_SKILL_EFFECTS;icntEffect++)
#else // _VS2008PORTING
	for(int i=0;i<MAX_SKILL_EFFECTS;i++)
#endif // _VS2008PORTING
	{
#ifdef _VS2008PORTING
		OBJECT *o = &m_SkillEffects[icntEffect];
#else // _VS2008PORTING
		OBJECT *o = &m_SkillEffects[i];
#endif // _VS2008PORTING
		if(!o->Live)
		{
			return o;
		}
	}

	assert(!"스킬이펙트 한계 도달!");
	return &m_SkillEffects[MAX_SKILL_EFFECTS - 1];
}

bool CSkillEffectMgr::DeleteEffect(int Type,OBJECT *Owner, int iSubType)
{
    bool bDelete = false;
	for(int i=0;i<MAX_SKILL_EFFECTS;i++)
	{
		OBJECT *o = &m_SkillEffects[i];
		if(o->Live && o->Type==Type)
		{
			if ( iSubType == -1 || iSubType == o->SubType)
			{
				if(o->Owner==Owner)
                {
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
					EffectDestructor(o);
                    bDelete = true;
#else	// YDG_ADD_SKILL_FLAME_STRIKE
					o->Live = false;
                    bDelete = true;
					o->Owner = NULL;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
                }
			}
		}
	}

    return bDelete;
}

void CSkillEffectMgr::DeleteEffect( int efftype )
{
	for(int i=0;i<MAX_SKILL_EFFECTS;i++)
	{
		OBJECT *o = &m_SkillEffects[i];
		if(o->Live && o->Type==efftype)
		{
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
			EffectDestructor(o);
#else	// YDG_ADD_SKILL_FLAME_STRIKE
			o->Live = false;
			o->Owner = NULL;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
		}
	}
}

void CSkillEffectMgr::DeleteAllEffects()
{
	for(int i=0;i<MAX_SKILL_EFFECTS;i++)
	{
		EffectDestructor(m_SkillEffects + i);
		m_SkillEffects[i].Live = false;
		m_SkillEffects[i].Owner = NULL;
	}
}

bool CSkillEffectMgr::SearchEffect(int iType, OBJECT* pOwner, int iSubType)
{
	for(int i=0; i<MAX_SKILL_EFFECTS; ++i)
	{
		OBJECT *o = &m_SkillEffects[i];
		if(o->Live && o->Type == iType && o->Owner == pOwner)
		{
			if(iSubType == -1 || o->SubType == iSubType)
			{
				return true;
			}
		}
	}

    return false;
}

BOOL CSkillEffectMgr::FindSameEffectOfSameOwner( int iType, OBJECT *pOwner)
{
	for(int i=0;i<MAX_SKILL_EFFECTS;i++)
	{
		OBJECT *o = &m_SkillEffects[i];
		if(o->Live)
		{
			if ( o->Type == iType && o->Owner == pOwner)
			{
				return ( TRUE);
			}
		}
	}

	return ( FALSE);
}

void CSkillEffectMgr::MoveEffects()
{
	for(int i=0;i<MAX_SKILL_EFFECTS;i++)
	{
		OBJECT *o = &m_SkillEffects[i];
		if(o->Live)
		{
			MoveEffect( o, i);
		}
	}
}

#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS