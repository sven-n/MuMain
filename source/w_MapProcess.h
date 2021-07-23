// w_MapProcess.h: interface for the MapProcess class.
//
//////////////////////////////////////////////////////////////////////

#ifdef PSW_ADD_MAPSYSTEM

#if !defined(AFX_W_MAPPROCESS_H__951C620C_F82C_4E25_90CB_A877040F49E2__INCLUDED_)
#define AFX_W_MAPPROCESS_H__951C620C_F82C_4E25_90CB_A877040F49E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_BaseMap.h"

BoostSmartPointer(MapProcess);
class MapProcess  
{
public:
	static MapProcessPtr Make();
	virtual ~MapProcess();
	
public:
	bool LoadMapData();

public:
	// 오브젝트 생성
	bool CreateObject(OBJECT* o);
	// 오브젝트 프로세서
	bool MoveObject(OBJECT* o);
	// 오브젝트 이펙트
	bool RenderObjectVisual(OBJECT* o, BMD* b);
	// 오브젝트 매쉬 이펙트
	bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
	// 맵 관련 오브젝트 이펙트
	void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);

#ifdef LDK_ADD_MAPPROCESS_RENDERBASESMOKE_FUNC
	// 맵 관련 화면 앞에 나오는 효과
	void RenderFrontSideVisual();
#endif //LDK_ADD_MAPPROCESS_RENDERBASESMOKE_FUNC
	
public:
	// 몬스터 생성
	CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
	// 몬스터(NPC) 프로세서
	bool MoveMonsterVisual(OBJECT* o, BMD* b);
	// 몬스터 스킬 블러 이펙트
	void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
	// 몬스터 이펙트 ( 일반 )	
	bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
	// 몬스터 이펙트 ( 스킬 )
	bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
	// 스킬 애니메이션 관련 함수
	bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);

public:
	bool PlayMonsterSound(OBJECT* o);

public:
	bool ReceiveMapMessage( BYTE code, BYTE subcode, BYTE* ReceiveBuffer );

public:
	// 등록
	void Register( BoostSmart_Ptr( BaseMap ) pMap );
	// 등록 해제
	void UnRegister( ENUM_WORLD type );

public:
	BaseMap& GetMap( int type );
	
private:
	bool FindMap( ENUM_WORLD type );
	BaseMap& FindBaseMap( ENUM_WORLD type );
	void Init();
	void Destroy();
	MapProcess();
	
private:
	typedef list< BoostSmart_Ptr(BaseMap) >		MapList;
	
private:
	MapList				m_MapList;
};

extern MapProcessPtr g_MapProcess;

extern MapProcess& TheMapProcess();

#endif // !defined(AFX_W_MAPPROCESS_H__951C620C_F82C_4E25_90CB_A877040F49E2__INCLUDED_)

#endif //PSW_ADD_MAPSYSTEM
