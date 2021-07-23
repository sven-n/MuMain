// NewUIGuildMakeWindow.cpp: implementation of the CNewUIGuildMakeWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIGuildMakeWindow.h"
#include "NewUIManager.h"
#include "NewUICommonMessageBox.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"

extern MARK_t		GuildMark[MAX_MARKS];
extern int			SelectMarkColor;

namespace
{
	BOOL IsGuildName( const char* szName )
	{
		if( strlen( szName ) >= 4 )
			return TRUE;
		else
			return FALSE;
	}
	
	BOOL IsGuildMark()
	{
		// 길드마크를 그렸는지 검사
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		BOOL bDraw = FALSE;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
		for( int i=0;i<64;i++ )
		{
			if( GuildMark[MARK_EDIT].Mark[i] != 0 )
				return TRUE;
		}
		
		return FALSE;
	}

	void UpdateEditGuildMark( int iPos_x, int iPos_y )
	{
		int i, j;
		float x, y;
		Hero->Object.Angle[2] = 90.f + 22.5f;
		for( i=0 ; i<8 ; ++i )
		{
			for( j=0; j<8 ; ++j )
			{
				x = iPos_x + j*15 + 50;
				y = iPos_y + i*15 + 100;
				if( MouseX>=x && MouseX<x+15 && MouseY>=y && MouseY<y+15 )
				{
					if( MouseLButton )
						GuildMark[MARK_EDIT].Mark[i*8+j] = SelectMarkColor;
					if( MouseRButton )
						GuildMark[MARK_EDIT].Mark[i*8+j] = 0;
				}
			}
		}
		for( i=0 ; i<2 ; ++i )
		{
			for( j=0 ; j<8 ; ++j )
			{
				x = iPos_x + j*20 + 15;
				y = iPos_y + i*20 + 260;
				if( MouseX>=x && MouseX<x+20 && MouseY>=y && MouseY<y+20 )
				{
					if( MouseLButtonPush )
					{
						MouseLButtonPush = FALSE;
						PlayBuffer( SOUND_CLICK01 );
						SelectMarkColor = i*8 + j;
					}
				}
			}
		}
	}

	void RenderEditGuildMark( int iPos_x, int iPos_y )
	{
		int i, j;
		float x, y;
		for( i=0 ; i<8 ; ++i )
		{
			for( j=0 ; j<8 ; ++j )
			{
				x = (float)iPos_x + j*15 + 50;
				y = (float)iPos_y + i*15 + 100;
				RenderGuildColor( x+1, y+1, 13, 13, GuildMark[MARK_EDIT].Mark[i*8+j] );
			}
		}
		for( i=0 ; i<2 ; ++i )
		{
			for( j=0 ; j<8 ; ++j )
			{
				x = (float)iPos_x + j*20 + 15;
				y = (float)iPos_y + i*20 + 260;
				RenderGuildColor( x+1, y+1, 18, 18, i*8+j );
			}
		}
		x = (float)iPos_x + 15;
		y = (float)iPos_y + 230;
		RenderGuildColor( x+1, y+1, 23, 23, SelectMarkColor );

		g_pRenderText->SetFont(g_hFont);
		g_pRenderText->SetTextColor(230, 230, 230, 255);
		g_pRenderText->SetBgColor(0, 0, 0, 0);
		g_pRenderText->RenderText(iPos_x+50, iPos_y+230, GlobalText[183]);
		g_pRenderText->RenderText(iPos_x+50, iPos_y+245, GlobalText[184]);
	}

	void RenderGoldRect(float x, float y, float sx, float sy, int fill = 0)
	{
		switch (fill)
		{
		case 1:
			glColor4ub(146, 144, 141, 200);
			RenderColor(x, y, sx, sy);
			EndRenderColor();
			break;
		};

		RenderBitmap(BITMAP_INVENTORY+19, x, y, sx, 2,
			10/256.f, 5/16.f, 170.f/256.f, 2.f/16.f);
		RenderBitmap(BITMAP_INVENTORY+19, x, y + sy, sx + 1, 2,
			10/256.f, 5/16.f, 170.f/256.f, 2.f/16.f);
		RenderBitmap(BITMAP_INVENTORY, x, y, 2, sy,
			1.f/256.f, 5/16.f, 2.f/256.f, 125.f/256.f);
		RenderBitmap(BITMAP_INVENTORY, x + sx, y, 2, sy,
			1.f/256.f, 5/16.f, 2.f/256.f, 125.f/256.f);
	}

