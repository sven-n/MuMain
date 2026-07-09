//************************************************************************
//
// FILE: ShopProduct.cpp
// Removed #ifdef _WIN32 guard (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopProduct.h"
#include "StringToken.h"

#include <cwchar>

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
    wcsncpy(this->ProductName, token.nextToken().c_str(), SHOPLIST_LENGTH_PRODUCTNAME - 1);
    this->ProductName[SHOPLIST_LENGTH_PRODUCTNAME - 1] = L'\0';
    wcsncpy(this->PropertyName, token.nextToken().c_str(), SHOPLIST_LENGTH_PRODUCTPROPERTYNAME - 1);
    this->PropertyName[SHOPLIST_LENGTH_PRODUCTPROPERTYNAME - 1] = L'\0';
    wcsncpy(this->Value, token.nextToken().c_str(), SHOPLIST_LENGTH_PRODUCTVALUE - 1);
    this->Value[SHOPLIST_LENGTH_PRODUCTVALUE - 1] = L'\0';
    wcsncpy(this->UnitName, token.nextToken().c_str(), SHOPLIST_LENGTH_PRODUCTUNITNAME - 1);
    this->UnitName[SHOPLIST_LENGTH_PRODUCTUNITNAME - 1] = L'\0';
    this->Price = _wtoi(token.nextToken().c_str());
    this->PriceSeq = _wtoi(token.nextToken().c_str());
    this->PropertyType = _wtoi(token.nextToken().c_str());
    this->MustFlag = _wtoi(token.nextToken().c_str());
    this->vOrder = _wtoi(token.nextToken().c_str());
    this->DeleteFlag = _wtoi(token.nextToken().c_str());
    this->StorageGroup = _wtoi(token.nextToken().c_str());
    this->ShareFlag = _wtoi(token.nextToken().c_str());
    wcsncpy(this->InGamePackageID, token.nextToken().c_str(), SHOPLIST_LENGTH_INGAMEPACKAGEID - 1);
    this->InGamePackageID[SHOPLIST_LENGTH_INGAMEPACKAGEID - 1] = L'\0';
    this->PropertySeq = _wtoi(token.nextToken().c_str());
    this->ProductType = _wtoi(token.nextToken().c_str());
    this->UnitType = _wtoi(token.nextToken().c_str());

    return true;
}
#endif
