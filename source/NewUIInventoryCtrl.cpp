// NewUIInventoryCtrl.cpp: implementation of the CNewUIInventoryCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIItemMng.h"
#include "NewUISystem.h"
#include "ZzzInventory.h"
#include "UIControls.h"
#include "CComGem.h"
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
#include "GIPetManager.h"
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
#ifdef KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY
#include "CSItemOption.h"
#include "SocketSystem.h"
#endif // KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY
#ifdef YDG_FIX_DARKSPIRIT_CHAOSCASTLE_CRASH
#include "CSChaosCastle.h"
#endif	// YDG_FIX_DARKSPIRIT_CHAOSCASTLE_CRASH

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIPickedItem::CNewUIPickedItem()
{
	m_pNewItemMng = NULL;
	m_pSrcInventory = NULL;
	m_pPickedItem = NULL;
	m_bShow = true;
	m_Pos.x = m_Pos.y = 0;
	m_Size.cx = m_Size.cy = 0;
}

SEASON3B::CNewUIPickedItem::~CNewUIPickedItem() 
{ 
	Release(); 
}

bool SEASON3B::CNewUIPickedItem::Create(CNewUIItemMng* pNewItemMng, CNewUIInventoryCtrl* pSrc, ITEM* pItem)
{
	if(g_pNewUI3DRenderMng == NULL || pNewItemMng == NULL || pItem == NULL)
		return false;
	
	m_pNewItemMng = pNewItemMng;
	m_pSrcInventory = pSrc;
	if( NULL == (m_pPickedItem = m_pNewItemMng->DuplicateItem(pItem)) )
	{
		return false;
	}

	g_pNewUI3DRenderMng->Add3DRenderObj(this, INFORMATION_CAMERA_Z_ORDER);
	
	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_pPickedItem->Type];
	m_Size.cx = pItemAttr->Width*INVENTORY_SQUARE_WIDTH;
	m_Size.cy = pItemAttr->Height*INVENTORY_SQUARE_HEIGHT;
	m_Pos.x = MouseX - m_Size.cx/2;
	m_Pos.y = MouseY - m_Size.cy/2;

	return true;
}

void SEASON3B::CNewUIPickedItem::Release()
{
	g_pNewUI3DRenderMng->Remove3DRenderObj(this);

#ifdef YDG_FIX_MEMORY_LEAK_0905_2ND
	m_pNewItemMng->DeleteDuplicatedItem(m_pPickedItem);
#else	// YDG_FIX_MEMORY_LEAK_0905_2ND
	m_pNewItemMng->DeleteItem(m_pPickedItem);
#endif	// YDG_FIX_MEMORY_LEAK_0905_2ND
	m_pPickedItem = NULL;
	
	m_pNewItemMng = NULL;
	m_pSrcInventory = NULL;
	m_bShow = true;
}

CNewUIInventoryCtrl* SEASON3B::CNewUIPickedItem::GetOwnerInventory() const 
{ 
	return m_pSrcInventory; 
}

ITEM* SEASON3B::CNewUIPickedItem::GetItem() const 
{ 
	return m_pPickedItem; 
}

const POINT& SEASON3B::CNewUIPickedItem::GetPos() const 
{ 
	return m_Pos; 
}

const SIZE& SEASON3B::CNewUIPickedItem::GetSize() const 
{ 
	return m_Size; 
}

void SEASON3B::CNewUIPickedItem::GetRect(RECT& rcBox)
{
	rcBox.left = m_Pos.x;
	rcBox.top = m_Pos.y;
	rcBox.right = rcBox.left + m_Size.cx;
	rcBox.bottom = rcBox.top + m_Size.cy;
}

int SEASON3B::CNewUIPickedItem::GetSourceLinealPos()
{
	if(m_pSrcInventory)
	{
		return m_pPickedItem->y*m_pSrcInventory->GetNumberOfColumn()+m_pPickedItem->x;
	}
	/* CNewUIInventoryCtrl를 사용하지 않는 예외 */
	/* ITEM::ex_src_type 값으로 구분한다. (ITEM구조체 참조) */
	else if(m_pPickedItem && m_pPickedItem->ex_src_type > 0)	// m_pPickedItem->ex_src_type==ITEM::y
	{
		return m_pPickedItem->lineal_pos; // m_pPickedItem->lineal_pos==ITEM::x
	}
	return -1;
}

bool SEASON3B::CNewUIPickedItem::GetTargetPos(CNewUIInventoryCtrl* pDest, int& iTargetColumnX, int& iTargetRowY)
{
	if(pDest != NULL)
	{
		RECT rcInventory;
		pDest->GetRect(rcInventory);
		
		ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_pPickedItem->Type];
		int iPickedItemX = MouseX - ((pItemAttr->Width - 1) * INVENTORY_SQUARE_WIDTH / 2);
		int iPickedItemY = MouseY - ((pItemAttr->Height - 1) * INVENTORY_SQUARE_HEIGHT / 2);
		
		return pDest->GetSquarePosAtPt(iPickedItemX, iPickedItemY, iTargetColumnX, iTargetRowY);
	}
	return false;
}

int SEASON3B::CNewUIPickedItem::GetTargetLinealPos(CNewUIInventoryCtrl* pDest)
{
	int iTargetColumnX, iTargetRowY;
	if(GetTargetPos(pDest, iTargetColumnX, iTargetRowY))
	{
		return iTargetRowY*pDest->GetNumberOfColumn() + iTargetColumnX;
	}
	return -1;
}

bool SEASON3B::CNewUIPickedItem::IsVisible() const 
{ 
	return m_bShow; 
}

void SEASON3B::CNewUIPickedItem::ShowPickedItem() 
{ 
	m_bShow = true; 
}

void SEASON3B::CNewUIPickedItem::HidePickedItem() 
{ 
	m_bShow = false; 
}

void SEASON3B::CNewUIPickedItem::Render3D() 
{	
#ifdef NEW_USER_INTERFACE
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PARTCHARGE_SHOP) == true)
	{
		return;
	}
#endif // NEW_USER_INTERFACE

	if(m_pPickedItem && m_pPickedItem->Type >= 0)
	{
		m_Pos.x = MouseX - m_Size.cx/2;
		m_Pos.y = MouseY - m_Size.cy/2;
		RenderItem3D(m_Pos.x, m_Pos.y, m_Size.cx, m_Size.cy, m_pPickedItem->Type, m_pPickedItem->Level, 
			m_pPickedItem->Option1, m_pPickedItem->ExtOption, true);	
	}
}

//////////////////////////////////////////////////////////////////////////
CNewUIPickedItem* SEASON3B::CNewUIInventoryCtrl::ms_pPickedItem = NULL;

SEASON3B::CNewUIInventoryCtrl::CNewUIInventoryCtrl() 
{ 
	Init(); 
}

SEASON3B::CNewUIInventoryCtrl::~CNewUIInventoryCtrl() 
{ 
	Release(); 
}

void SEASON3B::CNewUIInventoryCtrl::Init()
{
	m_pNew3DRenderMng = NULL;
	m_pNewItemMng = NULL;
	m_pOwner = NULL;
	
	m_Pos.x = m_Pos.y = 0;
	m_Size.cx = m_Size.cy = 0;
	m_nColumn = m_nRow = 0;
	m_pdwItemCheckBox = NULL;
	m_EventState = EVENT_NONE;
	m_iPointedSquareIndex = -1;
	m_bShow = true;
	m_bLock = false;

	m_ToolTipType = TOOLTIP_TYPE_INVENTORY; //. Default
	m_pToolTipItem = NULL;

	m_bRepairMode = false;
	m_bCanPushItem = true;

	Vector(0.1f, 0.4f, 0.8f, m_afColorStateNormal);
	Vector(1.f, 0.2f, 0.2f, m_afColorStateWarning);
}

