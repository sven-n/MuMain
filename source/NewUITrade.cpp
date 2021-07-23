//*****************************************************************************
// File: NewUITrade.cpp
//
// Desc: implementation of the CNewUITrade class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"

#include "NewUITrade.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "wsclientinline.h"
#include "CComGem.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUITrade::CNewUITrade()
{
	m_pNewUIMng = NULL;
	m_pYourInvenCtrl = m_pMyInvenCtrl = NULL;
	m_Pos.x = m_Pos.y = 0;
}

CNewUITrade::~CNewUITrade()
{
	Release();
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 창 생성.
// 매개 변수 : pNewUIMng	: CNewUIManager 오브젝트 주소.
//			   x			: x 좌표.
//			   y			: y 좌표.
//*****************************************************************************
bool CNewUITrade::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng
		|| NULL == g_pNewItemMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_TRADE, this);

	m_pYourInvenCtrl = new CNewUIInventoryCtrl;
	if (false == m_pYourInvenCtrl->Create(g_pNewUI3DRenderMng, g_pNewItemMng,
		this, x + 16, y + 68, COLUMN_TRADE_INVEN, ROW_TRADE_INVEN))
	{
		SAFE_DELETE(m_pYourInvenCtrl);
		return false;
	}
	m_pYourInvenCtrl->LockInventory();	// 상대방 물품은 클릭 안되게 함.

	m_pMyInvenCtrl = new CNewUIInventoryCtrl;
	if (false == m_pMyInvenCtrl->Create(g_pNewUI3DRenderMng, g_pNewItemMng,
		this, x + 16, y + 274, COLUMN_TRADE_INVEN, ROW_TRADE_INVEN))
	{
		SAFE_DELETE(m_pMyInvenCtrl);
		return false;
	}

	SetPos(x, y);

	LoadImages();

// 버튼들 생성.
	m_abtn[BTN_CLOSE].ChangeButtonImgState(true, IMAGE_TRADE_BTN_CLOSE);
	m_abtn[BTN_CLOSE].ChangeButtonInfo(x + 13, y + 390, 36, 29);
	m_abtn[BTN_CLOSE].ChangeToolTipText(GlobalText[1002], true);

	m_abtn[BTN_ZEN_INPUT].ChangeButtonImgState(true, IMAGE_TRADE_BTN_ZEN_INPUT);
	m_abtn[BTN_ZEN_INPUT].ChangeButtonInfo(x + 104, y + 390, 36, 29);
	m_abtn[BTN_ZEN_INPUT].ChangeToolTipText(GlobalText[227], true);

	::memset(m_szYourID, 0, MAX_ID_SIZE+1);
	m_bTradeAlert = false;

	InitTradeInfo();
	InitYourInvenBackUp();

	Show(false);	

	return true;
}

//*****************************************************************************
// 함수 이름 : InitTradeInfo()
// 함수 설명 : 각종 거래 관련 변수 초기화.
//*****************************************************************************
void CNewUITrade::InitTradeInfo()
{
	m_nYourLevel = 0;
	m_nYourGuildType = -1;
	m_nYourTradeGold = 0;
	m_nMyTradeGold = 0;
	m_nMyTradeWait = 0;
	m_bYourConfirm = m_bMyConfirm = false;
}

//*****************************************************************************
// 함수 이름 : InitYourInvenBackUp()
// 함수 설명 : 상대방 거래 아이템 백업 초기화.
//*****************************************************************************
void CNewUITrade::InitYourInvenBackUp()
{
	for (int i = 0; i < MAX_TRADE_INVEN; ++i)
		m_aYourInvenBackUp[i].Type = -1;
}

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : 창 Release.
//*****************************************************************************
void CNewUITrade::Release()
{
	UnloadImages();

	SAFE_DELETE(m_pMyInvenCtrl);
	SAFE_DELETE(m_pYourInvenCtrl);

	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}

	if (g_pNewUI3DRenderMng)
		g_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);
}

//*****************************************************************************
// 함수 이름 : SetPos()
// 함수 설명 : 창 위치 지정.
//*****************************************************************************
void CNewUITrade::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;

	m_posMyConfirm.x = m_Pos.x + 144;
	m_posMyConfirm.y = m_Pos.y + 390;
}

//*****************************************************************************
// 함수 이름 : UpdateMouseEvent()
// 함수 설명 : 마우스 이벤트 처리.
// 반환 값	 : true면 창 뒤로도 이벤트를 처리.
//*****************************************************************************
bool CNewUITrade::UpdateMouseEvent()
{
	if ((m_pYourInvenCtrl && false == m_pYourInvenCtrl->UpdateMouseEvent())
		|| (m_pMyInvenCtrl && false == m_pMyInvenCtrl->UpdateMouseEvent()))
	{
		// 자신 거래 물품을 집었을 때 확정이었다면 비확정으로.
		if (SEASON3B::IsPress(VK_LBUTTON)
			&& CNewUIInventoryCtrl::GetPickedItem()->GetOwnerInventory() == m_pMyInvenCtrl
			&& m_bMyConfirm)
		{
				m_bMyConfirm = false;
				SendRequestTradeResult(m_bMyConfirm);
		}

		return false;
	}

	ProcessMyInvenCtrl();

	if (ProcessBtns())
		return false;

	// 창 영역 클릭시 하위 UI처리 및 이동 불가
	if(CheckMouseIn(m_Pos.x, m_Pos.y, TRADE_WIDTH, TRADE_HEIGHT))
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

//*****************************************************************************
// 함수 이름 : UpdateKeyEvent()
// 함수 설명 : 키보드 입력 처리.
// 반환 값	 : true면 창 뒤로도 이벤트를 처리.
//*****************************************************************************
bool CNewUITrade::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			SendRequestTradeExit();
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_TRADE);
			PlayBuffer(SOUND_CLICK01);

			return false;
		}
	}
	return true;
}

