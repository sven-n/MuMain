#pragma once

#include "Render/Models/DrawLook.h"

//. soyaviper

namespace M34CryingWolf2nd
{
    bool IsCyringWolf2nd();
    //	bool IsInCryingWolfSection(const vec3_t Position);

    bool CreateCryingWolf2ndObject(OBJECT* pObject);
    bool MoveCryingWolf2ndObject(OBJECT* pObject);
    bool UpdateCryingWolf2ndObjectEffects(OBJECT* pObject, BMD* pModel);
    bool RenderCryingWolf2ndObjectMesh(const OBJECT* pObject, DrawLook& look, BMD* pModel);

    bool MoveCryingWolf2ndMonsterVisual(OBJECT* pObject, BMD* pModel);
    bool RenderCryingWolf2ndMonsterObjectMesh(const OBJECT* pObject, DrawLook& look, BMD* pModel);
    bool RenderCryingWolf2ndMonsterVisual(const CHARACTER* pCharacter, const OBJECT* pObject, BMD* pModel);

    void MoveCryingWolf2ndBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);

    bool AttackEffectCryingWolf2ndMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
    //	bool SetCurrentActionCryingWolf2ndMonster(CHARACTER* pCharacter, OBJECT* pObject);
}