void SEASON3B::CNewUIInventoryCtrl::LoadImages()
{
	LoadBitmap("Interface\\newui_item_box.tga", IMAGE_ITEM_SQUARE);
	
	LoadBitmap("Interface\\newui_item_table01(L).tga", IMAGE_ITEM_TABLE_TOP_LEFT);
	LoadBitmap("Interface\\newui_item_table01(R).tga", IMAGE_ITEM_TABLE_TOP_RIGHT);
	LoadBitmap("Interface\\newui_item_table02(L).tga", IMAGE_ITEM_TABLE_BOTTOM_LEFT);
	LoadBitmap("Interface\\newui_item_table02(R).tga", IMAGE_ITEM_TABLE_BOTTOM_RIGHT);
	LoadBitmap("Interface\\newui_item_table03(Up).tga", IMAGE_ITEM_TABLE_TOP_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(Dw).tga", IMAGE_ITEM_TABLE_BOTTOM_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(L).tga", IMAGE_ITEM_TABLE_LEFT_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(R).tga", IMAGE_ITEM_TABLE_RIGHT_PIXEL);
	
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
	LoadBitmap("Interface\\newui_inven_usebox_01.tga", IMAGE_ITEM_SQUARE_FOR_1_BY_1);
	LoadBitmap("Interface\\newui_inven_usebox_02.tga", IMAGE_ITEM_SQUARE_TOP_RECT);
	LoadBitmap("Interface\\newui_inven_usebox_03.tga", IMAGE_ITEM_SQUARE_BOTTOM_RECT);
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
}

void SEASON3B::CNewUIInventoryCtrl::UnloadImages()
{
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
	DeleteBitmap(IMAGE_ITEM_SQUARE_BOTTOM_RECT);
	DeleteBitmap(IMAGE_ITEM_SQUARE_TOP_RECT);
	DeleteBitmap(IMAGE_ITEM_SQUARE_FOR_1_BY_1);
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

	DeleteBitmap(IMAGE_ITEM_TABLE_RIGHT_PIXEL);
	DeleteBitmap(IMAGE_ITEM_TABLE_LEFT_PIXEL);
	DeleteBitmap(IMAGE_ITEM_TABLE_BOTTOM_PIXEL);
	DeleteBitmap(IMAGE_ITEM_TABLE_TOP_PIXEL);
	DeleteBitmap(IMAGE_ITEM_TABLE_BOTTOM_RIGHT);
	DeleteBitmap(IMAGE_ITEM_TABLE_BOTTOM_LEFT);
	DeleteBitmap(IMAGE_ITEM_TABLE_TOP_RIGHT);
	DeleteBitmap(IMAGE_ITEM_TABLE_TOP_LEFT);
	DeleteBitmap(IMAGE_ITEM_SQUARE);
}

void SEASON3B::CNewUIInventoryCtrl::SetItemColorState(ITEM* pItem)
{
	if(pItem == NULL)
	{
		return;
	}

	if(pItem->byColorState == ITEM_COLOR_TRADE_WARNING)
	{
		return;
	}

	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
	int iLevel = (pItem->Level>>3)&15;
	int iMaxDurability = calcMaxDurability(pItem, pItemAttr, iLevel);	

	if(pItem->Durability <= 0)
	{
		pItem->byColorState = ITEM_COLOR_DURABILITY_100;
	}
	else if(pItem->Durability <= (iMaxDurability*0.2f))
	{
		pItem->byColorState = ITEM_COLOR_DURABILITY_80;	
	}
	else if(pItem->Durability <= (iMaxDurability*0.3f))
	{
		pItem->byColorState = ITEM_COLOR_DURABILITY_70;
	}
	else if(pItem->Durability <= (iMaxDurability*0.5f))
	{
		pItem->byColorState = ITEM_COLOR_DURABILITY_50;
	}
	else
	{
		pItem->byColorState = ITEM_COLOR_NORMAL;	
	}
}


//인벤토리 안에서 내구력에 따라서 바탕색깔 바뀌는함수.
bool SEASON3B::CNewUIInventoryCtrl::CanChangeItemColorState(ITEM* pItem)
{
	if(pItem == NULL)
	{
		return false;
	}

	if(pItem->Type < ITEM_WING)
	{
		return true;
	}

	// 화살/석궁화살
	if( pItem->Type == ITEM_BOW+7 || pItem->Type == ITEM_BOW+15 )
	{
		return false;
	}

	// 용사/전사의 반지 제외
	if( pItem->Type == ITEM_HELPER+20 && ((pItem->Level>>3)&15) == 1 || ((pItem->Level>>3)&15) == 2 )
	{
		return false;
	}
	
	// 반지/목걸이
	if( pItem->Type >= ITEM_HELPER+8 && pItem->Type <= ITEM_HELPER+9 
		|| pItem->Type == ITEM_HELPER+10 
		|| pItem->Type >= ITEM_HELPER+12 && pItem->Type <= ITEM_HELPER+13
		|| pItem->Type == ITEM_HELPER+20
		|| pItem->Type >= ITEM_HELPER+21 && pItem->Type <= ITEM_HELPER+28
		|| pItem->Type >= ITEM_HELPER+38 && pItem->Type <= ITEM_HELPER+42 
#ifdef PJH_ADD_PANDA_CHANGERING
		|| pItem->Type == ITEM_HELPER+76
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
#endif	// YDG_ADD_SKELETON_CHANGE_RING
#ifdef PBG_FIX_CHANGEITEMCOLORSTATE
		|| pItem->Type == ITEM_HELPER+80	// 팬더펫
		|| pItem->Type == ITEM_HELPER+64	// 데몬
		|| pItem->Type == ITEM_HELPER+65	// 수호정령
#endif //PBG_FIX_CHANGEITEMCOLORSTATE
#ifdef YDG_ADD_SKELETON_PET
		|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif	// YDG_ADD_SKELETON_PET
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING 
		|| pItem->Type == ITEM_HELPER+107	// 치명적인 마법사의반지
#endif //YDG_ADD_CS7_CRITICAL_MAGIC_RING
#ifdef LDS_FIX_VISUALRENDER_PERIODITEM_EXPIRED_RE
		|| pItem->Type == ITEM_HELPER+109	// 인게임샾 아이템 // 신규 사파이어(푸른색)링	// MODEL_HELPER+109
		|| pItem->Type == ITEM_HELPER+110	// 인게임샾 아이템 // 신규 루비(붉은색)링		// MODEL_HELPER+110
		|| pItem->Type == ITEM_HELPER+111	// 인게임샾 아이템 // 신규 토파즈(주황)링		// MODEL_HELPER+111
		|| pItem->Type == ITEM_HELPER+112	// 인게임샾 아이템 // 신규 자수정(보라색)링	// MODEL_HELPER+112
		|| pItem->Type == ITEM_HELPER+113	// 인게임샾 아이템 // 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
		|| pItem->Type == ITEM_HELPER+114	// 인게임샾 아이템 // 신규 에메랄드(푸른) 목걸이// MODEL_HELPER+114
		|| pItem->Type == ITEM_HELPER+115 	// 인게임샾 아이템 // 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115
#endif // LDS_FIX_VISUALRENDER_PERIODITEM_EXPIRED_RE
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		)
	{
		return true;
	}
	
	// 아이템/펫
	if( pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_HELPER+5 
		|| pItem->Type == ITEM_HELPER+37
#ifdef LDK_ADD_CS7_UNICORN_PET
		|| pItem->Type == ITEM_HELPER+106	// 유니콘 펫
#endif //LDK_ADD_CS7_UNICORN_PET
	    )
	{
		return true;
	}
	
	// 날개아이템인가?
	if(IsWingItem(pItem) == true)
	{
		return true;
	}
	
	return false;
}

bool SEASON3B::CNewUIInventoryCtrl::Create(CNewUI3DRenderMng* pNew3DRenderMng, CNewUIItemMng* pNewItemMng, 
								 CNewUIObj* pOwner, int x, int y, int nColumn, int nRow)
{
	if(m_pdwItemCheckBox || false == m_vecItem.empty())
		return false;
	if(pNew3DRenderMng == NULL || pNewItemMng == NULL)
		return false;

	m_pNew3DRenderMng = pNew3DRenderMng;
	m_pNew3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);
	
	m_pNewItemMng = pNewItemMng;
	m_pOwner = pOwner;
	m_Pos.x = x;
	m_Pos.y = y;
	m_Size.cx = nColumn*INVENTORY_SQUARE_WIDTH;
	m_Size.cy = nRow*INVENTORY_SQUARE_HEIGHT;
	m_nColumn = nColumn;
	m_nRow = nRow;
	m_pdwItemCheckBox = new DWORD[nColumn*nRow];
	memset(m_pdwItemCheckBox, 0, sizeof(DWORD)*m_nColumn*m_nRow);

	LoadImages();

	return true;
}
void SEASON3B::CNewUIInventoryCtrl::Release()
{
	if(m_pNew3DRenderMng)
		m_pNew3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);

	RemoveAllItems();
	UnloadImages();

	SAFE_DELETE(m_pdwItemCheckBox);

	if(m_pNew3DRenderMng)
		m_pNew3DRenderMng->Remove3DRenderObj(this);

	Init();
}

bool SEASON3B::CNewUIInventoryCtrl::AddItem(int iLinealPos, BYTE* pbyItemPacket)
{
	if(iLinealPos < 0 || iLinealPos >= m_nColumn*m_nRow)
		return false;

	int iColumnX = iLinealPos % m_nColumn;
	int iRowY = iLinealPos / m_nColumn;

	return AddItem(iColumnX, iRowY, pbyItemPacket);
}

bool SEASON3B::CNewUIInventoryCtrl::AddItem(int iColumnX, int iRowY, BYTE* pbyItemPacket)
{
	if(iColumnX < 0 || iRowY < 0 || 
		iColumnX >= m_nColumn || iRowY >= m_nRow)
	{
		return false;
	}

	ITEM* pNewItem = m_pNewItemMng->CreateItem(pbyItemPacket);
	if(NULL == pNewItem)
		return false;

	if(!CanMove(iColumnX, iRowY, pNewItem))
	{
		//. 옮길 수 없다면
		m_pNewItemMng->DeleteItem(pNewItem);
		return false;
	}
	
	//. 장착 가능
	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pNewItem->Type];
	pNewItem->x = iColumnX;
	pNewItem->y = iRowY;
	
	for(int y=0; y<pItemAttr->Height; y++)
	{
		for(int x=0; x<pItemAttr->Width; x++)
		{
			int iCurIndex = (pNewItem->y+y)*m_nColumn+(pNewItem->x+x);
			m_pdwItemCheckBox[iCurIndex] = pNewItem->Key;	//. 체크박스에 아이템 추가
		}
	}
	m_vecItem.push_back(pNewItem);

	return true;
}

bool SEASON3B::CNewUIInventoryCtrl::AddItem(int iColumnX, int iRowY, ITEM* pItem)
{
	if(iColumnX < 0 || iRowY < 0 || 
		iColumnX >= m_nColumn || iRowY >= m_nRow) return false;

	ITEM* pNewItem = m_pNewItemMng->CreateItem(pItem);
	if(NULL == pNewItem)
		return false;
	
	if(!CanMove(iColumnX, iRowY, pNewItem))
	{
		//. 옮길 수 없다면
		m_pNewItemMng->DeleteItem(pNewItem);
		return false;
	}
	
	//. 장착 가능
	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pNewItem->Type];
	pNewItem->x = iColumnX;
	pNewItem->y = iRowY;
	
	for(int y=0; y<pItemAttr->Height; y++)
	{
		for(int x=0; x<pItemAttr->Width; x++)
		{
			int iCurIndex = (pNewItem->y+y)*m_nColumn+(pNewItem->x+x);
			m_pdwItemCheckBox[iCurIndex] = pNewItem->Key;	//. 체크박스에 아이템 추가
		}
	}
	m_vecItem.push_back(pNewItem);
	return true;
}

bool SEASON3B::CNewUIInventoryCtrl::AddItem(int iColumnX, int iRowY, BYTE byType, BYTE bySubType, BYTE byLevel /* = 0 */, 
								  BYTE byDurability /* = 255 */, BYTE byOption1 /* = 0 */, BYTE byOptionEx /* = 0 */, 
								  BYTE byOption380 /* = 0 */, BYTE byOptionHarmony /* = 0 */)
{
	if(iColumnX < 0 || iRowY < 0 || 
		iColumnX >= m_nColumn || iRowY >= m_nRow) return false;

	ITEM* pNewItem = m_pNewItemMng->CreateItem(byType, bySubType, byLevel, byDurability, 
		byOption1, byOptionEx, byOption380, byOptionHarmony);
	if(NULL == pNewItem)
		return false;

	if(!CanMove(iColumnX, iRowY, pNewItem))
	{
		//. 옮길 수 없다면
		m_pNewItemMng->DeleteItem(pNewItem);
		return false;
	}
	
	//. 장착 가능
	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pNewItem->Type];
	pNewItem->x = iColumnX;
	pNewItem->y = iRowY;
	
	for(int y=0; y<pItemAttr->Height; y++)
	{
		for(int x=0; x<pItemAttr->Width; x++)
		{
			int iCurIndex = (pNewItem->y+y)*m_nColumn+(pNewItem->x+x);
			m_pdwItemCheckBox[iCurIndex] = pNewItem->Key;	//. 체크박스에 아이템 추가
		}
	}
	m_vecItem.push_back(pNewItem);
	return true;
}

