// w_BaseMap.h: interface for the BaseMap class.
//
//////////////////////////////////////////////////////////////////////

#ifdef PSW_ADD_MAPSYSTEM

#if !defined(AFX_W_BASEMAP_H__CAA21BB2_801A_40AA_B225_1EFDF92DFF73__INCLUDED_)
#define AFX_W_BASEMAP_H__CAA21BB2_801A_40AA_B225_1EFDF92DFF73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZzzBMD.h"

class BaseMap
{
public:
	BaseMap() {}
	virtual ~BaseMap() { clear(); }
	
public:
	virtual bool LoadMapData() { return false; }

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
public:
	//===================================================================
	// 오브젝트
	//===================================================================
	virtual bool CreateObject(OBJECT* pObj)												// 오브젝트 생성
			{
				if(pObj == NULL)	return false;
				return true;
			}
	virtual bool MoveObject(OBJECT* pObj)												// 오브젝트 프로세서
			{
				if(pObj == NULL)	return false;
				return true;
			}			
	virtual bool RenderObjectVisual(OBJECT* pObj, BMD* pBMD)							// 오브젝트 이펙트
			{
				if(pObj == NULL)	return false;
				if(pBMD == NULL)	return false;

				return true;
			}
	virtual bool RenderObjectMesh(OBJECT* pObj, BMD* pBMD, bool bExtraMon = 0)			// 오브젝트 매쉬 이펙트
			{
				if(pObj == NULL)		return false;
				if(pBMD == NULL)		return false;
				if(bExtraMon == FALSE)	return false;

				return true;
			}
	virtual void RenderAfterObjectMesh(OBJECT* pObj, BMD* pBMD, bool bExtraMon = 0)	// 맵 관련 오브젝트 이펙트
			{
				if(pObj == NULL)		return;
				if(pBMD == NULL)		return;
				if(bExtraMon == FALSE)	return;

				return;
			}
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
public:	// Object
	// 오브젝트 생성
	virtual bool CreateObject(OBJECT* o) { return false; }
	// 오브젝트 프로세서
	virtual bool MoveObject(OBJECT* o) { return false; }
	// 오브젝트 이펙트
	virtual bool RenderObjectVisual(OBJECT* o, BMD* b) { return false; }
	// 오브젝트 매쉬 이펙트
	virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0) { return false; }
	// 맵 관련 오브젝트 이펙트
	virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0) {}
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

#ifdef LDK_ADD_MAPPROCESS_RENDERBASESMOKE_FUNC
	// 맵 관련 화면 앞에 나오는 효과
	virtual void RenderFrontSideVisual() {}
