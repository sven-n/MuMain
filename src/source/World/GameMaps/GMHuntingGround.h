#ifndef _GMHUNTINGGROUND_H_
#define _GMHUNTINGGROUND_H_

//. soyaviper

namespace M31HuntingGround {		//. 공성 사냥터
    bool IsInHuntingGround();
    bool IsInHuntingGroundSection2(const vec3_t Position);

    //. 오브젝트
    bool CreateHuntingGroundObject(OBJECT* pObject);
    bool MoveHuntingGroundObject(OBJECT* pObject);
    bool UpdateHuntingGroundObjectEffects(OBJECT* pObject, BMD* pModel);
    bool RenderHuntingGroundObjectMesh(const OBJECT* pObject, DrawLook& look, BMD* pModel, bool ExtraMon = 0);

    //. 몬스터
    bool MoveHuntingGroundMonsterVisual(OBJECT* pObject, BMD* pModel);
    void MoveHuntingGroundBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
    bool RenderHuntingGroundMonsterObjectMesh(const OBJECT* pObject, DrawLook& look, BMD* pModel, bool ExtraMon);
    bool UpdateHuntingGroundMonsterEffects(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);

    bool AttackEffectHuntingGroundMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);

    //. 화면 처리
    bool CreateMist(PARTICLE* pParticleObj);
}

#endif // _GMHUNTINGGROUND_H_