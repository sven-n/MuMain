//////////////////////////////////////////////////////////////////////
// SkillEffectMgr.cpp: implementation of the CSkillEffectMgr class.
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

CSkillEffectMgr g_SkillEffects;

void MoveEffect(OBJECT* o, int iIndex);

CSkillEffectMgr::CSkillEffectMgr()
{
    DeleteAllEffects();
}

CSkillEffectMgr::~CSkillEffectMgr()
{
}

OBJECT* CSkillEffectMgr::GetEffect(int iIndex)
{
    assert(iIndex < MAX_SKILL_EFFECTS);

    return m_SkillEffects + iIndex;
}

BOOL CSkillEffectMgr::IsSkillEffect(int Type, vec3_t Position, vec3_t Angle,
    vec3_t Light, int SubType, OBJECT* Owner, short PKKey, WORD SkillIndex, WORD Skill, WORD SkillSerialNum,
    float Scale, short int sTargetIndex)
{
    if (Owner != &Hero->Object)
        return FALSE;

    switch (Type)
    {
    case BITMAP_BOSS_LASER:
    case MODEL_SKILL_BLAST:
    case MODEL_DARK_SCREAM:
    case BITMAP_SWORD_FORCE:
        return TRUE;
    case MODEL_SKILL_INFERNO:
        if (SubType < 2) return TRUE;
        break;
    case MODEL_CIRCLE:
        if (SubType == 0) return TRUE;
        break;
    case BITMAP_FLAME:
        if (SubType == 0) return TRUE;
        break;
    case MODEL_STORM:
        if (SubType == 0) return TRUE;
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

OBJECT* CSkillEffectMgr::CreateEffect()
{
    for (int icntEffect = 0; icntEffect < MAX_SKILL_EFFECTS; icntEffect++)
    {
        OBJECT* o = &m_SkillEffects[icntEffect];
        if (!o->Live)
        {
            return o;
        }
    }

    assert(!"스킬이펙트 한계 도달!");
    return &m_SkillEffects[MAX_SKILL_EFFECTS - 1];
}

bool CSkillEffectMgr::DeleteEffect(int Type, OBJECT* Owner, int iSubType)
{
    bool bDelete = false;
    for (int i = 0; i < MAX_SKILL_EFFECTS; i++)
    {
        OBJECT* o = &m_SkillEffects[i];
        if (o->Live && o->Type == Type)
        {
            if (iSubType == -1 || iSubType == o->SubType)
            {
                if (o->Owner == Owner)
                {
                    EffectDestructor(o);
                    bDelete = true;
                }
            }
        }
    }

    return bDelete;
}

void CSkillEffectMgr::DeleteEffect(int efftype)
{
    for (int i = 0; i < MAX_SKILL_EFFECTS; i++)
    {
        OBJECT* o = &m_SkillEffects[i];
        if (o->Live && o->Type == efftype)
        {
            EffectDestructor(o);
        }
    }
}

void CSkillEffectMgr::DeleteAllEffects()
{
    for (int i = 0; i < MAX_SKILL_EFFECTS; i++)
    {
        EffectDestructor(m_SkillEffects + i);
        m_SkillEffects[i].Live = false;
        m_SkillEffects[i].Owner = NULL;
    }
}

bool CSkillEffectMgr::SearchEffect(int iType, OBJECT* pOwner, int iSubType)
{
    for (int i = 0; i < MAX_SKILL_EFFECTS; ++i)
    {
        OBJECT* o = &m_SkillEffects[i];
        if (o->Live && o->Type == iType && o->Owner == pOwner)
        {
            if (iSubType == -1 || o->SubType == iSubType)
            {
                return true;
            }
        }
    }

    return false;
}

BOOL CSkillEffectMgr::FindSameEffectOfSameOwner(int iType, OBJECT* pOwner)
{
    for (int i = 0; i < MAX_SKILL_EFFECTS; i++)
    {
        OBJECT* o = &m_SkillEffects[i];
        if (o->Live)
        {
            if (o->Type == iType && o->Owner == pOwner)
            {
                return (TRUE);
            }
        }
    }

    return (FALSE);
}

void CSkillEffectMgr::MoveEffects()
{
    for (int i = 0; i < MAX_SKILL_EFFECTS; i++)
    {
        OBJECT* o = &m_SkillEffects[i];
        if (o->Live)
        {
            MoveEffect(o, i);
        }
    }
}