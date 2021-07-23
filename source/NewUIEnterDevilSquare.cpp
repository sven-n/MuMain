// NewUIEnterBloodCastle.cpp: implementation of the CNewUIPartyInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIEnterDevilSquare.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "wsclientinline.h"
#include "ZzzCharacter.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIEnterDevilSquare::CNewUIEnterDevilSquare()
{
	m_pNewUIMng = NULL;
	memset( &m_Pos, 0, sizeof(POINT) );
	memset( &m_EnterUITextPos, 0, sizeof(POINT) );

	m_iNumActiveBtn = 1;
	m_BtnEnterStartPos.x = m_BtnEnterStartPos.y = 0;
	m_dwBtnTextColor[0] = RGBA( 150, 150, 150, 255 );
	m_dwBtnTextColor[1] = RGBA( 255, 255 ,255, 255 );

	// 블러드캐슬 레벨제한 index
	// 0~6	일반 클래스
	m_iDevilSquareLimitLevel[0][0] = 15; m_iDevilSquareLimitLevel[0][1] = 130;
	m_iDevilSquareLimitLevel[1][0] = 131; m_iDevilSquareLimitLevel[1][1] = 180;
	m_iDevilSquareLimitLevel[2][0] = 181; m_iDevilSquareLimitLevel[2][1] = 230;
	m_iDevilSquareLimitLevel[3][0] = 231; m_iDevilSquareLimitLevel[3][1] = 280;
	m_iDevilSquareLimitLevel[4][0] = 281; m_iDevilSquareLimitLevel[4][1] = 330;
	m_iDevilSquareLimitLevel[5][0] = 331; m_iDevilSquareLimitLevel[5][1] = 400;
	m_iDevilSquareLimitLevel[6][0] = 0; m_iDevilSquareLimitLevel[6][1] = 0;

	// 7~13 마검사, 다크로드 클래스
	m_iDevilSquareLimitLevel[7][0] = 15; m_iDevilSquareLimitLevel[7][1] = 110;
	m_iDevilSquareLimitLevel[8][0] = 111; m_iDevilSquareLimitLevel[8][1] = 160;
	m_iDevilSquareLimitLevel[9][0] = 161; m_iDevilSquareLimitLevel[9][1] = 210;
	m_iDevilSquareLimitLevel[10][0] = 211; m_iDevilSquareLimitLevel[10][1] = 260;
	m_iDevilSquareLimitLevel[11][0] = 261; m_iDevilSquareLimitLevel[11][1] = 310;
	m_iDevilSquareLimitLevel[12][0] = 311; m_iDevilSquareLimitLevel[12][1] = 400;
	m_iDevilSquareLimitLevel[13][0] = 0; m_iDevilSquareLimitLevel[13][1] = 0;
}

CNewUIEnterDevilSquare::~CNewUIEnterDevilSquare()
{
	Release();			// 꼭 하자!!
}

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIEnterDevilSquare::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_DEVILSQUARE, this );		// 인터페이스 오브젝트 등록
	
	SetPos(x, y);
	
	LoadImages();
	
	// Exit Button
	m_BtnExit.ChangeButtonImgState( true, IMAGE_ENTERDS_BASE_WINDOW_BTN_EXIT, false );
	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );		
	m_BtnExit.ChangeToolTipText( GlobalText[1002], true );	// 1002 "닫기"
	
	// Enter Button
	int iVal=0;
	for(int i=0 ; i<MAX_ENTER_GRADE ; i++)
	{
		iVal = ENTER_BTN_VAL * i;
		m_BtnEnter[i].ChangeButtonImgState( true, IMAGE_ENTERDS_BASE_WINDOW_BTN_ENTER, true );
		m_BtnEnter[i].ChangeButtonInfo( m_BtnEnterStartPos.x, m_BtnEnterStartPos.y+iVal, 180, 29 );
	}
	
	Show( false );
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void CNewUIEnterDevilSquare::Release()
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
void CNewUIEnterDevilSquare::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	m_EnterUITextPos.x = m_Pos.x + 3;
	m_EnterUITextPos.y = m_Pos.y + 45;
	
	SetBtnPos( m_Pos.x+6, m_Pos.y+155 );

	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );

	for( int i=0 ; i<MAX_ENTER_GRADE ; i++ )
	{
		int iVal = ENTER_BTN_VAL * i;
		m_BtnEnter[i].ChangeButtonInfo( m_BtnEnterStartPos.x, m_BtnEnterStartPos.y+iVal, 180, 29);
	}
}

