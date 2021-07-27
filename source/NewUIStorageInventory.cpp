//*****************************************************************************
// File: NewUIStorageInventory.cpp
//
// Desc: implementation of the CNewUIStorageInventory class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "NewUIStorageInventory.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "ZzzInventory.h"
#include "wsclientinline.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CNewUIStorageInventory::CNewUIStorageInventory() 
{
	m_pNewUIMng = NULL;
	m_pNewInventoryCtrl = NULL;
	m_Pos.x = m_Pos.y = 0;
}

CNewUIStorageInventory::~CNewUIStorageInventory()
{
	Release();
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 창고 창 생성.
// 매개 변수 : pNewUIMng	: CNewUIManager 오브젝트 주소.
//			   x			: x 좌표.
//			   y			: y 좌표.
//*****************************************************************************
bool CNewUIStorageInventory::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng
		|| NULL == g_pNewItemMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_STORAGE, this);

	m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
	if (false == m_pNewInventoryCtrl->Create(
		g_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15, y + 36, 8, 15))
	{
		SAFE_DELETE(m_pNewInventoryCtrl);
		return false;
	}

	SetPos(x, y);

	LoadImages();

// 버튼들 생성.
	int anBtnPosX[MAX_BTN] = { 38, 78, 118 };
	int anToolTipText[MAX_BTN] = { 235, 236, 242 };
	for (int i = BTN_INSERT_ZEN; i < MAX_BTN; ++i)
	{
		m_abtn[i].ChangeButtonImgState(true, IMAGE_STORAGE_BTN_INSERT_ZEN + i);
		m_abtn[i].ChangeButtonInfo(x + anBtnPosX[i], y + 390, 36, 29);
		m_abtn[i].ChangeToolTipText(GlobalText[anToolTipText[i]], true);
	}

	m_bLock = false;
	SetItemAutoMove(false);
	InitBackupItemInfo();

	Show(false);	

	return true;
}

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : 창 Release.
//*****************************************************************************
void CNewUIStorageInventory::Release()
{
	UnloadImages();

	SAFE_DELETE(m_pNewInventoryCtrl);

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

//*****************************************************************************
// 함수 이름 : SetPos()
// 함수 설명 : 창 위치 지정.
//*****************************************************************************
void CNewUIStorageInventory::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

//*****************************************************************************
// 함수 이름 : UpdateMouseEvent()
// 함수 설명 : 마우스 이벤트 처리.
// 반환 값	 : true면 창 뒤로도 이벤트를 처리.
//*****************************************************************************
bool CNewUIStorageInventory::UpdateMouseEvent()
{
	if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
		return false;

	ProcessInventoryCtrl();

	if (ProcessBtns())
		return false;

	// 창 영역 클릭시 하위 UI처리 및 이동 불가
	if(CheckMouseIn(m_Pos.x, m_Pos.y, STORAGE_WIDTH, STORAGE_HEIGHT))
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
bool CNewUIStorageInventory::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_STORAGE);
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
bool CNewUIStorageInventory::Update()
{
	if(m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
		return false;

	return true;
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 창 렌더.
// 반환 값	 : true면 성공.
//*****************************************************************************
bool CNewUIStorageInventory::Render()
{
	::EnableAlphaTest();

	::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderBackImage();
	RenderText();

	if (m_pNewInventoryCtrl)
		m_pNewInventoryCtrl->Render();

	for (int i = BTN_INSERT_ZEN; i < MAX_BTN; ++i)
		m_abtn[i].Render();

	::DisableAlphaBlend();
	
	return true;
}

//*****************************************************************************
// 함수 이름 : RenderBackImage()
// 함수 설명 : 창 바탕 이미지 렌더.
//*****************************************************************************
void CNewUIStorageInventory::RenderBackImage()
{
	RenderImage(IMAGE_STORAGE_BACK,
		m_Pos.x, m_Pos.y, float(STORAGE_WIDTH), float(STORAGE_HEIGHT));
	RenderImage(IMAGE_STORAGE_TOP,
		m_Pos.x, m_Pos.y, float(STORAGE_WIDTH), 64.f);
	RenderImage(IMAGE_STORAGE_LEFT,
		m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_STORAGE_RIGHT,
		m_Pos.x+STORAGE_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_STORAGE_BOTTOM,
		m_Pos.x, m_Pos.y+STORAGE_HEIGHT-45, float(STORAGE_WIDTH), 45.f);

	RenderImage(IMAGE_STORAGE_MONEY,
		m_Pos.x+10, m_Pos.y+342, 170.f, 46.f);
}

//*****************************************************************************
// 함수 이름 : RenderText()
// 함수 설명 : 텍스트 렌더.
//*****************************************************************************
void CNewUIStorageInventory::RenderText()
{
	unicode::t_char szTemp[128];
	int nTempZen;

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);

// 제목.
	unicode::_sprintf(
		szTemp, "%s (%s)", GlobalText[234], GlobalText[m_bLock ? 241 : 240]);
	if (m_bLock)
		g_pRenderText->SetTextColor(240, 32, 32, 255);
	else
		g_pRenderText->SetTextColor(216, 216, 216, 255);
	g_pRenderText->RenderText(
		m_Pos.x, m_Pos.y+11, szTemp, STORAGE_WIDTH, 0, RT3_SORT_CENTER);

// 젠.
	nTempZen = CharacterMachine->StorageGold;
    ::ConvertGold(nTempZen, szTemp);
	// 젠의 색 단위별 변화. (녹색 -> 주황색 -> 빨간색).
	g_pRenderText->SetTextColor(::getGoldColor(nTempZen));
	g_pRenderText->RenderText(
		m_Pos.x+168, m_Pos.y+342+8, szTemp, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

//  창고사용료.
	g_pRenderText->SetTextColor(240, 64, 64, 255);
	// 266 "창고사용료"
    g_pRenderText->RenderText(m_Pos.x+10+15, m_Pos.y+342+29, GlobalText[266]);

	// 아래 계산은 double로 계산해야 오차가 없음.
#ifdef CSK_FIX_WOPS_K30648_STORAGEVALUE
	
#ifdef LDK_FIX_STORAGE_CHARGE
	__int64 iTotalLevel = CharacterAttribute->Level + Master_Level_Data.nMLevel;	// 종합레벨
#else // LDK_FIX_STORAGE_CHARGE
	__int64 iTotalLevel = CharacterAttribute->Level + Master_Level_Data.nMLevel + 1;	// 종합레벨
#endif // LDK_FIX_STORAGE_CHARGE

	nTempZen = int(double(iTotalLevel) * double(iTotalLevel) * 0.04);
	nTempZen += m_bLock ? int(CharacterAttribute->Level) * 2 : 0;
#else // CSK_FIX_WOPS_K30648_STORAGEVALUE 
	nTempZen = int(double(CharacterAttribute->Level)
		* double(CharacterAttribute->Level) * 0.04);
	nTempZen += m_bLock ? int(CharacterAttribute->Level) * 2 : 0;
#endif // CSK_FIX_WOPS_K30648_STORAGEVALUE
    nTempZen = max(1, nTempZen);
    if (nTempZen >= 1000)
		nTempZen = nTempZen / 100 * 100;	// 10의 자리 00으로 만들기.
	else if (nTempZen >= 100)
		nTempZen = nTempZen / 10 * 10;		// 1의 자리 0으로 만들기.
    ::ConvertGold(nTempZen, szTemp);
	g_pRenderText->SetTextColor(255, 220, 150, 255);
	g_pRenderText->RenderText(
		m_Pos.x+168, m_Pos.y+342+29, szTemp, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);
}

//*****************************************************************************
// 함수 이름 : GetLayerDepth()
// 함수 설명 : 창의 레이어값을 얻음.
//*****************************************************************************
float CNewUIStorageInventory::GetLayerDepth()
{
	return 2.2f;
}

//*****************************************************************************
// 함수 이름 : GetInventoryCtrl()
// 함수 설명 : (격자 모양의) 소지품 컨트롤을 얻음.
//*****************************************************************************
CNewUIInventoryCtrl* CNewUIStorageInventory::GetInventoryCtrl() const
{
	return m_pNewInventoryCtrl;
}

//*****************************************************************************
// 함수 이름 : LoadImages()
// 함수 설명 : 이미지 리소스 로드.
//*****************************************************************************
void CNewUIStorageInventory::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_STORAGE_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_STORAGE_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_STORAGE_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_STORAGE_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_STORAGE_BOTTOM, GL_LINEAR);
	
	LoadBitmap("Interface\\newui_Bt_money01.tga", IMAGE_STORAGE_BTN_INSERT_ZEN, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_money02.tga", IMAGE_STORAGE_BTN_TAKE_ZEN, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_lock02.tga", IMAGE_STORAGE_BTN_UNLOCK, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_lock.tga", IMAGE_STORAGE_BTN_LOCK, GL_LINEAR);

	LoadBitmap("Interface\\newui_item_money3.tga", IMAGE_STORAGE_MONEY, GL_LINEAR);
}

//*****************************************************************************
// 함수 이름 : LoadImages()
// 함수 설명 : 이미지 리소스 삭제.
//*****************************************************************************
void CNewUIStorageInventory::UnloadImages()
{
	DeleteBitmap(IMAGE_STORAGE_MONEY);

	DeleteBitmap(IMAGE_STORAGE_BTN_LOCK);
	DeleteBitmap(IMAGE_STORAGE_BTN_UNLOCK);
	DeleteBitmap(IMAGE_STORAGE_BTN_TAKE_ZEN);
	DeleteBitmap(IMAGE_STORAGE_BTN_INSERT_ZEN);

	DeleteBitmap(IMAGE_STORAGE_BOTTOM);
	DeleteBitmap(IMAGE_STORAGE_RIGHT);
	DeleteBitmap(IMAGE_STORAGE_LEFT);
	DeleteBitmap(IMAGE_STORAGE_TOP);
	DeleteBitmap(IMAGE_STORAGE_BACK);
}

//*****************************************************************************
// 함수 이름 : LockStorage()
// 함수 설명 : 창고를 잠금.
// 매개 변수 : bLock	: true면 잠금.
//*****************************************************************************
void CNewUIStorageInventory::LockStorage(bool bLock)
{
	m_bLock = bLock;
	ChangeLockBtnImage();
}

//*****************************************************************************
// 함수 이름 : ChangeLockBtnImage()
// 함수 설명 : m_bLock의 값에 따라 잠금 버튼 이미지를 교체.
//*****************************************************************************
void CNewUIStorageInventory::ChangeLockBtnImage()
{
	m_abtn[BTN_LOCK].UnRegisterButtonState();
	if (m_bLock)
		m_abtn[BTN_LOCK].ChangeButtonImgState(true, IMAGE_STORAGE_BTN_LOCK);
	else
		m_abtn[BTN_LOCK].ChangeButtonImgState(true, IMAGE_STORAGE_BTN_UNLOCK);
}

//*****************************************************************************
// 함수 이름 : ProcessClosing()
// 함수 설명 : 창을 닫을 때 처리.
// 반환 값	 : true 면 닫기 성공.
//*****************************************************************************
bool CNewUIStorageInventory::ProcessClosing()
{
	// SendRequestEquipmentItem()후 서버로 부터 ReceiveEquipmentItem()을 받지
	//않았다면 창을 닫지 않음.
	if (EquipmentItem)
		return false;

	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	DeleteAllItems();
	SendRequestStorageExit();
	return true;
}

//*****************************************************************************
// 함수 이름 : InsertItem()
// 함수 설명 :창고에 아이템을 넣음.
// 반환 값	 : true면 성공.
// 매개 변수 : nIndex		: m_pNewInventoryCtrl에서 아이템 넣을 자리.
//			   pbyItemPacket: 서버에서 받는 아이템 정보.
//*****************************************************************************
bool CNewUIStorageInventory::InsertItem(int nIndex, BYTE* pbyItemPacket)
{
	if (m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->AddItem(nIndex, pbyItemPacket);
	
	return false;
}

//*****************************************************************************
// 함수 이름 : DeleteAllItems()
// 함수 설명 : 창고의 모든 아이템 삭제.
//*****************************************************************************
void CNewUIStorageInventory::DeleteAllItems()
{
	if (m_pNewInventoryCtrl)
		m_pNewInventoryCtrl->RemoveAllItems();
}

//*****************************************************************************
// 함수 이름 : ProcessInventoryCtrl()
// 함수 설명 : 창고 인벤토리 컨트롤 마우스 이벤트 처리.
//*****************************************************************************
void CNewUIStorageInventory::ProcessInventoryCtrl()
{
	if (NULL == m_pNewInventoryCtrl)
		return;
	CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

	if( pPickedItem )
	{
		ITEM* pItemObj = pPickedItem->GetItem();
		if (NULL == pItemObj)	return;
		if (SEASON3B::IsPress(VK_LBUTTON))
		{
			// 인벤토리에서 온 것인가?
			if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
			{
				int nSrcIndex = pPickedItem->GetSourceLinealPos();
				int nDstIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
				if (nDstIndex != -1 && m_pNewInventoryCtrl->CanMove(nDstIndex, pItemObj))
					SendRequestItemToStorage(pItemObj, MAX_EQUIPMENT_INDEX+nSrcIndex, nDstIndex);
			}
			// 창고에서 온 것인가?(자신 비교)
			else if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
			{
				int nSrcIndex = pPickedItem->GetSourceLinealPos();
				int nDstIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
				if (nDstIndex != -1 && m_pNewInventoryCtrl->CanMove(nDstIndex, pItemObj))
				{
					SendRequestEquipmentItem(REQUEST_EQUIPMENT_STORAGE, nSrcIndex,
						pItemObj, REQUEST_EQUIPMENT_STORAGE, nDstIndex);
				}
			}
			// 장비에서 온 것인가?(장비는 CNewUIInventoryCtrl를 사용하지 않으므로)
			else if (pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
			{
				int nSrcIndex = pPickedItem->GetSourceLinealPos();
				int nDstIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
				if (nDstIndex != -1 && m_pNewInventoryCtrl->CanMove(nDstIndex, pItemObj))
					SendRequestItemToStorage(pItemObj, nSrcIndex, nDstIndex);
			}
		}
		else
		{
		#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 창고 물품 등록시 인벤 Warning칼라 [lem_2010.9.8]
			if( ::IsStoreBan( pItemObj ) )	m_pNewInventoryCtrl->SetSquareColorNormal(1.0f, 0.0f, 0.0f );	
			else							m_pNewInventoryCtrl->SetSquareColorNormal(0.1f, 0.4f, 0.8f );
		#endif // LEM_ADD_LUCKYITEM
		}	
	}
	else if (SEASON3B::IsPress(VK_RBUTTON))
	{
		ProcessStorageItemAutoMove();
	}
}

//*****************************************************************************
// 함수 이름 : ProcessStorageItemAutoMove()
// 함수 설명 : 창고 아이템 자동 이동 처리.
//*****************************************************************************
void CNewUIStorageInventory::ProcessStorageItemAutoMove()
{
	// 커서에 아이템이 붙어 있으면 처리하지 않음.
	if (g_pPickedItem)
		if (g_pPickedItem->GetItem())
			return;

	// 아이템 자동 이동 중일 때 또 아이템 자동 이동인 경우는 막음.
	if (IsItemAutoMove())
		return;
	
	ITEM* pItemObj = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);
	if (pItemObj)
	{
		int nDstIndex = g_pMyInventory->FindEmptySlot(pItemObj);
		if (-1 != nDstIndex)
		{
			SetItemAutoMove(true);
			
			int nSrcIndex
				= pItemObj->y * m_pNewInventoryCtrl->GetNumberOfColumn()
					+ pItemObj->x;
			SendRequestItemToMyInven(pItemObj, nSrcIndex, MAX_EQUIPMENT_INDEX+nDstIndex);

			::PlayBuffer(SOUND_GET_ITEM01);
		}
	}
}

//*****************************************************************************
// 함수 이름 : ProcessMyInvenItemAutoMove()
// 함수 설명 : 인벤토리창에서 창고창으로 아이템 자동 이동 처리.
//*****************************************************************************
void CNewUIStorageInventory::ProcessMyInvenItemAutoMove()
{
	if (g_pPickedItem)
		if (g_pPickedItem->GetItem())
			return;
		
	// 아이템 자동 이동 중일 때 또 아이템 자동 이동인 경우는 막음.
	if (IsItemAutoMove())
		return;
		
	CNewUIInventoryCtrl* pMyInvenCtrl = g_pMyInventory->GetInventoryCtrl();
	ITEM* pItemObj = pMyInvenCtrl->FindItemAtPt(MouseX, MouseY);
	if (pItemObj)
	{
		if (pItemObj->Type == ITEM_HELPER+20)	// 마법사의 반지, 용사의 반지, 전사의 반지
			return;

		int nDstIndex = FindEmptySlot(pItemObj);
		if (-1 != nDstIndex)
		{
			SetItemAutoMove(true);
			
			int nSrcIndex
				= pItemObj->y * pMyInvenCtrl->GetNumberOfColumn()
				+ pItemObj->x;
			SendRequestItemToStorage(pItemObj, MAX_EQUIPMENT_INDEX+nSrcIndex, nDstIndex);
			
			::PlayBuffer(SOUND_GET_ITEM01);
		}
	}
}

//*****************************************************************************
// 함수 이름 : SendRequestItemToMyInven()
// 함수 설명 : 아이템을 인벤토리창으로 이동을 서버에 요구.
// 매개 변수 : pItemObj		: 아이템 오브젝트 주소.
//			   nStorageIndex: 창고에서의 아이템 위치.
//			   nInvenIndex	: 이동 시킬 인벤토리창 아이템 위치.
//*****************************************************************************
void CNewUIStorageInventory::SendRequestItemToMyInven(ITEM* pItemObj, int nStorageIndex, int nInvenIndex)
{
	if (!IsStorageLocked() || IsCorrectPassword())	// 잠겨있지 않고 패스워드 통과되어 있는 경우.
	{
		SendRequestEquipmentItem(REQUEST_EQUIPMENT_STORAGE, nStorageIndex,
			pItemObj, REQUEST_EQUIPMENT_INVENTORY, nInvenIndex);

	}
	else
	{
		SetBackupInvenIndex(nInvenIndex);
		if (!IsItemAutoMove())
			g_pPickedItem->HidePickedItem();

		SEASON3B::CreateMessageBox(
			MSGBOX_LAYOUT_CLASS(SEASON3B::CPasswordKeyPadMsgBoxLayout));
	}
}

//*****************************************************************************
// 함수 이름 : SendRequestItemToStorage()
// 함수 설명 : 아이템을 창고창으로 이동을 서버에 요구.
// 매개 변수 : pItemObj		: 아이템 오브젝트 주소.
//			   nStorageIndex: 인벤토리창에서의 아이템 위치.
//			   nInvenIndex	: 이동 시킬 창고창 아이템 위치.
//*****************************************************************************
void CNewUIStorageInventory::SendRequestItemToStorage(ITEM* pItemObj, int nInvenIndex, int nStorageIndex)
{
	if (::IsStoreBan(pItemObj))	// 창고에 넣을 수 없는 아이템인가?
	{
		g_pChatListBox->AddText(
			"", GlobalText[667], SEASON3B::TYPE_ERROR_MESSAGE);
		SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();

		if (IsItemAutoMove())
			SetItemAutoMove(false);
	}
	else
	{
		// 서버로 전송.
     	SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, nInvenIndex,
			pItemObj, REQUEST_EQUIPMENT_STORAGE, nStorageIndex);
	}
}

//*****************************************************************************
// 함수 이름 : ProcessBtns()
// 함수 설명 : 버튼 아이템 이벤트 처리.
// 반환 값	 : 처리 했으면 true.
//*****************************************************************************
bool CNewUIStorageInventory::ProcessBtns()
{
	if (m_abtn[BTN_INSERT_ZEN].UpdateMouseEvent())
	{
		SEASON3B::CreateMessageBox(
			MSGBOX_LAYOUT_CLASS(SEASON3B::CZenReceiptMsgBoxLayout));
		return true;
	}
	else if (m_abtn[BTN_TAKE_ZEN].UpdateMouseEvent())
	{
		SEASON3B::CreateMessageBox(
			MSGBOX_LAYOUT_CLASS(SEASON3B::CZenPaymentMsgBoxLayout));
		return true;
	}
	else if (m_abtn[BTN_LOCK].UpdateMouseEvent())
	{
		if (m_bLock)
#ifdef LDK_MOD_GLOBAL_STORAGELOCK_CHANGE
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CStorageUnlockMsgBoxLayout));
		}
#else //LDK_MOD_GLOBAL_STORAGELOCK_CHANGE
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CStorageUnlockKeyPadMsgBoxLayout));
		}
