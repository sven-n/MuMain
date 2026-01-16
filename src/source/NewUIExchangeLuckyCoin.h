// NewUIExchangeLuckyCoin.h: interface for the CNewUIExchangeLuckyCoin class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"

namespace SEASON3B
{
    class CNewUIExchangeLuckyCoin : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            // Base Window (Reference)
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,				//. newui_msgbox_back.jpg
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,			//. newui_item_back01.tga	(190,64)
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,		//. newui_item_back02-l.tga	(21,320)
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,		//. newui_item_back02-r.tga	(21,320)
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	//. newui_item_back03.tga	(190,45)
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BTN_EXIT = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,	//. newui_btn_empty.tga (64, 87)

            IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY			//. newui_btn_empty.tga	(108, 87)
        };

    private:
        enum ENTERBC_WINDOW_SIZE
        {
            EXCHANGE_LUCKYCOIN_WINDOW_WIDTH = 190,
            EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT = 429,
        };

    public:
        enum
        {
            MAX_EXCHANGE_BTN = 3,
            EXCHANGE_BTN_VAL = 33,
            EXCHANGE_TEXT_VAL = 14,
        };

    private:
        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;
        POINT						m_TextPos;
        POINT						m_FirstBtnPos;

        CNewUIButton				m_BtnExchange[MAX_EXCHANGE_BTN];
        CNewUIButton				m_BtnExit;

    public:
        CNewUIExchangeLuckyCoin();
        virtual ~CNewUIExchangeLuckyCoin();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 4.2f

        void OpenningProcess();
        void ClosingProcess();

        void LockExchangeBtn();
        void UnLockExchangeBtn();

    private:
        void SetBtnPos(int x, int y);
        void RenderFrame();
        void RenderTexts();
        void RenderBtn();
        void LoadImages();
        void UnloadImages();
    };
}
