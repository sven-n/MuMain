//*****************************************************************************
// File: NewUITrade.h
//
// Desc: interface for the CTrade class.
//       Trade Window class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_NEWUITRADE_H__25FC9B24_8F86_4791_B246_689326623DFB__INCLUDED_)
#define AFX_NEWUITRADE_H__25FC9B24_8F86_4791_B246_689326623DFB__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Quests/NewUIMyQuestInfoWindow.h"
#include "UI/Inventory/NewUIStorageInventory.h"

namespace mu::ui::window
{
    class CTrade : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_TRADE_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_TRADE_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_TRADE_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_TRADE_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_TRADE_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,

            IMAGE_TRADE_LINE = CMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
            IMAGE_TRADE_NICK_BACK = BITMAP_INTERFACE_NEW_TRADE_BEGIN,
            IMAGE_TRADE_MONEY = CMyInventory::IMAGE_INVENTORY_MONEY,
            IMAGE_TRADE_CONFIRM = BITMAP_INTERFACE_NEW_TRADE_BEGIN + 1,
            IMAGE_TRADE_WARNING_ARROW = BITMAP_CURSOR + 7,

            IMAGE_TRADE_BTN_CLOSE = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_TRADE_BTN_ZEN_INPUT = CStorageInventory::IMAGE_STORAGE_BTN_INSERT_ZEN,
        };

    private:
        enum
        {
            TRADE_WIDTH = 190,
            TRADE_HEIGHT = 429,
            CONFIRM_WIDTH = 36,
            CONFIRM_HEIGHT = 29,
            COLUMN_TRADE_INVEN = 8,
            ROW_TRADE_INVEN = 4,
            MAX_TRADE_INVEN = COLUMN_TRADE_INVEN * ROW_TRADE_INVEN,
        };

        enum TRADE_BUTTON
        {
            BTN_CLOSE = 0,            // Close window
            BTN_ZEN_INPUT,            // Zen input
            MAX_BTN
        };

        CManager* m_pNewUIMng;            // UI Manager
        POINT          m_Pos;                  // Window position
        CButton   m_abtn[MAX_BTN];        // Buttons
        POINT          m_posMyConfirm;         // My confirmation button position
        CInventoryCtrl* m_pYourInvenCtrl; // Other player's item control
        CInventoryCtrl* m_pMyInvenCtrl;   // My item control
        ITEM           m_aYourInvenBackUp[MAX_TRADE_INVEN]; // Other player's item backup

        wchar_t        m_szYourID[MAX_USERNAME_SIZE + 1]; // Other player's ID
        int            m_nYourLevel;           // Other player's level
        int            m_nYourGuildType;       // Other player's guild type
        int            m_nYourTradeGold;       // Other player's trade gold
        int            m_nMyTradeGold;         // My trade gold
        int            m_nTempMyTradeGold;     // Temporary buffer for my trade gold
        bool           m_bYourConfirm;         // Other player's confirmation status
        bool           m_bMyConfirm;           // My confirmation status
        int            m_nMyTradeWait;         // Delay to prevent spamming my confirm button
        bool           m_bTradeAlert;          // Trade warning alert

    public:
        CTrade();
        virtual ~CTrade();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 2.1f

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

        // Returns the other player's (grid-based) trade inventory control.
        CInventoryCtrl* GetYourInvenCtrl() const
        {
            return m_pYourInvenCtrl;
        }
        // Returns the local player's (grid-based) trade inventory control.
        CInventoryCtrl* GetMyInvenCtrl() const
        {
            return m_pMyInvenCtrl;
        }

        void ProcessCloseBtn();
        void ProcessClosing();

        void GetYourID(wchar_t* pszYourID);
        void SetYourTradeGold(int nGold) { m_nYourTradeGold = nGold; }

        void SendRequestMyGoldInput(int nInputGold);
        void SendRequestItemToMyInven(ITEM* pItemObj,
            int nTradeIndex, int nInvenIndex);

        void ProcessToReceiveTradeRequest(char* pbyYourID);
        void ProcessToReceiveTradeResult(LPPTRADE pTradeData);
        void ProcessToReceiveYourItemDelete(BYTE byYourInvenIndex);
        void ProcessToReceiveYourItemAdd(BYTE byYourInvenIndex, std::span<const BYTE> pbyItemPacket);
        void ProcessToReceiveMyTradeGold(BYTE bySuccess);
        void ProcessToReceiveYourConfirm(BYTE byState);
        void ProcessToReceiveTradeExit(BYTE byState);
        void ProcessToReceiveTradeItems(int nIndex, std::span<const BYTE> pbyItemPacket);

        void AlertTrade();

        int GetPointedItemIndexMyInven();
        int GetPointedItemIndexYourInven();

    private:
        void LoadImages();
        void UnloadImages();

        void RenderBackImage();
        void RenderText();
        void RenderWarningArrow();

        void ProcessMyInvenCtrl();
        bool ProcessBtns();

        void ConvertYourLevel(int& rnLevel, DWORD& rdwColor);

        void InitTradeInfo();
        void InitYourInvenBackUp();
        void BackUpYourInven(int nYourInvenIndex);
        void BackUpYourInven(ITEM* pYourItemObj);
        void AlertYourTradeInven();

        void SendRequestItemToTrade(ITEM* pItemObj, int nInvenIndex, int nTradeIndex);
    };
}

#endif // !defined(AFX_NEWUITRADE_H__25FC9B24_8F86_4791_B246_689326623DFB__INCLUDED_)
