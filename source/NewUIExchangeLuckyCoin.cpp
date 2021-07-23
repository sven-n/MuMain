// NewUIExchangeLuckyCoin.cpp: implementation of the CNewUIExchangeLuckyCoin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
#include "NewUIExchangeLuckyCoin.h"
#include "NewUISystem.h"
#include "NewUICommon.h"
#include "NewUICommonMessageBox.h"
#include "wsclientinline.h"


using namespace SEASON3B;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIExchangeLuckyCoin::CNewUIExchangeLuckyCoin()
{
	m_pNewUIMng = NULL;
	memset( &m_Pos, 0, sizeof(POINT) );
	memset( &m_TextPos, 0, sizeof(POINT) );
	memset( &m_FirstBtnPos, 0, sizeof(POINT) );
}

CNewUIExchangeLuckyCoin::~CNewUIExchangeLuckyCoin()
{
	Release();			// 꼭 하자!!
}

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIExchangeLuckyCoin::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN, this );		// 인터페이스 오브젝트 등록
	
	SetPos(x, y);
	
	LoadImages();	

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	unicode::t_char sztext[255] = {0, };
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	
	// Exit Button
	m_BtnExit.ChangeButtonImgState( true, IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BTN_EXIT, true );
	m_BtnExit.ChangeButtonInfo( m_Pos.x+((EXCHANGE_LUCKYCOIN_WINDOW_WIDTH/2)-(MSGBOX_BTN_EMPTY_SMALL_WIDTH/2)), m_Pos.y+360, 
								MSGBOX_BTN_EMPTY_SMALL_WIDTH, MSGBOX_BTN_EMPTY_HEIGHT );		
	m_BtnExit.ChangeText( GlobalText[1002] );				// 1002 "닫기"
	
	// Exchange Button
	m_BtnExchange[0].ChangeButtonImgState( true, IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN, true );		
	m_BtnExchange[0].SetFont( g_hFontBold );
	m_BtnExchange[0].ChangeText( GlobalText[1896] );		// "10개 교환"

	m_BtnExchange[1].ChangeButtonImgState( true, IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN, true );
	m_BtnExchange[1].SetFont( g_hFontBold );
	m_BtnExchange[1].ChangeText( GlobalText[1897] );		// "20개 교환"
	
	m_BtnExchange[2].ChangeButtonImgState( true, IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN, true );		
	m_BtnExchange[2].SetFont( g_hFontBold );
	m_BtnExchange[2].ChangeText( GlobalText[1898] );		// "30개 교환"
	
	Show( false );
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void CNewUIExchangeLuckyCoin::Release()
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
void CNewUIExchangeLuckyCoin::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	m_TextPos.x = m_Pos.x;
	m_TextPos.y = m_Pos.y + 80;

	SetBtnPos( m_Pos.x+((EXCHANGE_LUCKYCOIN_WINDOW_WIDTH/2)-(MSGBOX_BTN_EMPTY_WIDTH/2)), m_Pos.y+220 );

	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );

	for( int i=0 ; i<MAX_EXCHANGE_BTN ; i++ )
	{
		int iVal = EXCHANGE_BTN_VAL * i;
		m_BtnExchange[i].ChangeButtonInfo( m_FirstBtnPos.x, m_FirstBtnPos.y+iVal, MSGBOX_BTN_EMPTY_WIDTH, MSGBOX_BTN_EMPTY_HEIGHT);
	}
	
}

