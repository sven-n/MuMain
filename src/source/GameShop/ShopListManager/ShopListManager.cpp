//************************************************************************
//
// FILE: ShopListManager.cpp
// Removed #ifdef _WIN32 guard (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopListManager.h"

CShopListManager::CShopListManager()
{
    // cppcheck-suppress [noCopyConstructor, noOperatorEq]
    this->m_ShopList = new CShopList;

    this->m_vScriptFiles.push_back(SHOPLIST_SCRIPT_CATEGORY);
    this->m_vScriptFiles.push_back(SHOPLIST_SCRIPT_PACKAGE);
    this->m_vScriptFiles.push_back(SHOPLIST_SCRIPT_PRODUCT);
}

CShopListManager::~CShopListManager()
{
    SAFE_DELETE(m_ShopList);
}

WZResult CShopListManager::LoadScript(bool bDonwLoad)
{
    if (this->m_ShopList)
    {
        std::wstring path = this->GetScriptPath();

        this->m_Result = this->m_ShopList->LoadCategroy(std::wstring(path + SHOPLIST_SCRIPT_CATEGORY).c_str());

        if (this->m_Result.IsSuccess())
        {
            this->m_Result = this->m_ShopList->LoadPackage(std::wstring(path + SHOPLIST_SCRIPT_PACKAGE).c_str());

            if (this->m_Result.IsSuccess())
            {
                this->m_Result = this->m_ShopList->LoadProduct(std::wstring(path + SHOPLIST_SCRIPT_PRODUCT).c_str());
            }
        }
    }
    else
    {
        this->m_Result.SetResult(PT_NO_INFO, 0, L"[CShopListManager::LoadScript] Failed");
    }

    return this->m_Result;
}
#endif
