// NewUIExchangeLuckyCoin.h: interface for the CExchangeLuckyCoin class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Core/NewUIManager.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Widgets/NewUIButton.h"

namespace mu::ui::window
{
    class CExchangeLuckyCoin : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            // Base Window (Reference)
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,				//. newui_msgbox_back.jpg
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP2,			//. newui_item_back01.tga	(190,64)
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,		//. newui_item_back02-l.tga	(21,320)
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,		//. newui_item_back02-r.tga	(21,320)
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	//. newui_item_back03.tga	(190,45)
            IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BTN_EXIT = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,	//. newui_btn_empty.tga (64, 87)

            IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY			//. newui_btn_empty.tga	(108, 87)
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
        CManager* m_pNewUIMng;
        POINT						m_Pos;
        POINT						m_TextPos;
        POINT						m_FirstBtnPos;

        CButton				m_BtnExchange[MAX_EXCHANGE_BTN];
        CButton				m_BtnExit;

    public:
        CExchangeLuckyCoin();
        virtual ~CExchangeLuckyCoin();

        bool Create(CManager* pNewUIMng, int x, int y);
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
