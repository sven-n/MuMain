// GMDuelArena.h: interface for the CGMDuelArena class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GMDUELARENA_H__BD799B5C_04B6_49CA_A792_12E3D7111356__INCLUDED_)
#define AFX_GMDUELARENA_H__BD799B5C_04B6_49CA_A792_12E3D7111356__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef YDG_ADD_MAP_DUEL_ARENA

#include "w_BaseMap.h"

BoostSmartPointer( CGMDuelArena );
class CGMDuelArena : public BaseMap  
{
public:
	static CGMDuelArenaPtr Make();
	virtual ~CGMDuelArena();

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
	// 배경음악
	void PlayBGM();

public:
	void Init();
	void Destroy();

protected:
	CGMDuelArena();
};

extern bool IsDuelArena();

#endif	// YDG_ADD_MAP_DUEL_ARENA

#endif // !defined(AFX_GMDUELARENA_H__BD799B5C_04B6_49CA_A792_12E3D7111356__INCLUDED_)
