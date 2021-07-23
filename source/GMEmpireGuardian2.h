// GMEmpireGuardian2.h: interface for the GMEmpireGuardian2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GMEMPIREGUARDIAN2_H__823C7D40_34C6_4235_9EBD_69026B7F67FD__INCLUDED_)
#define AFX_GMEMPIREGUARDIAN2_H__823C7D40_34C6_4235_9EBD_69026B7F67FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef LDS_ADD_MAP_EMPIREGUARDIAN2

#include "w_BaseMap.h"

BoostSmartPointer( GMEmpireGuardian2 );

class GMEmpireGuardian2 : public BaseMap
{
public:
	static GMEmpireGuardian2Ptr Make();
	virtual ~GMEmpireGuardian2();
	
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
	
public: //Weather
	bool CreateRain( PARTICLE* o );
 	void SetWeather(int weather);

private:
	bool m_bCurrentIsRage_Bermont;			// 2군 버몬트의 광폭화 상태 여부 함수.
	
public:
	void Init();
	void Destroy();
	
protected:
	GMEmpireGuardian2();
};

extern bool IsEmpireGuardian2();

#endif // LDS_ADD_MAP_EMPIREGUARDIAN2

#endif // !defined(AFX_GMEMPIREGUARDIAN2_H__823C7D40_34C6_4235_9EBD_69026B7F67FD__INCLUDED_)
