// NewUIPartyInfo.h: interface for the CNewUIPartyInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIPARTYINFO_H__8F1E7392_C9CC_487E_8EAD_10DBD47AF20F__INCLUDED_)
#define AFX_NEWUIPARTYINFO_H__8F1E7392_C9CC_487E_8EAD_10DBD47AF20F__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"

namespace SEASON3B
{
    class CNewUIPartyInfoWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            // Base Window (Reference)
            IMAGE_PARTY_BASE_WINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,				//. newui_msgbox_back.jpg
            IMAGE_PARTY_BASE_WINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,			//. newui_item_back01.tga	(190,64)
            IMAGE_PARTY_BASE_WINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,		//. newui_item_back02-l.tga	(21,320)
            IMAGE_PARTY_BASE_WINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,		//. newui_item_back02-r.tga	(21,320)
            IMAGE_PARTY_BASE_WINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	//. newui_item_back03.tga	(190,45)
            IMAGE_PARTY_BASE_WINDOW_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,		//. newui_exit_00.tga

            IMAGE_PARTY_TABLE_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,			//. newui_item_table01(L).tga (14,14)
            IMAGE_PARTY_TABLE_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,			//. newui_item_table01(R).tga (14,14)
            IMAGE_PARTY_TABLE_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,		//. newui_item_table02(L).tga (14,14)
            IMAGE_PARTY_TABLE_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
            IMAGE_PARTY_TABLE_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
            IMAGE_PARTY_TABLE_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
            IMAGE_PARTY_TABLE_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
            IMAGE_PARTY_TABLE_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)

            IMAGE_PARTY_HPBAR_BACK = BITMAP_PARTY_INFO_BEGIN,		// newui_party_lifebar01.jpg	(151, 8)
            IMAGE_PARTY_HPBAR,										// newui_party_lifebar02.jpg	(147, 4)
            IMAGE_PARTY_FLAG,										// newui_party_flag.tga			(10, 12)
            IMAGE_PARTY_EXIT,										// newui_Party_x.tga			(13, 26)	// 임시
        };

    private:
        enum PARTY_WINDOW_SIZE
        {
            PARTY_INFO_WINDOW_WIDTH = 190,
            PARTY_INFO_WINDOW_HEIGHT = 429,
        };

    private:
        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;

        // Exit Button
        CNewUIButton				m_BtnExit;							// 파티창 나가기버튼
        CNewUIButton				m_BtnPartyExit[MAX_PARTYS];			// 파티탈퇴버튼

        int							m_iSelectedCharID;		// Party List에서 캐릭터 ID를 선택 (default : -1)

        bool						m_bParty;

    public:
        CNewUIPartyInfoWindow();
        virtual ~CNewUIPartyInfoWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 2.4f

        void OpenningProcess();
        void ClosingProcess();

        bool LeaveParty(const int iIndex);	// 파티장이 상대방을 강퇴

        void SetParty(bool bParty);

    private:
        void LoadImages();
        void UnloadImages();

        void InitButtons();
        void RenderGroupBox(int iPosX, int iPosY, int iWidth, int iHeight, int iTitleWidth = 60, int iTitleHeight = 20);

        void RenderMemberStatue(int iIndex, PARTY_t* pMember, bool bExitBtnRender = false);
        void RenderPartyMiniList(int iIndex, PARTY_t* pMember);
    };
}

#endif // !defined(AFX_NEWUIPARTYINFO_H__8F1E7392_C9CC_487E_8EAD_10DBD47AF20F__INCLUDED_)
