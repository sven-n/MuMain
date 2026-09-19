//************************************************************************
//
// FILE: ShopPackageList.cpp
// Removed #ifdef _WIN32 guard (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopPackageList.h"

// cppcheck-suppress uninitMemberVar
CShopPackageList::CShopPackageList()
{
    this->m_Packages.clear();
}

CShopPackageList::~CShopPackageList() {}

int CShopPackageList::GetSize()
{
    return static_cast<int>(this->m_Packages.size());
}

void CShopPackageList::Clear()
{
    this->m_Packages.clear();
}

void CShopPackageList::Append(CShopPackage package)
{
    this->m_Packages.insert(std::make_pair(package.PackageProductSeq, package));
}

void CShopPackageList::SetFirst()
{
    this->m_iter = this->m_Packages.begin();
}

bool CShopPackageList::GetNext(CShopPackage& package)
{
    if (this->m_iter == this->m_Packages.end())
        return false;

    package = this->m_iter->second;
    this->m_iter++;
    return true;
}

bool CShopPackageList::GetValueByKey(int nKey, CShopPackage& package)
{
    auto it = this->m_Packages.find(nKey);

    if (it == this->m_Packages.end())
        return false;

    package = it->second;

    return true;
}

bool CShopPackageList::GetValueByIndex(int nIndex, CShopPackage& package)
{
    if (nIndex < 0 || nIndex >= static_cast<int>(this->m_PackageIndex.size()))
    {
        return false;
    }

    return this->GetValueByKey(this->m_PackageIndex[nIndex], package);
}

bool CShopPackageList::SetPacketLeftCount(int PackageSeq, int nCount)
{
    auto it = this->m_Packages.find(PackageSeq);

    if (it == this->m_Packages.end())
        return false;

    it->second.LeftCount = nCount;

    return true;
}
#endif
