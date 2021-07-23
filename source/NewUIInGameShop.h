// NewUIInGameShop.h: interface for the NewUIInGameShop class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIINGAMESHOP_H__AE3CE531_70BE_4CBB_9938_0D80B26F21A8__INCLUDED_)
#define AFX_NEWUIINGAMESHOP_H__AE3CE531_70BE_4CBB_9938_0D80B26F21A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		enum LISTBOX_INDEX
		{
			IGS_SAFEKEEPING_LISTBOX = 0,
			IGS_PRESENTBOX_LISTBOX,
			IGS_TOTAL_LISTBOX,
		};
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT

		enum IMAGE_LIST
		{
			IMAGE_IGS_EXIT_BTN	= CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,	// newui_exit_00.tga (36, 58) - 2BtState
			IMAGE_IGS_BACK		= BITMAP_INGAMESHOP_FRAME,	// Ingame_shopback.jpg (640, 429)
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
#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT													// #ifndef
			IMAGE_IGS_DELETE_BTN,				// Ingame_Bt_Delete.tga (25, 75) - 3BtState
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_ITEMBOX_LOGO,				// Ingame_Itembox_logo.tga (57, 57)
			IMAGE_IGS_PAGE_LEFT,				// Ingame_Bt_page_L.tga (20, 69) - 3BtState
			IMAGE_IGS_PAGE_RIGHT,				// Ingame_Bt_page_R.tga (20, 69) - 3BtState
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_STORAGE_PAGE,				// IGS_Storage_Page.tga (80, 30)
			IMAGE_IGS_STORAGE_PAGE_LEFT,		// IGS_Storage_Page_Left.tga (20, 22) - 3BtState
			IMAGE_IGS_STORAGE_PAGE_RIGHT,		// IGS_Storage_Page_Right.tga (20, 22) - 3BtState
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_BANNER	= BITMAP_INGAMESHOP_BANNER	// 배너 이미지(152,63) - 파일명은 바뀜
		};
		
	private:
		enum INGAMESHOP_TEXT_INFO
		{
			TEXT_IGS_CHAR_NAME_POS_X	= 498,	// Character 정보
			TEXT_IGS_CHAR_NAME_POS_Y	= 23,
			TEXT_IGS_CHAR_NAME_WIDTH	= 122,
			TEXT_IGS_CASH_POS_X		= 498,		// Cash 정보
			TEXT_IGS_CASH_POS_Y		= 50,
			TEXT_IGS_CASH_WIDTH		= 130,
			TEXT_IGS_MILEAGE_POS_Y	= 65,		// Point 정보
			TEXT_IGS_POINT_POS_Y	= 80,		// Mileage 정보
			TEXT_IGS_STORAGE_NAME_POS_X	= 492,	// 보관함제목 - 이름
			TEXT_IGS_STORAGE_NAME_POS_Y = 233,
			TEXT_IGS_STORAGE_NAME_WIDTH	= 96,
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			TEXT_IGS_STORAGE_TIME_POS_X	= 592,	// 보관함제목 - 기간
			TEXT_IGS_STORAGE_TIME_WIDTH = 34,
			TEXT_IGS_PAGE_POS_X		= 251,		// Page 정보
			TEXT_IGS_PAGE_POS_Y		= 404,
			TEXT_IGS_STORAGE_PAGE_INFO_POS_X	= 518,	// 보관함 Page 정보
			TEXT_IGS_STORAGE_PAGE_INFO_POS_Y	= 376,
#else // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			TEXT_IGS_STORAGE_TIME_POS_X	= 591,	// 보관함제목 - 기간
			TEXT_IGS_STORAGE_TIME_WIDTH = 26,
			TEXT_IGS_PAGE_POS_X		= 264,		// Page 정보
			TEXT_IGS_PAGE_POS_Y		= 404,
			TEXT_IGS_PAGE_WIDTH		= 30,	
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		};
		
		enum INGAMESHOP_IMAGES_POS
		{
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_EXIT_BTN_POS_X	= 484,			// Exit Button
			IMAGE_IGS_EXIT_BTN_POS_Y	= 392,
#else // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_EXIT_BTN_POS_X	= 487,			// Exit Button
			IMAGE_IGS_EXIT_BTN_POS_Y	= 395,
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_BACK_POS_X		= 0,			// InGameShop Back
			IMAGE_IGS_BACK_POS_Y		= 0,
			IMAGE_IGS_CATEGORY_BTN_POS_X	= 13,		// Category Button
			IMAGE_IGS_CATEGORY_BTN_POS_Y	= 31,
			IMAGE_IGS_CATEGORY_BTN_DISTANCE	= 6,
			IMAGE_IGS_TAB_BTN_POS_X			= 486,		// Tab Button
			IMAGE_IGS_TAB_BTN_POS_Y			= 208,
			IMAGE_IGS_TAB_BTN_DISTANCE		= -2,
			IMAGE_IGS_ZONE_BTN_POS_X		= 95,		// Zone Button
			IMAGE_IGS_ZONE_BTN_POS_Y		= 0,
			IMAGE_IGS_VIEWDETAIL_BTN_POS_X	= 162,		// View Detail Button
			IMAGE_IGS_VIEWDETAIL_BTN_POS_Y	= 126,
			IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_X	= 122,
			IMAGE_IGS_VIEWDETAIL_BTN_DISTANCE_Y	= 121,
			IMAGE_IGS_ITEMGIFT_BTN_POS_X	= 519,		// Item Gift Button
			IMAGE_IGS_CASHGIFT_BTN_POS_X	= 546,		// Cash Gift Button
			IMAGE_IGS_REFRESH_BTN_POS_X		= 573,		// Refresh Button
			IMAGE_IGS_ICON_BTN_POS_Y		= 94,		// 각종 아이콘(Gift, Cash, Refrash) Button
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_USE_BTN_POS_X			= 572,		// 사용하기 버튼
			IMAGE_IGS_USE_BTN_POS_Y			= 396,
#else // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_USE_BTN_POS_X			= 486,		// 사용하기 버튼
			IMAGE_IGS_USE_BTN_POS_Y			= 366,										
			IMAGE_IGS_DELETE_BTN_POS_X		= 540,		// 보관함 아이템 삭제하기 버튼
			IMAGE_IGS_DELETE_BTN_POS_Y		= 366,
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_ITEMBOX_LOGO_POS_X	= 128,		// 아이템박스 로고
			IMAGE_IGS_ITEMBOX_LOGO_POS_Y	= 52,
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_PAGE_LEFT_POS_X		= 231,		// Page Left Button
			IMAGE_IGS_PAGE_RIGHT_POS_X		= 307,		// Page Right Button
#else // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_PAGE_LEFT_POS_X		= 242,		// Page Left Button
			IMAGE_IGS_PAGE_RIGHT_POS_X		= 295,		// Page Right Button
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_PAGE_BUTTON_POS_Y		= 397,
 			IMAGE_IGS_BANNER_POS_X			= 482,		// Banner
 			IMAGE_IGS_BANNER_POS_Y			= 133,
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMAGE_IGS_STORAGE_PAGE_POS_X		= 518,		// Storage Page
			IMAGE_IGS_STORAGE_PAGE_POS_Y		= 366,
			IMAGE_IGS_STORAGE_PAGE_LEFT_POS_X	= 512,		// Storage Page Left
			IMAGE_IGS_STORAGE_PAGE_RIGHT_POS_X	= 586,		// Storage Page Right
			IMAGE_IGS_STORAGE_PAGE_BTN_POS_Y	= 372,
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		};
		
		enum INGAMESHOP_IMAGES_SIZE
		{
			IMAGE_IGS_EXIT_BTN_WIDTH		= 36,		// Exit Button
			IMAGE_IGS_EXIT_BTN_HEIGHT		= 29,
			IMAGE_IGS_BACK_WIDTH			= 640,		// InGameShop Back

#ifdef MOD_INGAMESHOP_HEIGHT_MODIFY
			IMAGE_IGS_BACK_HEIGHT			= 480,
#else //MOD_INGAMESHOP_HEIGHT_MODIFY
			IMAGE_IGS_BACK_HEIGHT			= 429,
#endif //MOD_INGAMESHOP_HEIGHT_MODIFY

			IMAGE_IGS_CATEGORY_BTN_WIDTH	= 73,		// Category Button
			IMAGE_IGS_CATEGORY_BTN_HEIGHT	= 27,
			IMAGE_IGS_CATEGORY_DECO_MIDDLE_WIDTH	= 4,	// Category Deco Middle
			IMAGE_IGS_CATEGORY_DECO_MIDDLE_HEIGHT	= 8,
			IMAGE_IGS_CATEGORY_DECO_DOWN_WIDTH		= 47,	// Category Deco Down
			IMAGE_IGS_CATEGORY_DECO_DOWN_HEIGHT		= 100,
			IMAGE_IGS_TAB_BTN_WIDTH			= 49,		// Tab Button
			IMAGE_IGS_TAB_BTN_HEIGHT		= 20,
			IMAGE_IGS_ZONE_BTN_WIDTH		= 76,		// Zone Button
			IMAGE_IGS_ZONE_BTN_HEIGHT		= 23, 
			IMAGE_IGS_VIEWDETAIL_BTN_WIDTH	= 52,		// View Detail Button
			IMAGE_IGS_VIEWDETAIL_BTN_HEIGHT	= 26,
			IMAGE_IGS_ICON_BTN_WIDTH		= 25,		// 각종 아이콘(Gift, Cash, Refrash, Delete) Button
			IMAGE_IGS_ICON_BTN_HEIGHT		= 25,
			IMAGE_IGS_ITEMBOX_LOGO_SIZE		= 57,		// 아이템박스 로고
			IMAGE_IGS_PAGE_BTN_WIDTH		= 20,		// Page
			IMAGE_IGS_PAGE_BTN_HEIGHT		= 23,
			IMAGE_IGS_BANNER_WIDTH			= 153,		// Banner
			IMAGE_IGS_BANNER_HEIGHT			= 63,
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IMGAE_IGS_STORAGE_PAGE_WIDTH	= 80,		// Storage Page
			IMGAE_IGS_STORAGE_PAGE_HEIGHT	= 30,
			IMGAE_IGS_STORAGE_PAGE_BTN_WIDTH	= 20,	// Storage Page Btn
			IMGAE_IGS_STORAGE_PAGE_BTN_HEIGHT	= 22,
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		};
		
		enum INGAMESHOP_DISPLAY_ITEMS
		{
			IGS_WIDTH_POS_X		= 129,		// Display할 Item의 시작위치
			IGS_HEIGHT_POS_Y	= 59,		
			IGS_SIZE_WIDTH		= 60,		// Display할 Item의 크기
			IGS_SIZE_HEIGHT		= 49,		
			//IGS_WIDTH_DISTANCE	= 126,		// Display할 Item들 사이의 간격
			//IGS_HEIGHT_DISTANCE	= 126,		
			IGS_NUM_ITEMS_WIDTH		= 3,	// Display할 Item의 갯수
			IGS_NUM_ITEMS_HEIGHT	= 3,	
			IGS_PACKAGE_NAME_POS_X	= 105,	// Display할 Item의 Name 시작위치
			IGS_PACKAGE_NAME_POS_Y	= 40,
			IGS_PACKAGE_NAME_WIDTH	= 104,	// Display할 Item의 Name 범위
			IGS_PACKAGE_PRICE_POS_Y	= 60,	// Displat할 Item의 가격 시작위치
			IGS_ITEMRENDER_POS_X_STANDAD	= 102,	// 아템 랜더할 기준
			IGS_ITEMRENDER_POS_WIDTH		= 108,	// 아템 그릴 박스
			IGS_ITEMRENDER_POS_Y_STANDAD	= 51,	// 아템 랜더할 기준
			IGS_ITEMRENDER_POS_HEIGHT		= 58,	// 아템 그릴 박스
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
			IGS_STORAGE_TOTAL_ITEM_PER_PAGE	= 9,	// 보관함 페이지당 아이템의 갯수
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		};
		
	public:
		CNewUIInGameShop();
		virtual ~CNewUIInGameShop();
		
		bool Create(CNewUIManager* pNewUIMng, int x, int y);	//UI메니져 등록 및 로드
		
		void SetPos(int x, int y);
		const POINT& GetPos() { return m_Pos; }
		
		bool Render();
		bool Update();
		bool UpdateMouseEvent();	//마우스관련
		bool UpdateKeyEvent();		//키관련
		bool BtnProcess();			//버튼관련
		void SetBtnInfo();
		float GetLayerDepth() { return 10.08f; }
		
		bool GetItemRotation() { return m_ItemAngle; }	//아템 회전하기
		void SetItemRotation(bool _bInput) { m_ItemAngle = _bInput; }
		
		void OpeningProcess();	//열고닫을때의 프로세스
		void ClosingProcess(); 
		
		void Release();
		
		bool IsInGameShopOpen();		// 안전지대와 마을에서만 열릴수있도록 한다.
		bool IsInGameShop();			// 인게임샵이 열렸는가?
		
		void InitZoneBtn();			// 상위카테고리(Zone) 버튼 초기화
		void InitCategoryBtn();		// 하위카테고리(Category) 버튼 초기화
		
		// 보관함/선물함
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		void AddStorageItem(int iStorageSeq, int iStorageItemSeq, int iStorageGroupCode, int iProductSeq, int iPriceSeq,
							int iCashPoint, unicode::t_char chItemType, unicode::t_char* pszUserName = NULL, unicode::t_char* pszMessage = NULL);