void SEASON3B::CNewUIInventoryCtrl::RemoveItem(ITEM* pItem)
{
	type_vec_item::iterator li = m_vecItem.begin();
	for(; li != m_vecItem.end(); li++)
	{
		if((*li) == pItem)
		{
			m_vecItem.erase(li);

			ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
			for(int y=0; y<pItemAttr->Height; y++)
			{
				for(int x=0; x<pItemAttr->Width; x++)
				{
					int iCurIndex = (pItem->y+y)*m_nColumn+(pItem->x+x);
					m_pdwItemCheckBox[iCurIndex] = 0;	//. 체크박스에서 아이템 제거
				}
			}
			m_pNewItemMng->DeleteItem(pItem);
			break;
		}
	}
}

void SEASON3B::CNewUIInventoryCtrl::RemoveAllItems()
{
	memset(m_pdwItemCheckBox, 0, sizeof(DWORD)*m_nColumn*m_nRow);

	type_vec_item::iterator li = m_vecItem.begin();
	for(; li != m_vecItem.end(); li++)
	{
		ITEM* pItem = (*li);
		m_pNewItemMng->DeleteItem(pItem);
	}

	m_vecItem.clear();
}

size_t SEASON3B::CNewUIInventoryCtrl::GetNumberOfItems()
{ 
	return m_vecItem.size(); 
}

ITEM* SEASON3B::CNewUIInventoryCtrl::GetItem(int iIndex)
{
#ifdef _VS2008PORTING
	if(iIndex < 0 || iIndex >= (int)m_vecItem.size())
#else // _VS2008PORTING
	if(iIndex < 0 || iIndex >= m_vecItem.size())
#endif // _VS2008PORTING
		return NULL;
	return m_vecItem[iIndex];
}

void SEASON3B::CNewUIInventoryCtrl::SetSquareColorNormal(float fRed, float fGreen, float fBlue)
{
	Vector(fRed, fGreen, fBlue, m_afColorStateNormal);
}

void SEASON3B::CNewUIInventoryCtrl::GetSquareColorNormal(float* pfParams) const
{
	Vector(m_afColorStateNormal[0], m_afColorStateNormal[1], m_afColorStateNormal[2], pfParams);
}

void SEASON3B::CNewUIInventoryCtrl::SetSquareColorWarning(float fRed, float fGreen, float fBlue)
{
	Vector(fRed, fGreen, fBlue, m_afColorStateWarning);
}

void SEASON3B::CNewUIInventoryCtrl::GetSquareColorWarning(float* pfParams) const
{
	Vector(m_afColorStateWarning[0], m_afColorStateWarning[1], m_afColorStateWarning[2], pfParams);
}

ITEM* SEASON3B::CNewUIInventoryCtrl::FindItem(int iLinealPos)
{
	if(iLinealPos >= 0 && iLinealPos < m_nColumn*m_nRow)
	{
		DWORD dwKey = m_pdwItemCheckBox[iLinealPos];
		return FindItemByKey(dwKey);
	}
	return NULL;
}

ITEM* SEASON3B::CNewUIInventoryCtrl::FindItem(int iColumnX, int iRowY)
{
	return FindItem(iRowY*m_nColumn+iColumnX);
}

ITEM* SEASON3B::CNewUIInventoryCtrl::FindItemByKey(DWORD dwKey)
{
	type_vec_item::iterator li = m_vecItem.begin();
	for(; li != m_vecItem.end(); li++)
		if((*li)->Key == dwKey)
			return (*li);
	return NULL;
}

ITEM* SEASON3B::CNewUIInventoryCtrl::FindTypeItem(short int siType)
{
	type_vec_item::iterator li = m_vecItem.begin();
	for(; li != m_vecItem.end(); li++)
		if((*li)->Type == siType)
			return (*li);
	return NULL;
}

bool SEASON3B::CNewUIInventoryCtrl::IsItem(short int siType)
{
	type_vec_item::iterator li = m_vecItem.begin();
	for(; li != m_vecItem.end(); li++)
		if((*li)->Type == siType)
			return true;
	return false;
}

#ifdef PSW_EVENT_LENA
int SEASON3B::CNewUIInventoryCtrl::GetItemCount(short int siType, int iLevel)
#else //PSW_EVENT_LENA
int SEASON3B::CNewUIInventoryCtrl::GetItemCount(short int siType)
#endif //PSW_EVENT_LENA
{
	int count = 0;
	type_vec_item::iterator li = m_vecItem.begin();
	for(; li != m_vecItem.end(); li++) {
		if((*li)->Type == siType) {
#ifdef PSW_EVENT_LENA
			if( iLevel == -1 || (((*li)->Level>>3)&15) == iLevel )
#endif //PSW_EVENT_LENA
			{
				count += ((*li)->Durability==0) ? 1 : (*li)->Durability;
			}
		}
	}
	return count;
}

int SEASON3B::CNewUIInventoryCtrl::FindItemIndex( short int siType, int iLevel )
{
	type_vec_item::iterator li = m_vecItem.begin();
	for(; li != m_vecItem.end(); li++)
	{
		if( (*li)->Type == siType )
		{
			// 아이템 타입이 같고 레벨이 상관없거나 해당되는 레벨이면 위치값 리턴
			if( iLevel == -1 || (((*li)->Level>>3)&15) == iLevel )
			{
				return (*li)->y*GetNumberOfColumn()+(*li)->x;
			}
		}	
	}

	return -1;
}

int SEASON3B::CNewUIInventoryCtrl::FindItemReverseIndex(short sType, int iLevel)
{
	for(int x=m_nColumn-1; x>=0; x--)
	{
		for(int y=m_nRow-1; y>=0; y--)
		{
			ITEM* pItem = FindItem(x, y);
			
			if(pItem)
			{
				if(pItem->Type == sType)
				{
					// 아이템 타입이 같고 레벨이 상관없거나 해당되는 레벨이면 위치값 리턴
					if( iLevel == -1 || ((pItem->Level>>3)&15) == iLevel )
					{
						return (pItem->y * GetNumberOfColumn()) + pItem->x;
					}	
				}
			}
		}
	}

	return -1;
}	

// 인벤토리안의 아이템의 위치값으로 기본 인덱스를 구한다. (ex.서버에 보낼 위치값을 위해)
int SEASON3B::CNewUIInventoryCtrl::FindBaseIndexByITEM(ITEM* pItem)
{
	return pItem->y*GetNumberOfColumn()+pItem->x;
}

ITEM* SEASON3B::CNewUIInventoryCtrl::FindItemPointedSquareIndex()
{
	if(m_iPointedSquareIndex != -1)
	{
		ITEM* pItem = NULL;
		pItem = FindItemByKey(m_pdwItemCheckBox[m_iPointedSquareIndex]);
		return pItem;
	}

	return NULL;
}

int SEASON3B::CNewUIInventoryCtrl::GetPointedSquareIndex()
{
	return m_iPointedSquareIndex;
}


ITEM* SEASON3B::CNewUIInventoryCtrl::FindItemAtPt(int x, int y)
{
	int iIndex = GetSquareIndexAtPt(x, y);
	return FindItem(iIndex);
}

int SEASON3B::CNewUIInventoryCtrl::FindItemptIndex(int x, int y)
{
	int iIndex = GetSquareIndexAtPt(x, y);

	ITEM* p = FindItem(iIndex);

	if( p )
	{
		return p->y*GetNumberOfColumn()+p->x;
	}

	return -1;
}

int SEASON3B::CNewUIInventoryCtrl::FindEmptySlot(IN int cx, IN int cy)
{
	for(int i=0; i<m_nColumn*m_nRow; i++)
	{
#ifdef PBG_WOPS_INVENCHECK
		//인벤토리체크시에 해당 아이템 크기 x크기의 넓이가 넘어가서 체크시에
		//다음줄로 넘어가서 체크하는것을 방지하기 위해.
		if(CheckSlot(i, cx, cy) && (i%m_nColumn < (m_nColumn - (cx-1))))
#else //PBG_WOPS_INVENCHECK
		if(CheckSlot(i, cx, cy))
#endif //PBG_WOPS_INVENCHECK
		{
			return i;
		}
	}

	return -1;
}
bool SEASON3B::CNewUIInventoryCtrl::FindEmptySlot(IN int cx, IN int cy, OUT int& iColumnX, OUT int& iColumnY)
{
	for(int y=0; y<m_nRow; y++)
	{
		for(int x=0; x<m_nColumn; x++)
		{
			if(CheckSlot(x, y, cx, cy))
			{
				iColumnX = x;
				iColumnY = y;
				return true;
			}
		}
	}
	return false;
}

int SEASON3B::CNewUIInventoryCtrl::GetNumItemByKey( DWORD dwItemKey )
{
	int iCntItem = 0;
	for(int y=0; y<m_nRow; y++)
	{
		for(int x=0; x<m_nColumn; x++)
		{
			ITEM* pItem = NULL;
			pItem = FindItem(x, y);
			if( pItem == NULL)
				return 0;

			if( pItem->Key == dwItemKey )
			{
				iCntItem++;
			}
		}
	}

	return iCntItem;
}

int SEASON3B::CNewUIInventoryCtrl::GetNumItemByType(short sItemType)
{
	int iCntItem = 0;
	for(int y=0; y<m_nRow; y++)
	{
		for(int x=0; x<m_nColumn; x++)
		{
			ITEM* pItem = NULL;
			pItem = FindItem(x, y);
			if(pItem)			
			{
				if(pItem->Type == sItemType)
				{
					iCntItem++;
				}
			}
		}
	}

	return iCntItem;
}	

int SEASON3B::CNewUIInventoryCtrl::GetEmptySlotCount()
{
	int iResult = 0;
	for(int y=0; y<m_nRow; y++)
	{
		for(int x=0; x<m_nColumn; x++)
		{
			int iIndex = y*m_nColumn+x;
			if(m_pdwItemCheckBox[iIndex] == 0)
			{
				++iResult;
			}
		}
	}
	return iResult;
}