//*****************************************************************************
// 함수 이름 : Update()
// 함수 설명 : 기타 매 프레임 일어나는 이벤트 처리.
// 반환 값	 : true면 창 뒤로도 이벤트를 처리.
//*****************************************************************************
bool CNewUITrade::Update()
{
	if ((m_pYourInvenCtrl && false == m_pYourInvenCtrl->Update())
		|| (m_pMyInvenCtrl && false == m_pMyInvenCtrl->Update()))
		return false;

	return true;
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 창 렌더.
// 반환 값	 : true면 성공.
//*****************************************************************************
bool CNewUITrade::Render()
{
	::EnableAlphaTest();

	::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderBackImage();
	RenderText();

	if (m_pYourInvenCtrl)
		m_pYourInvenCtrl->Render();
	if (m_pMyInvenCtrl)
		m_pMyInvenCtrl->Render();

	for (int i = BTN_CLOSE; i < MAX_BTN; ++i)
		m_abtn[i].Render();

	if (g_pNewUI3DRenderMng)
		g_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER,
			UI2DEffectCallback, this, 0, 0);

	::DisableAlphaBlend();
	
	return true;
}

//*****************************************************************************
// 함수 이름 : UI2DEffectCallback()
// 함수 설명 : 아이템에 주의 표시 랜더하기 위한 Callback 함수.
//			   (3D 아이템 위에 랜더 되야 하므로.)
//*****************************************************************************
void CNewUITrade::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	if (pClass)
	{
		CNewUITrade* pNewUITrade = (CNewUITrade*)pClass;
		pNewUITrade->RenderWarningArrow();
	}
}

//*****************************************************************************
// 함수 이름 : RenderBackImage()
// 함수 설명 : 창 바탕 이미지 렌더.
//*****************************************************************************
void CNewUITrade::RenderBackImage()
{
	RenderImage(IMAGE_TRADE_BACK,
		m_Pos.x, m_Pos.y, float(TRADE_WIDTH), float(TRADE_HEIGHT));
	RenderImage(IMAGE_TRADE_TOP,
		m_Pos.x, m_Pos.y, float(TRADE_WIDTH), 64.f);
	RenderImage(IMAGE_TRADE_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_TRADE_RIGHT,
		m_Pos.x+TRADE_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_TRADE_BOTTOM,
		m_Pos.x, m_Pos.y+TRADE_HEIGHT-45, float(TRADE_WIDTH), 45.f);

	RenderImage(IMAGE_TRADE_LINE, m_Pos.x+1, m_Pos.y+220, 188.f, 21.f);
	RenderImage(IMAGE_TRADE_NICK_BACK, m_Pos.x+11, m_Pos.y+37, 171.f, 26.f);
	RenderImage(IMAGE_TRADE_MONEY, m_Pos.x+11, m_Pos.y+150, 170.f, 26.f);
	RenderImage(IMAGE_TRADE_NICK_BACK, m_Pos.x+11, m_Pos.y+243, 171.f, 26.f);
	RenderImage(IMAGE_TRADE_MONEY, m_Pos.x+11, m_Pos.y+356, 170.f, 26.f);

// 상대방 결정 버튼 이미지.
	float fSrcY = m_bYourConfirm ? 29.f : 0.f;
	RenderImage(IMAGE_TRADE_CONFIRM, m_Pos.x+146, m_Pos.y+186, CONFIRM_WIDTH, CONFIRM_HEIGHT, 0.f, fSrcY);

// 자신의 결정 버튼.
	DWORD dwColor = m_nMyTradeWait > 0
		? RGBA(255, 0, 0, 255) : RGBA(255, 255, 255, 255);
	fSrcY = m_bMyConfirm ? 29.f : 0.f;
	RenderImage(IMAGE_TRADE_CONFIRM, m_Pos.x+144, m_Pos.y+390, CONFIRM_WIDTH, CONFIRM_HEIGHT, 0.f, fSrcY, dwColor);
}

//*****************************************************************************
// 함수 이름 : RenderText()
// 함수 설명 : 텍스트 렌더.
//*****************************************************************************
void CNewUITrade::RenderText()
{
	unicode::t_char szTemp[128];

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);

// 제목.
	g_pRenderText->SetTextColor(216, 216, 216, 255);
	g_pRenderText->RenderText(
		m_Pos.x, m_Pos.y+11, GlobalText[226], TRADE_WIDTH, 0, RT3_SORT_CENTER);

// 상대방 길드.
	for (int i = 0; i < MAX_MARKS; ++i)
    {
        if (GuildMark[i].Key != -1 && GuildMark[i].Key == m_nYourGuildType)
        {
			::CreateGuildMark(i, false);
			::RenderBitmap(BITMAP_GUILD, (float)m_Pos.x+15, (float)m_Pos.y+42, 16, 16);
			g_pRenderText->RenderText(m_Pos.x+16, m_Pos.y+30, GuildMark[i].GuildName);
			break;
        }
    }

// 상대방 아이디.
	g_pRenderText->SetFont(g_hFontBig);

	g_pRenderText->SetTextColor(210, 230, 255, 255);
	g_pRenderText->RenderText(m_Pos.x+32, m_Pos.y+43, m_szYourID);