//---------------------------------------------------------------------------------------------
// SetBtnPos
void CNewUIExchangeLuckyCoin::SetBtnPos(int x, int y)
{
	m_FirstBtnPos.x = x;
	m_FirstBtnPos.y = y;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool CNewUIExchangeLuckyCoin::UpdateMouseEvent()
{
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;
	
	// 파티 창 내 영역 클릭시 하위 UI처리 및 이동 불
	// ( #include "NewUICommon" )
	if( SEASON3B::CheckMouseIn(m_Pos.x, m_Pos.y, EXCHANGE_LUCKYCOIN_WINDOW_WIDTH, EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT) )
		return false;

	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool CNewUIExchangeLuckyCoin::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN);
			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------------------------
// Update
bool CNewUIExchangeLuckyCoin::Update()
{
	if( !IsVisible() )
		return true;
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Render
bool CNewUIExchangeLuckyCoin::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	RenderFrame();
	RenderTexts();
	RenderBtn();

	DisableAlphaBlend();

	return true;
}

//---------------------------------------------------------------------------------------------
// RenderFrame
void CNewUIExchangeLuckyCoin::RenderFrame()
{
	// Base Window
	RenderImage(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BACK, m_Pos.x, m_Pos.y, 
						float(EXCHANGE_LUCKYCOIN_WINDOW_WIDTH), float(EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT));
	RenderImage(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_TOP, m_Pos.x, m_Pos.y, 
						float(EXCHANGE_LUCKYCOIN_WINDOW_WIDTH), 64.f);
	RenderImage(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_LEFT, m_Pos.x, m_Pos.y+64.f, 
						21.f, float(EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT)-64.f-45.f);
	RenderImage(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_RIGHT, m_Pos.x+float(EXCHANGE_LUCKYCOIN_WINDOW_WIDTH)-21.f, 
						m_Pos.y+64.f, 21.f, float(EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT)-64.f-45.f);
	RenderImage(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BOTTOM, m_Pos.x, m_Pos.y+float(EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT)-45.f, 
						float(EXCHANGE_LUCKYCOIN_WINDOW_WIDTH), 45.f);
}

//---------------------------------------------------------------------------------------------
// RenderTexts
void CNewUIExchangeLuckyCoin::RenderTexts()
{
	g_pRenderText->SetFont( g_hFontBold );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );
	
	// UI제목
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+25, GlobalText[1892], 190, 0, RT3_SORT_CENTER);		// "행운의 동전 교환"

	g_pRenderText->RenderText(m_TextPos.x, m_Pos.y+200, GlobalText[1940], 
								EXCHANGE_LUCKYCOIN_WINDOW_WIDTH, 0, RT3_SORT_CENTER);		// "교환"

	g_pRenderText->SetTextColor( 255, 255, 0, 255 );		// Yellow
	g_pRenderText->RenderText(m_TextPos.x, m_TextPos.y , GlobalText[1895], 
								EXCHANGE_LUCKYCOIN_WINDOW_WIDTH, 0, RT3_SORT_CENTER);		// "주의"

	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );
	int iTextPosy = m_TextPos.y+(EXCHANGE_TEXT_VAL*2);
	g_pRenderText->RenderText(m_TextPos.x, iTextPosy , GlobalText[1938], 
								EXCHANGE_LUCKYCOIN_WINDOW_WIDTH, 0, RT3_SORT_CENTER);		// "교환에 사용된 행운의 동전은"
	iTextPosy += EXCHANGE_TEXT_VAL;
	g_pRenderText->RenderText(m_TextPos.x, iTextPosy , GlobalText[1939], 
								EXCHANGE_LUCKYCOIN_WINDOW_WIDTH, 0, RT3_SORT_CENTER);		// "해당 개수만큼 차감됩니다."
}

//---------------------------------------------------------------------------------------------
// RenderBtn
void CNewUIExchangeLuckyCoin::RenderBtn()
{
	// 행운의동전 교환 버튼
	for( int i=0 ; i<MAX_EXCHANGE_BTN ; i++ )
	{
		m_BtnExchange[i].Render();
	}

	// Exit Button
	m_BtnExit.Render();	
}

