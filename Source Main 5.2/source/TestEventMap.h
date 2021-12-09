// TestEventMap.h: interface for the TestEventMap class.
//
//////////////////////////////////////////////////////////////////////

#ifdef PSW_ADD_TESTMAP

#if !defined(AFX_TESTEVENTMAP_H__9ABFF262_D93D_4B3C_9620_681662E7A384__INCLUDED_)
#define AFX_TESTEVENTMAP_H__9ABFF262_D93D_4B3C_9620_681662E7A384__INCLUDED_

#pragma once

#include "w_BaseMap.h"

BoostSmartPointer( TestEventMap );
class TestEventMap : public BaseMap
{
public:
	static TestEventMapPtr	Make();
	virtual ~TestEventMap();	

public:	// Object
	// 오브젝트 생성
	virtual bool CreateObject(OBJECT* o);
	// 오브젝트 프로세서
	virtual bool MoveObject(OBJECT* o);
	// 오브젝트 이펙트
	virtual bool RenderObjectVisual(OBJECT* o, BMD* b);
	// 오브젝트 매쉬 이펙트
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

public:
	virtual bool PlayMonsterSound(OBJECT* o);
	virtual void PlayObjectSound(OBJECT* o);

public:
	const bool GetTest() const;

public:
	TestEventMap();

private:
	bool m_IsTest;

};

inline
const bool TestEventMap::GetTest() const
{
	return m_IsTest;
}

#endif // !defined(AFX_TESTEVENTMAP_H__9ABFF262_D93D_4B3C_9620_681662E7A384__INCLUDED_)

#endif //PSW_ADD_TESTMAP
