
/**************************************************************************************************

전체 상품(속성) 목록 객체

iterator를 이용하여 순차적으로 상품(속성) 객체를 가져올 수 있다.
상품 번호를 이용하여 상품(속성) 객체를 가져올 수 있다.
상품 번호와 가격 번호를 이용하여 상품(속성) 객체를 가져올 수 있다.

**************************************************************************************************/

#pragma once

#include "ShopProduct.h"

class CShopProductList
{
public:
	CShopProductList(void);
	~CShopProductList(void);

	void Clear();	
	int GetSize();	

	virtual void Append(CShopProduct product);

	void SetFirst();
	bool GetNext(CShopProduct& product);

	void SetProductSeqFirst(int ProductSeq);				// 해당 상품 번호의 상품(속성) 목록 첫 번째 위치
	bool GetProductSeqNext(CShopProduct& product);			// 상품(속성) 객체를 가져오고 다음 상품(속성)에 위치

	void SetPriceSeqFirst(int ProductSeq, int PriceSeq);	// 해당 상품 번호와 가격 번호의 조합으로 상품(속성) 목록 첫 번째 위치
	bool GetPriceSeqNext(CShopProduct& product);			// 상품(속성) 객체를 가져오고 다음 상품(속성)에 위치

protected:	
	int	PriceSeqKey;
	std::multimap<int, CShopProduct> m_Products;	
	std::multimap<int, CShopProduct>::iterator m_ProductIter;
	std::multimap<int, CShopProduct>::iterator m_ProductSeqIter;
	std::multimap<int, CShopProduct>::iterator m_PriceSeqIter;	
	std::pair<std::multimap<int, CShopProduct>::iterator, std::multimap<int, CShopProduct>::iterator> m_ProductRange;
	std::pair<std::multimap<int, CShopProduct>::iterator, std::multimap<int, CShopProduct>::iterator> m_PriceRange;
};
