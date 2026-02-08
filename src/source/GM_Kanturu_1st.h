//*****************************************************************************
// File: GM_kanturu_1st.h
//
// Desc: 칸투르 1차(외부) 맵, 몬스터.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#ifndef _GM_KANTURU_1ST_H_
#define _GM_KANTURU_1ST_H_

namespace M37Kanturu1st
{
    bool IsKanturu1st();						// 칸투르 외부 맵인가?

    // 오브젝트 관련
    bool CreateKanturu1stObject(OBJECT* pObject);
    bool MoveKanturu1stObject(OBJECT* pObject);
    bool RenderKanturu1stObjectVisual(OBJECT* pObject, BMD* pModel);
    bool RenderKanturu1stObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);	// 오브젝트 렌더(몬스터 포함)
    void RenderKanturu1stAfterObjectMesh(OBJECT* o, BMD* b);				// 반투명 오브젝트 나중 랜더.

    // 몬스터 관련
    CHARACTER* CreateKanturu1stMonster(int iType, int PosX, int PosY, int Key);	// 몬스터 생성
    bool SetCurrentActionKanturu1stMonster(CHARACTER* c, OBJECT* o);		// 몬스터 현재 액션 세팅
    bool AttackEffectKanturu1stMonster(CHARACTER* c, OBJECT* o, BMD* b);	// 몬스터 공격 이펙트
    bool MoveKanturu1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);		// 몬스터 효과 업데이트
    void MoveKanturu1stBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);		// 몬스터 무기의 잔상 처리
    bool RenderKanturu1stMonsterObjectMesh(OBJECT* o, BMD* b, int ExtraMon);	// 몬스터 오브젝트 렌더링
    bool RenderKanturu1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);	// 몬스터 효과 렌더링
};

#endif	// _GM_KANTURU_1ST_H_