#endif //LDK_MOD_GLOBAL_STORAGELOCK_CHANGE
		else
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CStorageLockKeyPadMsgBoxLayout));
		}
		return true;
	}
	else if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x+169, m_Pos.y+7, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_STORAGE);
		return true;
	}

	return false;
}

//*****************************************************************************
// 함수 이름 : SetItemAutoMove()
// 함수 설명 : 아이템 자동 이동 세팅.
//*****************************************************************************
void CNewUIStorageInventory::SetItemAutoMove(bool bItemAutoMove)
{
	m_bItemAutoMove = bItemAutoMove;

	if (bItemAutoMove)
	{
		m_nBackupMouseX = MouseX;
		m_nBackupMouseY = MouseY;
	}
	else
		m_nBackupMouseX = m_nBackupMouseY = 0;
}

//*****************************************************************************
// 함수 이름 : InitBackupItemInfo()
// 함수 설명 : 백업 아이템 정보 초기화.
//*****************************************************************************
void CNewUIStorageInventory::InitBackupItemInfo()
{
	m_bTakeZen = false;
	m_nBackupTakeZen = 0;
	m_nBackupInvenIndex = -1;
}

//*****************************************************************************
// 함수 이름 : SetBackupTakeZen()
// 함수 설명 : 출금될 젠 백업.
// 매개 변수 : nZen	: 출금될 젠 액수.
//*****************************************************************************
void CNewUIStorageInventory::SetBackupTakeZen(int nZen)
{
	m_bTakeZen = true;
	m_nBackupTakeZen = nZen;
}

