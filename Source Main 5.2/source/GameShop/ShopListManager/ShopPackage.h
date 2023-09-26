/**************************************************************************************************

페키지 객체

현재 페키지 정보를 가지고 있다.
현재 페키지에 등록 되어있는 "상품 번호" 목록과 "가격 번호" 목록을 가지고 있다.

패키지에 여러 상품이 들어있는 경우에는 가격 번호가 한 개 이다.
패키지에 여러 가격이 설정되어 있다면 상품은 한 개 이다.

(exe 1)
A 페키지에 a와 a' 라는 상품이 두 개 있다면 여러 가격이 설정 될 수 없다.
-> A상품 a, a' 5000원

(exe 2)
B 페키지에 b 라는 상품이 하나만 있다면 1000원, 2000원, 3000원 으로 여러 가격이 설정 될 수 있다.
-> B상품 b 1주일 1000원
-> B상품 b 2주일 2000원
-> B상품 b 4주일 3000원
세 가지를 다른 페키지로 보여주어야 한다.

**************************************************************************************************/

#pragma once

#include "include.h"
#include <time.h>

class CShopPackage
{
public:
    CShopPackage();
    virtual ~CShopPackage();

    bool	SetPackage(std::wstring strdata);
    void	SetLeftCount(int nCount);

    int		GetProductCount();									// 패키지 내부의 상품 수 가져오기
    void	SetProductSeqFirst();								// 패키지 내부의 첫번째 상품 번호에 위치
    bool	GetProductSeqFirst(int& ProductSeq);				// 패키지 내부의 첫번째 상품 번호를 가져오고 다음 상품 번호로 이동
    bool	GetProductSeqNext(int& ProductSeq);					// 상품 번호 가져오고 다음 위치로 이동

    int		GetPriceCount();									// 패키지 내부의 가격 수 가져오기
    void	SetPriceSeqFirst();									// 패키지 내부의 첫번째 가격 번호에 위치
    bool	GetPriceSeqFirst(int& PriceSeq);					// 패키지 내부의 첫번째 가격 번호를 가져오고 다음 가격 번호로 이동
    bool	GetPriceSeqNext(int& PriceSeq);						// 가격 번호 가져오고 다음 위치로 이동

public:
    int		ProductDisplaySeq;									//  1. 패키지가 속해있는 카테고리 번호
    int		ViewOrder;											//  2. 노출 순서
    int		PackageProductSeq;									//  3. 패키지 번호
    wchar_t	PackageProductName[SHOPLIST_LENGTH_PACKAGENAME];	//  4. 패키지 명
    int		PackageProductType;									//  5. 패키지 유형 (170:일반 상품, 171:이벤트 상품)
    int		Price;												//  6. 가격
    wchar_t	Description[SHOPLIST_LENGTH_PACKAGEDESC];			//  7. 상세 설명
    wchar_t	Caution[SHOPLIST_LENGTH_PACKAGECAUTION];			//  8. 주의 사항
    int		SalesFlag;											//  9. 구매 가능 여부(구매버튼 노출여부) (182:가능, 183:불가)
    int		GiftFlag;											// 10. 선물 가능 여부(선물버튼 노출여부) (184:가능, 185:불가)
    tm		StartDate;											// 11. 판매 시작일
    tm		EndDate;											// 12. 판매 종료일
    int		CapsuleFlag;										// 13. 캡슐 상품 구분 (176:캡슐, 177:일반)
    int		CapsuleCount;										// 14. 패키지에 포함된 상품 개수
    wchar_t	ProductCashName[SHOPLIST_LENGTH_PACKAGECASHNAME];	// 15. 소진 캐시 명
    wchar_t	PricUnitName[SHOPLIST_LENGTH_PACKAGEPRICEUNIT];		// 16. 가격 단위 표시 명
    int		DeleteFlag;											// 17. 삭제 여부 (180:삭제, 181:활성)
    int		EventFlag;											// 18. 이벤트 상품 여부 (199:이벤트 상품, 200:일반 상품)
    int		ProductAmount;										// 19. 한정 상품 여부
    wchar_t	InGamePackageID[SHOPLIST_LENGTH_INGAMEPACKAGEID];	// 21. 패키지 아이템 코드
    int		ProductCashSeq;										// 22. 소진 캐시 유형 코드
    int		PriceCount;											// 23. 단위 상품 가격 정책 보유 개수 (단위 상품이 1개인 경우에만 PriceSeq가 여러 개일 수 있다.)
    bool	DeductMileageFlag;									// 25. 마일리지로 차감 상품 여부 (false : 일반, true : 마일리지 차감 상품)
    int		CashType;											// 26. 글로벌 전용 : Wcoin(C), WCoin(P) 구분
    int		CashTypeFlag;										// 27. 글로벌 전용 : Wcoin(C), WCoin(P) 선택 or 자동 여부(668: 개인선택, 669: 자동차감)

    int		LeftCount;											// 잔여 개수

private:
    void	SetProductSeqList(std::wstring strdata);
    void	SetPriceSeqList(std::wstring strdata);

    std::vector<int> ProductSeqList;							// 20. 패키지에 포함된 상품 번호 목록
    std::vector<int>::iterator ProductSeqIter;

    std::vector<int> PriceSeqList;								// 24. 패키지에 포함되는 가격 번호 목록 (상품이 1개인 경우에만 값을 제공.)
    std::vector<int>::iterator PriceSeqIter;
};
