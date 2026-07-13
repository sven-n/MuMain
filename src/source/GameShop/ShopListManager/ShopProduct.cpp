//************************************************************************
//
// FILE: ShopProduct.cpp
// Removed #ifdef _WIN32 guard (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopProduct.h"
#include "StringToken.h"

#include <iterator>

// cppcheck-suppress uninitMemberVar
CShopProduct::CShopProduct() {}
CShopProduct::~CShopProduct() {}

bool CShopProduct::SetProduct(std::wstring strdata)
{
    if (strdata.empty())
        return false;

    CStringToken token(strdata, L"@");

    if (token.hasMoreTokens() == 0)
        return false;

    this->ProductSeq = _wtoi(token.nextToken().c_str());
    StringCchCopy(this->ProductName, std::size(this->ProductName), token.nextToken().c_str());
    StringCchCopy(this->PropertyName, std::size(this->PropertyName), token.nextToken().c_str());
    StringCchCopy(this->Value, std::size(this->Value), token.nextToken().c_str());
    StringCchCopy(this->UnitName, std::size(this->UnitName), token.nextToken().c_str());
    this->Price = _wtoi(token.nextToken().c_str());
    this->PriceSeq = _wtoi(token.nextToken().c_str());
    this->PropertyType = _wtoi(token.nextToken().c_str());
    this->MustFlag = _wtoi(token.nextToken().c_str());
    this->vOrder = _wtoi(token.nextToken().c_str());
    this->DeleteFlag = _wtoi(token.nextToken().c_str());
    this->StorageGroup = _wtoi(token.nextToken().c_str());
    this->ShareFlag = _wtoi(token.nextToken().c_str());
    StringCchCopy(this->InGamePackageID, std::size(this->InGamePackageID), token.nextToken().c_str());
    this->PropertySeq = _wtoi(token.nextToken().c_str());
    this->ProductType = _wtoi(token.nextToken().c_str());
    this->UnitType = _wtoi(token.nextToken().c_str());

    return true;
}
#endif
