// NewUIInGameShop.h: interface for the NewUIInGameShop class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIINGAMESHOP_H__AE3CE531_70BE_4CBB_9938_0D80B26F21A8__INCLUDED_)
#define AFX_NEWUIINGAMESHOP_H__AE3CE531_70BE_4CBB_9938_0D80B26F21A8__INCLUDED_

#pragma once

#ifdef PBG_ADD_INGAMESHOP_UI_ITEMSHOP

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUICommonMessageBox.h"
#include "ZzzInventory.h"
#include "Sprite.h"
#include "InGameShopSystem.h"

namespace SEASON3B
{
    class CNewUIInGameShop : public CNewUIObj
    {
    public:
        enum LISTBOX_INDEX
        {
            IGS_SAFEKEEPING_LISTBOX = 0,
            IGS_PRESENTBOX_LISTBOX,
            IGS_TOTAL_LISTBOX,
        };

        enum IMAGE_LIST
        {
            IMAGE_IGS_EXIT_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,	// newui_exit_00.tga (36, 58) - 2BtState
            IMAGE_IGS_BACK = BITMAP_INGAMESHOP_FRAME,	// Ingame_shopback.jpg (640, 429)
            IMAGE_IGS_CATEGORY_BTN,				// Ingame_Bt01.tga (73, 81) - 3BtState
            IMAGE_IGS_CATEGORY_DECO_MIDDLE,		// Ingame_Deco_Center.tga (6, 8)
            IMAGE_IGS_CATEGORY_DECO_DOWN,		// Ingame_Deco_Dn.tga (47, 100)
            IMAGE_IGS_LEFT_TAB,					// Ingame_Tab01.tga (49, 21)
            IMAGE_IGS_RIGHT_TAB,				// Ingame_Tab02.tga (49, 21)
            IMAGE_IGS_ZONE_BTN,					// Ingame_Tab_Up.tga (76, 46) - 2BtState
            IMAGE_IGS_ITEMGIFT_BTN,				// Ingame_Bt_Gift.tga (25, 75) - 3BtState
            IMAGE_IGS_CASHGIFT_BTN,				// Ingame_Bt_Cash.tga (25, 75) - 3BtState
            IMAGE_IGS_REFRESH_BTN,				// Ingame_Bt_Reset.tga (25, 75) - 3BtState
            IMAGE_IGS_VIEWDETAIL_BTN,			// Ingame_Bt_Bt03.tga (52, 78) - 3BtState
            IMAGE_IGS_ITEMBOX_LOGO,				// Ingame_Itembox_logo.tga (57, 57)
            IMAGE_IGS_PAGE_LEFT,				// Ingame_Bt_page_L.tga (20, 69) - 3BtState
            IMAGE_IGS_PAGE_RIGHT,				// Ingame_Bt_page_R.tga (20, 69) - 3BtState
            IMAGE_IGS_STORAGE_PAGE,				// IGS_Storage_Page.tga (80, 30)
            IMAGE_IGS_STORAGE_PAGE_LEFT,		// IGS_Storage_Page_Left.tga (20, 22) - 3BtState
            IMAGE_IGS_STORAGE_PAGE_RIGHT,		// IGS_Storage_Page_Right.tga (20, 22) - 3BtState
            IMAGE_IGS_BANNER = BITMAP_INGAMESHOP_BANNER
        };

    private:
        enum INGAMESHOP_TEXT_INFO
        {
            TEXT_IGS_CHAR_NAME_POS_X = 498,
            TEXT_IGS_CHAR_NAME_POS_Y = 23,
            TEXT_IGS_CHAR_NAME_WIDTH = 122,
            TEXT_IGS_CASH_POS_X = 498,
            TEXT_IGS_CASH_POS_Y = 50,
            TEXT_IGS_CASH_WIDTH = 130,
            TEXT_IGS_MILEAGE_POS_Y = 65,
            TEXT_IGS_POINT_POS_Y = 80,
            TEXT_IGS_STORAGE_NAME_POS_X = 492,
            TEXT_IGS_STORAGE_NAME_POS_Y = 233,
            TEXT_IGS_STORAGE_NAME_WIDTH = 96,
            TEXT_IGS_STORAGE_TIME_POS_X = 592,
            TEXT_IGS_STORAGE_TIME_WIDTH = 34,
            TEXT_IGS_PAGE_POS_X = 251,
            TEXT_IGS_PAGE_POS_Y = 404,
            TEXT_IGS_STORAGE_PAGE_INFO_POS_X = 518,
            TEXT_IGS_STORAGE_PAGE_INFO_POS_Y = 376,
        };