#endif //LDK_ADD_MAPPROCESS_RENDERBASESMOKE_FUNC

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
public:	
	//===================================================================
	// Character
	//===================================================================
	virtual CHARACTER* CreateMonster(int iType, int iPosX, int iPosY, int iKey)	// 몬스터 생성
			{
				if(iType < 0)	return NULL;
				if(iPosX < 0)	return NULL;
				if(iPosY < 0)	return NULL;
				if(iKey < 0)	return NULL;

				return NULL;
			}
	virtual bool MoveMonsterVisual(OBJECT* pObj, BMD* pBMD)							// 몬스터(NPC) 프로세서
			{
				if(pObj == NULL)	return false;
				if(pBMD == NULL)	return false;

				return true;
			}
	
	virtual void MoveBlurEffect(CHARACTER* pChar, OBJECT* pObj, BMD* pBMD)			// 몬스터 스킬 블러 이펙트
			{
				if(pChar == NULL)	return;
				if(pObj == NULL)	return;
				if(pBMD == NULL)	return;

				return;
			}	
	virtual bool RenderMonsterVisual(CHARACTER* pChar, OBJECT* pObj, BMD* pBMD)	// 몬스터 이펙트 ( 일반 )
			{
				if(pChar == NULL)	return false;
				if(pObj == NULL)	return false;
				if(pBMD == NULL)	return false;

				return true;
			}	
	virtual bool AttackEffectMonster(CHARACTER* pChar, OBJECT* pObj, BMD* pBMD)	// 몬스터 이펙트 ( 스킬 )
			{
				if(pChar == NULL)	return false;
				if(pObj == NULL)	return false;
				if(pBMD == NULL)	return false;

				return true;
			}	
	virtual bool SetCurrentActionMonster(CHARACTER* pChar, OBJECT* pObj)			// 스킬 애니메이션 관련 함수
			{
				if(pChar == NULL)	return false;
				if(pObj == NULL)	return false;

				return true;
			}

	//===================================================================
	// Weather
	//===================================================================
	virtual bool CreateWeather( PARTICLE* pParticle )								// 날씨 생성
			{
				if(pParticle == NULL)	return false;

				return true;
			}

	//===================================================================
	// Sound
	//===================================================================
	virtual bool PlayMonsterSound(OBJECT* pObj)										// 몬스터 사운드
			{
				if(pObj == NULL)	return false;

				return true;
			}
	virtual void PlayObjectSound(OBJECT* pObj)										// 오브젝트 사운드
			{
				if(pObj == NULL)	return;

				return;
			}

	//===================================================================
	// Server Message
	//===================================================================
	virtual bool ReceiveMapMessage(BYTE byCode, BYTE bySubcode, BYTE* byReceiveBuffer)
			{
				if(byCode <= 0)				return false;
				if(bySubcode <= 0)			return false;
				if(byReceiveBuffer == NULL)	return false;

				return true;
			}
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
public:	// Character
	// 몬스터 생성
	virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key) { return NULL; }
	// 몬스터(NPC) 프로세서
	virtual bool MoveMonsterVisual(OBJECT* o, BMD* b) { return false; }
	// 몬스터 스킬 블러 이펙트
	virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b) {}
	// 몬스터 이펙트 ( 일반 )	
	virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b) { return false; }
	// 몬스터 이펙트 ( 스킬 )
	virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b) { return false; }
	// 스킬 애니메이션 관련 함수
	virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o) { return false; }

public: // Weather
	// 날씨 생성
	virtual bool CreateWeather( PARTICLE* o ) { return false; }

public: // Sound
	// 몬스터 사운드
	virtual bool PlayMonsterSound(OBJECT* o) { return false; }
	// 오브젝트 사운드
	virtual void PlayObjectSound(OBJECT* o) {}

public: // Server Message
	virtual bool ReceiveMapMessage( BYTE code, BYTE subcode, BYTE* ReceiveBuffer ) { return false; }
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	
public:
	void clear() { m_MapTypes.clear(); }

public:
	bool isMapIndex( ENUM_WORLD type ) {
#ifdef _VS2008PORTING
		for( int i = 0; i < (int)m_MapTypes.size(); ++i ) {
#else // _VS2008PORTING
		for( int i = 0; i < m_MapTypes.size(); ++i ) {
#endif // _VS2008PORTING
			if( type == m_MapTypes[i] ) {
				return true;
			}
		}
		return false;
	}

	ENUM_WORLD FindMapIndex( int index = 0 ) {
#ifdef _VS2008PORTING
		if( m_MapTypes.size() == 0 || index >= (int)m_MapTypes.size() ) return NUM_WD;
#else // _VS2008PORTING
		if( m_MapTypes.size() == 0 || index >= m_MapTypes.size() ) return NUM_WD;
#endif // _VS2008PORTING
		else return m_MapTypes[index];
	}

public:
	const bool IsCurrentMap( int type ) { return isMapIndex( static_cast<ENUM_WORLD>( type ) ); }

public:
	void AddMapIndex( ENUM_WORLD type ) { if( !isMapIndex( type ) ) m_MapTypes.push_back( type ); }

public:
	typedef vector<ENUM_WORLD>	WorldVector;

private:
	WorldVector		m_MapTypes;
};

#endif // !defined(AFX_W_BASEMAP_H__CAA21BB2_801A_40AA_B225_1EFDF92DFF73__INCLUDED_)

#endif //PSW_ADD_MAPSYSTEM
