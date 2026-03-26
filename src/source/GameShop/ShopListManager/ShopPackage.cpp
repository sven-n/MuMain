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

#include <cwchar>

// cppcheck-suppress uninitMemberVar
CShopPackage::CShopPackage()
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
    wcsncpy(this->PackageProductName, token.nextToken().c_str(), SHOPLIST_LENGTH_PACKAGENAME - 1);
    this->PackageProductName[SHOPLIST_LENGTH_PACKAGENAME - 1] = L'\0';
    this->PackageProductType = _wtoi(token.nextToken().c_str());
    this->Price = _wtoi(token.nextToken().c_str());
    wcsncpy(this->Description, token.nextToken().c_str(), SHOPLIST_LENGTH_PACKAGEDESC - 1);
    this->Description[SHOPLIST_LENGTH_PACKAGEDESC - 1] = L'\0';
    wcsncpy(this->Caution, token.nextToken().c_str(), SHOPLIST_LENGTH_PACKAGECAUTION - 1);
    this->Caution[SHOPLIST_LENGTH_PACKAGECAUTION - 1] = L'\0';
    this->SalesFlag = _wtoi(token.nextToken().c_str());
    this->GiftFlag = _wtoi(token.nextToken().c_str());
    CStringMethod::ConvertStringToDateTime(this->StartDate, token.nextToken());
    CStringMethod::ConvertStringToDateTime(this->EndDate, token.nextToken());
    this->CapsuleFlag = _wtoi(token.nextToken().c_str());
    this->CapsuleCount = _wtoi(token.nextToken().c_str());
    wcsncpy(this->ProductCashName, token.nextToken().c_str(), SHOPLIST_LENGTH_PACKAGECASHNAME - 1);
    this->ProductCashName[SHOPLIST_LENGTH_PACKAGECASHNAME - 1] = L'\0';
    wcsncpy(this->PricUnitName, token.nextToken().c_str(), SHOPLIST_LENGTH_PACKAGEPRICEUNIT - 1);
    this->PricUnitName[SHOPLIST_LENGTH_PACKAGEPRICEUNIT - 1] = L'\0';
    this->DeleteFlag = _wtoi(token.nextToken().c_str());
    this->EventFlag = _wtoi(token.nextToken().c_str());
    this->ProductAmount = _wtoi(token.nextToken().c_str());
    this->SetProductSeqList(token.nextToken());
    wcsncpy(this->InGamePackageID, token.nextToken().c_str(), SHOPLIST_LENGTH_INGAMEPACKAGEID - 1);
    this->InGamePackageID[SHOPLIST_LENGTH_INGAMEPACKAGEID - 1] = L'\0';
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
