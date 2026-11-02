#ifndef _GMAIDA_H_
#define _GMAIDA_H_

namespace M33Aida {
    bool IsInAida();
    bool IsInAidaSection2(const vec3_t Position);

    //. 오브젝트
    bool CreateAidaObject(OBJECT* pObject);
    bool MoveAidaObject(OBJECT* pObject);
    bool UpdateAidaObjectEffects(OBJECT* pObject, BMD* pModel);
    bool RenderAidaObjectVisual(const OBJECT* pObject, DrawLook& look, BMD* pModel);
    bool RenderAidaObjectMesh(const OBJECT* pObject, DrawLook& look, BMD* pModel, bool ExtraMon = 0);

    //. 몬스터
    CHARACTER* CreateAidaMonster(int iType, int PosX, int PosY, int Key);

    bool MoveAidaMonsterVisual(OBJECT* pObject, BMD* pModel);
    void MoveAidaBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
    bool RenderAidaMonsterObjectMesh(const OBJECT* pObject, DrawLook& look, BMD* pModel, bool ExtraMon);
    bool UpdateAidaMonsterEffects(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);

    bool AttackEffectAidaMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);

    //. 화면 처리
    bool CreateMist(PARTICLE* pParticleObj);
}

#endif //_GMAIDA_H_