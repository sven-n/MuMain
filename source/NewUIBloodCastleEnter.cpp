// NewUIBloodCastleEnter.cpp: implementation of the CNewUIPartyInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIBloodCastleEnter.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "NewUICommonMessageBox.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIEnterBloodCastle::CNewUIEnterBloodCastle()
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
	m_iBloodCastleLimitLevel[0][0] = 15;  m_iBloodCastleLimitLevel[0][1] = 80;
	m_iBloodCastleLimitLevel[1][0] = 81;  m_iBloodCastleLimitLevel[1][1] = 130;
	m_iBloodCastleLimitLevel[2][0] = 131; m_iBloodCastleLimitLevel[2][1] = 180;
	m_iBloodCastleLimitLevel[3][0] = 181; m_iBloodCastleLimitLevel[3][1] = 230;
	m_iBloodCastleLimitLevel[4][0] = 231; m_iBloodCastleLimitLevel[4][1] = 280;
	m_iBloodCastleLimitLevel[5][0] = 281; m_iBloodCastleLimitLevel[5][1] = 330;
	m_iBloodCastleLimitLevel[6][0] = 331; m_iBloodCastleLimitLevel[6][1] = 400;
	m_iBloodCastleLimitLevel[7][0] = 0;   m_iBloodCastleLimitLevel[7][1] = 0;


	// 7~13 마검사, 다크로드 클래스	
	m_iBloodCastleLimitLevel[8][0]  = 10;  m_iBloodCastleLimitLevel[8][1]  = 60;
	m_iBloodCastleLimitLevel[9][0]  = 61;  m_iBloodCastleLimitLevel[9][1]  = 110;
	m_iBloodCastleLimitLevel[10][0] = 111; m_iBloodCastleLimitLevel[10][1] = 160;
	m_iBloodCastleLimitLevel[11][0] = 161; m_iBloodCastleLimitLevel[11][1] = 210;
	m_iBloodCastleLimitLevel[12][0] = 211; m_iBloodCastleLimitLevel[12][1] = 260;
	m_iBloodCastleLimitLevel[13][0] = 261; m_iBloodCastleLimitLevel[13][1] = 310;
	m_iBloodCastleLimitLevel[14][0] = 311; m_iBloodCastleLimitLevel[14][1] = 400;
	m_iBloodCastleLimitLevel[15][0] = 0;   m_iBloodCastleLimitLevel[15][1] = 0;
}

CNewUIEnterBloodCastle::~CNewUIEnterBloodCastle()
{
	Release();			// 꼭 하자!!
}

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIEnterBloodCastle::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_BLOODCASTLE, this );		// 인터페이스 오브젝트 등록
	
	SetPos(x, y);
	
	LoadImages();
	
	// Exit Button
	m_BtnExit.ChangeButtonImgState( true, IMAGE_ENTERBC_BASE_WINDOW_BTN_EXIT, false );
	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );		
	m_BtnExit.ChangeToolTipText( GlobalText[1002], true );	// 1002 "닫기"
	
	// Enter Button
	int iVal=0;
	for(int i=0 ; i<MAX_ENTER_GRADE ; i++)
	{
		iVal = ENTER_BTN_VAL * i;
		m_BtnEnter[i].ChangeButtonImgState( true, IMAGE_ENTERBC_BASE_WINDOW_BTN_ENTER, true );
		m_BtnEnter[i].ChangeButtonInfo( m_BtnEnterStartPos.x, m_BtnEnterStartPos.y+iVal, 180, 29 );
	}
	
	Show( false );
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void CNewUIEnterBloodCastle::Release()
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
void CNewUIEnterBloodCastle::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	m_EnterUITextPos.x = m_Pos.x + 3;
	m_EnterUITextPos.y = m_Pos.y + 55;

	SetBtnPos( m_Pos.x+6, m_Pos.y+125 );

	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );

	for( int i=0 ; i<MAX_ENTER_GRADE ; i++ )
	{
		int iVal = ENTER_BTN_VAL * i;
		m_BtnEnter[i].ChangeButtonInfo( m_BtnEnterStartPos.x, m_BtnEnterStartPos.y+iVal, 180, 29);
	}
	
}

