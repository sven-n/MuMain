#ifndef _GMAIDA_H_
#define _GMAIDA_H_

namespace M33Aida {
    bool IsInAida();
    bool IsInAidaSection2(const vec3_t Position);

    //. 오브젝트
    bool CreateAidaObject(OBJECT* pObject);
    bool MoveAidaObject(OBJECT* pObject);
    bool RenderAidaObjectVisual(OBJECT* pObject, BMD* pModel);
    bool RenderAidaObjectMesh(OBJECT* pObject, BMD* pModel, bool ExtraMon = 0);

    //. 몬스터
    CHARACTER* CreateAidaMonster(int iType, int PosX, int PosY, int Key);

    bool MoveAidaMonsterVisual(OBJECT* pObject, BMD* pModel);
    void MoveAidaBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
    bool RenderAidaMonsterObjectMesh(OBJECT* pObject, BMD* pModel, bool ExtraMon);
    bool RenderAidaMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);

    bool AttackEffectAidaMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
    bool SetCurrentActionAidaMonster(CHARACTER* pCharacter, OBJECT* pObject);

    //. 화면 처리
    bool CreateMist(PARTICLE* pParticleObj);
}

#endif //_GMAIDA_H_