//---------------------------------------------------------------------------------------------
// SetBtnPos
void CNewUIEnterDevilSquare::SetBtnPos(int x, int y)
{
	m_BtnEnterStartPos.x = x;
	m_BtnEnterStartPos.y = y;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool CNewUIEnterDevilSquare::UpdateMouseEvent()
{
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;
	
	// 파티 창 내 영역 클릭시 하위 UI처리 및 이동 불
	// ( #include "NewUICommon" )
	if( CheckMouseIn(m_Pos.x, m_Pos.y, ENTERDS_BASE_WINDOW_WIDTH, ENTERDS_BASE_WINDOW_HEIGHT) )
		return false;

	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool CNewUIEnterDevilSquare::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_DEVILSQUARE);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------------------------
// Update
bool CNewUIEnterDevilSquare::Update()
{
	if( !IsVisible() )
		return true;
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Render
bool CNewUIEnterDevilSquare::Render()
{
	EnableAlphaTest();
	
	// Base Window
	RenderImage(IMAGE_ENTERDS_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, 
						float(ENTERDS_BASE_WINDOW_WIDTH), float(ENTERDS_BASE_WINDOW_HEIGHT));
	RenderImage(IMAGE_ENTERDS_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, 
						float(ENTERDS_BASE_WINDOW_WIDTH), 64.f);
	RenderImage(IMAGE_ENTERDS_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y+64.f, 
						21.f, float(ENTERDS_BASE_WINDOW_HEIGHT)-64.f-45.f);
	RenderImage(IMAGE_ENTERDS_BASE_WINDOW_RIGHT, m_Pos.x+float(ENTERDS_BASE_WINDOW_WIDTH)-21.f, 
						m_Pos.y+64.f, 21.f, float(ENTERDS_BASE_WINDOW_HEIGHT)-64.f-45.f);
	RenderImage(IMAGE_ENTERDS_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y+float(ENTERDS_BASE_WINDOW_HEIGHT)-45.f, 
						float(ENTERDS_BASE_WINDOW_WIDTH), 45.f);

	
	g_pRenderText->SetFont( g_hFontBold );
	g_pRenderText->SetTextColor( 0xFFFFFFFF );
	g_pRenderText->SetBgColor( 0x00000000 );
	
	// UI제목
	g_pRenderText->RenderText(m_Pos.x+60, m_Pos.y+12, GlobalText[39], 72, 0, RT3_SORT_CENTER);		// "악마의 광장"

	g_pRenderText->SetFont( g_hFont );

	// 대사
	g_pRenderText->RenderText(m_EnterUITextPos.x, m_EnterUITextPos.y, GlobalText[670], 190, 0, RT3_SORT_CENTER);		//  "그대는 자신의 힘을 시험해 볼"
	g_pRenderText->RenderText(m_EnterUITextPos.x, m_EnterUITextPos.y+15, GlobalText[671], 190, 0, RT3_SORT_CENTER);	// "기회를 얻었다. 지금까지"
	g_pRenderText->RenderText(m_EnterUITextPos.x, m_EnterUITextPos.y+30, GlobalText[672], 190, 0, RT3_SORT_CENTER);	// "그곳에 들어간 인간은 없었지."
	g_pRenderText->RenderText(m_EnterUITextPos.x, m_EnterUITextPos.y+45, GlobalText[673], 190, 0, RT3_SORT_CENTER);	// "눈에 보이는 것을 믿지 않는게"
	g_pRenderText->RenderText(m_EnterUITextPos.x, m_EnterUITextPos.y+60, GlobalText[674], 190, 0, RT3_SORT_CENTER);	// "좋을 것이다. 그대의 힘만이"
	g_pRenderText->RenderText(m_EnterUITextPos.x, m_EnterUITextPos.y+75, GlobalText[675], 190, 0, RT3_SORT_CENTER);	// "스스로를 구할 수 있을테니..."

	// 입장 버튼
	for( int i=0 ; i<MAX_ENTER_GRADE ; i++ )
	{
		m_BtnEnter[i].Render();
	}	
	
	// Exit Button
	m_BtnExit.Render();	

	DisableAlphaBlend();

	return true;
}

//---------------------------------------------------------------------------------------------
// BtnProcess
bool CNewUIEnterDevilSquare::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
	
	// Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_DEVILSQUARE );
		return true;
	}
	
	// Exit2 버튼 
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_DEVILSQUARE );
		return true;
	}

	// 데빌스퀘어 입장 버튼
	if( (m_iNumActiveBtn !=- 1) && (m_BtnEnter[m_iNumActiveBtn].UpdateMouseEvent() == true) )
	{	
		// 레벨에 맞는 데빌스퀘어로 입장!!
		int iItemIndex = -1;

		// 악마의광장 초대권 레벨 0은 무조건 입장 가능 (이벤트 아이템)
		iItemIndex = g_pMyInventory->FindItemIndex( ITEM_POTION+19, 0 );
		
		// 악마의광장 초대권을 검사
		if( iItemIndex == -1 )
		{
			iItemIndex = g_pMyInventory->FindItemIndex( ITEM_POTION+19, m_iNumActiveBtn+1 );
		}

#ifdef CSK_FREE_TICKET
		// 우선순위는 기존 입장권이 높다. 그래서 기존 입장권 검사후에 검사한다.
		// 악마의광장 자유입장권이 있는가 검사한다.
		if(iItemIndex == -1)
		{
			iItemIndex = g_pMyInventory->FindItemIndex( ITEM_HELPER+46, -1 );
		}
#endif // CSK_FREE_TICKET
		
		if( iItemIndex > -1)	
		{
			// 1번째인자는 입장할 square-1을 하여야 함
			SendRequestMoveToDevilSquare( m_iNumActiveBtn, MAX_EQUIPMENT+iItemIndex );	
		}
		else
		{
			// 데빌스퀘어 초대권이 없을때
			SEASON3B::CreateOkMessageBox( GlobalText[677] );		// "데빌스퀘어 초대권이 있어야 갈 수 있습니다."
		}
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_DEVILSQUARE );
		
	}
	
	return false;
}

