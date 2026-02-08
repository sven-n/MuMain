//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: ShopProduct.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopProduct.h"
#include "StringToken.h"

CShopProduct::CShopProduct() // OK
{
}
CShopProduct::~CShopProduct() // OK
{
}

bool CShopProduct::SetProduct(std::wstring strdata) // OK
{
    if (strdata.empty())
        return 0;

    CStringToken token(strdata, L"@");

    if (token.hasMoreTokens() == 0)
        return 0;

    this->ProductSeq = _wtoi(token.nextToken().c_str());
    StringCchCopy(this->ProductName, sizeof(this->ProductName), token.nextToken().c_str());
    StringCchCopy(this->PropertyName, sizeof(this->PropertyName), token.nextToken().c_str());
    StringCchCopy(this->Value, sizeof(this->Value), token.nextToken().c_str());
    StringCchCopy(this->UnitName, sizeof(this->UnitName), token.nextToken().c_str());
    this->Price = _wtoi(token.nextToken().c_str());
    this->PriceSeq = _wtoi(token.nextToken().c_str());
    this->PropertyType = _wtoi(token.nextToken().c_str());
    this->MustFlag = _wtoi(token.nextToken().c_str());
    this->vOrder = _wtoi(token.nextToken().c_str());
    this->DeleteFlag = _wtoi(token.nextToken().c_str());
    this->StorageGroup = _wtoi(token.nextToken().c_str());
    this->ShareFlag = _wtoi(token.nextToken().c_str());
    StringCchCopy(this->InGamePackageID, sizeof(this->InGamePackageID), token.nextToken().c_str());
    this->PropertySeq = _wtoi(token.nextToken().c_str());
    this->ProductType = _wtoi(token.nextToken().c_str());
    this->UnitType = _wtoi(token.nextToken().c_str());

    return 1;
}
#endif