// 상대방 레벨.
	g_pRenderText->SetFont(g_hFontBold);

    int nLevel;
	DWORD dwColor;
	ConvertYourLevel(nLevel, dwColor);
	// 369 "%d대"
	if(nLevel == 400)
	{
		unicode::_sprintf(szTemp, "%d", nLevel);
	}
	else
	{
		unicode::_sprintf(szTemp, GlobalText[369], nLevel);
	}
	g_pRenderText->SetTextColor(dwColor);
	g_pRenderText->RenderText(m_Pos.x+134, m_Pos.y+48, "Lv.");
    g_pRenderText->RenderText(m_Pos.x+148, m_Pos.y+48, szTemp);

// 상대방 젠.
    ::ConvertGold(m_nYourTradeGold, szTemp);
	// 젠의 색 단위별 변화. (녹색 -> 주황색 -> 빨간색).
	g_pRenderText->SetTextColor(::getGoldColor(m_nYourTradeGold));
	g_pRenderText->RenderText(
		m_Pos.x+170, m_Pos.y+150+8, szTemp, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

// 자신의 젠.
	::ConvertGold(m_nMyTradeGold, szTemp);
	// 젠의 색 단위별 변화. (녹색 -> 주황색 -> 빨간색).
	g_pRenderText->SetTextColor(::getGoldColor(m_nMyTradeGold));
	g_pRenderText->RenderText(
		m_Pos.x+170, m_Pos.y+356+8, szTemp, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

// 자신의 아이디.
	g_pRenderText->SetTextColor(210, 230, 255, 255);
	g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+253, Hero->ID);

//  경고 문구를 나타낸다.
    int nAlpha = int(min(255, sin(WorldTime/200)*200+275));
	g_pRenderText->SetTextColor(210, 0, 0, nAlpha);
    g_pRenderText->RenderText(m_Pos.x+20,m_Pos.y+185,GlobalText[370]);
	g_pRenderText->SetTextColor(255, 220, 150, 255);
    g_pRenderText->RenderText(m_Pos.x+45,m_Pos.y+185,GlobalText[365]);
    g_pRenderText->RenderText(m_Pos.x+20,m_Pos.y+200,GlobalText[366]);
    g_pRenderText->RenderText(m_Pos.x+20,m_Pos.y+215,GlobalText[367]);
}

//*****************************************************************************
// 함수 이름 : RenderWarningArrow()
// 함수 설명 : 상대방의 아이템 종류는 같으나 레벨 등이 낮을 때 주의 표시 랜더.
//			  (어떤 기준으로 주의표시를 하는지 잘 모르겠음. 기획팀도 잘 모름)
//			  (그 기준은 이전 소스 그대로 옮김.)
//*****************************************************************************
void CNewUITrade::RenderWarningArrow()
{
	::EnableAlphaTest();

	int nYourItems = m_pYourInvenCtrl->GetNumberOfItems();
	ITEM* pYourItemObj;
	int nWidth;
	float fX, fY;
	POINT ptYourInvenCtrl = m_pYourInvenCtrl->GetPos();

	for (int i = 0; i < nYourItems; ++i)
	{
		pYourItemObj = m_pYourInvenCtrl->GetItem(i);
		if (ITEM_COLOR_TRADE_WARNING == pYourItemObj->byColorState)
		{
            fX = (float)ptYourInvenCtrl.x
				+(pYourItemObj->x*INVENTORY_SQUARE_WIDTH);
            fY = (float)ptYourInvenCtrl.y
				+(pYourItemObj->y*INVENTORY_SQUARE_WIDTH)
				+sinf(WorldTime*0.015f);

			::glColor3f(0.f,1.f,1.f);
            ::RenderBitmap(IMAGE_TRADE_WARNING_ARROW, fX, fY+5, 24.f, 24.f,
				0.f, 0.4f);
            ::glColor3f(1.f,1.f,1.f);

			g_pRenderText->SetFont(g_hFontBold);
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			g_pRenderText->SetBgColor(210, 0, 0, 255);
			nWidth = (int)ItemAttribute[pYourItemObj->Type].Width
				* INVENTORY_SQUARE_WIDTH;
			g_pRenderText->RenderText((int)fX, (int)fY, GlobalText[370],
				nWidth, 0, RT3_SORT_CENTER);
		}
	}

	::DisableAlphaBlend();
}

//*****************************************************************************
// 함수 이름 : ConvertYourLevel()
// 함수 설명 : 상대방 정확한 레벨을 노출시키지 않기 위해 레벨대로 변환.
// 매개 변수 : rnLevel	: 레벨대를 저장하기위한 메모리 레퍼런스.
//			   rdwColor	: 레벨대 텍스트의 색상을 저장하기위한 메모리 레퍼런스.
//*****************************************************************************
void CNewUITrade::ConvertYourLevel(int& rnLevel, DWORD& rdwColor)
{
	if(m_nYourLevel >= 400)
	{
		rnLevel = 400;
		rdwColor = (255<<24)+(153<<16)+(153<<8)+(255);
	}
	else if (m_nYourLevel >= 300)
    {
        rnLevel = 300;
	    rdwColor = (255<<24)+(255<<16)+(153<<8)+(255);
    }
    else if (m_nYourLevel >= 200)	//  흰색.
    {
        rnLevel = 200;
	    rdwColor = (255<<24)+(255<<16)+(230<<8)+(210);
    }
    else if (m_nYourLevel >= 100)	//  녹색.
    {
        rnLevel = 100;
        rdwColor = (255<<24)+(24<<16)+(201<<8)+(0);
    }
    else if (m_nYourLevel >= 50)	//  주황색.
    {
        rnLevel = 50;
        rdwColor = (255<<24)+(0<<16)+(150<<8)+(255);
    }
    else							//  빨간색.
    {
        rnLevel = 10;
        rdwColor = (255<<24)+(0<<16)+(0<<8)+(255);
    }
}

