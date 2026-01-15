// GMDoppelGanger3.h: interface for the GMDoppelGanger3 class.
//////////////////////////////////////////////////////////////////////
#pragma once
#include "w_BaseMap.h"

SmartPointer(CGMDoppelGanger3);
class CGMDoppelGanger3 : public BaseMap
{
public:
    static CGMDoppelGanger3Ptr Make();
    virtual ~CGMDoppelGanger3();

public:	// Object
    // 오브젝트 생성
    virtual bool CreateObject(OBJECT* o);
    // 오브젝트 프로세서
    virtual bool MoveObject(OBJECT* o);
    // 오브젝트 이펙트
    virtual bool RenderObjectVisual(OBJECT* o, BMD* b);
    // 오브젝트 랜더
    virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
    // 맵 관련 오브젝트 이펙트
    virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);

public:	// Character
    // 몬스터 생성
    virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
    // 몬스터(NPC) 프로세서
    virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
    // 몬스터 스킬 블러 이펙트
    virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    // 몬스터 이펙트 ( 일반 )
    virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
    // 몬스터 이펙트 ( 스킬 )
    virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
    // 스킬 애니메이션 관련 함수
    virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);

public: // Sound
    // 몬스터 사운드
    virtual bool PlayMonsterSound(OBJECT* o);
    // 오브젝트 사운드
    virtual void PlayObjectSound(OBJECT* o);

public:
    void Init();
    void Destroy();

protected:
    CGMDoppelGanger3();
};

extern bool IsDoppelGanger3();
