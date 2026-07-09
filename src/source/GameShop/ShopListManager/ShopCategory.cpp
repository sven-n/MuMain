//************************************************************************
//
// FILE: ShopCategory.cpp
// Removed #ifdef _WIN32 guard (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopCategory.h"
#include "StringToken.h"

#include <cwchar>

// cppcheck-suppress uninitMemberVar
CShopCategory::CShopCategory()
{
    this->CategoryList.clear();
    this->PackageList.clear();
}

CShopCategory::~CShopCategory() {}

bool CShopCategory::SetCategory(std::wstring strdata)
{
    if (strdata.empty())
        return false;

    CStringToken token(strdata, L"@");

    if (token.hasMoreTokens() == 0)
        return false;

    this->ProductDisplaySeq = _wtoi(token.nextToken().c_str());
    wcsncpy(this->CategroyName, token.nextToken().c_str(), SHOPLIST_LENGTH_CATEGORYNAME - 1);
    this->CategroyName[SHOPLIST_LENGTH_CATEGORYNAME - 1] = L'\0';
    this->EventFlag = _wtoi(token.nextToken().c_str());
    this->OpenFlag = _wtoi(token.nextToken().c_str());
    this->ParentProductDisplaySeq = _wtoi(token.nextToken().c_str());
    this->DisplayOrder = _wtoi(token.nextToken().c_str());
    this->Root = _wtoi(token.nextToken().c_str());

    return true;
}

void CShopCategory::SetCategoryFirst()
{
    this->Categoryiter = this->CategoryList.begin();
}

bool CShopCategory::GetCategoryNext(int& CategorySeq)
{
    if (this->Categoryiter == this->CategoryList.end())
        return false;

    CategorySeq = (*this->Categoryiter);
    this->Categoryiter++;
    return true;
}

void CShopCategory::SetPackagSeqFirst()
{
    this->Packageiter = this->PackageList.begin();
}

bool CShopCategory::GetPackagSeqNext(int& PackagSeq)
{
    if (this->Packageiter == this->PackageList.end())
        return false;

    PackagSeq = (*this->Packageiter);
    this->Packageiter++;
    return true;
}

void CShopCategory::AddPackageSeq(int PackageSeq)
{
    this->PackageList.push_back(PackageSeq);
}

void CShopCategory::ClearPackageSeq()
{
    this->PackageList.clear();
}
#endif
