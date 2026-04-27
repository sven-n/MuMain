//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: ShopCategoryList.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopCategoryList.h"

CShopCategoryList::CShopCategoryList() // OK
{
    this->Clear();
}

CShopCategoryList::~CShopCategoryList() // OK
{
}

void CShopCategoryList::Clear() // OK
{
    this->m_Categroys.clear();
    this->m_CategoryIndex.clear();
}

int CShopCategoryList::GetSize() // OK
{
    return static_cast<int>(this->m_Categroys.size());
}

void CShopCategoryList::Append(CShopCategory category) // OK
{
    if (this->m_Categroys.find(category.ProductDisplaySeq) != this->m_Categroys.end())
        return;

    this->m_Categroys.insert(std::make_pair(category.ProductDisplaySeq, category));

    this->m_CategoryIndex.push_back(category.ProductDisplaySeq);

    if (category.Root != 1)
    {
        auto it = this->m_Categroys.find(category.ParentProductDisplaySeq);

        if (it == this->m_Categroys.end())
        {
            return;
        }

        it->second.CategoryList.push_back(category.ProductDisplaySeq);
    }
}

void CShopCategoryList::SetFirst() // OK
{
    this->m_Categoryiter = this->m_Categroys.begin();
}

bool CShopCategoryList::GetNext(CShopCategory& category) // OK
{
    if (this->m_Categoryiter == this->m_Categroys.end())
        return 0;

    category = this->m_Categoryiter->second;
    this->m_Categoryiter++;
    return 1;
}

bool CShopCategoryList::GetValueByKey(int nKey, CShopCategory& category) // OK
{
    auto it = this->m_Categroys.find(nKey);

    if (it == this->m_Categroys.end())
    {
        return 0;
    }

    category = it->second;
    return 1;
}

bool CShopCategoryList::GetValueByIndex(int nIndex, CShopCategory& category) // OK
{
    if (nIndex < 0 || nIndex >= static_cast<int>(this->m_CategoryIndex.size()))
    {
        return 0;
    }

    return this->GetValueByKey(this->m_CategoryIndex[nIndex], category);
}

bool CShopCategoryList::InsertPackage(int Category, int Package) // OK
{
    auto it = this->m_Categroys.find(Category);

    if (it == this->m_Categroys.end())
    {
        return 0;
    }

    it->second.AddPackageSeq(Package);
    return 1;
}

bool CShopCategoryList::RefreshPackageSeq(int Category, int PackageSeqs[], int PackageCount) // OK
{
    auto it = this->m_Categroys.find(Category);

    if (it == this->m_Categroys.end())
    {
        return 0;
    }

    it->second.ClearPackageSeq();

    for (int n = 0; n < PackageCount; n++)
    {
        it->second.AddPackageSeq(PackageSeqs[n]);
    }

    return 1;
}
#endif