// NewUIRegistrationLuckyCoin.cpp: implementation of the CNewUIRegistrationLuckyCoin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
#include "NewUIRegistrationLuckyCoin.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


namespace SEASON3B
{
	
	CNewUIRegistrationLuckyCoin::CNewUIRegistrationLuckyCoin()
	{
		m_width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
		m_height = MSGBOX_BTN_EMPTY_HEIGHT;
		m_RegistCount = 0;
		m_CoinItem = NULL;
		m_ItemAngle = false;
	}
	
	CNewUIRegistrationLuckyCoin::~CNewUIRegistrationLuckyCoin()
	{
		Release();
	}
	
	bool CNewUIRegistrationLuckyCoin::Create(CNewUIManager* pNewUIMng, int x, int y)
	{
		if(pNewUIMng  == NULL)
			return false;
		
		m_pNewUIMng = pNewUIMng;
		m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION, this);
		
		SetPos(x, y);
		LoadImages();
		//버튼이미지
		SetBtnInfo();
		
		//visible()을 flase로
		Show(false);
		
		return true;
	}
	
	void CNewUIRegistrationLuckyCoin::SetPos( int x, int y )
	{
		m_Pos.x = x; 
		m_Pos.y = y;
	}
	
	bool CNewUIRegistrationLuckyCoin::Render()
	{
		EnableAlphaTest();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		//프레임
		RenderFrame();
		//텍스트
		RenderTexts();
		//버튼
		RenderButtons();
		//행운의동전
		RenderLuckyCoin();
		
		DisableAlphaBlend();
		
		return true;
	}
	
	void CNewUIRegistrationLuckyCoin::RenderFrame()
	{
		RenderImage(IMAGE_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
		RenderImage(IMAGE_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
		RenderImage(IMAGE_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
		RenderImage(IMAGE_RIGHT, m_Pos.x+LUCKYCOIN_REG_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
		RenderImage(IMAGE_BOTTOM, m_Pos.x, m_Pos.y+LUCKYCOIN_REG_HEIGHT-45, 190.f, 45.f);	
	}
	
	void CNewUIRegistrationLuckyCoin::RenderTexts()	
	{
		unicode::t_char szText[256] = {0,};
		float _x = GetPos().x;
		float _y = GetPos().y + 25;
		
		g_pRenderText->SetBgColor(0, 0, 0, 0);
		g_pRenderText->SetTextColor(255, 255, 255, 255);
		g_pRenderText->SetFont(g_hFontBold);
		sprintf( szText, GlobalText[1891] );	// "행운의 동전 등록"
		g_pRenderText->RenderText(_x, _y, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
		
		g_pRenderText->SetFont(g_hFont);
#ifdef PBG_MOD_LUCKYCOINEVENT
		sprintf( szText, GlobalText[2855] );	// "이벤트 기간 중 행운의동전 255개를"
		g_pRenderText->RenderText(_x, _y+40, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
		sprintf( szText, GlobalText[2856] );	// "등록하시면 추첨을 통하여"
		g_pRenderText->RenderText(_x, _y+60, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
		sprintf( szText, GlobalText[2857] );	// "절대무기를 드립니다."
		g_pRenderText->RenderText(_x, _y+80, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
		sprintf( szText, GlobalText[2858] );	// "자세한 사항은 홈페이지를 참고해 주세요."
		g_pRenderText->RenderText(_x, _y+100, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
#else //PBG_MOD_LUCKYCOINEVENT
		sprintf( szText, GlobalText[1935] );	// "이벤트 기간 중 가장 많은 행운의 동전을"
		g_pRenderText->RenderText(_x, _y+40, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
		sprintf( szText, GlobalText[1936] );	// "등록하신 분들께 다양한 선물을 드립니다."
		g_pRenderText->RenderText(_x, _y+60, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
		sprintf( szText, GlobalText[1937] );	// "자세한 사항은 홈페이지를 참고해 주세요."
		g_pRenderText->RenderText(_x, _y+80, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
#endif //PBG_MOD_LUCKYCOINEVENT
		g_pRenderText->SetFont(g_hFontBold);
		
		sprintf(szText, GlobalText[1889]);		// 1889 "현재 등록"
		g_pRenderText->RenderText(_x, _y+120, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);	
		
		sprintf(szText, GlobalText[1893], GetRegistCount());	// 1893 " X %d 개"
#ifdef LJH_MOD_POSITION_OF_REGISTERED_LUCKY_COIN
		g_pRenderText->RenderText( _x+24, _y+150, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
#else  //LJH_MOD_POSITION_OF_REGISTERED_LUCKY_COIN
		g_pRenderText->RenderText( _x+10, _y+150, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
#endif //LJH_MOD_POSITION_OF_REGISTERED_LUCKY_COIN
	}
	
	void CNewUIRegistrationLuckyCoin::RenderLuckyCoin()
	{
		//인터페이스에 행운의 동전을 그린다.
		float x, y, width, height;
		
		x = GetPos().x -20;
		y = GetPos().y + 50;
		
		width = LUCKYCOIN_REG_WIDTH;
		height = LUCKYCOIN_REG_HEIGHT;
		
		EndBitmap();
		
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glViewport2(0,0,WindowWidth,WindowHeight);
		gluPerspective2(1.f, (float)(WindowWidth)/(float)(WindowHeight), RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		GetOpenGLMatrix(CameraMatrix);
		EnableDepthTest();
		EnableDepthMask();
		
		glClear(GL_DEPTH_BUFFER_BIT);
		
		SetItemRotation(true);
		RenderItem3D(x, y, width, height, m_CoinItem->Type, m_CoinItem->Level, 0, 0, true);
		SetItemRotation(false);
		
		UpdateMousePositionn();
		
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		
		BeginBitmap();
	}
	
	void CNewUIRegistrationLuckyCoin::RenderButtons()
	{
		m_CloseButton.Render();
		m_RegistButton.Render();
	}
	
	bool CNewUIRegistrationLuckyCoin::BtnProcess()
	{
		POINT pClose = {GetPos().x+169, GetPos().y+7};
		
		//close 버튼 좌상단의 x표시
		if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(pClose.x, pClose.y, 13, 12))
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION);
			return false;
		}
		
		if(m_CloseButton.UpdateMouseEvent() == true)
		{
			//닫기 버튼이 눌렸다면.종료시스템과 같음
			if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION) == true)
			{	
				//등록창을 닫을때 인벤도 닫는다.
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION);
				return true;
			}
			return false;
		}
		
		//등록버튼이 눌렸다면
		if(m_RegistButton.UpdateMouseEvent() == true)
		{	
			SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
			
			//send 등록갯수요청
			SendRequestRegistLuckyCoin();
			
			// 서버로 부터 리시브를 할때까지 Lock
			LockLuckyCoinRegBtn();
			
			return true;
		}
		return false;
	}
	
	void CNewUIRegistrationLuckyCoin::SetBtnInfo()
	{
		float _x = GetPos().x + LUCKYCOIN_REG_WIDTH/2.0f - MSGBOX_BTN_EMPTY_SMALL_WIDTH/2.0f;
		float _y = GetPos().y + LUCKYCOIN_REG_HEIGHT-220;
		
		m_RegistButton.ChangeButtonImgState(true, IMAGE_CLOSE_REGIST, true);
		m_RegistButton.ChangeButtonInfo(_x, _y, m_width, m_height);
		m_RegistButton.SetFont(g_hFontBold);
		// 1894 "등록"
		m_RegistButton.ChangeText(GlobalText[1894]);
		
		m_CloseButton.ChangeButtonImgState(true, IMAGE_CLOSE_REGIST, true);
		m_CloseButton.ChangeButtonInfo(_x, 360, m_width, m_height);
		m_CloseButton.SetFont(g_hFontBold);
		// 1002 "닫기"
		m_CloseButton.ChangeText(GlobalText[1002]);
	}
	
	bool CNewUIRegistrationLuckyCoin::Update()
	{
		//N/A
		return true;
	}
	
	bool CNewUIRegistrationLuckyCoin::UpdateMouseEvent()
	{
		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION) == false)
		{
			return true;
		}
		
		// 버튼 마우스이벤트 처리
		if(BtnProcess() == true)	// 처리가 완료 되었다면
		{
			return false;
		}
		
		// 인터페이스 내에 클릭하면 아무 동작 안되게 처리
		if(CheckMouseIn(m_Pos.x, m_Pos.y, LUCKYCOIN_REG_WIDTH, LUCKYCOIN_REG_HEIGHT))
		{
			if(SEASON3B::IsPress(VK_RBUTTON))
			{
				MouseRButton = false;
				MouseRButtonPop = false;
				MouseRButtonPush = false;
				return false;
			}
			
			if(SEASON3B::IsNone(VK_LBUTTON) == false)
			{
				return false;
			}
		}
		return true;
	}
	
	bool CNewUIRegistrationLuckyCoin::UpdateKeyEvent()
	{
		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION) == true)
		{
			//esc키를 사용하여 취소하는 경우
			if(SEASON3B::IsPress(VK_ESCAPE) == true)
			{
				//등록창을 닫을때 인벤도 닫는다.
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION);
				return false;
			}
		}
		return true;
	}
	
	void CNewUIRegistrationLuckyCoin::OpeningProcess()
	{	
		g_pMyInventory->GetInventoryCtrl()->LockInventory();

		m_RegistCount = 0;

		UnLockLuckyCoinRegBtn();

		//행운의 동전 등록갯수 요청
		SendRequestRegistedLuckyCoin();
		
		//창을 열었을때
		m_CoinItem = new ITEM;
		if(m_CoinItem == NULL)	return;
		memset(m_CoinItem, 0, sizeof(ITEM));
		
		m_CoinItem->Type = ITEM_POTION+100;
		m_CoinItem->Level = 0;
		m_CoinItem->Option1 = 0;
		m_CoinItem->ExtOption = 0;
	}
	
	void CNewUIRegistrationLuckyCoin::ClosingProcess()
	{
		//창을 닫을때
		SAFE_DELETE(m_CoinItem);
		
		g_pMyInventory->GetInventoryCtrl()->UnlockInventory();
		
		SendRequestMixExit();
	}
	
	void CNewUIRegistrationLuckyCoin::LoadImages()
	{
		LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_BACK, GL_LINEAR);
		LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_TOP, GL_LINEAR);
		LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_LEFT, GL_LINEAR);
		LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_RIGHT, GL_LINEAR);
		LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_BOTTOM, GL_LINEAR);
		LoadBitmap("Interface\\newui_btn_empty_small.tga", IMAGE_CLOSE_REGIST, GL_LINEAR);
	}
	
	void CNewUIRegistrationLuckyCoin::UnloadImages()
	{
		DeleteBitmap(IMAGE_CLOSE_REGIST);
		DeleteBitmap(IMAGE_BOTTOM);
		DeleteBitmap(IMAGE_RIGHT);
		DeleteBitmap(IMAGE_LEFT);
		DeleteBitmap(IMAGE_TOP);
		DeleteBitmap(IMAGE_BACK);
	}
	
	void CNewUIRegistrationLuckyCoin::Release()
	{
		UnloadImages();
		
		if(m_pNewUIMng)
		{
			m_pNewUIMng->RemoveUIObj(this);
			m_pNewUIMng = NULL;
		}
	}
	
	void CNewUIRegistrationLuckyCoin::LockLuckyCoinRegBtn()
	{			
		// 서버로 부터 리시브를 할때까지 등록버튼 비활성화
		m_RegistButton.Lock();
		m_RegistButton.ChangeTextColor(0xff808080);		// 회색
	}
	
	void CNewUIRegistrationLuckyCoin::UnLockLuckyCoinRegBtn()
	{
		// 등록버튼을 활성화 시킨다.
		m_RegistButton.UnLock();
		m_RegistButton.ChangeTextColor(0xffffffff);		// 흰색
	}
	
}

#endif // KJH_PBG_ADD_SEVEN_EVENT_2008