//---------------------------------------------------------------------------------------------
// SetBtnPos
void CNewUIEnterBloodCastle::SetBtnPos(int x, int y)
{
	m_BtnEnterStartPos.x = x;
	m_BtnEnterStartPos.y = y;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool CNewUIEnterBloodCastle::UpdateMouseEvent()
{
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;
	
	// 파티 창 내 영역 클릭시 하위 UI처리 및 이동 불
	// ( #include "NewUICommon" )
	if( CheckMouseIn(m_Pos.x, m_Pos.y, ENTERBC_BASE_WINDOW_WIDTH, ENTERBC_BASE_WINDOW_HEIGHT) )
		return false;

	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool CNewUIEnterBloodCastle::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_BLOODCASTLE);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------------------------
// CheckLimitLV
// 캐릭터 레벨에 따른 입성 레벨을 구한다.
int CNewUIEnterBloodCastle::CheckLimitLV( int iIndex )
{
	int	iVal = 0;
	int iRet = 0;
	
	if( iIndex == 1 )
	{
		iVal = 8;
	}

	int iLevel = CharacterAttribute->Level;
	
	if(IsMasterLevel( CharacterAttribute->Class ) == false )
	{
		for(int iCastleLV=0; iCastleLV<MAX_ENTER_GRADE-1; ++iCastleLV)
		{
			if ( iLevel >= m_iBloodCastleLimitLevel[iVal+iCastleLV][0] 
				&& iLevel <= m_iBloodCastleLimitLevel[iVal+iCastleLV][1] )
			{
				iRet = iCastleLV;	
				break;
			}
		}
	}
	else
		iRet = MAX_ENTER_GRADE-1;
	 
	
	return iRet;
}

//---------------------------------------------------------------------------------------------
// Update
bool CNewUIEnterBloodCastle::Update()
{
	if( !IsVisible() )
		return true;
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Render
bool CNewUIEnterBloodCastle::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	// Base Window
	RenderImage(IMAGE_ENTERBC_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, 
						float(ENTERBC_BASE_WINDOW_WIDTH), float(ENTERBC_BASE_WINDOW_HEIGHT));
	RenderImage(IMAGE_ENTERBC_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, 
						float(ENTERBC_BASE_WINDOW_WIDTH), 64.f);
	RenderImage(IMAGE_ENTERBC_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y+64.f, 
						21.f, float(ENTERBC_BASE_WINDOW_HEIGHT)-64.f-45.f);
	RenderImage(IMAGE_ENTERBC_BASE_WINDOW_RIGHT, m_Pos.x+float(ENTERBC_BASE_WINDOW_WIDTH)-21.f, 
						m_Pos.y+64.f, 21.f, float(ENTERBC_BASE_WINDOW_HEIGHT)-64.f-45.f);
	RenderImage(IMAGE_ENTERBC_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y+float(ENTERBC_BASE_WINDOW_HEIGHT)-45.f, 
						float(ENTERBC_BASE_WINDOW_WIDTH), 45.f);		
	
	g_pRenderText->SetFont( g_hFontBold );
	g_pRenderText->SetTextColor( 0xFFFFFFFF );
	g_pRenderText->SetBgColor( 0x00000000 );
	
	// UI제목
	g_pRenderText->RenderText(m_Pos.x+60, m_Pos.y+12, GlobalText[846], 72, 0, RT3_SORT_CENTER);		// "대천사의 전령"

	g_pRenderText->SetFont( g_hFont );

	// 대사
	char txtline[NUM_LINE_CMB][MAX_LENGTH_CMB];	
	// "대천사님을 도와주려는 용기가 무척 고맙군. 하지만 조심하게나. 블러드 캐슬은 무서운 곳이야.. 건투를 비네."
	int tl = SeparateTextIntoLines( GlobalText[832], txtline[0], NUM_LINE_CMB, MAX_LENGTH_CMB);
	for ( int j = 0; j < tl; ++j)
	{
		g_pRenderText->RenderText(m_EnterUITextPos.x, m_EnterUITextPos.y+j*20,txtline[j], 190, 0, RT3_SORT_CENTER);
	}
		
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
bool CNewUIEnterBloodCastle::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
	
	// Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_BLOODCASTLE );
		return true;
	}
	
	// Exit2 버튼 
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_BLOODCASTLE );
		return true;
	}

	// 블러드 캐슬 입장 버튼
	if( (m_iNumActiveBtn !=- 1) && (m_BtnEnter[m_iNumActiveBtn].UpdateMouseEvent() == true) )
	{	
		// 레벨에 맞는 블러드 캐슬로 입장!!
		int iItemIndex = -1;
		
		// 레벨 0짜리 투명망토는 무조건 입장 가능(사용안함)
		//iItemIndex = g_pMyInventory->findimteindex( ITEM_HELPER+18, 0 );

		// 투명망토 레벨을 검사
		iItemIndex = g_pMyInventory->FindItemIndex( ITEM_HELPER+18, m_iNumActiveBtn+1 );

#ifdef CSK_FREE_TICKET
		// 기존 투명망토를 검사한 후 아이템이 없으면
		// 블러드캐슬 자유입장권을 검사한다.
		if(iItemIndex == -1)
		{
			iItemIndex = g_pMyInventory->FindItemIndex( ITEM_HELPER+47, -1 ); // 블러드캐슬 자유입장권
		}
#endif // CSK_FREE_TICKET

// 		// 투명망토 + 6짜리 검사
// 		// 왜 이런 코드가 넣어져 있을까???
// 		if(m_iNumActiveBtn+1 == 7 && iItemIndex == -1) 
// 		{
// 			iItemIndex = g_pMyInventory->FindItemIndex( ITEM_HELPER+18, 6 );;
// 		}
		
		if( iItemIndex > -1)	
		{
			SendRequestMoveToEventMatch( m_iNumActiveBtn + 1, MAX_EQUIPMENT+iItemIndex);	
		}
		else
		{	
			// 투명 망토 유무 검사는 NPC와 대화를 시작 할때 한다.
			// 따라서, 여기까지 들어오게 되면 투명 망토는 있으나 투명망토의 레벨이 맞지 않을때 이다.
			// 이 메세지박스 처리 이외에는 WsClient의 ReceiveServerCommand()에서 처리
			SEASON3B::CreateOkMessageBox(GlobalText[854]);		// "투명 망토의 레벨이 맞지 않습니다."
		}
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_BLOODCASTLE );
	}
	
	return false;
}