//*****************************************************************************
// 함수 이름 : GetLayerDepth()
// 함수 설명 : 창의 레이어값을 얻음.
//*****************************************************************************
float CNewUITrade::GetLayerDepth()
{
	return 2.1f;
}

//*****************************************************************************
// 함수 이름 : LoadImages()
// 함수 설명 : 이미지 리소스 로드.
//*****************************************************************************
void CNewUITrade::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_TRADE_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_TRADE_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_TRADE_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_TRADE_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_TRADE_BOTTOM, GL_LINEAR);

	LoadBitmap("Interface\\newui_myquest_Line.tga", IMAGE_TRADE_LINE, GL_LINEAR);
	LoadBitmap("Interface\\newui_Account_title.tga", IMAGE_TRADE_NICK_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_money.tga", IMAGE_TRADE_MONEY, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_accept.tga", IMAGE_TRADE_CONFIRM, GL_LINEAR);
	LoadBitmap("Interface\\CursorSitDown.tga", IMAGE_TRADE_WARNING_ARROW, GL_LINEAR, GL_CLAMP);
	
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_TRADE_BTN_CLOSE, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_money01.tga", IMAGE_TRADE_BTN_ZEN_INPUT, GL_LINEAR);
}

//*****************************************************************************
// 함수 이름 : UnloadImages()
// 함수 설명 : 이미지 리소스 삭제.
//*****************************************************************************
void CNewUITrade::UnloadImages()
{
	DeleteBitmap(IMAGE_TRADE_BTN_ZEN_INPUT);
	DeleteBitmap(IMAGE_TRADE_BTN_CLOSE);

	DeleteBitmap(IMAGE_TRADE_WARNING_ARROW);
	DeleteBitmap(IMAGE_TRADE_CONFIRM);
	DeleteBitmap(IMAGE_TRADE_MONEY);
	DeleteBitmap(IMAGE_TRADE_NICK_BACK);
	DeleteBitmap(IMAGE_TRADE_LINE);

	DeleteBitmap(IMAGE_TRADE_BOTTOM);
	DeleteBitmap(IMAGE_TRADE_RIGHT);
	DeleteBitmap(IMAGE_TRADE_LEFT);
	DeleteBitmap(IMAGE_TRADE_TOP);
	DeleteBitmap(IMAGE_TRADE_BACK);
}

//*****************************************************************************
// 함수 이름 : ProcessClosing()
// 함수 설명 : 창을 닫을 때 처리.
//*****************************************************************************
void CNewUITrade::ProcessClosing()
{
	m_pYourInvenCtrl->RemoveAllItems();
	m_pMyInvenCtrl->RemoveAllItems();

	// 거래가 성립되었음.
	if (m_bTradeAlert)
		InitYourInvenBackUp();	// 상대방 백업 물품 삭제.
}

//*****************************************************************************
// 함수 이름 : ProcessMyInvenCtrl()
// 함수 설명 : 자신의 거래 인벤토리 컨트롤 마우스 이벤트 처리.
//*****************************************************************************
void CNewUITrade::ProcessMyInvenCtrl()
{
	if (NULL == m_pMyInvenCtrl)
		return;

	if (SEASON3B::IsPress(VK_LBUTTON))
	{
		CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
		if (NULL == pPickedItem)
			return;

		ITEM* pItemObj = pPickedItem->GetItem();
		// 인벤토리에서 온 것인가?
		if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
		{
			int nSrcIndex = pPickedItem->GetSourceLinealPos();
			int nDstIndex = pPickedItem->GetTargetLinealPos(m_pMyInvenCtrl);
			if (nDstIndex != -1 && m_pMyInvenCtrl->CanMove(nDstIndex, pItemObj))
				SendRequestItemToTrade(pItemObj, MAX_EQUIPMENT_INDEX+nSrcIndex, nDstIndex);
		}
		// 거래창에서 온 것인가?(자신 비교)
		else if (pPickedItem->GetOwnerInventory() == m_pMyInvenCtrl)
		{
			int nSrcIndex = pPickedItem->GetSourceLinealPos();
			int nDstIndex = pPickedItem->GetTargetLinealPos(m_pMyInvenCtrl);
			if (nDstIndex != -1 && m_pMyInvenCtrl->CanMove(nDstIndex, pItemObj))
			{
				SendRequestEquipmentItem(REQUEST_EQUIPMENT_TRADE, nSrcIndex,
					pItemObj, REQUEST_EQUIPMENT_TRADE, nDstIndex);
			}
		}
		// 장비에서 온 것인가?(장비는 CNewUIInventoryCtrl를 사용하지 않으므로)
		else if (pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
		{
			int nSrcIndex = pPickedItem->GetSourceLinealPos();
			int nDstIndex = pPickedItem->GetTargetLinealPos(m_pMyInvenCtrl);
			if (nDstIndex != -1 && m_pMyInvenCtrl->CanMove(nDstIndex, pItemObj))
				SendRequestItemToTrade(pItemObj, nSrcIndex, nDstIndex);
		}
	}
}

//*****************************************************************************
// 함수 이름 : SendRequestItemToTrade()
// 함수 설명 : 아이템을 거래창으로 이동을 서버에 요구.
// 매개 변수 : pItemObj		: 아이템 오브젝트 주소.
//			   nStorageIndex: 인벤토리창에서의 아이템 위치.
//			   nTradeIndex	: 이동 시킬 거래창 아이템 위치.
//*****************************************************************************
void CNewUITrade::SendRequestItemToTrade(ITEM* pItemObj, int nInvenIndex,
										 int nTradeIndex)
{
	if (::IsTradeBan(pItemObj))
	{
		g_pChatListBox->AddText(
			"", GlobalText[494], SEASON3B::TYPE_ERROR_MESSAGE);							
	}
	else
	{
		m_bMyConfirm = false;
		SendRequestTradeResult(m_bMyConfirm);

		SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, nInvenIndex,
			pItemObj, REQUEST_EQUIPMENT_TRADE, nTradeIndex);
	}
}