bool SEASON3B::CNewUIInventoryCtrl::UpdateMouseEvent()
{
	if(m_EventState == EVENT_NONE 
		&& SEASON3B::IsNone(VK_LBUTTON) 
		&& m_iPointedSquareIndex != -1)
	{
		m_EventState = EVENT_HOVER;
	}
	else if(m_EventState == EVENT_HOVER 
		&& SEASON3B::IsRelease(VK_LBUTTON) 
		&& m_iPointedSquareIndex != -1 
		&& NULL == GetPickedItem() 
		&& false == IsLocked()	// Lock이 걸렸다면 Picking 금지
		&& m_bRepairMode == false	// 수리모드가 아니면 
		)
	{
		m_EventState = EVENT_PICKING;
		ITEM* pItem = FindItemByKey(m_pdwItemCheckBox[m_iPointedSquareIndex]);
		if(pItem)
		{
			if(CreatePickedItem(this, pItem))
			{
				RemoveItem(pItem);
				return false;
			}
		}
	}
	else if(m_EventState == EVENT_HOVER 
		&& SEASON3B::IsNone(VK_LBUTTON) 
		&& m_iPointedSquareIndex != -1 
		&& NULL == GetPickedItem()
		&& (m_pdwItemCheckBox[m_iPointedSquareIndex] > 1) && g_pNewUIMng)
	{
		ITEM* pItem = FindItemByKey(m_pdwItemCheckBox[m_iPointedSquareIndex]);
		if(pItem != m_pToolTipItem)
		{
			//. 툴팁 생성
			CreateItemToolTip(pItem);

#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
			// 펫정보 요청 (Mouse커서 On)
			if( (pItem->Type == ITEM_HELPER+4) || (pItem->Type == ITEM_HELPER+5) )
			{
				ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_pToolTipItem->Type];
				int iTargetX = m_Pos.x + m_pToolTipItem->x*INVENTORY_SQUARE_WIDTH + pItemAttr->Width*INVENTORY_SQUARE_WIDTH/2;
				int iTargetY = m_Pos.y + m_pToolTipItem->y*INVENTORY_SQUARE_HEIGHT;
				giPetManager::RequestPetInfo(iTargetX, iTargetY, pItem);
			}	
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
		}
	}
	/*
	else if(m_EventState == EVENT_PICKING
		&& SEASON3B::IsRelease(VK_LBUTTON)
		&& m_iPointedSquareIndex != -1 
		&& ms_pPickedItem )
	{
		//. 제자리에 놓기
		int iTargetColumnX, iTargetRowY;
		if(ms_pPickedItem->GetTargetPos(this, iTargetColumnX, iTargetRowY))
		{
			ITEM* pItemObj = ms_pPickedItem->GetItem();
			if(iTargetColumnX == pItemObj->x && iTargetRowY == pItemObj->y)
			{
				m_EventState = EVENT_NONE;
				BackupPickedItem();
				return false;
			}
		}
	}
	*/
	return true;
}

bool SEASON3B::CNewUIInventoryCtrl::Update()
{
	if(IsVisible())
	{
		UpdateProcess();
	}
	return true;
}

void SEASON3B::CNewUIInventoryCtrl::UpdateProcess()
{
	int iCurSquareIndex = GetSquareIndexAtPt(MouseX, MouseY);
	if(iCurSquareIndex != m_iPointedSquareIndex)
	{
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
		// 펫정보 초기화
		if( (GetPickedItem() == NULL) && (g_pMyShopInventory->IsEnableInputValueTextBox() == false) )
			giPetManager::InitItemBackup();
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
		m_iPointedSquareIndex = iCurSquareIndex;
	}
	
	if(m_iPointedSquareIndex == -1 
		|| (m_iPointedSquareIndex != -1 && m_pdwItemCheckBox[m_iPointedSquareIndex] == 0))
	{
		m_EventState = EVENT_NONE;
		//. 툴팁이 있다면 제거
		DeleteItemToolTip();
	}
}

