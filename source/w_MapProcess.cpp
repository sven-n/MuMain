// w_MapProcess.cpp: implementation of the MapProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef PSW_ADD_MAPSYSTEM

#include "w_MapProcess.h"
// 맵 관련 include
#include "w_MapHeaders.h"

extern int World;

//////////////////////////////////////////////////////////////////////
// extern
//////////////////////////////////////////////////////////////////////

MapProcessPtr g_MapProcess;

MapProcess& TheMapProcess()
{
	assert( g_MapProcess );
	return *g_MapProcess;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MapProcessPtr MapProcess::Make()
{
	MapProcessPtr mapprocess( new MapProcess );
	mapprocess->Init();
	return mapprocess;
}

MapProcess::MapProcess()
{

}

MapProcess::~MapProcess()
{
	Destroy();
}

void MapProcess::Init()
{
	SEASON4A::CGM_RaklionPtr raklion = SEASON4A::CGM_Raklion::Make();
	raklion->AddMapIndex( WD_57ICECITY );
	raklion->AddMapIndex( WD_58ICECITY_BOSS );
	Register( raklion );
	
#ifdef YDG_ADD_MAP_SANTA_TOWN
	CGMSantaTownPtr santatown = CGMSantaTown::Make();
	santatown->AddMapIndex( WD_62SANTA_TOWN );
	Register( santatown );
#endif	// YDG_ADD_MAP_SANTA_TOWN
	
#ifdef PBG_ADD_PKFIELD
	CGM_PK_FieldPtr pkfield = CGM_PK_Field::Make();
	pkfield->AddMapIndex(WD_63PK_FIELD);
	Register(pkfield);
#endif //PBG_ADD_PKFIELD

#ifdef YDG_ADD_MAP_DUEL_ARENA
	CGMDuelArenaPtr duelarena = CGMDuelArena::Make();
	duelarena->AddMapIndex( WD_64DUELARENA );
	Register( duelarena );
#endif	// YDG_ADD_MAP_DUEL_ARENA

#ifdef YDG_ADD_MAP_DOPPELGANGER1
	CGMDoppelGanger1Ptr doppelganger1 = CGMDoppelGanger1::Make();
	doppelganger1->AddMapIndex( WD_65DOPPLEGANGER1 );
	Register( doppelganger1 );
#endif	// YDG_ADD_MAP_DOPPELGANGER1

#ifdef YDG_ADD_MAP_DOPPELGANGER2
	CGMDoppelGanger2Ptr doppelganger2 = CGMDoppelGanger2::Make();
	doppelganger2->AddMapIndex( WD_66DOPPLEGANGER2 );
	Register( doppelganger2 );
#endif	// YDG_ADD_MAP_DOPPELGANGER2

#ifdef YDG_ADD_MAP_DOPPELGANGER3
	CGMDoppelGanger3Ptr doppelganger3 = CGMDoppelGanger3::Make();
	doppelganger3->AddMapIndex( WD_67DOPPLEGANGER3 );
	Register( doppelganger3 );
#endif	// YDG_ADD_MAP_DOPPELGANGER3

#ifdef YDG_ADD_MAP_DOPPELGANGER4
	CGMDoppelGanger4Ptr doppelganger4 = CGMDoppelGanger4::Make();
	doppelganger4->AddMapIndex( WD_68DOPPLEGANGER4 );
	Register( doppelganger4 );
#endif	// YDG_ADD_MAP_DOPPELGANGER4
	
#ifdef LDK_ADD_MAP_EMPIREGUARDIAN1
	GMEmpireGuardian1Ptr empireguardian1 = GMEmpireGuardian1::Make();
	empireguardian1->AddMapIndex( WD_69EMPIREGUARDIAN1 );
	Register( empireguardian1 );
#endif //LDK_ADD_MAP_EMPIREGUARDIAN1

#ifdef LDS_ADD_MAP_EMPIREGUARDIAN2
	GMEmpireGuardian2Ptr empireguardian2 = GMEmpireGuardian2::Make();
	empireguardian2->AddMapIndex( WD_70EMPIREGUARDIAN2 );
	Register( empireguardian2 );
#endif //LDS_ADD_MAP_EMPIREGUARDIAN2
	
#ifdef LDK_ADD_MAP_EMPIREGUARDIAN3
	GMEmpireGuardian3Ptr empireguardian3 = GMEmpireGuardian3::Make();
	empireguardian3->AddMapIndex( WD_71EMPIREGUARDIAN3 );
	Register( empireguardian3 );
#endif //LDK_ADD_MAP_EMPIREGUARDIAN3

#ifdef LDS_ADD_MAP_EMPIREGUARDIAN4
	GMEmpireGuardian4Ptr empireguardian4 = GMEmpireGuardian4::Make();
	empireguardian4->AddMapIndex( WD_72EMPIREGUARDIAN4 );
	Register( empireguardian4 );
#endif //LDS_ADD_MAP_EMPIREGUARDIAN4

#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
	GMUnitedMarketPlacePtr unitedMarketPlace = GMUnitedMarketPlace::Make();
	unitedMarketPlace->AddMapIndex( WD_79UNITEDMARKETPLACE );
	Register( unitedMarketPlace );
#endif // LDS_ADD_MAP_UNITEDMARKETPLACE

#ifdef ASG_ADD_MAP_KARUTAN
	CGMKarutan1Ptr karutan1 = CGMKarutan1::Make();
	karutan1->AddMapIndex( WD_80KARUTAN1 );
	karutan1->AddMapIndex( WD_81KARUTAN2 );
	Register( karutan1 );
#endif	// ASG_ADD_MAP_KARUTAN
}

void MapProcess::Destroy()
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			m_MapList.erase( tempiter );
		}
	}
	m_MapList.clear();
}