float CNewUIEnterBloodCastle::GetLayerDepth()
{
	return 4.1f;
}

//---------------------------------------------------------------------------------------------
// OpenningProcess
void CNewUIEnterBloodCastle::OpenningProcess()
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
		unicode::_sprintf( sztext, GlobalText[ 847 ], i+1
			, m_iBloodCastleLimitLevel[ (iLimitLVIndex*MAX_ENTER_GRADE)+i ][ 0 ]
			, m_iBloodCastleLimitLevel[ (iLimitLVIndex*MAX_ENTER_GRADE)+i ][ 1 ] );
		m_BtnEnter[ i ].SetFont( g_hFontBold );
		m_BtnEnter[ i ].ChangeText( sztext );
	}
	
	// 1779	"제%d캐슬(마스터레벨)"
	unicode::_sprintf( sztext, GlobalText[ 1779 ], 8 );

	m_BtnEnter[ MAX_ENTER_GRADE-1 ].SetFont( g_hFontBold );
	m_BtnEnter[ MAX_ENTER_GRADE-1 ].ChangeText( sztext );
}

//---------------------------------------------------------------------------------------------
// ClosingProcess
void CNewUIEnterBloodCastle::ClosingProcess()
{
	// 왜 호출해야 하는거지??
	SendExitInventory();
}

//---------------------------------------------------------------------------------------------
// LoadImages
void CNewUIEnterBloodCastle::LoadImages()
{
	// 기본창
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_ENTERBC_BASE_WINDOW_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_ENTERBC_BASE_WINDOW_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_ENTERBC_BASE_WINDOW_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_ENTERBC_BASE_WINDOW_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_ENTERBC_BASE_WINDOW_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_ENTERBC_BASE_WINDOW_BTN_EXIT, GL_LINEAR);				// Exit Button
	LoadBitmap("Interface\\newui_btn_empty_big.tga", IMAGE_ENTERBC_BASE_WINDOW_BTN_ENTER, GL_LINEAR);		// Enter Button
}

//---------------------------------------------------------------------------------------------
// UnloadImages
void CNewUIEnterBloodCastle::UnloadImages()
{
	// 기본창
	DeleteBitmap( IMAGE_ENTERBC_BASE_WINDOW_BACK );
	DeleteBitmap( IMAGE_ENTERBC_BASE_WINDOW_TOP );
	DeleteBitmap( IMAGE_ENTERBC_BASE_WINDOW_LEFT );
	DeleteBitmap( IMAGE_ENTERBC_BASE_WINDOW_RIGHT );
	DeleteBitmap( IMAGE_ENTERBC_BASE_WINDOW_BOTTOM );
	DeleteBitmap( IMAGE_ENTERBC_BASE_WINDOW_BTN_EXIT );		// Exit Button
	DeleteBitmap( IMAGE_ENTERBC_BASE_WINDOW_BTN_ENTER );	// Enter Button
}