//*****************************************************************************
// 함수 이름 : SetBackupInvenIndex()
// 함수 설명 : 인벤토리창 아이템 위치 백업.
// 매개 변수 : nInvenIndex	: 인벤토리창 아이템 위치.
//*****************************************************************************
void CNewUIStorageInventory::SetBackupInvenIndex(int nInvenIndex)
{
	m_bTakeZen = false;
	m_nBackupInvenIndex = nInvenIndex;
}

//*****************************************************************************
// 함수 이름 : LockStorage()
// 함수 설명 : pItemObj 아이템이 창고에 들어 갈 수 있는 위치 찾음.
// 반환 값	 : 창고에 들어 갈 수 있는 위치.
//			아이템을 넣을 슬롯이 없으면 -1.
// 매개 변수 : pItemObj	: 체크할 아이템 오브젝트.
//*****************************************************************************
int CNewUIStorageInventory::FindEmptySlot(ITEM* pItemObj)
{
	if (pItemObj == NULL)
		return -1;

	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItemObj->Type];
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindEmptySlot(
			pItemAttr->Width, pItemAttr->Height);

	return -1;
}

//*****************************************************************************
// 함수 이름 : ProcessToReceiveStorageStatus()
// 함수 설명 : 서버에서 창고 상태 받았을 때 처리.
//			ReceiveStorageStatus() 함수 실행 시 호출.
// 매개 변수 : byStatus	: 창고 상태.
//*****************************************************************************
void CNewUIStorageInventory::ProcessToReceiveStorageStatus(BYTE byStatus)
{
	switch (byStatus)
	{
	case 0:	// 해제
		LockStorage(false);
		SetCorrectPassword(false);
		break;

	case 1: // 잠금
		LockStorage(true);
		SetCorrectPassword(false);
		break;

	case 10: // 비번 불일치
		SEASON3B::CreateOkMessageBox(GlobalText[440]);
		SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
		ProcessStorageItemAutoMoveFailure();
		break;

	case 11: // 창고가 잠겨져 있는데 또 잠그려 함
		SEASON3B::CreateOkMessageBox(GlobalText[441]);
		break;

	case 12: // 잠금 상태지만 비번을 맞게 입력했다.
		if (IsStorageLocked() && !IsCorrectPassword())
		{
			if (m_bTakeZen)	// 출금인가?
			{
				SendRequestStorageGold(1, GetBackupTakeZen());
				InitBackupItemInfo();
			}
			else
			{
				ITEM* pItemObj;
				int nStorageIndex;

				if (IsItemAutoMove())
				{
					pItemObj = m_pNewInventoryCtrl->FindItemAtPt(
						m_nBackupMouseX, m_nBackupMouseY);
					nStorageIndex
						= pItemObj->y * m_pNewInventoryCtrl->GetNumberOfColumn()
							+ pItemObj->x;
				}
				else
				{
					nStorageIndex = g_pPickedItem->GetSourceLinealPos();
					pItemObj = g_pPickedItem->GetItem();
				}

				SendRequestEquipmentItem(
					REQUEST_EQUIPMENT_STORAGE, nStorageIndex,
					pItemObj, REQUEST_EQUIPMENT_INVENTORY, GetBackupInvenIndex());

				InitBackupItemInfo();
			}
		}
		LockStorage(true);
		SetCorrectPassword(true);
		break;

	case 13: // 잠글때 주민번호 잘못됨
		SEASON3B::CreateOkMessageBox(GlobalText[401]);
		break;
	}
}