bool MapProcess::FindMap( ENUM_WORLD type )
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			bool result = basemap.lock()->IsCurrentMap( type );
			if( result ) {
				return true;
			}
		}
	}
	return false;
}

BaseMap& MapProcess::FindBaseMap( ENUM_WORLD type )
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( type ) == true ) {
				return **tempiter;
			}
		}
	}
    assert( 0 );
    throw;
}

void MapProcess::Register( BoostSmart_Ptr( BaseMap ) pMap )
{	
	ENUM_WORLD type = pMap->FindMapIndex();
	if( type == NUM_WD ) {
		assert( 0 );
		throw;
	}

	if( FindMap( type ) == false ) {
		m_MapList.push_back( pMap );
	}
}

void MapProcess::UnRegister( ENUM_WORLD type )
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			bool result = basemap.lock()->IsCurrentMap( type );
			if( result ) {
				m_MapList.erase( tempiter );
				return;
			}
		}
	}
}

BaseMap& MapProcess::GetMap( int type )
{
	return FindBaseMap( static_cast<ENUM_WORLD>(type) );
}

bool MapProcess::LoadMapData()
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				bool result = basemap.lock()->LoadMapData();
				if( result ) {
					return true;
				}
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
	return false;
}

bool MapProcess::CreateObject(OBJECT* o)
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			bool result = basemap.lock()->CreateObject( o );
			if( result ) {
				return true;
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
	return false;
}

// 오브젝트 프로세서
bool MapProcess::MoveObject(OBJECT* o)
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;

		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				basemap.lock()->PlayObjectSound( o );
				bool result = basemap.lock()->MoveObject( o );
				if( result ) {
					return true;
				}
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
	return false;
}

// 오브젝트 이펙트
bool MapProcess::RenderObjectVisual(OBJECT* o, BMD* b)
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				bool result = basemap.lock()->RenderObjectVisual( o, b );
				if( result ) {
					return true;
				}
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
	return false;
}

// 오브젝트 매쉬 이펙트
bool MapProcess::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				bool result = basemap.lock()->RenderObjectMesh( o, b, ExtraMon );
				if( result ) {
					return true;
				}
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
	return false;
}

// 맵 관련 오브젝트 이펙트
void MapProcess::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				basemap.lock()->RenderAfterObjectMesh( o, b, ExtraMon );
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
}

#ifdef LDK_ADD_MAPPROCESS_RENDERBASESMOKE_FUNC
// 맵 관련 화면 앞에 나오는 효과
void MapProcess::RenderFrontSideVisual()
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false )
		{
			if( basemap.lock()->IsCurrentMap( World ) )
			{
				basemap.lock()->RenderFrontSideVisual();
			}
		}
		else
		{
			m_MapList.erase( tempiter );
		}
	}
}
#endif //LDK_ADD_MAPPROCESS_RENDERBASESMOKE_FUNC

CHARACTER* MapProcess::CreateMonster( int iType, int PosX, int PosY, int Key )
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			CHARACTER* p = basemap.lock()->CreateMonster( iType, PosX, PosY, Key );
			if( p ) {
				return p;
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}

	return NULL;
}

// 몬스터(NPC) 프로세서
bool MapProcess::MoveMonsterVisual(OBJECT* o, BMD* b)
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				bool result = basemap.lock()->MoveMonsterVisual( o, b );
				if( result ) {
					return true;
				}
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
	return false;
}

// 몬스터 스킬 블러 이펙트
void MapProcess::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				basemap.lock()->MoveBlurEffect( c, o, b );
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
}

// 몬스터 이펙트 ( 일반 )	
bool MapProcess::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				bool result = basemap.lock()->RenderMonsterVisual( c, o, b );
				if( result ) {
					return true;
				}
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
	return false;
}

// 몬스터 이펙트 ( 스킬 )
bool MapProcess::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				bool result = basemap.lock()->AttackEffectMonster( c, o, b );
				if( result ) {
					return true;
				}
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
	return false;
}

// 스킬 애니메이션 관련 함수
bool MapProcess::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				bool result = basemap.lock()->SetCurrentActionMonster( c, o );
				if( result ) {
					return true;
				}
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
	return false;
}

bool MapProcess::PlayMonsterSound(OBJECT* o)
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				bool result = basemap.lock()->PlayMonsterSound( o );
				if( result ) {
					return true;
				}
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
	return false;
}

bool MapProcess::ReceiveMapMessage( BYTE code, BYTE subcode, BYTE* ReceiveBuffer )
{
	for( MapList::iterator iter = m_MapList.begin(); iter != m_MapList.end(); )
	{
		MapList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(BaseMap) basemap = *tempiter;
		
		if( basemap.expired() == false ) {
			if( basemap.lock()->IsCurrentMap( World ) ) {
				bool result = basemap.lock()->ReceiveMapMessage( code, subcode, ReceiveBuffer );
				if( result ) {
					return true;
				}
			}
		}
		else {
			m_MapList.erase( tempiter );
		}
	}
	return false;
}

#endif //PSW_ADD_MAPSYSTEM