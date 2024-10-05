//*****************************************************************************
// File: NewUITrade.h
//
// Desc: interface for the CNewUITrade class.
//		 거래창 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_NEWUITRADE_H__25FC9B24_8F86_4791_B246_689326623DFB__INCLUDED_)
#define AFX_NEWUITRADE_H__25FC9B24_8F86_4791_B246_689326623DFB__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIMyQuestInfoWindow.h"
#include "NewUIStorageInventory.h"

namespace SEASON3B
{
    class CNewUITrade : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_TRADE_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_TRADE_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_TRADE_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_TRADE_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_TRADE_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,

            IMAGE_TRADE_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
            IMAGE_TRADE_NICK_BACK = BITMAP_INTERFACE_NEW_TRADE_BEGIN,
            IMAGE_TRADE_MONEY = CNewUIMyInventory::IMAGE_INVENTORY_MONEY,
            IMAGE_TRADE_CONFIRM = BITMAP_INTERFACE_NEW_TRADE_BEGIN + 1,
            IMAGE_TRADE_WARNING_ARROW = BITMAP_CURSOR + 7,

            IMAGE_TRADE_BTN_CLOSE = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_TRADE_BTN_ZEN_INPUT = CNewUIStorageInventory::IMAGE_STORAGE_BTN_INSERT_ZEN,
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
            BTN_CLOSE = 0,			// 창 닫기.
            BTN_ZEN_INPUT,			// 젠 입력.
            MAX_BTN
        };

        CNewUIManager* m_pNewUIMng;			// UI 매니저.
        POINT					m_Pos;					// 창의 위치.
        CNewUIButton			m_abtn[MAX_BTN];		// 버튼.
        POINT					m_posMyConfirm;			// 내 확정 버튼 위치.
        CNewUIInventoryCtrl* m_pYourInvenCtrl;		// 상대방 물품 컨트롤.
        CNewUIInventoryCtrl* m_pMyInvenCtrl;			// 내 물품 컨트롤.
        ITEM					m_aYourInvenBackUp[MAX_TRADE_INVEN];// 상대방 물품 백업.

        wchar_t					m_szYourID[MAX_ID_SIZE + 1];// 거래 사용자의 아이디.
        int						m_nYourLevel;			// 거래 사용자의 레벨.
        int						m_nYourGuildType;		// 상대방 길드 타입.
        int						m_nYourTradeGold;		// 상대방 거래할 돈.
        int						m_nMyTradeGold;			// 자신의 거래할 돈.
        int						m_nTempMyTradeGold;		// 자신의 거래할 돈 임시 공간.
        bool					m_bYourConfirm;			// 상대방 거래 결정 상태.
        bool					m_bMyConfirm;			// 자신의 거래 결정 상태.
        int						m_nMyTradeWait;			// 자신의 거래 결정 버튼 못 누르게 하는 대기 시간.
        bool					m_bTradeAlert;			// 거래시 경고.

    public:
        CNewUITrade();
        virtual ~CNewUITrade();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 2.1f

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

        // (격자 모양의) 상대편 거래 소지품 컨트롤을 얻음.
        CNewUIInventoryCtrl* GetYourInvenCtrl() const
        {
            return m_pYourInvenCtrl;
        }
        // (격자 모양의) 자신의 거래 소지품 컨트롤을 얻음.
        CNewUIInventoryCtrl* GetMyInvenCtrl() const
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
