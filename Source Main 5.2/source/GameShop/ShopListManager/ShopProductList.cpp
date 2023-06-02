//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: ShopProductList.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopProductList.h"

CShopProductList::CShopProductList() // OK
{
    this->PriceSeqKey = -1;
    this->m_Products.clear();
}
CShopProductList::~CShopProductList() // OK
{
}

void CShopProductList::Clear() // OK
{
    this->m_Products.clear();
}

int CShopProductList::GetSize() // OK
{
    return static_cast<int>(this->m_Products.size());
}

void CShopProductList::Append(CShopProduct product) // OK
{
    this->m_Products.insert(std::make_pair(product.ProductSeq, product));
}

void CShopProductList::SetFirst() // OK
{
    this->m_ProductIter = this->m_Products.begin();
}

bool CShopProductList::GetNext(CShopProduct& product) // OK
{
    if (this->m_ProductIter == this->m_Products.end())
        return 0;

    product = this->m_ProductIter->second;
    this->m_ProductIter++;

    return 1;
}

void CShopProductList::SetProductSeqFirst(int ProductSeq) // OK
{
    this->m_ProductRange = this->m_Products.equal_range(ProductSeq);

    this->m_ProductSeqIter = this->m_ProductRange.first;
}

bool CShopProductList::GetProductSeqNext(CShopProduct& product) // OK
{
    if (this->m_ProductSeqIter == this->m_ProductRange.second)
        return 0;

    product = this->m_ProductSeqIter->second;

    this->m_ProductSeqIter++;

    return 1;
}

void CShopProductList::SetPriceSeqFirst(int ProductSeq, int PriceSeq) // OK
{
    this->PriceSeqKey = PriceSeq;

    this->m_PriceRange = this->m_Products.equal_range(ProductSeq);

    this->m_PriceSeqIter = this->m_PriceRange.first;
}

bool CShopProductList::GetPriceSeqNext(CShopProduct& product) // OK
{
    if (this->m_PriceSeqIter == this->m_PriceRange.second)
        return 0;

    if (this->PriceSeqKey == this->m_PriceSeqIter->second.PriceSeq)
    {
        product = this->m_PriceSeqIter->second;
        this->m_PriceSeqIter++;
        return 1;
    }

    this->m_PriceSeqIter++;

    return this->GetPriceSeqNext(product);
}
#endif