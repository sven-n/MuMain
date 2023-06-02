// NewUIBloodCastleEnter.h: interface for the CNewUIPartyInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _ENTERBLOODCASTLE_H_
#define _ENTERBLOODCASTLE_H_

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"
#include "NewUIMessageBox.h"

namespace SEASON3B
{
    class CNewUIEnterBloodCastle : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            // Base Window (Reference)
            IMAGE_ENTERBC_BASE_WINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,				//. newui_msgbox_back.jpg
            IMAGE_ENTERBC_BASE_WINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,			//. newui_item_back01.tga	(190,64)
            IMAGE_ENTERBC_BASE_WINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,			//. newui_item_back02-l.tga	(21,320)
            IMAGE_ENTERBC_BASE_WINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,		//. newui_item_back02-r.tga	(21,320)
            IMAGE_ENTERBC_BASE_WINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,		//. newui_item_back03.tga	(190,45)
            IMAGE_ENTERBC_BASE_WINDOW_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,		//. newui_exit_00.tga

            IMAGE_ENTERBC_BASE_WINDOW_BTN_ENTER = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_BIG	//. newui_btn_empty_big.tga	(180, 87)
        };

    private:
        enum ENTERBC_WINDOW_SIZE
        {
            ENTERBC_BASE_WINDOW_WIDTH = 190,
            ENTERBC_BASE_WINDOW_HEIGHT = 429,
        };

        enum ENTERBC_ENTERBTN_STATE
        {
            ENTERBTN_DISABLE = 0,
            ENTERBTN_ENABLE,
        };

        enum
        {
            ENTER_BTN_VAL = 33,					// 블러드캐슬 입장 버튼 사이의 간격

            MAX_ENTER_GRADE = 8,
        };

    private:
        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;
        POINT						m_BtnEnterStartPos;
        POINT						m_EnterUITextPos;

        CNewUIButton				m_BtnExit;				// Exit Button Class
        CNewUIButton				m_BtnEnter[MAX_ENTER_GRADE];			// Blood Castle Enter Button

        int							m_iBloodCastleLimitLevel[MAX_ENTER_GRADE * 2][2];
        int							m_iNumActiveBtn;		// 활성화 되어있는 버튼
        DWORD						m_dwBtnTextColor[2];	// 0 - Disabled, 1 - Enable

    public:
        CNewUIEnterBloodCastle();
        virtual ~CNewUIEnterBloodCastle();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 4.1f

        void OpenningProcess();
        void ClosingProcess();

    private:
        void SetBtnPos(int x, int y);
        void LoadImages();
        void UnloadImages();

        int	CheckLimitLV(int iIndex);
    };
}

#endif // _ENTERBLOODCASTLE_H_
