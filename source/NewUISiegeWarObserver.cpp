// NewUISiegeWarObserver.cpp: implementation of the CNewUISiegeWarObserver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUISiegeWarObserver.h"
#include "wsclientinline.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUISiegeWarObserver::CNewUISiegeWarObserver()
{

}

CNewUISiegeWarObserver::~CNewUISiegeWarObserver()
{

}

//---------------------------------------------------------------------------------------------
// OnCreate
bool SEASON3B::CNewUISiegeWarObserver::OnCreate( int x, int y )
{
	return true;
}

//---------------------------------------------------------------------------------------------
// OnRelease
void SEASON3B::CNewUISiegeWarObserver::OnRelease()
{
	
}

//---------------------------------------------------------------------------------------------
// OnUpdate
bool SEASON3B::CNewUISiegeWarObserver::OnUpdate()
{
	return true;
}

//---------------------------------------------------------------------------------------------
// OnRender
bool SEASON3B::CNewUISiegeWarObserver::OnRender()
{
	EnableAlphaTest();
	glColor4f( 1.f, 1.f, 1.f, m_fMiniMapAlpha );
	
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );

	// 미니맵에 모든 캐릭터의 위치 렌더
	RenderCharPosInMiniMap();

	DisableAlphaBlend();
	
	return true;
}

//---------------------------------------------------------------------------------------------
// OnSetPos
void SEASON3B::CNewUISiegeWarObserver::OnSetPos( int x, int y )
{

}

//---------------------------------------------------------------------------------------------
// RenderCharPosInMiniMap
// 미니맵에 모든 캐릭터의 위치 렌더
void SEASON3B::CNewUISiegeWarObserver::RenderCharPosInMiniMap()
{
	float fPosX, fPosY;	
	
	glColor4f( 0.8f, 0.8f, 0.8f, m_fMiniMapAlpha );	

    for ( int i=0; i<MAX_CHARACTERS_CLIENT; ++i )
    {
        CHARACTER* c = &CharactersClient[i];
        if ( c!=NULL && c->Object.Live && c!=Hero && ( c->Object.Kind==KIND_PLAYER || c->Object.Kind==KIND_MONSTER || c->Object.Kind==KIND_NPC ) )
        {
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
            OBJECT* o = &c->Object;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

			fPosX = ( ( c->PositionX ) )/m_iMiniMapScale - m_MiniMapScaleOffset.x + m_MiniMapPos.x;
            fPosY = ( 256 - ( c->PositionY ) )/m_iMiniMapScale - m_MiniMapScaleOffset.y + m_MiniMapPos.y;
            RenderColor( fPosX, fPosY, 3, 3 );
		}
	}
}

//---------------------------------------------------------------------------------------------
// OnUpdateMouseEvent
bool SEASON3B::CNewUISiegeWarObserver::OnUpdateMouseEvent()
{
	if( OnBtnProcess() )
		return false;	

	return true;
}

//---------------------------------------------------------------------------------------------
// OnUpdateKeyEvent
bool SEASON3B::CNewUISiegeWarObserver::OnUpdateKeyEvent()
{
	return true;
}

//---------------------------------------------------------------------------------------------
// OnBtnProcess
bool SEASON3B::CNewUISiegeWarObserver::OnBtnProcess()
{
	return false;
}

//---------------------------------------------------------------------------------------------
// OnLoadImages
void SEASON3B::CNewUISiegeWarObserver::OnLoadImages()
{

}

//---------------------------------------------------------------------------------------------
// OnUnloadImages
void SEASON3B::CNewUISiegeWarObserver::OnUnloadImages()
{

}
