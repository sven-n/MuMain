// CNewUIEmpireGuardianNPC.cpp: implementation of the CNewUIEmpireGuardianNPC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUISystem.h"
#include "NewUICommon.h"
#include "NewUIEmpireGuardianNPC.h"
#include "wsclientinline.h"
#ifdef KJH_FIX_EMPIREGUARDIAN_ENTER_UI_RENDER_TEXT_ONLY_GLOBAL
#include "UIControls.h"
#endif // KJH_FIX_EMPIREGUARDIAN_ENTER_UI_RENDER_TEXT_ONLY_GLOBAL

#ifdef LDK_ADD_EMPIREGUARDIAN_UI

using namespace SEASON3B;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIEmpireGuardianNPC::CNewUIEmpireGuardianNPC()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_bCanClick = true;

}

CNewUIEmpireGuardianNPC::~CNewUIEmpireGuardianNPC()
{
	Release( );
}

bool CNewUIEmpireGuardianNPC::Create( CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y )
{
	if(NULL == pNewUIMng || NULL == pNewUI3DRenderMng || NULL == g_pNewItemMng)
		return false;
	
	m_pNewUIMng = pNewUIMng;
 	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC, this );		// 인터페이스 오브젝트 등록

	m_pNewUI3DRenderMng = pNewUI3DRenderMng;
	m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

	SetPos(x, y);
	
	LoadImages();

	InitButton(&m_btPositive, x + (NPC_WINDOW_WIDTH/2) - 27, y + 190, GlobalText[1593]);
	InitButton(&m_btNegative, x + (NPC_WINDOW_WIDTH/2) - 27, y + 380, GlobalText[1002]);

	Show( false );
	
	return true;
}

void CNewUIEmpireGuardianNPC::InitButton(CNewUIButton * pNewUIButton, int iPos_x, int iPos_y, const unicode::t_char * pCaption)
{
	pNewUIButton->ChangeText(pCaption);
	pNewUIButton->ChangeTextBackColor(RGBA(255,255,255,0));
	pNewUIButton->ChangeButtonImgState( true, IMAGE_EMPIREGUARDIAN_NPC_BTN, true );
	pNewUIButton->ChangeButtonInfo(iPos_x, iPos_y, 53, 23);
	pNewUIButton->ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	pNewUIButton->ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

void CNewUIEmpireGuardianNPC::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

void CNewUIEmpireGuardianNPC::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CNewUIEmpireGuardianNPC::UpdateMouseEvent()
{
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;
	
	// 파티 창 내 영역 클릭시 하위 UI처리 및 이동 불
	// ( #include "NewUICommon.h" )
 	if( CheckMouseIn( m_Pos.x, m_Pos.y, NPC_WINDOW_WIDTH, NPC_WINDOW_HEIGHT) )
 		return false;

	return true;
}

bool CNewUIEmpireGuardianNPC::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	return true;
}

bool CNewUIEmpireGuardianNPC::Update()
{
	if ( !IsVisible() )
		return true;

	return true;
}

bool CNewUIEmpireGuardianNPC::IsVisible() const
{
	return CNewUIObj::IsVisible();
}

bool CNewUIEmpireGuardianNPC::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);

 	// 프레임
	RenderFrame();

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
 	char szText[256] = {NULL, };
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	POINT Position = {m_Pos.x+(NPC_WINDOW_WIDTH/2), m_Pos.y};

	// 가이온님의 명령서가 없다면
	// 제국 수호군 요새에 들어갈 수 없다네.
	// 명령서를 보여주겠는가?
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->SetTextColor(220, 220, 220, 255);
#ifdef KJH_FIX_EMPIREGUARDIAN_ENTER_UI_RENDER_TEXT_ONLY_GLOBAL
	g_pRenderText->RenderText(m_Pos.x, Position.y+50, GlobalText[2795], 190, 0, RT3_SORT_CENTER);
	char szTextOut[2][300];
	CutStr(GlobalText[2796], szTextOut[0], 150, 2, 300);
	g_pRenderText->RenderText(m_Pos.x, Position.y+70, szTextOut[0], 190, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x, Position.y+90, szTextOut[1], 190, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x, Position.y+110, GlobalText[2797], 190, 0, RT3_SORT_CENTER);
#else // KJH_FIX_EMPIREGUARDIAN_ENTER_UI_RENDER_TEXT_ONLY_GLOBAL
#ifdef PBG_FIX_JELINTNPC_TEXTWIDTH
	g_pRenderText->RenderText(m_Pos.x, Position.y+60, GlobalText[2795], 190, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x, Position.y+80, GlobalText[2796], 190, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x, Position.y+100, GlobalText[2797], 190, 0, RT3_SORT_CENTER);