void SEASON3B::CNewUIInventoryCtrl::Render()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_NEWUIINVENTORY_INVENTORYCTRLRENDER, PROFILING_RENDER_NEWUIINVENTORY_INVENTORYCTRLRENDER );
#endif // DO_PROFILING
	//. 배경 그리기
	int x, y;
	for(y=0; y<m_nRow; y++)
	{
		for(x=0; x<m_nColumn; x++)
		{
			int iCurSquareIndex = y*m_nColumn+x;
#ifdef DEBUG_INVENTORY_BASE_TEST
			EnableAlphaTest();
			if(m_iPointedSquareIndex == iCurSquareIndex)
			{
				glColor4f(0.3f, 0.9f, 0.4f, 0.9f);
			}
			else if(m_pdwItemCheckBox[iCurSquareIndex] > 1)		//. 아이템이 있을경우
			{
				glColor4f(0.3f, 0.5f, 0.5f, 0.6f);
			}
			else
			{
				glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
			}
			RenderColor(m_Pos.x+(x*INVENTORY_SQUARE_WIDTH), m_Pos.y+(y*INVENTORY_SQUARE_HEIGHT),
				INVENTORY_SQUARE_WIDTH, INVENTORY_SQUARE_HEIGHT);
			EndRenderColor();
#else // DEBUG_INVENTORY_BASE_TEST
			if(m_pdwItemCheckBox[iCurSquareIndex] > 1)			//. 아이템이 있을경우
			{
				EnableAlphaTest();

				ITEM* pItem = FindItemByKey(m_pdwItemCheckBox[iCurSquareIndex]);

				if(pItem)
				{
					if(CanChangeItemColorState(pItem) == true)
					{
						SetItemColorState(pItem);
					}
					
					if(pItem->byColorState == ITEM_COLOR_NORMAL)
					{
						glColor4f(0.3f, 0.5f, 0.5f, 0.6f);
					}
					else if(pItem->byColorState == ITEM_COLOR_DURABILITY_50)
					{
						glColor4f(1.0f, 1.0f, 0.f, 0.4f);
					}
					else if(pItem->byColorState == ITEM_COLOR_DURABILITY_70)
					{
						glColor4f(1.0f, 0.66f, 0.f, 0.4f);
					}
					else if(pItem->byColorState == ITEM_COLOR_DURABILITY_80)
					{
						glColor4f(1.0f, 0.33f, 0.f, 0.4f);
					}
					else if(pItem->byColorState == ITEM_COLOR_DURABILITY_100)
					{
						glColor4f(1.0f, 0.f, 0.f, 0.4f);
					}
					else if(pItem->byColorState == ITEM_COLOR_TRADE_WARNING)
					{
						glColor4f(1.0f, 0.2f, 0.1f, 0.4f);
					}
				}
				else
				{
					glColor4f(0.3f, 0.5f, 0.5f, 0.6f);
				}
				
				RenderColor(m_Pos.x+(x*INVENTORY_SQUARE_WIDTH), m_Pos.y+(y*INVENTORY_SQUARE_HEIGHT),
					INVENTORY_SQUARE_WIDTH, INVENTORY_SQUARE_HEIGHT);
				EndRenderColor();
			}
#endif // DEBUG_INVENTORY_BASE_TEST
#ifdef DEBUG_INVENTORY_BASE_TEST
			//. 테두리
			EnableAlphaTest();
			glColor4f(1.0f, 0.0f, 0.0f, 1.f);
			RenderColor(m_Pos.x+(x*INVENTORY_SQUARE_WIDTH), m_Pos.y+(y*INVENTORY_SQUARE_HEIGHT),
				INVENTORY_SQUARE_WIDTH, 1);
			RenderColor(m_Pos.x+(x*INVENTORY_SQUARE_WIDTH), m_Pos.y+(y*INVENTORY_SQUARE_HEIGHT),
				1, INVENTORY_SQUARE_HEIGHT);
			RenderColor(m_Pos.x+(x*INVENTORY_SQUARE_WIDTH), m_Pos.y+((y+1)*INVENTORY_SQUARE_HEIGHT),
				INVENTORY_SQUARE_WIDTH+1, 1);
			RenderColor(m_Pos.x+((x+1)*INVENTORY_SQUARE_WIDTH), m_Pos.y+(y*INVENTORY_SQUARE_HEIGHT),
				1, INVENTORY_SQUARE_HEIGHT+1);
			EndRenderColor();
#endif // DEBUG_INVENTORY_BASE_TEST

			EnableAlphaTest();
		// 인벤토리창의 보관하는 부분의 각각의 정사각형 칸 렌더 
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY 
			ITEM* pItem = NULL;
			int iItemHeight = 0;
			
			if(m_pdwItemCheckBox[iCurSquareIndex] > 1)
			{
				pItem = FindItemByKey(m_pdwItemCheckBox[iCurSquareIndex]);
				iItemHeight = ItemAttribute[pItem->Type].Height;
			}
			
			// 지금 칸에 아이템이 인벤 장착 시스템에 장착되어 있는 아이템이 존재 하지 않으면 일반 정사각형 칸 렌더 
			if (pItem == NULL || !(pItem->Durability == 254 && g_pMyInventory->IsInvenItem(pItem->Type)))
			{
				RenderImage(IMAGE_ITEM_SQUARE, m_Pos.x+(x*INVENTORY_SQUARE_WIDTH), 
					m_Pos.y+(y*INVENTORY_SQUARE_HEIGHT), 21, 21);
			}
			else 
			{
				// 1 by 1 짜리 인벤 장착 시스템 배경 정사각형 칸 렌더 
				if (iItemHeight == 1)
				{
					RenderImage(IMAGE_ITEM_SQUARE_FOR_1_BY_1,	m_Pos.x+(x*INVENTORY_SQUARE_WIDTH), 
						m_Pos.y+(y*INVENTORY_SQUARE_HEIGHT), 21, 21);
				}
				// 인벤토리 배경 중 인벤 시스템에서 장착 한 아이템의 아래쪽 사각형 부분 렌더
				else if (y == m_nRow-1 || pItem == FindItem(x,y-1))
				{
					RenderImage(IMAGE_ITEM_SQUARE_BOTTOM_RECT,	m_Pos.x+(x*INVENTORY_SQUARE_WIDTH), 
						m_Pos.y+(y*INVENTORY_SQUARE_HEIGHT), 21, 21);
				}
				// 인벤토리 배경 중 인벤 시스템에서 장착 한 아이템의 위쪽 사각형 부분 렌더
				else
				{
					RenderImage(IMAGE_ITEM_SQUARE_TOP_RECT,		m_Pos.x+(x*INVENTORY_SQUARE_WIDTH), 
						m_Pos.y+(y*INVENTORY_SQUARE_HEIGHT), 21, 21);
				}
			}
#else  //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
			RenderImage(IMAGE_ITEM_SQUARE, m_Pos.x+(x*INVENTORY_SQUARE_WIDTH), 
				m_Pos.y+(y*INVENTORY_SQUARE_HEIGHT), 21, 21);
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		}
	}

	//. 프레임 그리기
	EnableAlphaTest();
	RenderImage(IMAGE_ITEM_TABLE_TOP_LEFT, m_Pos.x-WND_LEFT_EDGE, m_Pos.y-WND_TOP_EDGE, 14, 14);
	RenderImage(IMAGE_ITEM_TABLE_TOP_RIGHT, m_Pos.x+m_Size.cx-WND_RIGHT_EDGE, m_Pos.y-WND_TOP_EDGE, 14, 14);
	RenderImage(IMAGE_ITEM_TABLE_BOTTOM_LEFT, m_Pos.x-WND_LEFT_EDGE, m_Pos.y+m_Size.cy-WND_BOTTOM_EDGE, 14, 14);
	RenderImage(IMAGE_ITEM_TABLE_BOTTOM_RIGHT, m_Pos.x+m_Size.cx-WND_RIGHT_EDGE, m_Pos.y+m_Size.cy-WND_BOTTOM_EDGE, 14, 14);
	
	for(x=m_Pos.x-WND_LEFT_EDGE+14; x<m_Pos.x+m_Size.cx-WND_RIGHT_EDGE; x++)
	{
		RenderImage(IMAGE_ITEM_TABLE_TOP_PIXEL, x, m_Pos.y-WND_TOP_EDGE, 1, 14);
		RenderImage(IMAGE_ITEM_TABLE_BOTTOM_PIXEL, x, m_Pos.y+m_Size.cy-WND_BOTTOM_EDGE, 1, 14);
	}
	for(y=m_Pos.y-WND_TOP_EDGE+14; y<m_Pos.y+m_Size.cy-WND_BOTTOM_EDGE; y++)
	{
		RenderImage(IMAGE_ITEM_TABLE_LEFT_PIXEL, m_Pos.x-WND_LEFT_EDGE, y, 14, 1);
		RenderImage(IMAGE_ITEM_TABLE_RIGHT_PIXEL, m_Pos.x+m_Size.cx-WND_RIGHT_EDGE, y, 14, 1);
	}

	//. PickedItem 과의 상태 그리기
	if( ms_pPickedItem )
	{
		bool pickitemvisible = ms_pPickedItem->IsVisible();
		
		if( pickitemvisible )
		{
			RECT rcPickedItem, rcInventory, rcIntersect;
			ms_pPickedItem->GetRect(rcPickedItem);
			GetRect(rcInventory);
			
			//. PickedItem 영역이 인벤 영역 안으로 들어왔을경우
			if(IntersectRect(&rcIntersect, &rcPickedItem, &rcInventory))
			{
				ITEM* pPickItem = ms_pPickedItem->GetItem();
				ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pPickItem->Type];
				int iPickedItemX = MouseX - ((pItemAttr->Width - 1) * INVENTORY_SQUARE_WIDTH / 2);
				int iPickedItemY = MouseY - ((pItemAttr->Height - 1) * INVENTORY_SQUARE_HEIGHT / 2);
				
				int iColumnX = 0, iRowY = 0;
				int nItemColumn = pItemAttr->Width, nItemRow = pItemAttr->Height;
				if(false == GetSquarePosAtPt(iPickedItemX, iPickedItemY, iColumnX, iRowY))
				{
					//. iColumnX, iRowY 보정 (negative value)
					iColumnX = ((iPickedItemX - rcInventory.left) / INVENTORY_SQUARE_WIDTH);
					
					if(iPickedItemX - rcInventory.left < 0)
						iColumnX = ((iPickedItemX - rcInventory.left) / INVENTORY_SQUARE_WIDTH) - 1;
					else
						iColumnX = (iPickedItemX - rcInventory.left) / INVENTORY_SQUARE_WIDTH;
					
					if(iPickedItemY - rcInventory.top < 0)
						iRowY = ((iPickedItemY - rcInventory.top) / INVENTORY_SQUARE_HEIGHT) - 1;
					else
						iRowY = (iPickedItemY - rcInventory.top) / INVENTORY_SQUARE_HEIGHT;
				}
				
				bool bWarning = false;
				//. Clipping
				if(iColumnX < 0 && iColumnX >= -nItemColumn)
				{
					nItemColumn = nItemColumn + iColumnX;
					iColumnX = 0;
					bWarning = true;
				}
				if(iColumnX+nItemColumn > m_nColumn && iColumnX < m_nColumn)
				{
					nItemColumn = m_nColumn - iColumnX;
					bWarning = true;
				}
				if(iRowY < 0 && iRowY >= -nItemRow)
				{
					nItemRow = nItemRow + iRowY;
					iRowY = 0;
					bWarning = true;
				}
				if(iRowY+nItemRow > m_nRow && iRowY < m_nRow)
				{
					nItemRow = m_nRow - iRowY;
					bWarning = true;
				}
				
				int iDestPosX = m_Pos.x + iColumnX * INVENTORY_SQUARE_WIDTH;
				int iDestPosY = m_Pos.y + iRowY * INVENTORY_SQUARE_HEIGHT;
				int iDestWidth = nItemColumn * INVENTORY_SQUARE_WIDTH;
				int iDestHeight = nItemRow * INVENTORY_SQUARE_HEIGHT;
				
				m_bCanPushItem = bWarning;
				
				//. Rendering Routine
				if(bWarning)
				{
					EnableAlphaTest();
					glColor4f(1.f, 0.2f, 0.2f, 0.4f);
					RenderColor(iDestPosX, iDestPosY, iDestWidth, iDestHeight);
					EndRenderColor();
				}
				else
				{
					if(iColumnX >= 0 && iColumnX < m_nColumn && iRowY >= 0 && iRowY < m_nRow)
					{
						EnableAlphaTest();
						for(int y=0; y<nItemRow; y++)
						{
							for(int x=0; x<nItemColumn; x++)
							{
								int iSquarePosX = iColumnX+x;
								int iSquarePosY = iRowY+y;
								int iCurSquareIndex = iSquarePosY*m_nColumn+iSquarePosX;
								if(m_pdwItemCheckBox[iCurSquareIndex] > 1)
								{
#ifdef KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY
									bool bSuccess = false;
#else // KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY
									bool bSuccess = true;
#endif // KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY
									
									ITEM* pTargetItem = FindItemByKey(m_pdwItemCheckBox[iCurSquareIndex]);
									if(pTargetItem)
									{
										int	iType = pTargetItem->Type;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
										int	iLevel = (pTargetItem->Level >> 3) & 15;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
										int	iDurability = pTargetItem->Durability;
										
										
#ifdef KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY
										// 축석/영석
										if((pPickItem->Type==ITEM_POTION+13)    
											|| (pPickItem->Type==ITEM_POTION+14)) 
										{	   
											bSuccess = CanUpgradeItem(pPickItem, pTargetItem);
										}
										else if(pPickItem->Type == ITEM_POTION+42 )		// 조화의 보석
										{
											// 강화되지 않은 아이템만 강화
											// 강화된 아이템은 환원을 한 다음 다시 강화...
											if(pTargetItem->Jewel_Of_Harmony_Option == 0  )
											{
												StrengthenItem strengthitem = g_pUIJewelHarmonyinfo->GetItemType( static_cast<int>(pTargetItem->Type) );
												
												// 소켓, 세트아이템은 조화의 보석으로 강화할수 없다.
												if( (strengthitem != SI_None) && (!g_SocketItemMgr.IsSocketItem(pTargetItem)) 
													&& ((pTargetItem->ExtOption % 0x04) != EXT_A_SET_OPTION && (pTargetItem->ExtOption % 0x04) != EXT_B_SET_OPTION))
												{
													// 검류, 도끼류, 둔기류, 창류, 활/석궁류, 지팡이, 방어구
													bSuccess = true;
												}
											}
										}
										else if(pPickItem->Type == ITEM_POTION+43		// 하급제련석
											|| pPickItem->Type == ITEM_POTION+44 )	// 상급제련석
										{
											// 조화의 보석으로 강화한 후 제련
											if( pTargetItem->Jewel_Of_Harmony_Option != 0 )
											{
												bSuccess = true;
											}
										}

										//^ 펜릴 수리 관련(메세지 박스)
										if(pPickItem->Type == ITEM_POTION+13 
											&& iType == ITEM_HELPER+37 && iDurability != 255)	// 펜릴의 뿔피리
										{
											bSuccess = true;
										}
										
										// 겹치기 가능한 아이템인가? 
										if(bSuccess == false && m_pOwner == g_pMyInventory)	// 자기인벤토리에서만 겹치기 가능함
										{
											bSuccess = IsOverlayItem(pPickItem, pTargetItem);
										}
#else // KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY
										// 날개가 아니면 false
										if(IsWingItem(pTargetItem) == false)
										{
											bSuccess = false;
										}
										// 축석에 레벨 6이상인 경우. ( 0, 1, 2, 3, 4, 5 ) -> ( 1, 2, 3, 4, 5, 6 )
										// 영석에 레벨 9이상인 경우  ( 6, 7, 8 ) -> ( 7, 8, 9 )
										if((pPickItem->Type==ITEM_POTION+13 && iLevel <= 6)    
											|| (pPickItem->Type==ITEM_POTION+14 && (iLevel >= 7 && iLevel <= 9))) 
										{	    
											bSuccess = true;
										}
										//^ 펜릴 수리 관련(메세지 박스)
										if(pPickItem->Type == ITEM_POTION+13 
											&& iType == ITEM_HELPER+37 && iDurability != 255)	// 펜릴의 뿔피리
										{
											bSuccess = true;
										}
										if(pPickItem->Type == ITEM_POTION+42 )		// 조화의 보석
										{
											if(pTargetItem->Jewel_Of_Harmony_Option != 0  )
											{
												//강화된 아이템은 강화를 못한다..
												// 환원을 한 다음 다시 강화...
												bSuccess = false;
											}
											else
											{
												StrengthenItem strengthitem = g_pUIJewelHarmonyinfo->GetItemType( static_cast<int>(pTargetItem->Type) );
												
												if( strengthitem == SI_None )
												{
													//검류, 도끼류, 둔기류, 창류, 활/석궁류, 지팡이, 방어구
													bSuccess = false;
												}
											}
										}
										if(pPickItem->Type == ITEM_POTION+43		// 하급제련석
											|| pPickItem->Type == ITEM_POTION+44 )	// 상급제련석
										{
											if( pTargetItem->Jewel_Of_Harmony_Option == 0 )
											{
												bSuccess = false;
											}
										}
										
										// 겹치기 가능한 아이템인가? 
										if(bSuccess == false && m_pOwner == g_pMyInventory)	// 자기인벤토리에서만 겹치기 가능함
										{
											bSuccess = IsOverlayItem(pPickItem, pTargetItem);
										}
#endif // KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY
#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 인벤토리 내에 아이템이 서로 겹칠 때의 예외처리 [lem_2010.9.8]
										if( Check_LuckyItem( pTargetItem->Type ) )
										{
											bSuccess	= false;
											if( pPickItem->Type == ITEM_POTION+161 )
											{
												if(pTargetItem->Jewel_Of_Harmony_Option == 0  )	bSuccess = true;
											}
											else if( pPickItem->Type == ITEM_POTION+160 )
											{
												if( pTargetItem->Durability > 0)				bSuccess = true;
											}
										}			
#endif // LEM_ADD_LUCKYITEM
									}
									
									if(bSuccess)
									{
										SetSquareColorWarning(0.2f, 0.4f, 0.2f);
									}
									else
									{
										SetSquareColorWarning(1.f, 0.2f, 0.2f);
									}
									
									glColor4f(m_afColorStateWarning[0], m_afColorStateWarning[1], m_afColorStateWarning[2], 0.4f);
								}
								else
								{
									glColor4f(m_afColorStateNormal[0], m_afColorStateNormal[1], m_afColorStateNormal[2], 0.4f);
								}
								RenderColor(m_Pos.x+(iSquarePosX*INVENTORY_SQUARE_WIDTH), m_Pos.y+(iSquarePosY*INVENTORY_SQUARE_HEIGHT), 
									INVENTORY_SQUARE_WIDTH, INVENTORY_SQUARE_HEIGHT);
							}
						}
						EndRenderColor();
					}
				}
			}
#if defined (DEBUG_INVENTORY_BASE_TEST)// || defined (_DEBUG)
		//. PickedItem 영역 출력
		EnableAlphaTest();
		glColor4f(0.8f, 0.4f, 0.8f, 0.9f);
		RenderColor(rcPickedItem.left, rcPickedItem.top,
			rcPickedItem.right-rcPickedItem.left, 1);
		RenderColor(rcPickedItem.left, rcPickedItem.bottom,
			rcPickedItem.right-rcPickedItem.left, 1);
		RenderColor(rcPickedItem.left, rcPickedItem.top,
			1, rcPickedItem.bottom-rcPickedItem.top);
		RenderColor(rcPickedItem.right, rcPickedItem.top,
			1, rcPickedItem.bottom-rcPickedItem.top);
		EndRenderColor();
#endif // defined (DEBUG_INVENTORY_BASE_TEST) || defined (_DEBUG)
		}
	}