	void RenderText( char* text, int x, int y, int sx, int sy, DWORD color, DWORD backcolor, int sort )
	{
		g_pRenderText->SetFont(g_hFont);

		DWORD backuptextcolor = g_pRenderText->GetTextColor();
		DWORD backuptextbackcolor = g_pRenderText->GetBgColor();

		g_pRenderText->SetTextColor(color);
		g_pRenderText->SetBgColor(backcolor);
		g_pRenderText->RenderText(x, y, text, sx, sy, sort);

		g_pRenderText->SetTextColor(backuptextcolor);
		g_pRenderText->SetBgColor(backuptextbackcolor);
	}
};

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIGuildMakeWindow::CNewUIGuildMakeWindow() : m_pNewUIMng(NULL), m_EditBox(NULL), m_Button(NULL),
 m_GuildMakeState( GUILDMAKE_INFO )
{

}

CNewUIGuildMakeWindow::~CNewUIGuildMakeWindow()
{
	Release();
}

bool CNewUIGuildMakeWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng)
		return false;

	LoadImages();
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_NPCGUILDMASTER, this);
	SetPos(x, y);

	m_EditBox = new CUITextInputBox;
	m_EditBox->Init(g_hWnd, 200, 14, MAXGUILDNAME);
	m_EditBox->SetPosition(m_Pos.x+50, m_Pos.y+66);
	m_EditBox->SetTextColor(255, 255, 230, 210);
	m_EditBox->SetBackColor(0, 0, 0, 25);
	m_EditBox->SetFont(g_hFont);
	m_EditBox->SetState(UISTATE_NORMAL);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	m_EditBox->SetOption(UIOPTION_NOLOCALIZEDCHARACTERS);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	m_Button = new CNewUIButton[GUILDMAKEBUTTON_COUNT];

	for( int i = 0; i < GUILDMAKEBUTTON_COUNT; ++i )
	{
		if( i == 0 )
		{
			m_Button[i].ChangeButtonImgState( true, IMAGE_GUILDMAKE_MAKEBUTTON, true );
			m_Button[i].ChangeButtonInfo( 0, 0, 108, 29 );
		}
		else
		{
			m_Button[i].ChangeButtonImgState( true, IMAGE_GUILDMAKE_NEXTBUTTON, true );
			m_Button[i].ChangeButtonInfo( 0, 0, 64, 29 );
		}
	}

	// Exit Button
	m_BtnExit.ChangeButtonImgState( true, IMAGE_GUILDMAKE_BTN_EXIT, false );
	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );		
	m_BtnExit.ChangeToolTipText( GlobalText[1002], true );	// 1002 "닫기"
	
	Show(false);

	return true;
}

void CNewUIGuildMakeWindow::Release()
{
	SAFE_DELETE_ARRAY(m_Button);
	SAFE_DELETE(m_EditBox);

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}

	UnloadImages();
}

void CNewUIGuildMakeWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_GUILDMAKE_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_btn_empty.tga", IMAGE_GUILDMAKE_MAKEBUTTON, GL_LINEAR);
	LoadBitmap("Interface\\newui_btn_empty_small.tga", IMAGE_GUILDMAKE_NEXTBUTTON, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_GUILDMAKE_BACK_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_GUILDMAKE_BACK_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_GUILDMAKE_BACK_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_GUILDMAKE_BACK_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_GUILDMAKE_BTN_EXIT, GL_LINEAR);	
	LoadBitmap("Interface\\newui_guildmakeeditbox.tga", IMAGE_GUILDMAKE_EDITBOX, GL_LINEAR);
}

void CNewUIGuildMakeWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_GUILDMAKE_EDITBOX);
	DeleteBitmap(IMAGE_GUILDMAKE_BTN_EXIT);
	DeleteBitmap(IMAGE_GUILDMAKE_NEXTBUTTON);
	DeleteBitmap(IMAGE_GUILDMAKE_MAKEBUTTON);
	DeleteBitmap(IMAGE_GUILDMAKE_BACK_BOTTOM);
	DeleteBitmap(IMAGE_GUILDMAKE_BACK_RIGHT);
	DeleteBitmap(IMAGE_GUILDMAKE_BACK_LEFT);
	DeleteBitmap(IMAGE_GUILDMAKE_BACK_TOP);
	DeleteBitmap(IMAGE_GUILDMAKE_BACK);
}

float CNewUIGuildMakeWindow::GetLayerDepth()
{
	return 4.3f;
}

void CNewUIGuildMakeWindow::ClosingProcess()
{
	ChangeWindowState(GUILDMAKE_INFO);
	ChangeEditBox(UISTATE_HIDE);

	SendRequestGuildMaster( FALSE );
}

void CNewUIGuildMakeWindow::ChangeWindowState(const GUILDMAKE_STATE state)
{
	m_GuildMakeState = state;
}

void CNewUIGuildMakeWindow::ChangeEditBox(const UISTATES type)
{
	m_EditBox->SetState(type);

	if( type == UISTATE_NORMAL )
	{
		m_EditBox->GiveFocus();
	}

	m_EditBox->SetText(NULL);
}

