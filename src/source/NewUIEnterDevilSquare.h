// NewUIEnterDevilSquare.h: interface for the CNewUIPartyInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _ENTERDEVILSQUARE_H_
#define _ENTERDEVILSQUARE_H_

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"
#include "NewUIMessageBox.h"

namespace SEASON3B
{
    class CNewUIEnterDevilSquare : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            // Base Window (Reference)
            IMAGE_ENTERDS_BASE_WINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,				//. newui_msgbox_back.jpg
            IMAGE_ENTERDS_BASE_WINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,			//. newui_item_back01.tga	(190,64)
            IMAGE_ENTERDS_BASE_WINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,			//. newui_item_back02-l.tga	(21,320)
            IMAGE_ENTERDS_BASE_WINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,		//. newui_item_back02-r.tga	(21,320)
            IMAGE_ENTERDS_BASE_WINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,		//. newui_item_back03.tga	(190,45)
            IMAGE_ENTERDS_BASE_WINDOW_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,		//. newui_exit_00.tga

            IMAGE_ENTERDS_BASE_WINDOW_BTN_ENTER = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_BIG	//. newui_btn_empty_big.tga	(180, 87)
        };

    private:
        enum ENTERDS_WINDOW_SIZE
        {
            ENTERDS_BASE_WINDOW_WIDTH = 190,
            ENTERDS_BASE_WINDOW_HEIGHT = 429,
        };

        enum ENTERDS_ENTERBTN_STATE
        {
            ENTERBTN_DISABLE = 0,
            ENTERBTN_ENABLE,
        };

        enum
        {
            ENTER_BTN_VAL = 33,			// 버튼 사이의 간격

            MAX_ENTER_GRADE = 7,
        };

    private:
        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;
        POINT						m_BtnEnterStartPos;
        POINT						m_EnterUITextPos;

        CNewUIButton				m_BtnExit;								// Exit Button Class
        CNewUIButton				m_BtnEnter[MAX_ENTER_GRADE];			// Devil Square Enter Button

        int							m_iDevilSquareLimitLevel[MAX_ENTER_GRADE * 2][2];
        int							m_iNumActiveBtn;		// 활성화 되어있는 버튼
        DWORD						m_dwBtnTextColor[2];	// 0 - Disabled, 1 - Enable

    public:
        CNewUIEnterDevilSquare();
        virtual ~CNewUIEnterDevilSquare();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 4.0f

        void OpenningProcess();
        void ClosingProcess();

    private:
        void SetBtnPos(int x, int y);
        void LoadImages();
        void UnloadImages();

        int	CheckLimitLV(int iIndex);
    };
}

#endif // _ENTERDEVILSQUARE_H_
