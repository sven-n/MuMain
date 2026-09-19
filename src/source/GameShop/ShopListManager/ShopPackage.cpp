//************************************************************************
//
// FILE: ShopPackage.cpp
// Removed #ifdef _WIN32 guard (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopPackage.h"
#include "StringToken.h"
#include "StringMethod.h"

#include <iterator>

// cppcheck-suppress uninitMemberVar
CShopPackage::CShopPackage() // OK
{
    this->LeftCount = -1;
    this->ProductSeqList.clear();
    this->PriceSeqList.clear();
}
CShopPackage::~CShopPackage() {}

bool CShopPackage::SetPackage(std::wstring strdata)
{
    if (strdata.empty())
        return false;

    CStringToken token(strdata, L"@");

    if (token.hasMoreTokens() == 0)
        return false;

    this->ProductDisplaySeq = _wtoi(token.nextToken().c_str());
    this->ViewOrder = _wtoi(token.nextToken().c_str());
    this->PackageProductSeq = _wtoi(token.nextToken().c_str());
    StringCchCopy(this->PackageProductName, std::size(this->PackageProductName), token.nextToken().c_str());
    this->PackageProductType = _wtoi(token.nextToken().c_str());
    this->Price = _wtoi(token.nextToken().c_str());
    StringCchCopy(this->Description, std::size(this->Description), token.nextToken().c_str());
    StringCchCopy(this->Caution, std::size(this->Caution), token.nextToken().c_str());
    this->SalesFlag = _wtoi(token.nextToken().c_str());
    this->GiftFlag = _wtoi(token.nextToken().c_str());
    CStringMethod::ConvertStringToDateTime(this->StartDate, token.nextToken());
    CStringMethod::ConvertStringToDateTime(this->EndDate, token.nextToken());
    this->CapsuleFlag = _wtoi(token.nextToken().c_str());
    this->CapsuleCount = _wtoi(token.nextToken().c_str());
    StringCchCopy(this->ProductCashName, std::size(this->ProductCashName), token.nextToken().c_str());
    StringCchCopy(this->PricUnitName, std::size(this->PricUnitName), token.nextToken().c_str());
    this->DeleteFlag = _wtoi(token.nextToken().c_str());
    this->EventFlag = _wtoi(token.nextToken().c_str());
    this->ProductAmount = _wtoi(token.nextToken().c_str());
    this->SetProductSeqList(token.nextToken());
    StringCchCopy(this->InGamePackageID, std::size(this->InGamePackageID), token.nextToken().c_str());
    this->ProductCashSeq = _wtoi(token.nextToken().c_str());
    this->PriceCount = _wtoi(token.nextToken().c_str());
    this->SetPriceSeqList(token.nextToken());
    this->DeductMileageFlag = _wtoi(token.nextToken().c_str()) != 0;
    this->CashType = _wtoi(token.nextToken().c_str());
    this->CashTypeFlag = _wtoi(token.nextToken().c_str());

    return true;
}

void CShopPackage::SetLeftCount(int nCount)
{
    this->LeftCount = nCount;
}

int CShopPackage::GetProductCount()
{
    return static_cast<int>(this->ProductSeqList.size());
}

void CShopPackage::SetProductSeqFirst()
{
    this->ProductSeqIter = this->ProductSeqList.begin();
}

bool CShopPackage::GetProductSeqFirst(int& ProductSeq)
{
    this->ProductSeqIter = this->ProductSeqList.begin();

    if (this->ProductSeqIter == this->ProductSeqList.end())
        return false;
    ProductSeq = (*this->ProductSeqIter);
    this->ProductSeqIter++;
    return true;
}

bool CShopPackage::GetProductSeqNext(int& ProductSeq)
{
    if (this->ProductSeqIter == this->ProductSeqList.end())
        return false;
    ProductSeq = (*this->ProductSeqIter);
    this->ProductSeqIter++;
    return true;
}

int CShopPackage::GetPriceCount()
{
    return static_cast<int>(this->PriceSeqList.size());
}

void CShopPackage::SetPriceSeqFirst()
{
    this->PriceSeqIter = this->PriceSeqList.begin();
}

bool CShopPackage::GetPriceSeqFirst(int& PriceSeq)
{
    this->PriceSeqIter = this->PriceSeqList.begin();

    if (this->PriceSeqIter == this->PriceSeqList.end())
        return false;
    PriceSeq = (*this->PriceSeqIter);
    this->PriceSeqIter++;
    return true;
}

bool CShopPackage::GetPriceSeqNext(int& PriceSeq)
{
    if (this->PriceSeqIter == this->PriceSeqList.end())
        return false;
    PriceSeq = (*this->PriceSeqIter);
    this->PriceSeqIter++;
    return true;
}

void CShopPackage::SetProductSeqList(std::wstring strdata)
{
    CStringToken token(strdata, L"|");

    while (true)
    {
        std::wstring data = token.nextToken();

        if (data.empty())
            break;

        this->ProductSeqList.push_back(_wtoi(data.c_str()));
    }
}

void CShopPackage::SetPriceSeqList(std::wstring strdata)
{
    CStringToken token(strdata, L"|");

    while (true)
    {
        std::wstring data = token.nextToken();

        if (data.empty())
            break;

        this->PriceSeqList.push_back(_wtoi(data.c_str()));
    }
}
#endif