//*****************************************************************************
// 함수 이름 : SendRequestItemToMyInven()
// 함수 설명 : 아이템을 인벤토리창으로 이동을 서버에 요구.
// 매개 변수 : pItemObj		: 아이템 오브젝트 주소.
//			   nTradeIndex	: 거래창에서의 아이템 위치.
//			   nInvenIndex	: 이동 시킬 인벤토리창 아이템 위치.
//*****************************************************************************
void CNewUITrade::SendRequestItemToMyInven(ITEM* pItemObj, int nTradeIndex,
										   int nInvenIndex)
{
	SendRequestEquipmentItem(REQUEST_EQUIPMENT_TRADE, nTradeIndex,
		pItemObj, REQUEST_EQUIPMENT_INVENTORY, nInvenIndex);

#ifdef YDG_FIX_TRADE_BUTTON_LOCK_WHNE_ITEM_MOVED
	if (m_bMyConfirm)
	{
		AlertTrade();
	}
	m_nMyTradeWait = 150;
#else	// YDG_FIX_TRADE_BUTTON_LOCK_WHNE_ITEM_MOVED
	if (!m_bMyConfirm)
        m_nMyTradeWait = 150;
#endif	// YDG_FIX_TRADE_BUTTON_LOCK_WHNE_ITEM_MOVED
}

//*****************************************************************************
// 함수 이름 : SendRequestMyGoldInput()
// 함수 설명 : 돈 입력 창에서 자신의 돈을 거래창에 올릴 때 서버로 보내는 처리.
// 매개 변수 : nInputGold	: 입력한 돈.
//*****************************************************************************
void CNewUITrade::SendRequestMyGoldInput(int nInputGold)
{

#ifdef _VS2008PORTING
	if (nInputGold <= (int)CharacterMachine->Gold+m_nMyTradeGold)
#else // _VS2008PORTING
	if (nInputGold <= CharacterMachine->Gold+m_nMyTradeGold)
#endif // _VS2008PORTING
	{
		if (m_bMyConfirm)
		{
			m_bMyConfirm = false;
			SendRequestTradeResult(m_bMyConfirm);
		}

		if (m_nMyTradeGold > 0)
			m_nMyTradeWait = 150;

		m_nTempMyTradeGold = nInputGold;
		SendRequestTradeGold(nInputGold);
	}
	else
	{
		// 423 "젠이 부족합니다."
		SEASON3B::CreateOkMessageBox(GlobalText[423]);
	}

}

//*****************************************************************************
// 함수 이름 : ProcessCloseBtn()
// 함수 설명 : 닫기 버튼 처리.
//*****************************************************************************
void CNewUITrade::ProcessCloseBtn()
{
	if (CNewUIInventoryCtrl::GetPickedItem() == NULL)
	{
		m_bTradeAlert = false;
		SendRequestTradeExit();
	}
}

//*****************************************************************************
// 함수 이름 : ProcessBtns()
// 함수 설명 : 버튼 아이템 이벤트 처리.
// 반환 값	 : 처리 했으면 true.
//*****************************************************************************
bool CNewUITrade::ProcessBtns()
{
	if (m_nMyTradeWait > 0)
		--m_nMyTradeWait;

	if (m_abtn[BTN_CLOSE].UpdateMouseEvent())
	{
		::PlayBuffer(SOUND_CLICK01);	// 버튼 모듈에서 사운드 나게 해줄 것.
		ProcessCloseBtn();
		return true;
	}
	else if (SEASON3B::IsPress(VK_LBUTTON)
		&& CheckMouseIn(m_Pos.x+169, m_Pos.y+7, 13, 12))
	{
		::PlayBuffer(SOUND_CLICK01);
		ProcessCloseBtn();
		return true;
	}
	else if (m_abtn[BTN_ZEN_INPUT].UpdateMouseEvent())
	{
		SEASON3B::CreateMessageBox(
			MSGBOX_LAYOUT_CLASS(SEASON3B::CTradeZenMsgBoxLayout));
		::PlayBuffer(SOUND_CLICK01);	// 버튼 모듈에서 사운드 나게 해줄 것.
		return true;
	}
	else if (SEASON3B::IsRelease(VK_LBUTTON)
		&& CheckMouseIn(m_posMyConfirm.x, m_posMyConfirm.y, CONFIRM_WIDTH, CONFIRM_HEIGHT))
	{
		if (0 == m_nMyTradeWait && CNewUIInventoryCtrl::GetPickedItem() == NULL)
		{
			::PlayBuffer(SOUND_CLICK01);
			//  경고 발생.
			if (m_bTradeAlert && !m_bMyConfirm)
			{
				SEASON3B::CreateMessageBox(
					MSGBOX_LAYOUT_CLASS(SEASON3B::CTradeAlertMsgBoxLayout));
			}
			else
			{
				AlertTrade();
			}
		}
		return true;
	}

	return false;
}

