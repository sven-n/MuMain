// NewUIHeroPositionInfo.cpp: implementation of the CNewUIHeroPositionInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIHeroPositionInfo.h"
#include "NewUISystem.h"
#include "wsclientinline.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIHeroPositionInfo::CNewUIHeroPositionInfo()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_CurHeroPosition.x = m_CurHeroPosition.y = 0;
}

CNewUIHeroPositionInfo::~CNewUIHeroPositionInfo()
{
	Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIHeroPositionInfo::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_HERO_POSITION_INFO, this );		// 인터페이스 오브젝트 등록

#ifdef PJH_ADD_MINIMAP
	WidenX = HERO_POSITION_INFO_BASEB_WINDOW_WIDTH;
	if(WindowWidth == 800)
	{
		WidenX = (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH + (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH * 0.4f));
	}
	else
	if(WindowWidth == 1024)
	{
		WidenX = (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH + (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH * 0.2f));
	}
#endif //PJH_ADD_MINIMAP

	SetPos(x, y);
	
	LoadImages();
	
	Show( true );
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void CNewUIHeroPositionInfo::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

//---------------------------------------------------------------------------------------------
// SetPos
void CNewUIHeroPositionInfo::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

//---------------------------------------------------------------------------------------------
// BtnProcess
bool CNewUIHeroPositionInfo::BtnProcess()
{
	
	return false;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool CNewUIHeroPositionInfo::UpdateMouseEvent()
{
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;

#ifdef PJH_ADD_MINIMAP
	// 파티 창 내 영역 클릭시 하위 UI처리 및 이동 불가
	int Width = HERO_POSITION_INFO_BASEA_WINDOW_WIDTH + WidenX + HERO_POSITION_INFO_BASEC_WINDOW_WIDTH;
	if( CheckMouseIn(m_Pos.x, m_Pos.y, Width, HERO_POSITION_INFO_BASE_WINDOW_HEIGHT) )
		return false;
#else
	// 파티 창 내 영역 클릭시 하위 UI처리 및 이동 불가
	if( CheckMouseIn(m_Pos.x, m_Pos.y, HERO_POSITION_INFO_BASE_WINDOW_WIDTH, HERO_POSITION_INFO_BASE_WINDOW_HEIGHT) )
		return false;
#endif //PJH_ADD_MINIMAP	
	
	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool CNewUIHeroPositionInfo::UpdateKeyEvent()
{
	return true;
}

//---------------------------------------------------------------------------------------------
// Update
bool CNewUIHeroPositionInfo::Update()
{
	if( (IsVisible() == true) && (Hero != NULL) )
	{
		m_CurHeroPosition.x = ( Hero->PositionX );
		m_CurHeroPosition.y = ( Hero->PositionY );
	}

	return true;
}

//---------------------------------------------------------------------------------------------
// Render
bool CNewUIHeroPositionInfo::Render()
{
	unicode::t_char szText[256] = {NULL, };

	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );

#ifdef PJH_ADD_MINIMAP
	RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW, m_Pos.x, m_Pos.y, 
						float(HERO_POSITION_INFO_BASEA_WINDOW_WIDTH), float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT));

	RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1, m_Pos.x+HERO_POSITION_INFO_BASEA_WINDOW_WIDTH, m_Pos.y, 
			float(WidenX), float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT),
			0.1f,0.f,22.4f/32.f,25.f/32.f);


	RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW+2, m_Pos.x+HERO_POSITION_INFO_BASEA_WINDOW_WIDTH+WidenX, m_Pos.y, 
						float(HERO_POSITION_INFO_BASEC_WINDOW_WIDTH), float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT));
#else
	RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW, m_Pos.x, m_Pos.y, 
						float(HERO_POSITION_INFO_BASE_WINDOW_WIDTH), float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT));
#endif //PJH_ADD_MINIMAP


//GetMapName( pMember->Map )
#ifdef PJH_ADD_MINIMAP
	unicode::_sprintf( szText, "%s (%d , %d)",  GetMapName( World ), m_CurHeroPosition.x, m_CurHeroPosition.y);
	g_pRenderText->RenderText( m_Pos.x+10, m_Pos.y+5, szText, WidenX + 20, 13-4, RT3_SORT_CENTER );
#else

	unicode::_sprintf( szText, "%d %d", m_CurHeroPosition.x, m_CurHeroPosition.y );
	g_pRenderText->RenderText( m_Pos.x+16
#ifdef PJH_ADD_MINIMAP
		+ (WidenX - 25)
#endif //PJH_ADD_MINIMAP
		, m_Pos.y+4, szText, 50-16, 13-4, RT3_SORT_CENTER );
#endif //PJH_ADD_MINIMAP

	DisableAlphaBlend();
		
	
	return true;
}

//---------------------------------------------------------------------------------------------
// GetLayerDepth
float CNewUIHeroPositionInfo::GetLayerDepth()
{
#ifdef PJH_ADD_MINIMAP
	#ifdef MOD_HERO_POS_INFO_DEPTH
	return 8.15f;
	#else //MOD_HERO_POS_INFO_DEPTH
	return 8.2f;
	#endif //MOD_HERO_POS_INFO_DEPTH
#endif //PJH_ADD_MINIMAP
	return 4.3f;
}

//---------------------------------------------------------------------------------------------
// OpenningProcess
void CNewUIHeroPositionInfo::OpenningProcess()
{
	
}

//---------------------------------------------------------------------------------------------
// ClosingProcess
void CNewUIHeroPositionInfo::ClosingProcess()
{
	
}

void CNewUIHeroPositionInfo::SetCurHeroPosition( int x, int y )
{
	m_CurHeroPosition.x = x;
	m_CurHeroPosition.y = y;
}

//---------------------------------------------------------------------------------------------
// LoadImages
void CNewUIHeroPositionInfo::LoadImages()
{
#ifdef PJH_ADD_MINIMAP
	LoadBitmap("Interface\\Minimap_positionA.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW, GL_LINEAR);
	LoadBitmap("Interface\\Minimap_positionB.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW+1, GL_LINEAR);
	LoadBitmap("Interface\\Minimap_positionC.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW+2, GL_LINEAR);
#else
	LoadBitmap("Interface\\newui_position02.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW, GL_LINEAR);
#endif //PJH_ADD_MINIMAP

}

//---------------------------------------------------------------------------------------------
// UnloadImages
void CNewUIHeroPositionInfo::UnloadImages()
{
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW);
#ifdef PJH_ADD_MINIMAP
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW+1);
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW+2);
#endif //PJH_ADD_MINIMAP
}


