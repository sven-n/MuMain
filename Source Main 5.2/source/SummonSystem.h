#pragma once

#include "zzzinfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include <map>

class CSummonSystem
{
public:
    CSummonSystem();
    virtual ~CSummonSystem();

    void MoveEquipEffect(CHARACTER* pCharacter, int iItemType, int iItemLevel, int iItemOption1);
    void RemoveEquipEffects(CHARACTER* pCharacter);
    void RemoveEquipEffect_Summon(CHARACTER* pCharacter);

    BOOL SendRequestSummonSkill(int iSkill, CHARACTER* pCharacter, OBJECT* pObject);
    void CastSummonSkill(int iSkill, CHARACTER* pCharacter, OBJECT* pObject, int iTargetPos_X, int iTargetPos_Y);

    void CreateDamageOfTimeEffect(int iSkill, OBJECT* pObject);
    void RemoveDamageOfTimeEffect(int iSkill, OBJECT* pObject);
    void RemoveAllDamageOfTimeEffect(OBJECT* pObject);

protected:
    void CreateEquipEffect_WristRing(CHARACTER* pCharacter, int iItemType, int iItemLevel, int iItemOption1);
    void RemoveEquipEffect_WristRing(CHARACTER* pCharacter);
    void CreateEquipEffect_Summon(CHARACTER* pCharacter, int iItemType, int iItemLevel, int iItemOption1);

    void CreateCastingEffect(vec3_t vPosition, vec3_t vAngle, int iSubType);

    void CreateSummonObject(int iSkill, CHARACTER* pCharacter, OBJECT* pObject, float fTargetPos_X, float fTargetPos_Y);
    void SetPlayerSummon(CHARACTER* pCharacter, OBJECT* pObject);

protected:
    std::map<SHORT, BYTE> m_EquipEffectRandom;
};

extern CSummonSystem g_SummonSystem;
