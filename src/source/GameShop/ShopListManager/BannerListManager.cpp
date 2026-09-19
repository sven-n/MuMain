//************************************************************************
//
// FILE: BannerListManager.cpp
// Removed #ifdef _WIN32 guard (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "BannerListManager.h"

CBannerListManager::CBannerListManager()
{
    // cppcheck-suppress [noCopyConstructor, noOperatorEq]
    this->m_BannerInfoList = new CBannerInfoList();

    this->m_vScriptFiles.push_back(BANNER_SCRIPT_FILENAME);
}

CBannerListManager::~CBannerListManager()
{
    SAFE_DELETE(m_BannerInfoList);
}

WZResult CBannerListManager::LoadScript(bool bDonwLoad)
{
    std::wstring path = this->GetScriptPath();

    return this->m_BannerInfoList->LoadBanner(path, BANNER_SCRIPT_FILENAME, bDonwLoad);
}

#endif