#else // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		void AddStorageItem(int iListBoxIndex, int iStorageSeq, int iStorageItemSeq, int iStorageGroupCode, int iProductSeq, 
			int iPriceSeq, int iCashPoint, unicode::t_char chItemType, unicode::t_char* pszUserName = NULL, unicode::t_char* pszMessage = NULL);

		void DelStorageItem(int iStorageIndex, int iIndex);
		void DelStorageSelectedItem();
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT

		void ClearAllStorageItem();

#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		void InitStorage(int iTotalItemCnt, int iCurrentPageItemCnt, int iTotalPage, int iCurrentPage );
		char GetCurrentStorageCode();				// 현재 보관함종류의 코드 return

		void StoragePrevPage();
		void StorageNextPage();
		void UpdateStorageItemList();
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		
		// 배너 관련
		void InitBanner(unicode::t_char* pszFileName, unicode::t_char* pszBannerURL);
		void ReleaseBanner();
		
#ifdef PBG_ADD_ITEMRESIZE
		// 인벤에서의 사이즈대로 변경하기 위함
		void SetRateScale(int _ItemType);
		float GetRateScale() { return m_fRate_Scale; }
		// 인벤의 좌표값으로 변경
		void SetConvertInvenCoord(WORD _ItemType, float _Width, float _Height);
		POINT GetConvertPos()	{ return m_fRePos; }
		POINT GetConvertSize()	{ return m_fReSize; }
		// 인게임샵의 범위안에 있는가?
		bool IsInGameShopRect(float _x,float _y);

	private:
		float m_fRate_Scale;
		// 인벤 비율의 좌표 변경위함
		POINT m_fRePos;
		POINT m_fReSize;
