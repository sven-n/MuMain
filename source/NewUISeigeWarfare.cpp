// CNewUISiegeWarfare.cpp: implementation of the CNewUISiegeWarfare class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUISeigeWarfare.h"
#include "NewUISystem.h"
#include "NewUISiegeWarCommander.h"
#include "NewUISiegeWarSoldier.h"
#include "NewUISiegeWarObserver.h"
#include "ZzzInventory.h"
#include "UIGuildInfo.h"
#include "GMBattleCastle.h"


using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUISiegeWarfare::CNewUISiegeWarfare()
{
	m_pNewUIMng = NULL;
	m_pSiegeWarUI = NULL;
	m_iCurSiegeWarType = SIEGEWAR_TYPE_NONE;
	m_byGuildStatus = G_NONE;
	m_sGuildMarkIndex = -1;

	m_iHour = 0;
	m_iMinute = 0;
	m_iSecond = 0;
	m_dwSyncTime = 0;

	m_bCreated = true;

	memset( &m_Pos, 0, sizeof(POINT)) ;
}

SEASON3B::CNewUISiegeWarfare::~CNewUISiegeWarfare()
{
	Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool SEASON3B::CNewUISiegeWarfare::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_SIEGEWARFARE, this );		// 인터페이스 오브젝트 등록
	
	Show( true );
	
	SetPos(x, y);
    
	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void SEASON3B::CNewUISiegeWarfare::Release()
{
	if( m_pSiegeWarUI )
	{
		m_pSiegeWarUI->UnLoadImages();
		m_pSiegeWarUI->Release();
		SAFE_DELETE( m_pSiegeWarUI );
	}
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

//---------------------------------------------------------------------------------------------
// SetPos
void SEASON3B::CNewUISiegeWarfare::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;	
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool SEASON3B::CNewUISiegeWarfare::UpdateMouseEvent()
{
	if( m_pSiegeWarUI )
	{
		if( !m_pSiegeWarUI->UpdateMouseEvent() )
			return false;
	}
	
	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool SEASON3B::CNewUISiegeWarfare::UpdateKeyEvent()
{
	if( m_pSiegeWarUI )
	{
		if( !m_pSiegeWarUI->UpdateKeyEvent() )
			return false;
	}

	return true;
}

//---------------------------------------------------------------------------------------------
// Update
bool SEASON3B::CNewUISiegeWarfare::Update()
{		
	if( IsVisible() == false )
		return true;

	if( m_pSiegeWarUI == NULL ) 
		return true;
	
	if( battleCastle::InBattleCastle() && battleCastle::IsBattleCastleStart() == true )
	{
		m_iSecond = m_iSecond - (GetTickCount()-m_dwSyncTime);
		if( m_iSecond <= 0 )
		{
			if( m_iMinute <= 0 )
			{
				if( m_iHour <= 0 )
				{
					m_iSecond = 0;
					m_iMinute = 0;
					m_iHour = 0;
				}
				else
				{
					--m_iHour;
					m_iMinute = m_iMinute + 60;
				}
			}
			else
			{
				--m_iMinute;
				m_iSecond = m_iSecond + 60000;
			}
		}
		
		m_dwSyncTime = GetTickCount();
		
		m_pSiegeWarUI->SetTime( m_iHour, m_iMinute );			
	}

	m_pSiegeWarUI->Update();

	return true;
}

//---------------------------------------------------------------------------------------------
// Render
bool SEASON3B::CNewUISiegeWarfare::Render()
{
	if( m_pSiegeWarUI == NULL || InBattleCastle() == false )
	{
		return true;		
	}
	
	m_pSiegeWarUI->Render();
	
	return true;
}

//---------------------------------------------------------------------------------------------
// GetLayerDepth
float SEASON3B::CNewUISiegeWarfare::GetLayerDepth()
{
	return 1.6f;
}

//---------------------------------------------------------------------------------------------
// OpenningProcess
void SEASON3B::CNewUISiegeWarfare::OpenningProcess()
{
	
}

//---------------------------------------------------------------------------------------------
// ClosingProcess
void SEASON3B::CNewUISiegeWarfare::ClosingProcess()
{
	
}

//---------------------------------------------------------------------------------------------
// Init
void SEASON3B::CNewUISiegeWarfare::SetGuildData( const CHARACTER* pCharacter )
{
	m_sGuildMarkIndex = pCharacter->GuildMarkIndex;
	m_byGuildStatus = pCharacter->GuildStatus;
}

//---------------------------------------------------------------------------------------------
// CreateMiniMapUI
bool SEASON3B::CNewUISiegeWarfare::CreateMiniMapUI()
{
	if( m_pSiegeWarUI != NULL )
	{
		InitMiniMapUI();
	}
	
	// 길드장이거나 배틀마스터가 아닐때 
	if ( !(Hero->EtcPart == PARTS_ATTACK_TEAM_MARK 
		|| Hero->EtcPart == PARTS_ATTACK_TEAM_MARK2
		|| Hero->EtcPart == PARTS_ATTACK_TEAM_MARK3
		|| Hero->EtcPart == PARTS_DEFENSE_TEAM_MARK) 
		|| Hero->GuildStatus == G_PERSON)
	{
		m_byGuildStatus = G_NONE;
	}
	
	switch ( m_byGuildStatus )
	{
	case G_NONE:
		{
			m_pSiegeWarUI = new CNewUISiegeWarObserver;		// Observer
			m_iCurSiegeWarType = SIEGEWAR_TYPE_OBSERVER;
			m_bCreated = false;
		}break;	
	case G_MASTER:
		{
			if ( strcmp( GuildMark[m_sGuildMarkIndex].UnionName, "" ) == NULL 
				|| strcmp( GuildMark[m_sGuildMarkIndex].GuildName, GuildMark[m_sGuildMarkIndex].UnionName ) == NULL )
			{
				
				m_pSiegeWarUI = new CNewUISiegeWarCommander;	// Commander
				m_iCurSiegeWarType = SIEGEWAR_TYPE_COMMANDER;
				
			}
			else 
			{	
				m_pSiegeWarUI = new CNewUISiegeWarSoldier;	// Soldier
				m_iCurSiegeWarType = SIEGEWAR_TYPE_SOLDIER;	
			}
			m_bCreated = true;
		}break;		
	default:
		{
			m_pSiegeWarUI = new CNewUISiegeWarSoldier;	// Soldier
			m_iCurSiegeWarType = SIEGEWAR_TYPE_SOLDIER;
			m_bCreated = true;
		}break;
	}

	// UI생성
	m_pSiegeWarUI->LoadImages();
	m_pSiegeWarUI->Create( m_Pos.x, m_Pos.y );
	Show( true );
	
	return true;
}

void SEASON3B::CNewUISiegeWarfare::ClearGuildMemberLocation( void )
{
	if( m_iCurSiegeWarType == SIEGEWAR_TYPE_COMMANDER )
	{
		((CNewUISiegeWarCommander*)m_pSiegeWarUI)->ClearGuildMemberLocation( );
	}
}

void SEASON3B::CNewUISiegeWarfare::SetGuildMemberLocation ( BYTE type, int x, int y )
{
	if( m_iCurSiegeWarType == SIEGEWAR_TYPE_COMMANDER )
	{
		((CNewUISiegeWarCommander*)m_pSiegeWarUI)->SetGuildMemberLocation( type, x, y );
	}
}

//---------------------------------------------------------------------------------------------
// ResetType
void SEASON3B::CNewUISiegeWarfare::InitMiniMapUI( ) 
{
	if( m_pSiegeWarUI == NULL )
	{
		return;
	}

	m_pSiegeWarUI->UnLoadImages();
	m_pSiegeWarUI->Release();

	SAFE_DELETE( m_pSiegeWarUI );
	
	m_iCurSiegeWarType = SIEGEWAR_TYPE_NONE;
	m_byGuildStatus = G_NONE;
	m_sGuildMarkIndex = -1;
}

//---------------------------------------------------------------------------------------------
// SetTime
// 서버에서 시간을 받는다.
void  SEASON3B::CNewUISiegeWarfare::SetTime( BYTE byHour, BYTE byMinute )
{
    m_iHour = (int)byHour;
    m_iMinute = (int)byMinute;
	m_iSecond = 60000;
	m_dwSyncTime = GetTickCount();
}

//---------------------------------------------------------------------------------------------
// SetMapInfo
void SEASON3B::CNewUISiegeWarfare::SetMapInfo( GuildCommander& data )
{
	if( m_pSiegeWarUI == NULL ) 
	{
		return;	
	}
	
	m_pSiegeWarUI->SetMapInfo( data );
}

void SEASON3B::CNewUISiegeWarfare::InitSkillUI()
{
	if( m_pSiegeWarUI == NULL )
	{
		return;	
	}
	
	m_pSiegeWarUI->InitBattleSkill();
}

void SEASON3B::CNewUISiegeWarfare::ReleaseSkillUI()
{
	if( m_pSiegeWarUI == NULL )
	{
		return;
	}

	m_pSiegeWarUI->ReleaseBattleSkill();	
}