        enum INGAMESHOP_IMAGES_POS
        {
            IMAGE_IGS_EXIT_BTN_POS_X = 484,			// Exit Button
            IMAGE_IGS_EXIT_BTN_POS_Y = 392,
            IMAGE_IGS_BACK_POS_X = 0,			// InGameShop Back
            IMAGE_IGS_BACK_POS_Y = 0,
            IMAGE_IGS_CATEGORY_BTN_POS_X = 13,		// Category Button
            IMAGE_IGS_CATEGORY_BTN_POS_Y = 31,
            IMAGE_IGS_CATEGORY_BTN_DISTANCE = 6,
            IMAGE_IGS_TAB_BTN_POS_X = 486,		// Tab Button
            IMAGE_IGS_TAB_BTN_POS_Y = 208,
            IMAGE_IGS_TAB_BTN_DISTANCE = -2,
            IMAGE_IGS_ZONE_BTN_POS_X = 95,		// Zone Button
            IMAGE_IGS_ZONE_BTN_POS_Y = 0,
            IMAGE_IGS_VIEWDETAIL_BTN_POS_X = 162,		// View Detail Button
            IMAGE_IGS_VIEWDETAIL_BTN_POS_Y = 126,
            IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_X = 122,
            IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_Y = 121,
            IMAGE_IGS_ITEMGIFT_BTN_POS_X = 519,		// Item Gift Button
            IMAGE_IGS_CASHGIFT_BTN_POS_X = 546,		// Cash Gift Button
            IMAGE_IGS_REFRESH_BTN_POS_X = 573,		// Refresh Button
            IMAGE_IGS_ICON_BTN_POS_Y = 94,
            IMAGE_IGS_USE_BTN_POS_X = 572,
            IMAGE_IGS_USE_BTN_POS_Y = 396,
            IMAGE_IGS_ITEMBOX_LOGO_POS_X = 128,
            IMAGE_IGS_ITEMBOX_LOGO_POS_Y = 52,
            IMAGE_IGS_PAGE_LEFT_POS_X = 231,		// Page Left Button
            IMAGE_IGS_PAGE_RIGHT_POS_X = 307,		// Page Right Button
            IMAGE_IGS_PAGE_BUTTON_POS_Y = 397,
            IMAGE_IGS_BANNER_POS_X = 482,		// Banner
            IMAGE_IGS_BANNER_POS_Y = 133,
            IMAGE_IGS_STORAGE_PAGE_POS_X = 518,		// Storage Page
            IMAGE_IGS_STORAGE_PAGE_POS_Y = 366,
            IMAGE_IGS_STORAGE_PAGE_LEFT_POS_X = 512,		// Storage Page Left
            IMAGE_IGS_STORAGE_PAGE_RIGHT_POS_X = 586,		// Storage Page Right
            IMAGE_IGS_STORAGE_PAGE_BTN_POS_Y = 372,
        };

        enum INGAMESHOP_IMAGES_SIZE
        {
            IMAGE_IGS_EXIT_BTN_WIDTH = 36,		// Exit Button
            IMAGE_IGS_EXIT_BTN_HEIGHT = 29,
            IMAGE_IGS_BACK_WIDTH = 640,		// InGameShop Back
            IMAGE_IGS_BACK_HEIGHT = 429,
            IMAGE_IGS_CATEGORY_BTN_WIDTH = 73,		// Category Button
            IMAGE_IGS_CATEGORY_BTN_HEIGHT = 27,
            IMAGE_IGS_CATEGORY_DECO_MIDDLE_WIDTH = 4,	// Category Deco Middle
            IMAGE_IGS_CATEGORY_DECO_MIDDLE_HEIGHT = 8,
            IMAGE_IGS_CATEGORY_DECO_DOWN_WIDTH = 47,	// Category Deco Down
            IMAGE_IGS_CATEGORY_DECO_DOWN_HEIGHT = 100,
            IMAGE_IGS_TAB_BTN_WIDTH = 49,		// Tab Button
            IMAGE_IGS_TAB_BTN_HEIGHT = 20,
            IMAGE_IGS_ZONE_BTN_WIDTH = 76,		// Zone Button
            IMAGE_IGS_ZONE_BTN_HEIGHT = 23,
            IMAGE_IGS_VIEWDETAIL_BTN_WIDTH = 52,		// View Detail Button
            IMAGE_IGS_VIEWDETAIL_BTN_HEIGHT = 26,
            IMAGE_IGS_ICON_BTN_WIDTH = 25,
            IMAGE_IGS_ICON_BTN_HEIGHT = 25,
            IMAGE_IGS_ITEMBOX_LOGO_SIZE = 57,
            IMAGE_IGS_PAGE_BTN_WIDTH = 20,		// Page
            IMAGE_IGS_PAGE_BTN_HEIGHT = 23,
            IMAGE_IGS_BANNER_WIDTH = 153,		// Banner
            IMAGE_IGS_BANNER_HEIGHT = 63,
            IMGAE_IGS_STORAGE_PAGE_WIDTH = 80,		// Storage Page
            IMGAE_IGS_STORAGE_PAGE_HEIGHT = 30,
            IMGAE_IGS_STORAGE_PAGE_BTN_WIDTH = 20,	// Storage Page Btn
            IMGAE_IGS_STORAGE_PAGE_BTN_HEIGHT = 22,
        };

