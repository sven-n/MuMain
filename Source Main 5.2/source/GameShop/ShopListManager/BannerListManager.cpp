//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: BannerListManager.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "BannerListManager.h"

CBannerListManager::CBannerListManager() // OK
{
    this->m_BannerInfoList = new CBannerInfoList();

    this->m_vScriptFiles.push_back(BANNER_SCRIPT_FILENAME);
}

CBannerListManager::~CBannerListManager() // OK
{
    SAFE_DELETE(m_BannerInfoList);
}

WZResult			CBannerListManager::LoadScript(bool bDonwLoad) // OK
{
    std::wstring path = this->GetScriptPath();

    return this->m_BannerInfoList->LoadBanner(path, BANNER_SCRIPT_FILENAME, bDonwLoad);
}

#endif