#else //PBG_FIX_JELINTNPC_TEXTWIDTH
	g_pRenderText->RenderText(Position.x-55, Position.y+60, GlobalText[2795], 110, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(Position.x-100, Position.y+80, GlobalText[2796], 200, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(Position.x-55, Position.y+100, GlobalText[2797], 110, 0, RT3_SORT_CENTER);
#endif //PBG_FIX_JELINTNPC_TEXTWIDTH
#endif // KJH_FIX_EMPIREGUARDIAN_ENTER_UI_RENDER_TEXT_ONLY_GLOBAL

	// 가이온의 명령서
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor( 255, 240, 0, 255 );
	g_pRenderText->RenderText(Position.x-55, Position.y+170, GlobalText[2783], 110, 0, RT3_SORT_CENTER);

	//입장
	m_btPositive.Render();

	// 라인
	RenderImage(IMAGE_EMPIREGUARDIAN_NPC_LINE, m_Pos.x+1, m_Pos.y+220, 188.f, 21.f);

	//주의
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor( 255, 0, 0, 255 );
	g_pRenderText->RenderText(Position.x-55, Position.y+260, GlobalText[2223], 110, 0, RT3_SORT_CENTER);

	//7차(일요일) 맵 진행 중
	//마지막 존 입장 시에는
	//완전한 세크로미콘을 소지해야
	//진행이 가능합니다.
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor( 220, 220, 220, 255 );
	g_pRenderText->RenderText(Position.x-100, Position.y+280, GlobalText[2835], 200, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(Position.x-100, Position.y+300, GlobalText[2836], 200, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(Position.x-100, Position.y+320, GlobalText[2837], 200, 0, RT3_SORT_CENTER);
#ifdef KJH_FIX_EMPIREGUARDIAN_ENTER_UI_RENDER_TEXT_ONLY_GLOBAL
	CutStr(GlobalText[2838], szTextOut[0], 155, 2, 300);
	g_pRenderText->RenderText(m_Pos.x, Position.y+340, szTextOut[0], 200, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x, Position.y+360, szTextOut[1], 200, 0, RT3_SORT_CENTER);
#else // KJH_FIX_EMPIREGUARDIAN_ENTER_UI_RENDER_TEXT_ONLY_GLOBAL
	g_pRenderText->RenderText(Position.x-100, Position.y+340, GlobalText[2838], 200, 0, RT3_SORT_CENTER);
#endif // KJH_FIX_EMPIREGUARDIAN_ENTER_UI_RENDER_TEXT_ONLY_GLOBAL
	
	//닫기
	m_btNegative.Render();

	DisableAlphaBlend();

	return true;
}

bool CNewUIEmpireGuardianNPC::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
	
	//. Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC);
	}

	if (m_btNegative.UpdateMouseEvent())
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC);
	}

	if (m_btPositive.UpdateMouseEvent())
	{
		// 조건 필요??
		SendRequestEnterEmpireGuardianEvent();
		::PlayBuffer(SOUND_INTERFACE01);
		m_bCanClick = false;
		return true;
	}

	return false;
}

float CNewUIEmpireGuardianNPC::GetLayerDepth()
{
	return 1.2f;
}

void CNewUIEmpireGuardianNPC::OpenningProcess()
{
}

void CNewUIEmpireGuardianNPC::ClosingProcess()
{
}

void CNewUIEmpireGuardianNPC::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_EMPIREGUARDIAN_NPC_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_EMPIREGUARDIAN_NPC_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_EMPIREGUARDIAN_NPC_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_EMPIREGUARDIAN_NPC_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_EMPIREGUARDIAN_NPC_BOTTOM, GL_LINEAR);

	//btn
	LoadBitmap("Interface\\newui_btn_empty_very_small.tga", IMAGE_EMPIREGUARDIAN_NPC_BTN, GL_LINEAR);

	//line
	LoadBitmap("Interface\\newui_myquest_Line.tga", IMAGE_EMPIREGUARDIAN_NPC_LINE, GL_LINEAR);
}

void CNewUIEmpireGuardianNPC::UnloadImages()
{
	DeleteBitmap( IMAGE_EMPIREGUARDIAN_NPC_TOP );
	DeleteBitmap( IMAGE_EMPIREGUARDIAN_NPC_LEFT );
	DeleteBitmap( IMAGE_EMPIREGUARDIAN_NPC_RIGHT );
	DeleteBitmap( IMAGE_EMPIREGUARDIAN_NPC_BOTTOM );
	DeleteBitmap( IMAGE_EMPIREGUARDIAN_NPC_BACK );
	DeleteBitmap( IMAGE_EMPIREGUARDIAN_NPC_BTN );
	DeleteBitmap( IMAGE_EMPIREGUARDIAN_NPC_LINE );
}

void CNewUIEmpireGuardianNPC::RenderFrame()
{
	// 프레임
	RenderImage(IMAGE_EMPIREGUARDIAN_NPC_BACK, m_Pos.x, m_Pos.y, float(NPC_WINDOW_WIDTH), float(NPC_WINDOW_HEIGHT));
	RenderImage(IMAGE_EMPIREGUARDIAN_NPC_TOP, m_Pos.x, m_Pos.y, float(NPC_WINDOW_WIDTH), 64.f);
	RenderImage(IMAGE_EMPIREGUARDIAN_NPC_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_EMPIREGUARDIAN_NPC_RIGHT, m_Pos.x+NPC_WINDOW_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_EMPIREGUARDIAN_NPC_BOTTOM, m_Pos.x, m_Pos.y+NPC_WINDOW_HEIGHT-45, float(NPC_WINDOW_WIDTH), 45.f);

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(220, 220, 220, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	
	// header : 보좌관 제린트
	g_pRenderText->RenderText(m_Pos.x+(NPC_WINDOW_WIDTH/2)-55, m_Pos.y+13, GlobalText[2794], 110, 0, RT3_SORT_CENTER);
}

void CNewUIEmpireGuardianNPC::Render3D()
{
	RenderItem3D();
}

void CNewUIEmpireGuardianNPC::RenderItem3D()
{
	POINT ptOrigin = { m_Pos.x, m_Pos.y+50 };
	
	int nItemType = ITEM_POTION+102;
    int nItemLevel = 0;
	
	::RenderItem3D(ptOrigin.x+(190-20)/2, ptOrigin.y+70, 20.0f, 27.0f, nItemType, nItemLevel, 0, 0, false);
}
#endif	// LDK_ADD_EMPIREGUARDIAN_UI