#endif //PBG_ADD_ITEMRESIZE
		
	private:
		void Init();
		void LoadImages();
		void UnloadImages();
		void RenderFrame();		//전체 프레임
		void RenderTexts();		//텍스트 찍기
		void RenderButtons();	//버튼그리기
		void RenderListBox();						//리스트 박스랜더링
		void RenderDisplayItems();		// 전시될 아이템 Render

		void RenderBanner();
		bool UpdateBanner();
		
	private:
		CNewUIManager* m_pNewUIMng;
		POINT m_Pos;
		bool m_ItemAngle;				//인터페이스에 그려질 아이템
		
		CNewUIRadioGroupButton	m_ZoneButton;		// 상위 카테고리(Zone) 버튼
		CNewUIRadioGroupButton	m_CategoryButton;	// 하위 카테고리(Category) 버튼
		CNewUIRadioGroupButton	m_ListBoxTabButton;	// 보관함 Tab 버튼 - Radio버튼 활성화 상태가 현재 보관함의 Index이다.
		CNewUIButton	m_ViewDetailButton[INGAMESHOP_DISPLAY_ITEMLIST_SIZE];	// 상세보기 버튼
		CNewUIButton	m_CashGiftButton;		// 아이템선물하기 버튼
		CNewUIButton	m_CashChargeButton;		// 캐시충전하기 버튼
		CNewUIButton	m_CashRefreshButton;	// 캐시Refresh 버튼
		CNewUIButton	m_UseButton;			// 사용하기 버튼
