#pragma once

#ifdef NEW_STRUCTS

//////////////////////////////////////////////////////////////////////////
// [해더 코드]
//////////////////////////////////////////////////////////////////////////

typedef struct _HEAD2
{
	BYTE c;			// 프로토콜 코드
	BYTE size;		// 프로토콜 크기( 헤더 크기 포함 )
	BYTE headcode;	// 프로토콜 종류
	BYTE subcode;	// 서브 프로토콜 종류

} PBMSG_HEAD2, * LPPBMSG_HEAD2;

//////////////////////////////////////////////////////////////////////////
// [캐쉬샵 아이템 정보] 
//////////////////////////////////////////////////////////////////////////

#pragma pack(1)

typedef struct _CASHSHOP_ITEMLIST
{
	DWORD				s_dwItemGuid;						// 판매 고유번호 1
	BYTE				s_btCategoryCode;					// 탭 코드 1
	BYTE				s_btItemDuration;					// 내구도 (개수)
	BYTE				s_btItemSaleRatio;				    // 할인률 1
	WORD				s_wItemPrice;						// 가격 1
	BYTE				s_btSpecialOption;				    // 특수 옵션 (신규, 히트 아이템) 1
	BYTE				s_btItemInfo[ITEM_BUFFER_SIZE];	    // 아이템 정보를 클라이언트에 보내 줄 7 바이트 포멧으로 변환
	DWORD				s_itemTime;							// 기간제 아이템 때문에 넣었음. 내구도 255 밖에 안 됨..--;;;;

	_CASHSHOP_ITEMLIST() : s_dwItemGuid(0), s_btCategoryCode(0),
		                   s_btItemSaleRatio(0), s_wItemPrice(0),
						   s_btSpecialOption(0), s_itemTime(0)
	
	{
		for( int i = 0; i < ITEM_BUFFER_SIZE; ++i )
		{
			s_btItemInfo[i] = 0;
		}
	}

	// 16바이트
} CASHSHOP_ITEMLIST, *LPCASHSHOP_ITEMLIST;

//////////////////////////////////////////////////////////////////////////
// [캐쉬샵 오픈 요청 결과] 0xF5, 0x02
//////////////////////////////////////////////////////////////////////////

typedef struct _PMSG_ANS_CASHSHOPOPEN
{
	PBMSG_HEAD2			s_head;

	BYTE				s_btResult;				             // 캐쉬 샵 오픈 요청 결과 ( 0:성공, 1:다른 인터페이스 사용 중, 2:캐쉬샵에 연결할 수 없음, 3:준비되지 않음, 4:네트워크 오류 )
} PMSG_ANS_CASHSHOPOPEN, *LPPMSG_ANS_CASHSHOPOPEN;

//////////////////////////////////////////////////////////////////////////
// [캐쉬 포인트 잔량 요청 결과] 0xF5, 0x04
//////////////////////////////////////////////////////////////////////////

typedef struct _PMSG_ANS_CASHPOINT
{
	PBMSG_HEAD2			s_head;
	int					s_iCashPoint;				              // 캐쉬 잔량
} PMSG_ANS_CASHPOINT, *LPPMSG_ANS_CASHPOINT;

//////////////////////////////////////////////////////////////////////////
// [캐쉬 아이템 리스트 요청 결과] 0xF5, 0x06
//////////////////////////////////////////////////////////////////////////

typedef struct _PMSG_ANS_CASHITEMLIST
{
	PBMSG_HEAD2						s_head;
	BYTE							s_btCategoryIndex;		  // 아이템 리스트 카테고리
	BYTE							s_btPageIndex;			  // 페이지 인덱스
	BYTE							s_btItemCount;			  // 해당 페이지의 아이템 카운트
	CASHSHOP_ITEMLIST				s_ItemInfo[MAX_CASHITEM_LIST_PER_PAGE];	// 아이템 리스트 (한 페이지)
	// 151바이트
} PMSG_ANS_CASHITEMLIST, *LPPMSG_ANS_CASHITEMLIST;

//////////////////////////////////////////////////////////////////////////
// [캐쉬 아이템 구매 요청 결과] 0xF5, 0x08
//////////////////////////////////////////////////////////////////////////

typedef struct _PMSG_ANS_CASHITEM_BUY
{
	PBMSG_HEAD2						s_head;
	BYTE							s_btResult;				   // 아이템 구매 결과
	// 1:성공, 0:캐쉬 잔량 부족, 2:기타 실패
} PMSG_ANS_CASHITEM_BUY, *LPPMSG_ANS_CASHITEM_BUY;

#ifdef PSW_CHARACTER_CARD
//////////////////////////////////////////////////////////////////////////
// [계정 캐쉬아이템 정보] 0xDE, 0x00
//////////////////////////////////////////////////////////////////////////
typedef struct _PMSG_ANS_ACCOUNTINFO
{
	PBMSG_HEAD2		s_head;
	BYTE			s_bSummoner;	// 소환술사 캐릭터 카드 이용 유/무
} PMSG_ANS_ACCOUNTINFO, *LPPMSG_ANS_ACCOUNTINFO;
//////////////////////////////////////////////////////////////////////////
// [계정 캐쉬아이템 구매 결과] 0xDE, 0x01
//////////////////////////////////////////////////////////////////////////
typedef struct _PMSG_ANS_SETACCOUNTINFO
{
	PBMSG_HEAD2	s_head;
	BYTE		s_bResult;
} PMSG_ANS_SETACCOUNTINFO, *LPPMSG_ANS_SETACCOUNTINFO;

