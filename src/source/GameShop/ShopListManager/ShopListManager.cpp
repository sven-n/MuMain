//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: ShopListManager.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "ShopListManager.h"

CShopListManager::CShopListManager() // OK
{
    this->m_ShopList = new CShopList;

    this->m_vScriptFiles.push_back(SHOPLIST_SCRIPT_CATEGORY);
    this->m_vScriptFiles.push_back(SHOPLIST_SCRIPT_PACKAGE);
    this->m_vScriptFiles.push_back(SHOPLIST_SCRIPT_PRODUCT);

    //setlocale(0,"Koraen");
}

CShopListManager::~CShopListManager() // OK
{
    SAFE_DELETE(m_ShopList);
}

WZResult		CShopListManager::LoadScript(bool bDonwLoad) // OK
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