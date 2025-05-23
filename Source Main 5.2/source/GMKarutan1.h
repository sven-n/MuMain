//*****************************************************************************
// File: GMKarutan1.h
//
// Desc: 킬루탄1 맵, 몬스터.
//
// producer: Ahn Sang-Kyu (10.08.03)
//*****************************************************************************

#if !defined(AFX_GMKARUTAN1_H__A2F56C80_26D8_4474_AECE_63DA2DA511A9__INCLUDED_)
#define AFX_GMKARUTAN1_H__A2F56C80_26D8_4474_AECE_63DA2DA511A9__INCLUDED_

#pragma once

#ifdef ASG_ADD_MAP_KARUTAN

#include "w_BaseMap.h"

SmartPointer(CGMKarutan1);

class CGMKarutan1 : public BaseMap
{
protected:
    CGMKarutan1();

public:
    virtual ~CGMKarutan1();

    static CGMKarutan1Ptr Make();

    // Object
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

#ifdef ASG_ADD_KARUTAN_MONSTERS
    // Character
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

    // Sound
        // 몬스터 사운드
    virtual bool PlayMonsterSound(OBJECT* o);
#endif	// ASG_ADD_KARUTAN_MONSTERS
    // 오브젝트 사운드
    virtual void PlayObjectSound(OBJECT* o);
    // 배경음악
    void PlayBGM();
};

extern bool IsKarutanMap();

#endif	// ASG_ADD_MAP_KARUTAN

#endif // !defined(AFX_GMKARUTAN1_H__A2F56C80_26D8_4474_AECE_63DA2DA511A9__INCLUDED_)