//*****************************************************************************
// 함수 이름 : AlertTrade()
// 함수 설명 : 거래 경고 발생 시킴.
//*****************************************************************************
void CNewUITrade::AlertTrade()
{
	m_bMyConfirm = !m_bMyConfirm;

	m_bTradeAlert = true;
	SendRequestTradeResult(m_bMyConfirm);
}

//*****************************************************************************
// 함수 이름 : GetYourID()
// 함수 설명 : 상대방 ID를 얻음.
// 매개 변수 : pszYourID	: 상대방 ID를 담을 메모리 주소.
//*****************************************************************************
void CNewUITrade::GetYourID(char* pszYourID)
{
	::strcpy(pszYourID, m_szYourID);
}

//*****************************************************************************
// 함수 이름 : ProcessToReceiveTradeRequest()
// 함수 설명 : 상대방에게서 거래 신청이 왔다고 서버에서 알려줄 때 처리.
// 매개 변수 : pbyYourID	: 상대방 ID.
//*****************************************************************************
void CNewUITrade::ProcessToReceiveTradeRequest(BYTE* pbyYourID)
{
	if (g_pNewUISystem->IsImpossibleTradeInterface())
	{
		SendRequestTradeAnswer(false);
		return;
	}

	::memcpy(m_szYourID, pbyYourID, MAX_ID_SIZE);
	m_szYourID[MAX_ID_SIZE] = NULL;

	SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CTradeMsgBoxLayout));

	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
}

//*****************************************************************************
// 함수 이름 : ProcessToReceiveTradeResult()
// 함수 설명 : 상대방에게 거래 요청 후 그 결과를 서버에서 알려줄 때 처리.
// 매개 변수 : pTradeData	: 거래 구조체 주소.
//*****************************************************************************
void CNewUITrade::ProcessToReceiveTradeResult(LPPTRADE pTradeData)
{
	switch(pTradeData->SubCode)
	{
	case 0:		// 492 "거래를 취소했습니다."
		g_pChatListBox->AddText("", GlobalText[492], SEASON3B::TYPE_ERROR_MESSAGE);
		break;

	case 2:		// 493 "거래할 수 없는 상황입니다."
		g_pChatListBox->AddText("", GlobalText[493], SEASON3B::TYPE_ERROR_MESSAGE);
		break;

	case 1:		// 상대방이 거래에 응함.
		g_pNewUISystem->Show(SEASON3B::INTERFACE_TRADE);

		InitTradeInfo();

		int x = 260*MouseX/640;
		::SetCursorPos(x*WindowWidth/640, MouseY*WindowHeight/480);

		// 바로 전에 거래 실패한 캐릭터와 같은 캐릭터가 아니면 백업한 거래
		//데이터 정보를 클리어 한다.
		char szTempID[MAX_ID_SIZE+1];
		::memcpy(szTempID, pTradeData->ID, MAX_ID_SIZE);
		szTempID[MAX_ID_SIZE] = NULL;
		
		if (!m_bTradeAlert && ::strcmp(m_szYourID, szTempID))
			InitYourInvenBackUp();

        m_bTradeAlert = false;
        m_nYourGuildType = pTradeData->GuildKey;
        ::memcpy(m_szYourID, pTradeData->ID, MAX_ID_SIZE);
		m_szYourID[MAX_ID_SIZE] = NULL;
        m_nYourLevel = pTradeData->Level;   //  상대방 레벨.
		break;
	}
}

//*****************************************************************************
// 함수 이름 : ProcessToReceiveYourItemDelete()
// 함수 설명 : 서버에서 상대방의 아이템이 삭제 되었다고 알려줄 때 처리.
// 매개 변수 : byYourInvenIndex	: 삭제된 상대방 아이템 위치 정보.
//*****************************************************************************
void CNewUITrade::ProcessToReceiveYourItemDelete(BYTE byYourInvenIndex)
{
	BackUpYourInven(int(byYourInvenIndex));
	ITEM* pYourItemObj = m_pYourInvenCtrl->FindItem(int(byYourInvenIndex));
	m_pYourInvenCtrl->RemoveItem(pYourItemObj);
	AlertYourTradeInven();
   	::PlayBuffer(SOUND_GET_ITEM01);
}

//*****************************************************************************
// 함수 이름 : BackUpYourInven()
// 함수 설명 : 상대방 중요 아이템 백업.
//			   (백업된 아이템이 없거나, 백업된 아이템과 비교하여 다르거나, 같은
//			  타잎의 아이템일 경우 매개 변수의 아이템이 더 좋은 경우에만 백업.)
// 매개 변수 : nYourInvenIndex	: 상대방 거래 아이템 위치.
//*****************************************************************************
void CNewUITrade::BackUpYourInven(int nYourInvenIndex)
{
	ITEM* pYourItemObj = m_pYourInvenCtrl->FindItem(nYourInvenIndex);
	BackUpYourInven(pYourItemObj);
}