#if defined (DEBUG_INVENTORY_BASE_TEST) || defined (_DEBUG)
	//. 선택 영역 Item Key 출력
	if(m_iPointedSquareIndex != -1 && m_pdwItemCheckBox[m_iPointedSquareIndex] != 0)
	{
		g_pRenderText->SetBgColor(0, 0, 0, 255);
		g_pRenderText->SetTextColor(200, 200, 200, 255);
		
		char szText[64] = {0, };
		sprintf(szText, "Item Key: 0x%00000008X", m_pdwItemCheckBox[m_iPointedSquareIndex]);
		g_pRenderText->RenderText(MouseX+20, MouseY+5, szText);
	}
#endif // defined (DEBUG_INVENTORY_BASE_TEST) || defined (_DEBUG)

	bool tooltipvisible = true;

#ifdef NEW_USER_INTERFACE_SHELL
	tooltipvisible = g_pNewUISystem->IsVisible( SEASON3B::INTERFACE_PARTCHARGE_SHOP ) ? false : true;
#endif //NEW_USER_INTERFACE_SHELL

	if(m_pNew3DRenderMng)
	{
#ifdef NEW_USER_INTERFACE_UISYSTEM
		// 부분유료화 상점에서는 갯수 렌더링과 툴팁렌더링을 따로 호출하고 있기 때문에 리턴
		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PARTCHARGE_SHOP))
		{
#ifdef DO_PROFILING
			g_pProfiler->EndUnit( EPROFILING_RENDER_NEWUIINVENTORY_INVENTORYCTRLRENDER );
#endif // DO_PROFILING
			return;
		}
#endif // NEW_USER_INTERFACE_UISYSTEM
		//. 아이템 갯수 랜더링
		m_pNew3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, RENDER_NUMBER_OF_ITEM, 0);
		if(m_pToolTipItem && GetPickedItem() == NULL)
		{	//. 툴팁 랜더링
			if( tooltipvisible )
			{
				m_pNew3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, RENDER_ITEM_TOOLTIP, 0);
			}
		}
	}
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_NEWUIINVENTORY_INVENTORYCTRLRENDER );
#endif // DO_PROFILING
}

void SEASON3B::CNewUIInventoryCtrl::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

const POINT& SEASON3B::CNewUIInventoryCtrl::GetPos() const 
{ 
	return m_Pos; 
}

int SEASON3B::CNewUIInventoryCtrl::GetNumberOfColumn() const 
{ 
	return m_nColumn; 
}

int SEASON3B::CNewUIInventoryCtrl::GetNumberOfRow() const 
{ 
	return m_nRow; 
}

void SEASON3B::CNewUIInventoryCtrl::GetRect(RECT& rcBox)
{
	rcBox.left = m_Pos.x;
	rcBox.top = m_Pos.y;
	rcBox.right = rcBox.left + m_Size.cx;
	rcBox.bottom = rcBox.top + m_Size.cy;
}

CNewUIInventoryCtrl::EVENT_STATE SEASON3B::CNewUIInventoryCtrl::GetEventState()
{ 
	return m_EventState; 
}

CNewUIObj* SEASON3B::CNewUIInventoryCtrl::GetOwner() const
{ 
	return m_pOwner; 
}

bool SEASON3B::CNewUIInventoryCtrl::IsVisible() const
{
	if(m_pOwner)
		return (m_pOwner->IsVisible() && m_bShow);	//. 부모가 있다면 부모의 상태를 따른다.
	return m_bShow;
}

void SEASON3B::CNewUIInventoryCtrl::ShowInventory()
{ 
	m_bShow = true; 
}

void SEASON3B::CNewUIInventoryCtrl::HideInventory()
{ 
	m_bShow = false; 
}

bool SEASON3B::CNewUIInventoryCtrl::IsLocked() const
{ 
	return m_bLock; 
}

void SEASON3B::CNewUIInventoryCtrl::LockInventory()
{ 
	m_bLock = true; 
}

void SEASON3B::CNewUIInventoryCtrl::UnlockInventory()
{ 
	m_bLock = false; 
}

int SEASON3B::CNewUIInventoryCtrl::GetSquareIndexAtPt(int x, int y)
{
	int iColumnX, iRowY;
	if(GetSquarePosAtPt(x, y, iColumnX, iRowY))
		return iRowY*m_nColumn+iColumnX;
	return -1;
}

bool SEASON3B::CNewUIInventoryCtrl::GetSquarePosAtPt(int x, int y, int& iColumnX, int& iRowY)
{
	RECT rcBox;
	GetRect(rcBox);

	if(x < rcBox.left || x >= rcBox.right || y < rcBox.top || y >= rcBox.bottom)
		return false;

	iColumnX = (x - rcBox.left) / INVENTORY_SQUARE_WIDTH;
	iRowY = (y - rcBox.top) / INVENTORY_SQUARE_HEIGHT;

	return true;
}

bool SEASON3B::CNewUIInventoryCtrl::CheckSlot(int iLinealPos, int cx, int cy)
{
	for(int y=0; y<cy; y++)
	{
		for(int x=0; x<cx; x++)
		{
			int iIndex = iLinealPos + (y * m_nColumn) + x;

			if(iIndex >= (m_nColumn * m_nRow))
			{
				return false;
			}
			else if(m_pdwItemCheckBox[iIndex] != 0)
			{
				return false;
			}
		}
	}

	return true;
}

bool SEASON3B::CNewUIInventoryCtrl::CheckSlot(int iColumnX, int iRowY, int cx, int cy)
{
	int iIndex = iRowY*m_nColumn+iColumnX;
	return CheckSlot(iIndex, cx, cy);
}

bool SEASON3B::CNewUIInventoryCtrl::CheckPtInRect(int x, int y)
{
	RECT rcSquare;
	GetRect(rcSquare);

	if(x < rcSquare.left || x >= rcSquare.right || y < rcSquare.top || y >= rcSquare.bottom)
		return false;
	return true;
}

bool SEASON3B::CNewUIInventoryCtrl::CheckRectInRect(const RECT& rcBox)
{
	RECT rcSquare;
	GetRect(rcSquare);

	if(rcBox.left >= rcSquare.left && rcBox.right <= rcSquare.right 
		&& rcBox.top >= rcSquare.top && rcBox.bottom <= rcSquare.bottom)
		return true;
	return false;
}

bool SEASON3B::CNewUIInventoryCtrl::CanMove(int iLinealPos, ITEM* pItem)
{
	if(iLinealPos < 0 || iLinealPos >= m_nColumn*m_nRow)
		return false;

	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
	return CheckSlot(iLinealPos, pItemAttr->Width, pItemAttr->Height);
}

bool SEASON3B::CNewUIInventoryCtrl::CanMove(int iColumnX, int iRowY, ITEM* pItem)
{
	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
	return CheckSlot(iColumnX, iRowY, pItemAttr->Width, pItemAttr->Height);
}

bool SEASON3B::CNewUIInventoryCtrl::CanMoveToPt(int x, int y, ITEM* pItem)
{
	int iColumnX, iRowY;
	if(GetSquarePosAtPt(x, y, iColumnX, iRowY))
		return CanMove(iColumnX, iRowY, pItem);
	return false;
}

void SEASON3B::CNewUIInventoryCtrl::SetToolTipType(TOOLTIP_TYPE ToolTipType)
{
	m_ToolTipType = ToolTipType;
}

void SEASON3B::CNewUIInventoryCtrl::CreateItemToolTip(ITEM* pItem)
{
	if(m_pToolTipItem)
		DeleteItemToolTip();

	if(g_pNewItemMng)
		m_pToolTipItem = g_pNewItemMng->CreateItem(pItem);		//. Reference Count 증가
}