// 손을 좀 더 보자..
// 글로벌 변수에 뒤죽 박죽...-=-;;;
bool CNewUIGuildMakeWindow::UpdateGMInfo()
{
	m_Button[GUILDMAKEBUTTON_INFO_MAKE].SetPos( m_Pos.x+((190/2)-(108/2)), m_Pos.y+100 );
	// 1303 "길드 만들기"
	m_Button[GUILDMAKEBUTTON_INFO_MAKE].ChangeText(GlobalText[1303]);

	//길드 만들겠다는 버튼
	if( m_Button[GUILDMAKEBUTTON_INFO_MAKE].UpdateMouseEvent() )
	{
		SendRequestGuildMaster( TRUE );
		ChangeWindowState(GUILDMAKE_MARK);
		ChangeEditBox(UISTATE_NORMAL);
		return true;
	}

	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCGUILDMASTER);
		return true;
	}

	return false;
}

bool CNewUIGuildMakeWindow::UpdateGMMark()
{
	m_EditBox->DoAction();

	// 길드 마크 그리기
	UpdateEditGuildMark( m_Pos.x, m_Pos.y );

	//button
	m_Button[GUILDMAKEBUTTON_MARK_LNEXT].SetPos(m_Pos.x+15, m_Pos.y+379);
	m_Button[GUILDMAKEBUTTON_MARK_LNEXT].ChangeText(GlobalText[1306]); //"이전"

	m_Button[GUILDMAKEBUTTON_MARK_RNEXT].SetPos(m_Pos.x+110, m_Pos.y+379);
	m_Button[GUILDMAKEBUTTON_MARK_RNEXT].ChangeText(GlobalText[1305]); //"다음"

	if( m_Button[GUILDMAKEBUTTON_MARK_LNEXT].UpdateMouseEvent() )
	{
		ChangeWindowState(GUILDMAKE_INFO);
		ChangeEditBox(UISTATE_HIDE);
		return true;
	}

	if( m_Button[GUILDMAKEBUTTON_MARK_RNEXT].UpdateMouseEvent() )
	{
		//길드 마크가 있는지 길드명이 있는지 검사
		char tempText[100];
		memset(&tempText, 0, sizeof(char)*100);

		m_EditBox->GetText( tempText );

		if(CheckSpecialText(tempText) == true)
		{
			// 391 "특수문자는 사용하실 수 없습니다."
			SEASON3B::CreateOkMessageBox(GlobalText[391]);
		}
		else if( IsGuildName( tempText ) == FALSE)
		{
			// 390 "2글자이상 입력해주세요."
			CreateOkMessageBox(GlobalText[390]);
		}
		else if(IsGuildMark() == FALSE)
		{
			// 426 "길드 마크를 그려주세요."
			CreateOkMessageBox(GlobalText[426]);
		}
		else
		{
			strcpy( GuildMark[MARK_EDIT].GuildName, tempText );
			ChangeWindowState(GUILDMAKE_RESULTINFO);
			ChangeEditBox(UISTATE_HIDE);

			return true;
		}
	}

	return false;
}

bool CNewUIGuildMakeWindow::UpdateGMResultInfo()
{
	m_Button[GUILDMAKEBUTTON_RESULTINFO_LNEXT].SetPos(m_Pos.x+15, m_Pos.y+379);
	m_Button[GUILDMAKEBUTTON_RESULTINFO_LNEXT].ChangeText(GlobalText[1306]);

	m_Button[GUILDMAKEBUTTON_RESULTINFO_RNEXT].SetPos(m_Pos.x+110, m_Pos.y+379);
	m_Button[GUILDMAKEBUTTON_RESULTINFO_RNEXT].ChangeText(GlobalText[1305]);

	if( m_Button[GUILDMAKEBUTTON_RESULTINFO_LNEXT].UpdateMouseEvent() )
	{
		ChangeWindowState(GUILDMAKE_MARK);
		ChangeEditBox(UISTATE_NORMAL);
		return true;
	}

	if( m_Button[GUILDMAKEBUTTON_RESULTINFO_RNEXT].UpdateMouseEvent() )
	{
		// 길드마크 얻기
		BYTE Mark[32];
		for( int i=0 ; i<64; i++ )
		{
			if( i % 2 == 0 )
      			Mark[i / 2] = GuildMark[MARK_EDIT].Mark[i] << 4;
			else
     			Mark[i / 2] += GuildMark[MARK_EDIT].Mark[i];
		}

		// 길드 생성시
		SendRequestCreateGuild( 0, (BYTE*)GuildMark[MARK_EDIT].GuildName, Mark );
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCGUILDMASTER);
		return true;
	}

	return false;
}