//---------------------------------------------------------------------------------------------
// BtnProcess
bool CNewUIExchangeLuckyCoin::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
	
	// Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && SEASON3B::CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN );
		return true;
	}
	
	// Exit2 버튼 
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN );
		return true;
	}

	// 행운의동전을 검사하여 send
	if(m_BtnExchange[0].UpdateMouseEvent() == true)
	{
		/*	* 인벤에 있는 행운의동전 검사루틴
			* 서버에서 검사해주기 때문에 검사할 필요 없다.

		// 행운의동전이 10개이상이 있다면.
		if( g_pMyInventory->GetNumItemByType(ITEM_POTION+100) >= 10 )
		{
			SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
			SendRequestExChangeLuckyCoin(10);
		}
		else
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseRegistLuckyCoinMsgBoxLayout));
		}
		*/

		LockExchangeBtn();
		SendRequestExChangeLuckyCoin(10);
	}

	if(m_BtnExchange[1].UpdateMouseEvent() == true)
	{
		/*	* 인벤에 있는 행운의동전 검사루틴
			* 서버에서 검사해주기 때문에 검사할 필요 없다.

		// 행운의동전이 20개이상이 있다면.
		if( g_pMyInventory->GetNumItemByType(ITEM_POTION+100) >= 20 )
		{
			EASON3B::CNewUIInventoryCtrl::BackupPickedItem();
			SendRequestExChangeLuckyCoin(20);
		}	
		else
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseRegistLuckyCoinMsgBoxLayout));
		}
		*/

		LockExchangeBtn();
		SendRequestExChangeLuckyCoin(20);
	}
	
	if(m_BtnExchange[2].UpdateMouseEvent() == true)
	{
		/*	* 인벤에 있는 행운의동전 검사루틴
			* 서버에서 검사해주기 때문에 검사할 필요 없다.

		// 행운의동전이 30개이상이 있다면.
		if( g_pMyInventory->GetNumItemByType(ITEM_POTION+100) >= 30 )
		{
			SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
			SendRequestExChangeLuckyCoin(30);
		}	
		else
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseRegistLuckyCoinMsgBoxLayout));
		}
		*/
		
		LockExchangeBtn();
		SendRequestExChangeLuckyCoin(30);
	}
	
	return false;
}

//---------------------------------------------------------------------------------------------
// GetLayerDepth
float CNewUIExchangeLuckyCoin::GetLayerDepth()
{
	return 4.2f;
}

//---------------------------------------------------------------------------------------------
// OpenningProcess
void CNewUIExchangeLuckyCoin::OpenningProcess()
{
	g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY);
	UnLockExchangeBtn();
	g_pMyInventory->GetInventoryCtrl()->LockInventory();
	PlayBuffer(SOUND_CLICK01);
}

//---------------------------------------------------------------------------------------------
// ClosingProcess
void CNewUIExchangeLuckyCoin::ClosingProcess()
{
	PlayBuffer(SOUND_CLICK01);
	g_pMyInventory->GetInventoryCtrl()->UnlockInventory();
	SendRequestMixExit();
}

//---------------------------------------------------------------------------------------------
// LockExchangeBtn
void CNewUIExchangeLuckyCoin::LockExchangeBtn()
{
	for( int i=0 ; i<3 ; i++ )
	{
		m_BtnExchange[i].Lock();
		m_BtnExchange[i].ChangeTextColor(0xff808080);
	}
}

//---------------------------------------------------------------------------------------------
// UnLockExchangeBtn
void CNewUIExchangeLuckyCoin::UnLockExchangeBtn()
{
	for( int i=0 ; i<3 ; i++ )
	{
		m_BtnExchange[i].UnLock();
		m_BtnExchange[i].ChangeTextColor(0xffffffff);
	}
}

//---------------------------------------------------------------------------------------------
// LoadImages
void CNewUIExchangeLuckyCoin::LoadImages()
{
	// 기본창
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_btn_empty_small.tga", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BTN_EXIT, GL_LINEAR);	// Exit Button
	LoadBitmap("Interface\\newui_btn_empty.tga", IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN, GL_LINEAR);				// Exchange Button
}

//---------------------------------------------------------------------------------------------
// UnloadImages
void CNewUIExchangeLuckyCoin::UnloadImages()
{
	// 기본창
	DeleteBitmap( IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BACK );
	DeleteBitmap( IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_TOP );
	DeleteBitmap( IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_LEFT );
	DeleteBitmap( IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_RIGHT );
	DeleteBitmap( IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BOTTOM );
	DeleteBitmap( IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BTN_EXIT );		// Exit Button
	DeleteBitmap( IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN );			// Exchange Button
}

#endif // KJH_PBG_ADD_SEVEN_EVENT_2008




























