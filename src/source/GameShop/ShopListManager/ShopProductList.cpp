//************************************************************************
//
// FILE: ShopProductList.cpp
// Removed #ifdef _WIN32 guard (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopProductList.h"

// cppcheck-suppress uninitMemberVar
CShopProductList::CShopProductList()
{
    this->PriceSeqKey = -1;
    this->m_Products.clear();
}
CShopProductList::~CShopProductList() {}

void CShopProductList::Clear()
{
    this->m_Products.clear();
}

int CShopProductList::GetSize()
{
    return static_cast<int>(this->m_Products.size());
}

void CShopProductList::Append(CShopProduct product)
{
    this->m_Products.insert(std::make_pair(product.ProductSeq, product));
}

void CShopProductList::SetFirst()
{
    this->m_ProductIter = this->m_Products.begin();
}

bool CShopProductList::GetNext(CShopProduct& product)
{
    if (this->m_ProductIter == this->m_Products.end())
        return false;

    product = this->m_ProductIter->second;
    this->m_ProductIter++;

    return true;
}

void CShopProductList::SetProductSeqFirst(int ProductSeq)
{
    this->m_ProductRange = this->m_Products.equal_range(ProductSeq);

    this->m_ProductSeqIter = this->m_ProductRange.first;
}

bool CShopProductList::GetProductSeqNext(CShopProduct& product)
{
    if (this->m_ProductSeqIter == this->m_ProductRange.second)
        return false;

    product = this->m_ProductSeqIter->second;

    this->m_ProductSeqIter++;

    return true;
}

void CShopProductList::SetPriceSeqFirst(int ProductSeq, int PriceSeq)
{
    this->PriceSeqKey = PriceSeq;

    this->m_PriceRange = this->m_Products.equal_range(ProductSeq);

    this->m_PriceSeqIter = this->m_PriceRange.first;
}

bool CShopProductList::GetPriceSeqNext(CShopProduct& product)
{
    if (this->m_PriceSeqIter == this->m_PriceRange.second)
        return false;

    if (this->PriceSeqKey == this->m_PriceSeqIter->second.PriceSeq)
    {
        product = this->m_PriceSeqIter->second;
        this->m_PriceSeqIter++;
        return true;
    }

    this->m_PriceSeqIter++;

    return this->GetPriceSeqNext(product);
}
#endif
