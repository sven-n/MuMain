#ifndef _GMHUNTINGGROUND_H_
#define _GMHUNTINGGROUND_H_

//. soyaviper

namespace M31HuntingGround {		//. 공성 사냥터
    bool IsInHuntingGround();
    bool IsInHuntingGroundSection2(const vec3_t Position);

    //. 오브젝트
    bool CreateHuntingGroundObject(OBJECT* pObject);
    bool MoveHuntingGroundObject(OBJECT* pObject);
    bool RenderHuntingGroundObjectVisual(OBJECT* pObject, BMD* pModel);
    bool RenderHuntingGroundObjectMesh(OBJECT* pObject, BMD* pModel, bool ExtraMon = 0);

    //. 몬스터
    CHARACTER* CreateHuntingGroundMonster(int iType, int PosX, int PosY, int Key);

    bool MoveHuntingGroundMonsterVisual(OBJECT* pObject, BMD* pModel);
    void MoveHuntingGroundBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
    bool RenderHuntingGroundMonsterObjectMesh(OBJECT* pObject, BMD* pModel, bool ExtraMon);
    bool RenderHuntingGroundMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);

    bool AttackEffectHuntingGroundMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
    bool SetCurrentActionHuntingGroundMonster(CHARACTER* pCharacter, OBJECT* pObject);

    //. 화면 처리
    bool CreateMist(PARTICLE* pParticleObj);
}

#endif // _GMHUNTINGGROUND_H_