void SEASON3B::CNewUIInventoryCtrl::DeleteItemToolTip()
{
	if(m_pToolTipItem && g_pNewItemMng)
	{
		g_pNewItemMng->DeleteItem(m_pToolTipItem);
		m_pToolTipItem = NULL;
	}
}

void SEASON3B::CNewUIInventoryCtrl::SetRepairMode(bool bRepair)
{
	m_bRepairMode = bRepair;

	if(m_bRepairMode == true)
	{
		SetToolTipType(TOOLTIP_TYPE_REPAIR);
	}
	else
	{
		SetToolTipType(TOOLTIP_TYPE_INVENTORY);
	}
}

bool SEASON3B::CNewUIInventoryCtrl::IsRepairMode()
{
	return m_bRepairMode;
}

void SEASON3B::CNewUIInventoryCtrl::RenderNumberOfItem()
{
	EnableAlphaTest();
	glColor3f(1.f, 1.f, 1.f);
	type_vec_item::iterator li = m_vecItem.begin();
	for(; li != m_vecItem.end(); li++)
	{
		ITEM* pItem = (*li);
		ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
		float x = m_Pos.x+(pItem->x*INVENTORY_SQUARE_WIDTH);
		float y = m_Pos.y+(pItem->y*INVENTORY_SQUARE_HEIGHT);
		float width = pItemAttr->Width*INVENTORY_SQUARE_WIDTH;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		float height = pItemAttr->Height*INVENTORY_SQUARE_HEIGHT;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

		if(pItem->Type>=ITEM_POTION && pItem->Type<=ITEM_POTION+8 && pItem->Durability > 1)
		{
			glColor3f(1.f,0.9f,0.7f);
			SEASON3B::RenderNumber(x+width-6, y+1, pItem->Durability);
		}
#ifdef HELLOWIN_EVENT
		else if(pItem->Type>=ITEM_POTION+46 && pItem->Type<=ITEM_POTION+50 && pItem->Durability > 1)
		{
			glColor3f(1.f,0.9f,0.7f);
			SEASON3B::RenderNumber(x+width-6, y+1, pItem->Durability);
		}
#endif // HELLOWIN_EVENT
		else if(pItem->Type>=ITEM_POTION+35 && pItem->Type<=ITEM_POTION+40 && pItem->Durability > 1)
		{
			glColor3f(1.f,0.9f,0.7f);
			SEASON3B::RenderNumber(x+width-6, y+1, pItem->Durability);
		}
#ifdef PSW_ELITE_ITEM						// 엘리트 물약
		else if(pItem->Type>=ITEM_POTION+70 && pItem->Type<=ITEM_POTION+71 && pItem->Durability > 1)
		{
			glColor3f(1.f,0.9f,0.7f);
			SEASON3B::RenderNumber(x+width-6, y+1, pItem->Durability);
		}
#endif //PSW_ELITE_ITEM						// 엘리트 물약
#ifdef PSW_NEW_ELITE_ITEM                   // 중간 엘리트 물약
		else if(pItem->Type==ITEM_POTION+94 && pItem->Durability > 1) 
		{
			glColor3f(1.f,0.9f,0.7f);
			SEASON3B::RenderNumber(x+width-6, y+1, pItem->Durability);
		}
#endif //PSW_NEW_ELITE_ITEM                 // 중간 엘리트 물약
#ifdef PSW_SECRET_ITEM						// 강화의 비약
		else if(pItem->Type>=ITEM_POTION+78 && pItem->Type<=ITEM_POTION+82 && pItem->Durability > 1)
		{
			glColor3f(1.f,0.9f,0.7f);
			SEASON3B::RenderNumber(x+width-6, y+1, pItem->Durability);
		}
#endif //PSW_SECRET_ITEM					// 강화의 비약
#ifdef CSK_EVENT_CHERRYBLOSSOM
		else if(pItem->Type>=ITEM_POTION+85 && pItem->Type<=ITEM_POTION+90 && pItem->Durability > 1)
		{
			glColor3f(1.f,0.9f,0.7f);
			SEASON3B::RenderNumber(x+width-6, y+1, pItem->Durability);
		}
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
		else if(pItem->Type==ITEM_POTION+133 && pItem->Durability > 1)	// 엘리트 SD회복물약
		{
			glColor3f(1.f,0.9f,0.7f);
			SEASON3B::RenderNumber(x+width-6, y+1, pItem->Durability);
		}
#endif	// YDG_ADD_CS7_ELITE_SD_POTION
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		else if(pItem->Type>=ITEM_POTION+153 && pItem->Type<=ITEM_POTION+156 && pItem->Durability > 1)	// 스타더스트 ~ 잿더미 도살자의 몽둥이
		{
			glColor3f(1.f,0.9f,0.7f);
			SEASON3B::RenderNumber(x+width-6, y+1, pItem->Durability);
		}
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
		else if(COMGEM::isCompiledGem(pItem)) 
		{
			int Level = (pItem->Level>>3)&15;
			glColor3f(1.f,0.9f,0.7f);
			SEASON3B::RenderNumber(x+width-6, y+1, (Level + 1) * COMGEM::FIRST);
		}
	}
	glColor3f(1.f, 1.f, 1.f);
	DisableAlphaBlend();
}

void SEASON3B::CNewUIInventoryCtrl::RenderItemToolTip()
{	
	if(m_pToolTipItem)
	{
		ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_pToolTipItem->Type];
		int iTargetX = m_Pos.x + m_pToolTipItem->x*INVENTORY_SQUARE_WIDTH + pItemAttr->Width*INVENTORY_SQUARE_WIDTH/2;
		int iTargetY = m_Pos.y + m_pToolTipItem->y*INVENTORY_SQUARE_HEIGHT;
#ifdef YDG_FIX_SMALL_ITEM_TOOLTIP_POSITION
		if (pItemAttr->Height == 1)
		{
			iTargetY += INVENTORY_SQUARE_HEIGHT/2;
		}
#endif	// YDG_FIX_SMALL_ITEM_TOOLTIP_POSITION
		
		if(m_ToolTipType == TOOLTIP_TYPE_INVENTORY)
		{
			RenderItemInfo(iTargetX, iTargetY, m_pToolTipItem, false);
		}
		else if(m_ToolTipType == TOOLTIP_TYPE_REPAIR)
		{
			RenderRepairInfo(iTargetX, iTargetY, m_pToolTipItem, false);
		}
		else if(m_ToolTipType == TOOLTIP_TYPE_NPC_SHOP)
		{
			RenderItemInfo(iTargetX, iTargetY, m_pToolTipItem, true);
		}
		else if( m_ToolTipType == TOOLTIP_TYPE_MY_SHOP )
		{
			RenderItemInfo(iTargetX, iTargetY, m_pToolTipItem, false, m_ToolTipType);
		}
		else if( m_ToolTipType == TOOLTIP_TYPE_PURCHASE_SHOP )
		{
			RenderItemInfo(iTargetX, iTargetY, m_pToolTipItem, false, m_ToolTipType);
		}
	}	
}

void SEASON3B::CNewUIInventoryCtrl::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	if(pClass)
	{
		CNewUIInventoryCtrl* pInventoryCtrl = (CNewUIInventoryCtrl*)pClass;
		if(dwParamA == RENDER_NUMBER_OF_ITEM)
			pInventoryCtrl->RenderNumberOfItem();
		else if(dwParamA == RENDER_ITEM_TOOLTIP)
			pInventoryCtrl->RenderItemToolTip();
	}
}

CNewUIPickedItem* SEASON3B::CNewUIInventoryCtrl::GetPickedItem()
{ 
	return ms_pPickedItem; 
}

bool SEASON3B::CNewUIInventoryCtrl::CreatePickedItem(CNewUIInventoryCtrl* pSrc, ITEM* pItem)
{
	if(g_pNewItemMng)
	{
		ms_pPickedItem = new CNewUIPickedItem;
		return ms_pPickedItem->Create(g_pNewItemMng, pSrc, pItem);
	}
	return false;
}

void SEASON3B::CNewUIInventoryCtrl::DeletePickedItem()
{
	if(ms_pPickedItem)
	{
		CNewUIInventoryCtrl* pOwner = ms_pPickedItem->GetOwnerInventory();
		if(pOwner)
		{
#ifdef _VS2008PORTING
			pOwner->SetEventState(CNewUIInventoryCtrl::EVENT_NONE);
#else // _VS2008PORTING
			pOwner->SetEventState(CNewUIInventoryCtrl::EVENT_STATE::EVENT_NONE);
#endif // _VS2008PORTING
		}
	}

	SAFE_DELETE(ms_pPickedItem);
}

void SEASON3B::CNewUIInventoryCtrl::BackupPickedItem()
{
	if(ms_pPickedItem && EquipmentItem == false)
	{
		CNewUIInventoryCtrl* pOwner = ms_pPickedItem->GetOwnerInventory();
		ITEM* pItemObj = ms_pPickedItem->GetItem();
		if(pOwner)
		{
			pOwner->AddItem(pItemObj->x, pItemObj->y, pItemObj);
			
			DeletePickedItem();
		}
		/* CNewUIInventoryCtrl를 사용하지 않는 예외 */
		/* ITEM::ex_src_type 값으로 구분한다. (ITEM구조체 참조) */
		else if(pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
		{
			ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[pItemObj->lineal_pos];
			memcpy(pEquipmentItemSlot, pItemObj, sizeof(ITEM));		//. 캐릭터 버퍼로 복사
			//. 장착효과 생성
			
			// 해외팀의 펫 들었다 놨다 여러번 반복할때 문제로 작업 했다가,
			// 국내팀의 착용중인 아이템 들었다 놨다 여러번 반복 할때도 같은 문제가 발생.
			// 추후에 문제가 발견되지 않는다면 제거가 되어야 함.
// 주석 임시삭제@@@@@
// #if !defined(LDK_FIX_RECALL_CREATEEQUIPPINGEFFECT) && !defined(LDS_FIX_RECALL_CREATEEQUIPPINGEFFECT) 
			g_pMyInventory->CreateEquippingEffect(pEquipmentItemSlot);
// #endif //LDK_FIX_RECALL_CREATEEQUIPPINGEFFECT
#ifdef PBG_FIX_DARKPET_RENDER
			// 다크스피릿
			if (pEquipmentItemSlot->Type == ITEM_HELPER+5
#ifdef YDG_FIX_DARKSPIRIT_CHAOSCASTLE_CRASH
				&& !InChaosCastle()
#endif	// YDG_FIX_DARKSPIRIT_CHAOSCASTLE_CRASH
				)
			{
				PET_INFO* pPetInfo = giPetManager::GetPetInfo(pEquipmentItemSlot);
				giPetManager::CreatePetDarkSpirit_Now(Hero);
				((CSPetSystem*)Hero->m_pPet)->SetPetInfo(pPetInfo);
			}
#endif //PBG_FIX_DARKPET_RENDER
			DeletePickedItem();
		}
	}
}

void SEASON3B::CNewUIInventoryCtrl::SetEventState(EVENT_STATE es)
{
	m_EventState = es;
}

void SEASON3B::CNewUIInventoryCtrl::Render3D()
{
	type_vec_item::iterator li = m_vecItem.begin();
	for(; li != m_vecItem.end(); li++)
	{
		ITEM* pItem = (*li);
		ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];

		float x = m_Pos.x+(pItem->x*INVENTORY_SQUARE_WIDTH);
		float y = m_Pos.y+(pItem->y*INVENTORY_SQUARE_HEIGHT);
 		float width = pItemAttr->Width*INVENTORY_SQUARE_WIDTH;
		float height = pItemAttr->Height*INVENTORY_SQUARE_HEIGHT;
		glColor4f(1.f, 1.f, 1.f, 1.f);
		
		RenderItem3D(x, y, width, height, pItem->Type, pItem->Level, pItem->Option1, pItem->ExtOption, false);
	}
}

