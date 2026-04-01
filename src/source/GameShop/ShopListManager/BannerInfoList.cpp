//************************************************************************
//
// FILE: BannerInfoList.cpp
// Removed #ifdef _WIN32 guard (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "BannerInfoList.h"
#include "BannerInfo.h"
#include <fstream>

// cppcheck-suppress uninitMemberVar
CBannerInfoList::CBannerInfoList()
{
    this->Clear();
}

CBannerInfoList::~CBannerInfoList() {}

WZResult CBannerInfoList::LoadBanner(std::wstring strDirPath, std::wstring strScriptFileName, bool bDonwLoad)
{
    static WZResult result;

    result.BuildSuccessResult();

    std::wifstream ifs;

    std::wstring path = strDirPath + strScriptFileName;
    const auto narrowPath = mu_narrow_path(path.c_str());

    ifs.open(narrowPath.c_str(), std::ifstream::in);

    if (ifs.is_open())
    {
        this->Clear();

        wchar_t buff[1024] = {0};

        while (true)
        {
            if (!ifs.getline(buff, sizeof(buff)))
                break;

            CBannerInfo info;

            if (info.SetBanner(buff, strDirPath, bDonwLoad))
            {
                this->Append(info);
            }
        }

        ifs.close();
    }
    else
    {
        result.SetResult(6, 0, L"Banner file open fail");
    }

    return result;
}

void CBannerInfoList::Clear()
{
    this->m_BannerInfos.clear();
}

int CBannerInfoList::GetSize()
{
    return this->m_BannerInfos.size();
}

void CBannerInfoList::Append(CBannerInfo banner)
{
    this->m_BannerInfos.insert(std::make_pair(banner.BannerSeq, banner));
}

void CBannerInfoList::SetFirst()
{
    this->m_BannerInfoIter = this->m_BannerInfos.begin();
}
bool CBannerInfoList::GetNext(CBannerInfo& banner)
{
    if (this->m_BannerInfoIter == this->m_BannerInfos.end())
        return false;

    banner = this->m_BannerInfoIter->second;

    this->m_BannerInfoIter++;
    return true;
}
#endif
