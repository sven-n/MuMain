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

    void SetProductSeqFirst(int ProductSeq);
    bool GetProductSeqNext(CShopProduct& product);

    void SetPriceSeqFirst(int ProductSeq, int PriceSeq);
    bool GetPriceSeqNext(CShopProduct& product);

protected:
    int	PriceSeqKey;
    std::multimap<int, CShopProduct> m_Products;
    std::multimap<int, CShopProduct>::iterator m_ProductIter;
    std::multimap<int, CShopProduct>::iterator m_ProductSeqIter;
    std::multimap<int, CShopProduct>::iterator m_PriceSeqIter;
    std::pair<std::multimap<int, CShopProduct>::iterator, std::multimap<int, CShopProduct>::iterator> m_ProductRange;
    std::pair<std::multimap<int, CShopProduct>::iterator, std::multimap<int, CShopProduct>::iterator> m_PriceRange;
};
