//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: ShopCategory.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopCategory.h"
#include "StringToken.h"

CShopCategory::CShopCategory() // OK
{
	this->CategoryList.clear();
	this->PackageList.clear();
}

CShopCategory::~CShopCategory() // OK
{

}

bool CShopCategory::SetCategory(std::string strdata) // OK
{
	if(strdata.empty())
		return 0;

	CStringToken token(strdata,"@");

	if(token.hasMoreTokens()==0)
		return 0;

	this->ProductDisplaySeq = atoi(token.nextToken().c_str());
	StringCchCopy(this->CategroyName,sizeof(this->CategroyName),token.nextToken().c_str());
	this->EventFlag = atoi(token.nextToken().c_str());
	this->OpenFlag = atoi(token.nextToken().c_str());
	this->ParentProductDisplaySeq = atoi(token.nextToken().c_str());
	this->DisplayOrder = atoi(token.nextToken().c_str());
	this->Root = atoi(token.nextToken().c_str());

	return 1;
}

void CShopCategory::SetCategoryFirst() // OK
{
	this->Categoryiter = this->CategoryList.begin();
}

bool CShopCategory::GetCategoryNext(int& CategorySeq) // OK
{
	if(this->Categoryiter==this->CategoryList.end())
		return 0;

	CategorySeq = (*this->Categoryiter);
	this->Categoryiter++;
	return 1;
}

void CShopCategory::SetPackagSeqFirst() // OK
{
	this->Packageiter = this->PackageList.begin();
}

bool CShopCategory::GetPackagSeqNext(int& PackagSeq) // OK
{
	if(this->Packageiter==this->PackageList.end())
		return 0;

	PackagSeq = (*this->Packageiter);
	this->Packageiter++;
	return 1;
}

void CShopCategory::AddPackageSeq(int PackageSeq) // OK
{
	this->PackageList.push_back(PackageSeq);
}

void CShopCategory::ClearPackageSeq()
{
	this->PackageList.clear();
}
#endif