//*****************************************************************************
// 함수 이름 : BackUpYourInven()
// 함수 설명 : 상대방 중요 아이템 백업.
//			   (백업된 아이템이 없거나, 백업된 아이템과 비교하여 다르거나, 같은
//			  타잎의 아이템일 경우 매개 변수의 아이템이 더 좋은 경우에만 백업.)
// 매개 변수 : pYourItemObj	: 백업할 상대방 거래 아이템.
//*****************************************************************************
void CNewUITrade::BackUpYourInven(ITEM* pYourItemObj)
{
    if ((pYourItemObj->Type>=ITEM_HELPER && pYourItemObj->Type<=ITEM_HELPER+4)	//  수호, 사탄.
		|| (pYourItemObj->Type==ITEM_POTION+13 || pYourItemObj->Type==ITEM_POTION+14 || pYourItemObj->Type==ITEM_POTION+16)	//  축석, 영석, 생석.
		|| (pYourItemObj->Type>=ITEM_POTION+31)									//  수호의 보석
		|| (COMGEM::isCompiledGem(pYourItemObj))
		|| (pYourItemObj->Type>=ITEM_WING && pYourItemObj->Type<=ITEM_WING+6)	//  날개.
		|| (pYourItemObj->Type>=ITEM_HELPER+30)									//  군주의 망토.
#ifdef ADD_ALICE_WINGS_1
		|| (pYourItemObj->Type>=ITEM_WING+36 && pYourItemObj->Type<=ITEM_WING+43)// 폭풍의 날개 ~ 제왕의 망토, 소환술사 날개.
#else	// ADD_ALICE_WINGS_1
		|| (pYourItemObj->Type>=ITEM_WING+36 && pYourItemObj->Type<=ITEM_WING+40 )// 폭풍의 날개 ~ 제왕의 망토
#endif	// ADD_ALICE_WINGS_1
        || (pYourItemObj->Type==ITEM_WING+15 )									//  혼돈의 보석.
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (pYourItemObj->Type>=ITEM_WING+49 && pYourItemObj->Type<=ITEM_WING+50)// 레이지파이터 날개
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		|| ((((pYourItemObj->Level>>3)&15)>4 && pYourItemObj->Type<ITEM_WING) || (pYourItemObj->Option1&63)>0))	//  레벨 4이상의 아이템.
    {
		int nCompareValue;
        bool bSameItem = false;

		for (int i = 0; i < MAX_TRADE_INVEN; ++i)
		{
			if (-1 == m_aYourInvenBackUp[i].Type)
				continue;
			
			nCompareValue = ::CompareItem(m_aYourInvenBackUp[i], *pYourItemObj);
			if (0 == nCompareValue)			// 같은 아이템.
			{
				bSameItem = true;
				break;
			}
			else if (-1 == nCompareValue)	// pYourItemObj이 더 좋은 아이템.
			{
				bSameItem = true;
				m_aYourInvenBackUp[i] = *pYourItemObj;
				break;
			}
			else if (2 != nCompareValue)	// pYourBackUpItemObj이 더 좋은 아이템.
			{
				bSameItem = true;
			}
		}

        if (!bSameItem)		// 다른 아이템인 경우.
        {
			// 비어있는 곳에 아이템 복사.
#ifdef _VS2008PORTING
            for (int i = 0; i < MAX_TRADE_INVEN; ++i)
#else // _VS2008PORTING
			for(i = 0; i < MAX_TRADE_INVEN; ++i)
#endif // _VS2008PORTING
            {
                if (-1 == m_aYourInvenBackUp[i].Type)
                {
                    m_aYourInvenBackUp[i] = *pYourItemObj;
                    break;
                }
            }
        }
    }
}

//*****************************************************************************
// 함수 이름 : ProcessToReceiveYourItemAdd()
// 함수 설명 : 서버에서 상대방의 아이템이 추가되었다고 알려줄 때 처리.
// 매개 변수 : byYourInvenIndex	: 상대방 거래 아이템 위치 정보.
//			   pbyItemPacket	: 상대방 거래 아이템 정보.
//*****************************************************************************
void CNewUITrade::ProcessToReceiveYourItemAdd(BYTE byYourInvenIndex,
											  BYTE* pbyItemPacket)
{
	m_pYourInvenCtrl->AddItem(byYourInvenIndex, pbyItemPacket);
	AlertYourTradeInven();
	::PlayBuffer(SOUND_GET_ITEM01);
}

//*****************************************************************************
// 함수 이름 : AlertYourTradeInven()
// 함수 설명 : 상대방의 변경된 아이템을 비교하여 주의하라는 판단을 하는 함수.
//*****************************************************************************
void CNewUITrade::AlertYourTradeInven()
{
#ifndef _VS2008PORTING		// #ifndef
	int i, j;
#endif // _VS2008PORTING
    int nCount = 0;
    int nCompareItemType[10];

    m_bTradeAlert = false;

    // 변경된 아이템이 무엇인지 검사한다.
	int nYourItems = m_pYourInvenCtrl->GetNumberOfItems();
	ITEM* pYourItemObj;
	int nCompareValue;

#ifdef _VS2008PORTING
	for (int i = 0; i < nYourItems; ++i)
#else // _VS2008PORTING
	for (i = 0; i < nYourItems; ++i)
#endif // _VS2008PORTING
	{
		pYourItemObj = m_pYourInvenCtrl->GetItem(i);
#ifdef _VS2008PORTING
		for (int j = 0; j < MAX_TRADE_INVEN; ++j)
#else // _VS2008PORTING
		for (j = 0; j < MAX_TRADE_INVEN; ++j)
#endif // _VS2008PORTING
		{
			if (m_aYourInvenBackUp[j].Type == pYourItemObj->Type)
			{
				nCompareValue = ::CompareItem(m_aYourInvenBackUp[j], *pYourItemObj);
				if (1 == nCompareValue)		// m_aYourInvenBackUp[j]이 더 좋은 경우.
                {
                    m_bTradeAlert = true;
					pYourItemObj->byColorState = ITEM_COLOR_TRADE_WARNING;
				}
				else
				{
					if (0 == nCompareValue)	// 같은 경우.
						nCompareItemType[nCount++] = m_aYourInvenBackUp[j].Type;

					pYourItemObj->byColorState = ITEM_COLOR_NORMAL;
					break;
				}
			}
		}
	}

    // 이미 같은 아이템이 등록되었을 때.
    if (nCount > 0)
    {
        m_bTradeAlert = false;
#ifdef _VS2008PORTING
        for (int i = 0; i < nCount; ++i)
#else // _VS2008PORTING
        for (i = 0; i < nCount; ++i)
#endif // _VS2008PORTING
        {
#ifdef _VS2008PORTING
			for (int j= 0; j < nYourItems; ++j)
#else // _VS2008PORTING
			for (j= 0; j < nYourItems; ++j)
#endif // _VS2008PORTING
			{
				pYourItemObj = m_pYourInvenCtrl->GetItem(j);
				if (nCompareItemType[i] == pYourItemObj->Type)
					pYourItemObj->byColorState = ITEM_COLOR_NORMAL;
			}
        }
    }
}