#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT				// #ifndef
		CNewUIButton	m_DeleteButton;			// 보관함 아이템 지우기 버튼
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		CNewUIButton	m_PrevButton;			// 이전 버튼
		CNewUIButton	m_NextButton;			// 다음 버튼
		CNewUIButton	m_CloseButton;			// 닫기버튼	

#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		CNewUIButton	m_StoragePrevButton;			// 보관함 이전 버튼
		CNewUIButton	m_StorageNextButton;			// 보관함 다음 버튼
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		
		bool m_bLoadBanner;
		bool m_bBannerLink;
		unicode::t_char m_szBannerURL[INTERNET_MAX_URL_LENGTH];

#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		// 보관함/선물함 정보
		int		m_iStorageTotalItemCnt;				// 보관함/선물함의 전체 아이템 갯수
		int		m_iStorageCurrentPageItemCnt;		// 보관함/선물함의 현재 페이지 아이템 갯수
		int		m_iStorageTotalPage;				// 전체 보관함/선물함 페이지
		int		m_iStorageCurrentPage;				// 현재 보관함/선물함 페이지
		int		m_iSelectedStorageItemIndex;		// 보관함/선물함에 선택된 아이템 Index

		int		m_iStorageCurrentPageReceiveItemCnt;	// 현재 서버로부터 받고있는 아이템 카운트
		bool	m_bRequestCurrentPage;

		// 현재 선택된 보관함의 아이템 리스트 박스
		CUIInGameShopListBox	m_StorageItemListBox;
#else // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		CUIInGameShopListBox	m_InGameShopListBox[CInGameShopSystem::IGS_TOTAL_LISTBOX];	
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
	};
}
	
#endif //PBG_ADD_INGAMESHOP_UI_ITEMSHOP
	
#endif // !defined(AFX_NEWUIINGAMESHOP_H__AE3CE531_70BE_4CBB_9938_0D80B26F21A8__INCLUDED_)
