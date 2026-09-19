#if !defined(AFX_NEWUITRADE_H__25FC9B24_8F86_4791_B246_689326623DFB__INCLUDED_)
#define AFX_NEWUITRADE_H__25FC9B24_8F86_4791_B246_689326623DFB__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Quests/MyQuestInfoWindow.h"
#include "UI/Inventory/StorageInventory.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CTrade : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            // Cursor-following warning-arrow overlay only -- every other sprite in this window
            // (frame/nick-back/money/confirm/line/buttons) is RmlUi (trade.rml/trade_bg.rml);
            // see RenderWarningArrow()/LoadImages().
            IMAGE_TRADE_WARNING_ARROW = BITMAP_CURSOR + 7,
        };

    private:
        enum
        {
            TRADE_WIDTH = 190,
            TRADE_HEIGHT = 429,
            COLUMN_TRADE_INVEN = 8,
            ROW_TRADE_INVEN = 4,
            MAX_TRADE_INVEN = COLUMN_TRADE_INVEN * ROW_TRADE_INVEN,
        };

        CManager* m_pNewUIMng;            // UI Manager
        POINT          m_Pos;                  // Window position
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

        // Window frame/title/both nickname displays/both gold strips/both confirm checkboxes/
        // divider/buttons are RmlUi. Both CInventoryCtrl grids stay native since their icons are
        // live 3D model renders (same reasoning as CMyInventory/CStorageInventoryExt). The
        // guild-mark emblem (RenderGuildMark()) and the cursor-following warning arrow
        // (RenderWarningArrow()) also stay native -- both are live-rendered/live-tracking effects,
        // not static chrome.
        struct TradeRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;

            Rml::String title;

            Rml::String yourIdText;
            bool yourGuildVisible = false;
            Rml::String yourGuildName;
            Rml::String yourLevelText;
            Rml::String yourLevelColor;
            Rml::String yourGoldText;
            Rml::String yourGoldColor;
            bool yourConfirmChecked = false;

            Rml::String myIdText;
            Rml::String myGoldText;
            Rml::String myGoldColor;
            bool myConfirmChecked = false;
            bool myConfirmWaiting = false;

            Rml::String warningLabel;
            Rml::String noticeLine1;
            Rml::String noticeLine2;
            Rml::String noticeLine3;
            float warningOpacity = 1.f;

            Rml::String closeTooltip;
            Rml::String zenTooltip;
        };
        RmlModelBinder<TradeRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // The frame background panel must render behind both grids' live 3D icons, but RmlUi's
        // main context always renders last -- so it goes through
        // RmlUiRuntime::GetBackgroundContext()/RenderBackgroundLayer() instead (see CMyInventory's
        // identical MyInventoryBgRmlModel for the full mechanism).
        struct TradeBgRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        };
        RmlModelBinder<TradeBgRmlModel> m_BgRmlBinder;
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

        void SyncRmlModel();

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

        void RenderGuildMark();
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