bool SEASON3B::CNewUIInventoryCtrl::IsOverlayItem(ITEM* pSourceItem, ITEM* pTargetItem)
{
	if(pSourceItem == NULL || pTargetItem == NULL)
	{
		return false;
	}

	int iSrcType = pSourceItem->Type;
	int iTarType = pTargetItem->Type;
	int iSrcLevel = pSourceItem->Level;
	int iTarLevel = pTargetItem->Level;
	int iSrcDurability = pSourceItem->Durability;
	int iTarDurability = pTargetItem->Durability;

	if(iSrcType == iTarType)
	{
		// 물약, 마나 종류
		// 축복의물약, 영혼의 물약은 255개까지 겹쳐진다.
		// 치료물약과 마나 물약은 3개까지 겹쳐진다.
		if(iSrcType == ITEM_POTION+7 && iTarType == ITEM_POTION+7 
			&& (iSrcDurability < 250 && iTarDurability < 250)
			)
		{
			return true;
		}
		else if((iSrcType >= ITEM_POTION && iSrcType <= ITEM_POTION+8 && iSrcType != ITEM_POTION+7)
			&& (iTarType >= ITEM_POTION && iTarType <= ITEM_POTION+8 && iTarType != ITEM_POTION+7)
			&& (iSrcDurability < 3 && iTarDurability < 3)
			)
		{
			return true;
		}

		// SD회복물약, 복합물약 종류
		if((iSrcType >= ITEM_POTION+38 && iSrcType <= ITEM_POTION+40)
			&& (iTarType >= ITEM_POTION+38 && iTarType <= ITEM_POTION+40)
			&& (iSrcDurability < 3 && iTarDurability < 3))
		{
			return true;
		}

		// 석궁용 화살 종류
		if((iSrcType == ITEM_BOW+7 && iTarType == ITEM_BOW+7)
			&& (iSrcLevel == iTarLevel))
		{
			return true;
		}

		// 화살
		if((iSrcType == ITEM_BOW+15 && iTarType == ITEM_BOW+15)
			&& (iSrcLevel == iTarLevel))
		{
			return true;
		}

		// 쿤둔의 표식
		if(iSrcType == ITEM_POTION+29 && iTarType == ITEM_POTION+29)
		{
			return true;
		}

		// 펜릴 아이템 관련
		if((iSrcType >= ITEM_HELPER+32 && iSrcType <= ITEM_HELPER+34)
			&& (iTarType >= ITEM_HELPER+32 && iTarType <= ITEM_HELPER+34))
		{
			return true;
		}

		// 할로윈 이벤트 아이템 관련
		if((iSrcType >= ITEM_POTION+46 && iSrcType <= ITEM_POTION+50)
			&& (iTarType >= ITEM_POTION+46 && iTarType <= ITEM_POTION+50)
			&& (iSrcDurability < 3 && iTarDurability < 3))
		{
			return true;
		}
#ifdef PSW_ELITE_ITEM
		//  부분유료화 엘리트 체력 물약. 50개 까지
		else if(iSrcType == ITEM_POTION+70 && iTarType == ITEM_POTION+70 
			&& (iSrcDurability < 50 && iTarDurability < 50)
			)
		{
			return true;
		}

		//  부분유료화 엘리트 마나 물약. 50개 까지
		else if(iSrcType == ITEM_POTION+71 && iTarType == ITEM_POTION+71
			&& (iSrcDurability < 50 && iTarDurability < 50)
			)
		{
			return true;
		}
#endif //PSW_ELITE_ITEM
#ifdef PSW_ELITE_ITEM
		//  부분유료화 힘의 비약 3개 까지
		else if(iSrcType == ITEM_POTION+78 && iTarType == ITEM_POTION+78
			&& (iSrcDurability < 3 && iTarDurability < 3)
			)
		{
			return true;
		}
		//  부분유료화 민첩의 비약 3개 까지
		else if(iSrcType == ITEM_POTION+79 && iTarType == ITEM_POTION+79
			&& (iSrcDurability < 3 && iTarDurability < 3)
			)
		{
			return true;
		}

		//  부분유료화 체력의 비약 3개 까지
		else if(iSrcType == ITEM_POTION+80 && iTarType == ITEM_POTION+80
			&& (iSrcDurability < 3 && iTarDurability < 3)
			)
		{
			return true;
		}

		//  부분유료화 에너지의 비약 3개 까지
		else if(iSrcType == ITEM_POTION+81 && iTarType == ITEM_POTION+81
			&& (iSrcDurability < 3 && iTarDurability < 3)
			)
		{
			return true;
		}

		//  부분유료화 통솔의 비약 3개 까지
		else if(iSrcType == ITEM_POTION+82 && iTarType == ITEM_POTION+82
			&& (iSrcDurability < 3 && iTarDurability < 3)
			)
		{
			return true;
		}
#endif //PSW_ELITE_ITEM
#ifdef PSW_NEW_ELITE_ITEM
		//  부분유료화 엘리트 중간 치료 물약. 50개 까지
		else if(iSrcType == ITEM_POTION+94 && iTarType == ITEM_POTION+94
			&& (iSrcDurability < 50 && iTarDurability < 50)
			)
		{
			return true;
		}
#endif //PSW_NEW_ELITE_ITEM
#ifdef CSK_EVENT_CHERRYBLOSSOM
		else if(iSrcType == ITEM_POTION+85 && iTarType == ITEM_POTION+85
			&& (iSrcDurability < 3 && iTarDurability < 3))
		{
			return true;
		}
		else if(iSrcType == ITEM_POTION+86 && iTarType == ITEM_POTION+86
			&& (iSrcDurability < 3 && iTarDurability < 3))
		{
			return true;
		}
		else if(iSrcType == ITEM_POTION+87 && iTarType == ITEM_POTION+87
			&& (iSrcDurability < 3 && iTarDurability < 3))
		{
			return true;
		}
		else if(iSrcType == ITEM_POTION+88 && iTarType == ITEM_POTION+88
			&& (iSrcDurability < 10 && iTarDurability < 10))
		{
			return true;
		}
		else if(iSrcType == ITEM_POTION+89 && iTarType == ITEM_POTION+89
			&& (iSrcDurability < 30 && iTarDurability < 30))
		{
			return true;
		}
		else if(iSrcType == ITEM_POTION+90 && iTarType == ITEM_POTION+90
#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
			&& (iSrcDurability < 255 && iTarDurability < 255)
#else //SELECTED_LANGUAGE == LANGUAGE_KOREAN
			&& (iSrcDurability < 50 && iTarDurability < 50)
#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
			)
		{
			return true;
		}
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
		//행운의 동전
		else if(iSrcType == ITEM_POTION+100 && (iSrcDurability < 255 && iTarDurability < 255))
		{
			return true;
		}
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_DOPPELGANGER_ITEM
		// 차원의 표식 
		else if(iSrcType == ITEM_POTION+110 && iTarType == ITEM_POTION+110)
		{
			return true;
		}
#endif	// YDG_ADD_DOPPELGANGER_ITEM
#ifdef LDK_ADD_EMPIREGUARDIAN_ITEM
		else if(iSrcType == ITEM_POTION+101 && iTarType == ITEM_POTION+101
			&& (iSrcDurability < 5 && iTarDurability < 5))
		{
			return true;
		}
#endif //LDK_ADD_EMPIREGUARDIAN_ITEM
#ifdef PBG_FIX_SDELITE_OVERLAY
		else if(iSrcType == ITEM_POTION+133		//엘리트 SD물약 50개까지
			&& (iSrcDurability < 50 && iTarDurability < 50))
		{
			return true;
		}
#endif //PBG_FIX_SDELITE_OVERLAY
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		// 스타더스트 ~ 잿더미 도살자의 몽둥이 까지 각각 겹치기 가능
		else if (iSrcType >= ITEM_POTION+153 && iSrcType <= ITEM_POTION+156)
			return true;
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
	}

	return false;
}

bool SEASON3B::CNewUIInventoryCtrl::CanPushItem()
{
	return m_bCanPushItem;
}

#ifdef KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY
bool SEASON3B::CNewUIInventoryCtrl::CanUpgradeItem(ITEM* pSourceItem, ITEM* pTargetItem)
{
	int	iTargetLevel = (pTargetItem->Level >> 3) & 15;

	if( ((pTargetItem->Type >= ITEM_SWORD && pTargetItem->Type < ITEM_WING )
		&&(pTargetItem->Type != ITEM_BOW+7)
		&&(pTargetItem->Type != ITEM_BOW+15))
		||(pTargetItem->Type >= ITEM_WING && pTargetItem->Type <= ITEM_WING+6)
		||(pTargetItem->Type >= ITEM_WING+36 && pTargetItem->Type <= ITEM_WING+43)
		)
	{
		if( (pSourceItem->Type == ITEM_POTION+13) 				// 축석 (Lv0~5)
			&& (iTargetLevel >= 0 && iTargetLevel <= 5) )		
		{
			return true;
		}
		else if( (pSourceItem->Type == ITEM_POTION+14)			// 영석 (Lv0~8)
			&& (iTargetLevel >= 0 && iTargetLevel <= 8) )	
		{
			return true;
		}
	}

	return false;
}
#endif // KJH_FIX_JP0459_CAN_MIX_JEWEL_OF_HARMONY
