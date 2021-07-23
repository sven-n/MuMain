// NewUICharacterInfoWindow.h: interface for the CNewUICharacterInfoWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICHARACTERINFOWINDOW_H__947150CE_5919_4C74_B1C9_A27CA422FE95__INCLUDED_)
#define AFX_NEWUICHARACTERINFOWINDOW_H__947150CE_5919_4C74_B1C9_A27CA422FE95__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"

namespace SEASON3B
{

	class CNewUICharacterInfoWindow : public CNewUIObj  
	{
	public:
		enum IMAGE_LIST
		{
			// 기본창
			IMAGE_CHAINFO_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
			IMAGE_CHAINFO_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,		// newui_item_back04.tga	(190,64)
			IMAGE_CHAINFO_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
			IMAGE_CHAINFO_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
			IMAGE_CHAINFO_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)

			// 테이블 작성
			IMAGE_CHAINFO_TABLE_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga (14,14)
			IMAGE_CHAINFO_TABLE_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga (14,14)
			IMAGE_CHAINFO_TABLE_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga (14,14)
			IMAGE_CHAINFO_TABLE_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
			IMAGE_CHAINFO_TABLE_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
			IMAGE_CHAINFO_TABLE_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
			IMAGE_CHAINFO_TABLE_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
			IMAGE_CHAINFO_TABLE_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)

			IMAGE_CHAINFO_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,

			IMAGE_CHAINFO_TEXTBOX = BITMAP_INTERFACE_NEW_CHAINFO_WINDOW_BEGIN,
			IMAGE_CHAINFO_BTN_STAT,
			IMAGE_CHAINFO_BTN_QUEST,
			IMAGE_CHAINFO_BTN_PET,
			IMAGE_CHAINFO_BTN_MASTERLEVEL,
		};

		enum
		{
			CHAINFO_WINDOW_WIDTH = 190,
			CHAINFO_WINDOW_HEIGHT = 429,

			HEIGHT_STRENGTH = 120,
			HEIGHT_DEXTERITY = 175,
			HEIGHT_VITALITY = 240,
			HEIGHT_ENERGY = 295,
			HEIGHT_CHARISMA = 350,

			BTN_STAT_COUNT = 5,

			STAT_STRENGTH = 0,
			STAT_DEXTERITY,
			STAT_VITALITY,
			STAT_ENERGY,
			STAT_CHARISMA,
		};
		
	private:
		CNewUIManager*				m_pNewUIMng;
		POINT						m_Pos;

		CNewUIButton m_BtnStat[BTN_STAT_COUNT];
		CNewUIButton m_BtnExit;
		CNewUIButton m_BtnQuest;
		CNewUIButton m_BtnPet;
		CNewUIButton m_BtnMasterLevel;

	public:
		CNewUICharacterInfoWindow();
		virtual ~CNewUICharacterInfoWindow();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		float GetLayerDepth();	//. 5.1f

		void OpenningProcess();
		
	private:
		void LoadImages();
		void UnloadImages();

#ifdef KJH_FIX_WOPS_K28597_EQUIPED_LV10_DEFENSEITEM
		void ResetEquipmentLevel();
#endif // KJH_FIX_WOPS_K28597_EQUIPED_LV10_DEFENSEITEM

		void SetButtonInfo();

		bool BtnProcess();

		void RenderFrame();
		void RenderTexts();
		void RenderSubjectTexts();	// ID, 서버명, 클래스명
		void RenderTableTexts();	// 레벨, 포인트, 경험치	
		void RenderAttribute();	// 속성 
		void RenderButtons();
	};


}

#endif // !defined(AFX_NEWUICHARACTERINFOWINDOW_H__947150CE_5919_4C74_B1C9_A27CA422FE95__INCLUDED_)
