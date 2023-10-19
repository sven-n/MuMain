//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: ShopPackage.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopPackage.h"
#include "StringToken.h"
#include "StringMethod.h"

CShopPackage::CShopPackage() // OK
{
    this->LeftCount = -1;
    this->ProductSeqList.clear();
    this->PriceSeqList.clear();
}
CShopPackage::~CShopPackage() // OK
{
}

bool	CShopPackage::SetPackage(std::wstring strdata) // OK
{
    if (strdata.empty())
        return 0;

    CStringToken token(strdata, L"@");

    if (token.hasMoreTokens() == 0)
        return 0;

    this->ProductDisplaySeq = _wtoi(token.nextToken().c_str());
    this->ViewOrder = _wtoi(token.nextToken().c_str());
    this->PackageProductSeq = _wtoi(token.nextToken().c_str());
    StringCchCopy(this->PackageProductName, sizeof(this->PackageProductName), token.nextToken().c_str());
    this->PackageProductType = _wtoi(token.nextToken().c_str());
    this->Price = _wtoi(token.nextToken().c_str());
    StringCchCopy(this->Description, sizeof(this->Description), token.nextToken().c_str());
    StringCchCopy(this->Caution, sizeof(this->Caution), token.nextToken().c_str());
    this->SalesFlag = _wtoi(token.nextToken().c_str());
    this->GiftFlag = _wtoi(token.nextToken().c_str());
    CStringMethod::ConvertStringToDateTime(this->StartDate, token.nextToken());
    CStringMethod::ConvertStringToDateTime(this->EndDate, token.nextToken());
    this->CapsuleFlag = _wtoi(token.nextToken().c_str());
    this->CapsuleCount = _wtoi(token.nextToken().c_str());
    StringCchCopy(this->ProductCashName, sizeof(this->ProductCashName), token.nextToken().c_str());
    StringCchCopy(this->PricUnitName, sizeof(this->PricUnitName), token.nextToken().c_str());
    this->DeleteFlag = _wtoi(token.nextToken().c_str());
    this->EventFlag = _wtoi(token.nextToken().c_str());
    this->ProductAmount = _wtoi(token.nextToken().c_str());
    this->SetProductSeqList(token.nextToken());
    StringCchCopy(this->InGamePackageID, sizeof(this->InGamePackageID), token.nextToken().c_str());
    this->ProductCashSeq = _wtoi(token.nextToken().c_str());
    this->PriceCount = _wtoi(token.nextToken().c_str());
    this->SetPriceSeqList(token.nextToken());
    this->DeductMileageFlag = _wtoi(token.nextToken().c_str()) != 0;
    this->CashType = _wtoi(token.nextToken().c_str());
    this->CashTypeFlag = _wtoi(token.nextToken().c_str());

    return 1;
}

void	CShopPackage::SetLeftCount(int nCount) // OK
{
    this->LeftCount = nCount;
}

int		CShopPackage::GetProductCount() // OK
{
    return static_cast<int>(this->ProductSeqList.size());
}

void	CShopPackage::SetProductSeqFirst() // OK
{
    this->ProductSeqIter = this->ProductSeqList.begin();
}

bool	CShopPackage::GetProductSeqFirst(int& ProductSeq) // OK
{
    this->ProductSeqIter = this->ProductSeqList.begin();

    if (this->ProductSeqIter == this->ProductSeqList.end())
        return 0;
    ProductSeq = (*this->ProductSeqIter);
    this->ProductSeqIter++;
    return 1;
}

bool	CShopPackage::GetProductSeqNext(int& ProductSeq) // OK
{
    if (this->ProductSeqIter == this->ProductSeqList.end())
        return 0;
    ProductSeq = (*this->ProductSeqIter);
    this->ProductSeqIter++;
    return 1;
}

int		CShopPackage::GetPriceCount()
{
    return static_cast<int>(this->PriceSeqList.size());
}

void	CShopPackage::SetPriceSeqFirst()
{
    this->PriceSeqIter = this->PriceSeqList.begin();
}

bool	CShopPackage::GetPriceSeqFirst(int& PriceSeq) // OK
{
    this->PriceSeqIter = this->PriceSeqList.begin();

    if (this->PriceSeqIter == this->PriceSeqList.end())
        return 0;
    PriceSeq = (*this->PriceSeqIter);
    this->PriceSeqIter++;
    return 1;
}

bool	CShopPackage::GetPriceSeqNext(int& PriceSeq) // OK
{
    if (this->PriceSeqIter == this->PriceSeqList.end())
        return 0;
    PriceSeq = (*this->PriceSeqIter);
    this->PriceSeqIter++;
    return 1;
}

void	CShopPackage::SetProductSeqList(std::wstring strdata) // OK
{
    CStringToken token(strdata, L"|");

    while (true)
    {
        std::wstring data = token.nextToken();

        if (data.empty()) break;

        this->ProductSeqList.push_back(_wtoi(data.c_str()));
    }
}

void	CShopPackage::SetPriceSeqList(std::wstring strdata) // OK
{
    CStringToken token(strdata, L"|");

    while (true)
    {
        std::wstring data = token.nextToken();

        if (data.empty()) break;

        this->PriceSeqList.push_back(_wtoi(data.c_str()));
    }
}
#endif