float CNewUIEnterDevilSquare::GetLayerDepth()
{
	return 4.0f;
}

//---------------------------------------------------------------------------------------------
// CheckLimitLV
// 캐릭터 레벨에 따른 입성 레벨을 구한다.
int CNewUIEnterDevilSquare::CheckLimitLV( int iIndex )
{
	int	iVal = 0;
	int iRet = 0;
	
	if( iIndex == 1 )
	{
		iVal = 7;
	}

	int iLevel;
	if(IsMasterLevel( CharacterAttribute->Class ) == true )
		iLevel = Master_Level_Data.nMLevel;
	else
	iLevel = CharacterAttribute->Level;
//Master_Level_Data.nMLevel

	if(IsMasterLevel( CharacterAttribute->Class ) == false )
	{
		for(int iCastleLV=0; iCastleLV<MAX_ENTER_GRADE-1; ++iCastleLV)
		{
			if ( iLevel >= m_iDevilSquareLimitLevel[iVal+iCastleLV][0] 
				&& iLevel <= m_iDevilSquareLimitLevel[iVal+iCastleLV][1])
			{
				iRet = iCastleLV;	
				break;
			}
		}
	}
	else
		iRet = MAX_ENTER_GRADE - 1;
	
	
	return iRet;
}

//---------------------------------------------------------------------------------------------
// OpenningProcess
void CNewUIEnterDevilSquare::OpenningProcess()
{
	SendExitInventory();		// 왜 호출하는지 알아보자!!

	for( int i=0 ; i<MAX_ENTER_GRADE ; i++ )
	{
		m_BtnEnter[i].ChangeTextColor( m_dwBtnTextColor[ ENTERBTN_DISABLE ] );
		m_BtnEnter[i].Lock( );
	}

	// 레벨 제한 체크
	int iLimitLVIndex = 0;
	if( GetBaseClass(Hero->Class)==CLASS_DARK || GetBaseClass(Hero->Class)==CLASS_DARK_LORD 
#ifdef PBG_ADD_NEWCHAR_MONK
		|| GetBaseClass(Hero->Class)==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
		)
	{
		iLimitLVIndex = 1;
	}

	m_iNumActiveBtn = CheckLimitLV( iLimitLVIndex );

	m_BtnEnter[ m_iNumActiveBtn ].UnLock();
	m_BtnEnter[ m_iNumActiveBtn ].ChangeTextColor( m_dwBtnTextColor[ ENTERBTN_ENABLE ] );

	// 캐슬 입장 버튼 텍스트
	unicode::t_char sztext[255] = {0, };

#ifdef _VS2008PORTING
	for( int i=0 ; i<MAX_ENTER_GRADE-1 ; i++)
#else // _VS2008PORTING
	for( i=0 ; i<MAX_ENTER_GRADE-1 ; i++)
#endif // _VS2008PORTING
	{
		unicode::_sprintf( sztext, GlobalText[ 645 ], i+1
			, m_iDevilSquareLimitLevel[ (iLimitLVIndex*(MAX_ENTER_GRADE))+i ][ 0 ]
			, m_iDevilSquareLimitLevel[ (iLimitLVIndex*(MAX_ENTER_GRADE))+i ][ 1 ] );
		m_BtnEnter[ i ].SetFont( g_hFontBold );
		m_BtnEnter[ i ].ChangeText( sztext );
	}

	// 1778	"제 %d광장(마스터레벨)"
	unicode::_sprintf( sztext, GlobalText[ 1778 ], 7 );
	m_BtnEnter[ MAX_ENTER_GRADE-1 ].SetFont( g_hFontBold );
	m_BtnEnter[ MAX_ENTER_GRADE-1 ].ChangeText( sztext );
}