//*****************************************************************************
// 함수 이름 : ProcessToReceiveStorageItems()
// 함수 설명 : 서버에서 창고 아이템을 넣으라는 지시 받았을 때 처리.
// 매개 변수 : nIndex		: 창고에 넣을 아이템 위치 정보.
//			   pbyItemPacket: 서버에서 받는 아이템 정보.
//*****************************************************************************
void CNewUIStorageInventory::ProcessToReceiveStorageItems(int nIndex, BYTE* pbyItemPacket)
{
	SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();

	if (nIndex >= 0 && nIndex < (m_pNewInventoryCtrl->GetNumberOfColumn()
		* m_pNewInventoryCtrl->GetNumberOfRow()))
	{
		if (IsItemAutoMove())
		{
			CNewUIInventoryCtrl* pMyInvenCtrl = g_pMyInventory->GetInventoryCtrl();
			ITEM* pItemObj = pMyInvenCtrl->FindItemAtPt(m_nBackupMouseX, m_nBackupMouseY);
			g_pMyInventory->GetInventoryCtrl()->RemoveItem(pItemObj);

			SetItemAutoMove(false);
		}

		InsertItem(nIndex, pbyItemPacket);
	}
}

//*****************************************************************************
// 함수 이름 : ProcessStorageItemAutoMoveSuccess()
// 함수 설명 : 창고에서 우클릭으로 인벤토리로 아이템 자동 이동을 시켰을 때 서버
//			  에서 성공했다고 알려줄 때 처리.
//*****************************************************************************
void CNewUIStorageInventory::ProcessStorageItemAutoMoveSuccess()
{
	if (!IsVisible())
		return;

	if (IsItemAutoMove())
	{
		ITEM* pItemObj = m_pNewInventoryCtrl->FindItemAtPt(m_nBackupMouseX, m_nBackupMouseY);
		m_pNewInventoryCtrl->RemoveItem(pItemObj);

		SetItemAutoMove(false);
	}
}

//*****************************************************************************
// 함수 이름 : ProcessStorageItemAutoMoveFailure()
// 함수 설명 : 창고에서 우클릭으로 인벤토리로 아이템 자동 이동을 시켰을 때 서버
//			  에서 실패했다고 알려줄 때 처리.
//*****************************************************************************
void CNewUIStorageInventory::ProcessStorageItemAutoMoveFailure()
{
	if (!IsVisible())
		return;

	InitBackupItemInfo();

	// 서버에서 0.3초이내 아이템 이동 패킷을 또 받으면 실패 처리.
	SetItemAutoMove(false);
}

int SEASON3B::CNewUIStorageInventory::GetPointedItemIndex()
{
	return m_pNewInventoryCtrl->GetPointedSquareIndex();
}