#endif // PSW_CHARACTER_CARD

///////////////////////////////////////////////////////////////////////////
//Client Code[ 상점 카테고리 ]
///////////////////////////////////////////////////////////////////////////

typedef struct _SHOPCATEGORY
{
	int     s_ShopType;
	int		s_HighCategory;                                    // 상위 카테고리 위치
	int		s_LowCategory;                                     // 하위 카테고리 위치
	char	s_ButtonName[STRUCTSMEMBERNAME];                   // 버튼 이름

	_SHOPCATEGORY() : s_ShopType( -1 ), s_HighCategory( -1 ), s_LowCategory( -1 )
	{
		memset( &s_ButtonName, 0, sizeof(char)*STRUCTSMEMBERNAME );
	}
} SHOPCATEGORY;

typedef struct _SHOPCATEGORY_UNICODE
{
	int     s_ShopType;
	int		s_HighCategory;                                    // 상위 카테고리 위치
	int		s_LowCategory;                                     // 하위 카테고리 위치
	wchar_t	s_ButtonName[STRUCTSMEMBERNAME];                   // 버튼 이름

	_SHOPCATEGORY_UNICODE() : s_ShopType( -1 ), s_HighCategory( -1 ), s_LowCategory( -1 )
	{
		memset( &s_ButtonName, 0, sizeof(wchar_t)*STRUCTSMEMBERNAME );
	}
} SHOPCATEGORY_UNICODE;




///////////////////////////////////////////////////////////////////////////
//Client Code[ 상점 아이템 ]
///////////////////////////////////////////////////////////////////////////

typedef struct _SHOPCATEGORYITEM
{
	WORD    s_bShoplistCode;     // 상점 구분 :      ( 0 ) 일반 상점, ( 1 ) 부분 유료화 상점
	WORD	s_HighCategory;	     // 상위 카테고리 위치
	WORD	s_LowCategory;       // 하위 카테고리 위치

	WORD	s_bItemCodeType;     // 아이템 상위 타입  MAX_ITEM_TYPE  = 16개
	WORD	s_bItemCodeIndex;    // 아이템 하위 타입  MAX_ITEM_INDEX = 512개

	WORD	s_bItemCode;		 // 아이템 코드	( ShopServer에서 정해줌 )
	WORD	s_bItemCodeLevel;    // 아이템 레벨
	WORD	s_btAddOption;       // 일반 옵션
	WORD	s_btExOption;        // 엘셀 옵션

	WORD    s_wNeedMoney;

	_SHOPCATEGORYITEM() : s_bShoplistCode(-1), s_HighCategory(-1), s_LowCategory(-1), s_bItemCode( -1 ), s_bItemCodeType(-1), s_bItemCodeIndex(-1),
					     s_bItemCodeLevel(-1), s_btAddOption(-1), s_btExOption(-1), s_wNeedMoney(-1) {}
} SHOPCATEGORYITEM;

typedef struct _MESSAGEBOXHANDLER
{
    virtual ~_MESSAGEBOXHANDLER() = 0{}
    virtual void OnMessageBox_OK( int id ){}
    virtual void OnMessageBox_YES( int id ){}
    virtual void OnMessageBox_NO( int id ){}
    virtual void OnMessageBox_CANCEL( int id ){}
} MESSAGEBOXHANDLER;

typedef struct _MESSAGEBOXDATA 
{
	weak_ptr<MESSAGEBOXHANDLER>		s_handler;
	eMessageBoxType					s_type;
	list<string>					s_message;
	int								s_id;
	bool							s_bModal;

	_MESSAGEBOXDATA() : s_type(), s_bModal( false ), s_id( -1 ) {}
} MESSAGEBOXDATA;

typedef list<SHOPCATEGORY>										ShopCategory_List;
typedef list<SHOPCATEGORY_UNICODE>								ShopCategory_Unicode_List;
typedef list<SHOPCATEGORYITEM>									ShopCategoryItem_List;
typedef list<CASHSHOP_ITEMLIST>									ShopCashShopItem_List;

typedef map< eHighCategoryType, ShopCategory_List >				ShopCategory_Map;
typedef map< eHighCategoryType, ShopCategory_Unicode_List >     ShopCategory_Unicode_Map;
typedef map< WORD, SHOPCATEGORYITEM >							ShopCategoryItem_Map;
typedef map< WORD, SHOPCATEGORYITEM >							ShopMyCartItem_Map;
typedef map< WORD, CASHSHOP_ITEMLIST >							ChashShopMyCartItem_Map;;

typedef map< WORD, ShopCategoryItem_List >						ShopPageItem_Map;
typedef map< eLowCategoryType, ShopPageItem_Map >				ShopItem_Map;

typedef map< WORD, ShopCashShopItem_List >						ShopChashShopPageItem_Map;
typedef map< eLowCategoryType, ShopChashShopPageItem_Map >		ShopChashShopItem_Map;

#pragma pack()

#endif //NEW_STRUCTS
