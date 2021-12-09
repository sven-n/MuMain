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

bool CShopProduct::SetProduct(std::string strdata) // OK
{
	if(strdata.empty())
		return 0;

	CStringToken token(strdata,"@");

	if(token.hasMoreTokens()==0)
		return 0;

	this->ProductSeq = atoi(token.nextToken().c_str());
	StringCchCopyA(this->ProductName,sizeof(this->ProductName),token.nextToken().c_str());
	StringCchCopyA(this->PropertyName,sizeof(this->PropertyName),token.nextToken().c_str());
	StringCchCopyA(this->Value,sizeof(this->Value),token.nextToken().c_str());
	StringCchCopyA(this->UnitName,sizeof(this->UnitName),token.nextToken().c_str());
	this->Price = atoi(token.nextToken().c_str());
	this->PriceSeq = atoi(token.nextToken().c_str());
	this->PropertyType = atoi(token.nextToken().c_str());
	this->MustFlag = atoi(token.nextToken().c_str());
	this->vOrder = atoi(token.nextToken().c_str());
	this->DeleteFlag = atoi(token.nextToken().c_str());
	this->StorageGroup = atoi(token.nextToken().c_str());
	this->ShareFlag = atoi(token.nextToken().c_str());
	StringCchCopyA(this->InGamePackageID,sizeof(this->InGamePackageID),token.nextToken().c_str());
	this->PropertySeq = atoi(token.nextToken().c_str());
	this->ProductType = atoi(token.nextToken().c_str());
	this->UnitType = atoi(token.nextToken().c_str());

	return 1;
}
#endif