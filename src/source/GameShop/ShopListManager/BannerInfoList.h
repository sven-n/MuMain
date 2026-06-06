#pragma once

#include "BannerInfo.h"

class CBannerInfoList
{
public:
    CBannerInfoList(void);
    ~CBannerInfoList(void);

    WZResult LoadBanner(std::wstring strDirPath, std::wstring strScriptFileName, bool bDonwLoad);

    void Clear();
    int GetSize();

    virtual void Append(CBannerInfo banner);

    void SetFirst();
    bool GetNext(CBannerInfo& banner);

protected:
    std::multimap<int, CBannerInfo> m_BannerInfos;
    std::multimap<int, CBannerInfo>::iterator m_BannerInfoIter;
};