void CNewUIGuildMakeWindow::RenderGMInfo()
{
	char Text[100];

	memset(&Text, 0, sizeof(char)*100);
	sprintf( Text, GlobalText[181]); //"길드 마스터가 되시겠습니까?"
	RenderText( Text, m_Pos.x, m_Pos.y+50, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER );

	m_Button[GUILDMAKEBUTTON_INFO_MAKE].Render();

	m_BtnExit.Render();
}

void CNewUIGuildMakeWindow::RenderGMMark()
{
	//edit box

	char Text[100];
	memset(&Text, 0, sizeof(char)*100);
	sprintf( Text, GlobalText[182]); //"길  드"
	RenderText( Text, m_Pos.x+10, m_Pos.y+66, 190, 0, 0xFF49B0FF, 0x00000000, RT3_SORT_LEFT );

	RenderImage(IMAGE_GUILDMAKE_EDITBOX, m_Pos.x+45, m_Pos.y+60, 108.f, 23.f);
	m_EditBox->Render();

	//명칭이 이상하다..그리는 부위 인데...수정을 해보자..
	RenderGoldRect( m_Pos.x+45, m_Pos.y+95, 130.f, 130.f );
	CreateGuildMark( MARK_EDIT );
	RenderEditGuildMark( m_Pos.x, m_Pos.y );

	m_Button[GUILDMAKEBUTTON_MARK_LNEXT].Render();
	m_Button[GUILDMAKEBUTTON_MARK_RNEXT].Render();
}

void CNewUIGuildMakeWindow::RenderGMResultInfo()
{
	// 길드마크
	RenderGoldRect( m_Pos.x+72, m_Pos.y+70, 53.f, 53.f );
	CreateGuildMark( MARK_EDIT );
	RenderBitmap( BITMAP_GUILD, m_Pos.x+72, m_Pos.y+74, 48, 48 );

	char Text[100];
	memset(&Text, 0, sizeof(char)*100);
	sprintf( Text, "%s : %s", GlobalText[182], GuildMark[MARK_EDIT].GuildName );
	RenderText( Text, m_Pos.x, m_Pos.y+140, 190, 0, 0xFF49B0FF, 0x00000000, RT3_SORT_CENTER );

	m_Button[GUILDMAKEBUTTON_RESULTINFO_LNEXT].Render();
	m_Button[GUILDMAKEBUTTON_RESULTINFO_RNEXT].Render();
}

void CNewUIGuildMakeWindow::RenderFrame()
{
	RenderImage(IMAGE_GUILDMAKE_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_GUILDMAKE_BACK_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_GUILDMAKE_BACK_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_GUILDMAKE_BACK_RIGHT, m_Pos.x+190-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_GUILDMAKE_BACK_BOTTOM, m_Pos.x, m_Pos.y+429-45, 190.f, 45.f);

	char Text[100];
	memset(&Text, 0, sizeof(char)*100);
	sprintf( Text, GlobalText[180]); //"길  드"
	RenderText( Text, m_Pos.x, m_Pos.y+15, 190, 0, 0xFF49B0FF, 0x00000000, RT3_SORT_CENTER );
}

bool CNewUIGuildMakeWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCGUILDMASTER) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCGUILDMASTER);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

	return true;
}

bool CNewUIGuildMakeWindow::Update()
{
	return true;
}

bool CNewUIGuildMakeWindow::UpdateMouseEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCGUILDMASTER) == false)
	{
		return true;
	}

	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
	//. Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCGUILDMASTER);
		return false;
	}

	bool bResult = false;

	switch( m_GuildMakeState )
	{
	case GUILDMAKE_INFO: bResult = UpdateGMInfo(); 
		break;
	case GUILDMAKE_MARK: bResult = UpdateGMMark(); 
		break;
	case GUILDMAKE_RESULTINFO: bResult = UpdateGMResultInfo(); 
		break;
	}

	if(bResult == true)
	{
		return false;
	}
	
	// 인터페이스 내에 클릭하면 아무 동작 안되게 처리
	if(CheckMouseIn(m_Pos.x, m_Pos.y, GUILDMAKE_WIDTH, GUILDMAKE_HEIGHT))
	{
		if(SEASON3B::IsPress(VK_RBUTTON))
		{
			MouseRButton = false;
			MouseRButtonPop = false;
			MouseRButtonPush = false;
		}

		return false;
	}

	return true;
}

bool CNewUIGuildMakeWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);

	//윈도우 기본 프레임
	RenderFrame();

	switch( m_GuildMakeState )
	{
	case GUILDMAKE_INFO: RenderGMInfo(); break;
	case GUILDMAKE_MARK: RenderGMMark(); break;
	case GUILDMAKE_RESULTINFO: RenderGMResultInfo(); break;
	}

	DisableAlphaBlend();

	return true;
}
