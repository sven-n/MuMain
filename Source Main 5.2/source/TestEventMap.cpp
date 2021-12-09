// TestEventMap.cpp: implementation of the TestEventMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestEventMap.h"

#ifdef PSW_ADD_TESTMAP

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TestEventMapPtr	TestEventMap::Make()
{
	TestEventMapPtr map( new TestEventMap );
	return map;
}

TestEventMap::TestEventMap() : m_IsTest( true )
{

}

TestEventMap::~TestEventMap()
{

}


// 오브젝트 생성
bool TestEventMap::CreateObject(OBJECT* o)
{
	return false;
}

// 오브젝트 프로세서
bool TestEventMap::MoveObject(OBJECT* o)
{
	return false;
}

// 오브젝트 이펙트
bool TestEventMap::RenderObjectVisual(OBJECT* o, BMD* b)
{
	return false;
}

// 오브젝트 매쉬 이펙트
bool TestEventMap::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	return false;
}

// 맵 관련 오브젝트 이펙트
void TestEventMap::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{

}

// 몬스터 생성
CHARACTER* TestEventMap::CreateMonster(int iType, int PosX, int PosY, int Key)
{
	CHARACTER* pCharacter = NULL;
	return pCharacter;
}

// 몬스터(NPC) 프로세서
bool TestEventMap::MoveMonsterVisual(OBJECT* o, BMD* b)
{
	return false;
}
// 몬스터 스킬 블러 이펙트
void TestEventMap::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{

}

// 몬스터 이펙트 ( 일반 )	
bool TestEventMap::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	return false;
}

// 몬스터 이펙트 ( 스킬 )
bool TestEventMap::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
	return false;
}

// 스킬 애니메이션 관련 함수
bool TestEventMap::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
	return false;
}

bool TestEventMap::PlayMonsterSound(OBJECT* o)
{
	return false;
}

void TestEventMap::PlayObjectSound(OBJECT* o)
{

}

#endif //PSW_ADD_TESTMAP