//---------------------------------------------------------------------------------------------
// ClosingProcess
void CNewUIEnterDevilSquare::ClosingProcess()
{
	SendExitInventory();
}

//---------------------------------------------------------------------------------------------
// LoadImages
void CNewUIEnterDevilSquare::LoadImages()
{
	// 기본창
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_ENTERDS_BASE_WINDOW_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_ENTERDS_BASE_WINDOW_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_ENTERDS_BASE_WINDOW_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_ENTERDS_BASE_WINDOW_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_ENTERDS_BASE_WINDOW_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_ENTERDS_BASE_WINDOW_BTN_EXIT, GL_LINEAR);				// Exit Button
	LoadBitmap("Interface\\newui_btn_empty_big.tga", IMAGE_ENTERDS_BASE_WINDOW_BTN_ENTER, GL_LINEAR);		// Enter Button
}

//---------------------------------------------------------------------------------------------
// UnloadImages
void CNewUIEnterDevilSquare::UnloadImages()
{
	// 기본창
	DeleteBitmap( IMAGE_ENTERDS_BASE_WINDOW_BACK );
	DeleteBitmap( IMAGE_ENTERDS_BASE_WINDOW_TOP );
	DeleteBitmap( IMAGE_ENTERDS_BASE_WINDOW_LEFT );
	DeleteBitmap( IMAGE_ENTERDS_BASE_WINDOW_RIGHT );
	DeleteBitmap( IMAGE_ENTERDS_BASE_WINDOW_BOTTOM );
	DeleteBitmap( IMAGE_ENTERDS_BASE_WINDOW_BTN_EXIT );		// Exit Button
	DeleteBitmap( IMAGE_ENTERDS_BASE_WINDOW_BTN_ENTER );	// Enter Button
}
