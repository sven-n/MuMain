// GMEmpireGuardian1.h: interface for the GMEmpireGuardian1 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GMEMPIREGUARDIAN1_H__26010684_3099_4F4C_8A33_26FF2281BB35__INCLUDED_)
#define AFX_GMEMPIREGUARDIAN1_H__26010684_3099_4F4C_8A33_26FF2281BB35__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef LDK_ADD_MAP_EMPIREGUARDIAN1

#include "w_BaseMap.h"

BoostSmartPointer( GMEmpireGuardian1 );

class GMEmpireGuardian1 : public BaseMap
{
public:
	static GMEmpireGuardian1Ptr Make();
	virtual ~GMEmpireGuardian1();

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
	enum WEATHER_TYPE					// 오늘의 날씨
	{
		WEATHER_SUN		= 0,
		WEATHER_RAIN	= 1,	
		WEATHER_FOG		= 2,
		WEATHER_STORM	= 3,
	};

	bool CreateRain( PARTICLE* o );

	void SetWeather(int weather) { m_iWeather = weather; }
	int GetWeather() { return m_iWeather; }

private:
	int m_iWeather;

	// 몬스터별 광폭화 여부
private:
	bool m_bCurrentIsRage_Raymond;			// 부사령관 레이몬드의 광폭화 상태 여부.
	bool m_bCurrentIsRage_Ercanne;			// 지휘관 에르칸느의 광폭화 상태 여부.
	bool m_bCurrentIsRage_Daesuler;			// 1군 데슬러의 광폭화 상태 여부.
	bool m_bCurrentIsRage_Gallia;			// 4군 갈리아의 광폭화 상태 여부.

protected:
	GMEmpireGuardian1();
};


// 제국수호군 영역인지 외부에서 파악이 용이하도록 1,2,3,4 맵을 총괄 체크 하도록 합니다.
#ifdef LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL
extern bool IsEmpireGuardian();
#endif // LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL

extern bool IsEmpireGuardian1();

#endif	// LDK_ADD_MAP_EMPIREGUARDIAN1

#endif // !defined(AFX_GMEMPIREGUARDIAN1_H__26010684_3099_4F4C_8A33_26FF2281BB35__INCLUDED_)