//*****************************************************************************
// 함수 이름 : ProcessToReceiveMyTradeGold()
// 함수 설명 : 서버에서 자신의 돈을 거래창에 넣은 결과를 알려줄 때 처리.
// 매개 변수 : bySuccess	: 성공이면 1, 실패면 0.
//*****************************************************************************
void CNewUITrade::ProcessToReceiveMyTradeGold(BYTE bySuccess)
{
	m_nMyTradeGold = bySuccess ? m_nTempMyTradeGold : 0;
}

//*****************************************************************************
// 함수 이름 : ProcessToReceiveYourConfirm()
// 함수 설명 : 서버에서 상대방이 거래 확인(결정) 버튼을 눌렀다고 알려줄 때 처리.
// 매개 변수 : byState	: 거래 확인(결정) 상태 값.
//*****************************************************************************
void CNewUITrade::ProcessToReceiveYourConfirm(BYTE byState)
{
	switch (byState)
	{
	case 0:	m_bYourConfirm = false;	break;	// 거래 결정 버튼이 눌렸다.
	case 1:	m_bYourConfirm = true;	break;	// 거래 결정 취소.
	case 2:	m_bMyConfirm = false;
#ifdef YDG_FIX_TRADE_BUTTON_LOCK_WHNE_ITEM_MOVED
		m_bYourConfirm = false;
        m_nMyTradeWait = 150;
#endif	// YDG_FIX_TRADE_BUTTON_LOCK_WHNE_ITEM_MOVED
		break;	// 잠시 교환 불가(상대가 거래창에서 이동을 취했다는 경고)
	case 3:	break;							// 잠시 교환 불가 상태 풀림.
	}

	PlayBuffer(SOUND_CLICK01);
}

//*****************************************************************************
// 함수 이름 : ProcessToReceiveTradeExit()
// 함수 설명 : 서버에서 거래 종료라는 지시 받았을 때 처리.
// 매개 변수 : byState	: 거래 종료 상태 값.
//						  정상적 거래 성공시 값은 1.
//*****************************************************************************
void CNewUITrade::ProcessToReceiveTradeExit(BYTE byState)
{
	switch (byState)
	{
	case 0:		// 492 "거래를 취소했습니다."(상대가 거래 취소)
		{
			g_pChatListBox->AddText("", GlobalText[492], SEASON3B::TYPE_ERROR_MESSAGE);
     
			// 거래 창에 등록된 아이템을 백업한다.
			m_bTradeAlert = false;

			int nYourItems = m_pYourInvenCtrl->GetNumberOfItems();
			for (int i = 0; i < nYourItems; ++i)
				BackUpYourInven(m_pYourInvenCtrl->GetItem(i));

		}
		break;

	case 2:		// 495 "장비창이 가득차서 거래가 취소되었습니다."
		g_pChatListBox->AddText("", GlobalText[495], SEASON3B::TYPE_ERROR_MESSAGE);
		break;

	case 3:		// 496 "거래 요청이 취소되었습니다."
		g_pChatListBox->AddText("", GlobalText[496], SEASON3B::TYPE_ERROR_MESSAGE);
		break;

	case 4:		// 2108 "강화 아이템은 거래 하실 수 없습니다."
		g_pChatListBox->AddText("", GlobalText[2108], SEASON3B::TYPE_ERROR_MESSAGE);
		break;
	}

   	SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();

	g_MessageBox->PopMessageBox();	// 현재 떠 있는 메시지 박스 닫음.

	g_pNewUISystem->Hide(SEASON3B::INTERFACE_TRADE);
}

//*****************************************************************************
// 함수 이름 : ProcessToReceiveTradeItems()
// 함수 설명 : 서버에서 거래 아이템을 넣으라는 지시 받았을 때 처리.
// 매개 변수 : nIndex		: 거래창에 넣을 아이템 위치 정보.
//			   pbyItemPacket: 서버에서 받는 아이템 정보.
//*****************************************************************************
void CNewUITrade::ProcessToReceiveTradeItems(int nIndex, BYTE* pbyItemPacket)
{
	SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();

	if (nIndex >= 0 && nIndex < (m_pMyInvenCtrl->GetNumberOfColumn()
		* m_pMyInvenCtrl->GetNumberOfRow()))
		m_pMyInvenCtrl->AddItem(nIndex, pbyItemPacket);
}

int SEASON3B::CNewUITrade::GetPointedItemIndexMyInven()
{
	return m_pMyInvenCtrl->GetPointedSquareIndex();
}

int SEASON3B::CNewUITrade::GetPointedItemIndexYourInven()
{
	return m_pYourInvenCtrl->GetPointedSquareIndex();
}
