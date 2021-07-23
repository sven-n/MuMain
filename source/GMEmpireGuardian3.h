// GMEmpireGuardian3.h: interface for the GMEmpireGuardian3 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GMEmpireGuardian3_H__26010684_3099_4F4C_8A33_26FF2281BB35__INCLUDED_)
#define AFX_GMEmpireGuardian3_H__26010684_3099_4F4C_8A33_26FF2281BB35__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef LDK_ADD_MAP_EMPIREGUARDIAN3

#include "w_BaseMap.h"

BoostSmartPointer( GMEmpireGuardian3 );

class GMEmpireGuardian3 : public BaseMap
{
public:
	static GMEmpireGuardian3Ptr Make();
	virtual ~GMEmpireGuardian3();

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
	// 맵 관련 화면 앞에 나오는 효과
	virtual void RenderFrontSideVisual();

	// 몬스터 렌더링
	bool RenderMonster(OBJECT* o, BMD* b, bool ExtraMon = 0);

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

	// 몬스터(NPC) 프로세서
	bool MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
	
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

public: //Weather
	bool CreateRain( PARTICLE* o );
	void SetWeather(int weather);

private:
	bool m_bCurrentIsRage_Kato;			// 3군 카토의 광폭화 상태 여부 함수.
	
protected:
	GMEmpireGuardian3();
};

extern bool IsEmpireGuardian3();

#endif	// LDK_ADD_MAP_EMPIREGUARDIAN3

#endif // !defined(AFX_GMEmpireGuardian3_H__26010684_3099_4F4C_8A33_26FF2281BB35__INCLUDED_)
