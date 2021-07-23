// GMDoppelGanger2.h: interface for the GMDoppelGanger2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GMDOPPELGANGER2_H__492710D3_0D2E_4903_85DD_ABF78B68BD7A__INCLUDED_)
#define AFX_GMDOPPELGANGER2_H__492710D3_0D2E_4903_85DD_ABF78B68BD7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef YDG_ADD_MAP_DOPPELGANGER2

#include "w_BaseMap.h"

BoostSmartPointer( CGMDoppelGanger2 );
class CGMDoppelGanger2 : public BaseMap  
{
public:
	static CGMDoppelGanger2Ptr Make();
	virtual ~CGMDoppelGanger2();

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
public:
	// 맵에 불씨 날리게 하는 효과
	bool CreateFireSpark(PARTICLE* o);
	
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
	CGMDoppelGanger2();
};

extern bool IsDoppelGanger2();

#endif	// YDG_ADD_MAP_DOPPELGANGER2

#endif // !defined(AFX_GMDOPPELGANGER2_H__492710D3_0D2E_4903_85DD_ABF78B68BD7A__INCLUDED_)
