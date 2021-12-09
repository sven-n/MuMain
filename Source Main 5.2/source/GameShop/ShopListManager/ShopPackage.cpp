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

bool	CShopPackage::SetPackage(std::string strdata) // OK
{
	if(strdata.empty())
		return 0;

	CStringToken token(strdata,"@");

	if(token.hasMoreTokens()==0)
		return 0;

	this->ProductDisplaySeq = atoi(token.nextToken().c_str());
	this->ViewOrder = atoi(token.nextToken().c_str());
	this->PackageProductSeq = atoi(token.nextToken().c_str());
	StringCchCopy(this->PackageProductName,sizeof(this->PackageProductName),token.nextToken().c_str());
	this->PackageProductType = atoi(token.nextToken().c_str());
	this->Price = atoi(token.nextToken().c_str());
	StringCchCopy(this->Description,sizeof(this->Description),token.nextToken().c_str());
	StringCchCopy(this->Caution,sizeof(this->Caution),token.nextToken().c_str());
	this->SalesFlag = atoi(token.nextToken().c_str());
	this->GiftFlag = atoi(token.nextToken().c_str());
	CStringMethod::ConvertStringToDateTime(this->StartDate,token.nextToken());
	CStringMethod::ConvertStringToDateTime(this->EndDate,token.nextToken());
	this->CapsuleFlag = atoi(token.nextToken().c_str());
	this->CapsuleCount = atoi(token.nextToken().c_str());
	StringCchCopy(this->ProductCashName,sizeof(this->ProductCashName),token.nextToken().c_str());
	StringCchCopy(this->PricUnitName,sizeof(this->PricUnitName),token.nextToken().c_str());
	this->DeleteFlag = atoi(token.nextToken().c_str());
	this->EventFlag = atoi(token.nextToken().c_str());
	this->ProductAmount = atoi(token.nextToken().c_str());
	this->SetProductSeqList(token.nextToken());
	StringCchCopy(this->InGamePackageID,sizeof(this->InGamePackageID),token.nextToken().c_str());
	this->ProductCashSeq = atoi(token.nextToken().c_str());
	this->PriceCount = atoi(token.nextToken().c_str());
	this->SetPriceSeqList(token.nextToken());
	this->DeductMileageFlag = atoi(token.nextToken().c_str())!=0;
	this->CashType = atoi(token.nextToken().c_str());
	this->CashTypeFlag = atoi(token.nextToken().c_str());

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

	if(this->ProductSeqIter==this->ProductSeqList.end())
		return 0;
	ProductSeq = (*this->ProductSeqIter);
	this->ProductSeqIter++;
	return 1;
}

bool	CShopPackage::GetProductSeqNext(int& ProductSeq) // OK
{
	if(this->ProductSeqIter==this->ProductSeqList.end())
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

	if(this->PriceSeqIter==this->PriceSeqList.end())
		return 0;
	PriceSeq = (*this->PriceSeqIter);
	this->PriceSeqIter++;
	return 1;
}

bool	CShopPackage::GetPriceSeqNext(int& PriceSeq) // OK
{
	if(this->PriceSeqIter==this->PriceSeqList.end())
		return 0;
	PriceSeq = (*this->PriceSeqIter);
	this->PriceSeqIter++;
	return 1;
}

void	CShopPackage::SetProductSeqList(std::string strdata) // OK
{
	CStringToken token(strdata,"|");

	while(true)
	{
		std::string data = token.nextToken();

		if(data.empty()) break;

		this->ProductSeqList.push_back(atoi(data.c_str()));
	}
}

void	CShopPackage::SetPriceSeqList(std::string strdata) // OK
{
	CStringToken token(strdata,"|");

	while(true)
	{
		std::string data = token.nextToken();

		if(data.empty()) break;

		this->PriceSeqList.push_back(atoi(data.c_str()));
	}
}
#endif