        enum INGAMESHOP_DISPLAY_ITEMS
        {
            IGS_WIDTH_POS_X = 129,
            IGS_HEIGHT_POS_Y = 59,
            IGS_SIZE_WIDTH = 60,
            IGS_SIZE_HEIGHT = 49,
            IGS_NUM_ITEMS_WIDTH = 3,
            IGS_NUM_ITEMS_HEIGHT = 3,
            IGS_PACKAGE_NAME_POS_X = 105,
            IGS_PACKAGE_NAME_POS_Y = 40,
            IGS_PACKAGE_NAME_WIDTH = 104,
            IGS_PACKAGE_PRICE_POS_Y = 60,
            IGS_ITEMRENDER_POS_X_STANDAD = 102,
            IGS_ITEMRENDER_POS_WIDTH = 108,
            IGS_ITEMRENDER_POS_Y_STANDAD = 51,
            IGS_ITEMRENDER_POS_HEIGHT = 58,
            IGS_STORAGE_TOTAL_ITEM_PER_PAGE = 9,
        };

    public:
        CNewUIInGameShop();
        virtual ~CNewUIInGameShop();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);

        void SetPos(int x, int y);
        const POINT& GetPos() { return m_Pos; }

        bool Render();
        bool Update();
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool BtnProcess();
        void SetBtnInfo();
        float GetLayerDepth() { return 10.08f; }

        bool GetItemRotation() { return m_ItemAngle; }
        void SetItemRotation(bool _bInput) { m_ItemAngle = _bInput; }

        void OpeningProcess();
        void ClosingProcess();

        void Release();

        bool IsInGameShopOpen();
        bool IsInGameShop();

        void InitZoneBtn();
        void InitCategoryBtn();

        void AddStorageItem(int iStorageSeq, int iStorageItemSeq, int iStorageGroupCode, int iProductSeq, int iPriceSeq, int iCashPoint, wchar_t chItemType, wchar_t* pszUserName = NULL, wchar_t* pszMessage = NULL);

        void ClearAllStorageItem();

        void InitStorage(int iTotalItemCnt, int iCurrentPageItemCnt, int iTotalPage, int iCurrentPage);
        char GetCurrentStorageCode();

        void StoragePrevPage();
        void StorageNextPage();
        void UpdateStorageItemList();

        void InitBanner(wchar_t* pszFileName, wchar_t* pszBannerURL);
        void ReleaseBanner();

        void SetRateScale(int _ItemType);
        float GetRateScale() { return m_fRate_Scale; }
        void SetConvertInvenCoord(WORD _ItemType, float _Width, float _Height);
        POINT GetConvertPos() { return m_fRePos; }
        POINT GetConvertSize() { return m_fReSize; }
        bool IsInGameShopRect(float _x, float _y);

    private:
        float m_fRate_Scale;
        POINT m_fRePos;
        POINT m_fReSize;

    private:
        void Init();
        void LoadImages();
        void UnloadImages();
        void RenderFrame();
        void RenderTexts();
        void RenderButtons();
        void RenderListBox();
        void RenderDisplayItems();

        void RenderBanner();
        bool UpdateBanner();

    private:
        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;
        bool m_ItemAngle;

        CNewUIRadioGroupButton	m_ZoneButton;
        CNewUIRadioGroupButton	m_CategoryButton;
        CNewUIRadioGroupButton	m_ListBoxTabButton;
        CNewUIButton	m_ViewDetailButton[INGAMESHOP_DISPLAY_ITEMLIST_SIZE];
        CNewUIButton	m_CashGiftButton;
        CNewUIButton	m_CashChargeButton;
        CNewUIButton	m_CashRefreshButton;
        CNewUIButton	m_UseButton;
        CNewUIButton	m_PrevButton;
        CNewUIButton	m_NextButton;
        CNewUIButton	m_CloseButton;
        CNewUIButton	m_StoragePrevButton;
        CNewUIButton	m_StorageNextButton;

        bool m_bLoadBanner;
        bool m_bBannerLink;
        wchar_t m_szBannerURL[INTERNET_MAX_URL_LENGTH];

        int		m_iStorageTotalItemCnt;
        int		m_iStorageCurrentPageItemCnt;
        int		m_iStorageTotalPage;
        int		m_iStorageCurrentPage;
        int		m_iSelectedStorageItemIndex;

        int		m_iStorageCurrentPageReceiveItemCnt;
        bool	m_bRequestCurrentPage;

        CUIInGameShopListBox	m_StorageItemListBox;
    };
}

#endif //PBG_ADD_INGAMESHOP_UI_ITEMSHOP
#endif // !defined(AFX_NEWUIINGAMESHOP_H__AE3CE531_70BE_4CBB_9938_0D80B26F21A8__INCLUDED_)
