// GM_Kanturu_In.h: interface for the GM_Kanturu_In class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GM_KANTURU_IN_H__37F1E344_37B1_497A_9B04_84409C768A74__INCLUDED_)
#define AFX_GM_KANTURU_IN_H__37F1E344_37B1_497A_9B04_84409C768A74__INCLUDED_

#pragma once

#include "ZzzBMD.h"

// TODO
namespace M38Kanturu2nd
{
    // 칸투루 오브젝트 관련
    bool Create_Kanturu2nd_Object(OBJECT* o);										// 오브젝트 생성
    bool Move_Kanturu2nd_Object(OBJECT* o);											// 오브젝트 업데이트
    bool Render_Kanturu2nd_ObjectVisual(OBJECT* o, BMD* b);							// 오브젝트 이펙트 효과
    bool Render_Kanturu2nd_ObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);			// 오브젝트 렌더링(몬스터 포함)
    void Render_Kanturu2nd_AfterObjectMesh(OBJECT* o, BMD* b);

    // 칸투루내부 몬스터 관련
    CHARACTER* Create_Kanturu2nd_Monster(int iType, int PosX, int PosY, int Key);	// 몬스터 생성 함수
    bool	Set_CurrentAction_Kanturu2nd_Monster(CHARACTER* c, OBJECT* o);		// 몬스터 현재 액션 세팅
    bool	AttackEffect_Kanturu2nd_Monster(CHARACTER* c, OBJECT* o, BMD* b);	// 몬스터 공격 이펙트
    bool	Move_Kanturu2nd_MonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);		// 몬스터 효과 업데이트
    void	Move_Kanturu2nd_BlurEffect(CHARACTER* c, OBJECT* o, BMD* b);		// 몬스터 무기의 잔상 처리
    bool	Render_Kanturu2nd_MonsterObjectMesh(OBJECT* o, BMD* b, int ExtraMon);	// 몬스터 오브젝트 렌더링
    bool	Render_Kanturu2nd_MonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);	// 몬스터 효과 렌더링

    // 칸투루내부 맵 관련
    bool		Is_Kanturu2nd();						// 칸투루내부 맵인가?
    bool		Is_Kanturu2nd_3rd();					// 칸투루내부와 3차 맵인가?

    // 사운드
    void	Sound_Kanturu2nd_Object(OBJECT* o);		// 오브젝트 사운드
    void	PlayBGM();
};

class CTrapCanon
{
public:
    CTrapCanon();
    ~CTrapCanon();

private:
    void Initialize();
    void Destroy();

public:
    void Open_TrapCanon();
    CHARACTER* Create_TrapCanon(int iPosX, int iPosY, int iKey);
    void Render_Object(OBJECT* o, BMD* b);
    void Render_Object_Visual(CHARACTER* c, OBJECT* o, BMD* b);
    void Render_AttackEffect(CHARACTER* c, OBJECT* o, BMD* b);
};

extern CTrapCanon g_TrapCanon;

#endif // !defined(AFX_GM_KANTURU_IN_H__37F1E344_37B1_497A_9B04_84409C768A74__INCLUDED_)
