/**************************************************************************************************

작성일: 2009.10.06
작성자: 진혜진

**************************************************************************************************/

#pragma once

#include "include.h"
#include "BannerInfoList.h"
#include "ListManager.h"

class CBannerListManager : public CListManager
{
public:
    CBannerListManager();
    virtual ~CBannerListManager();

    CBannerInfoList* GetListPtr() { return m_BannerInfoList; };

private:
    CBannerInfoList* m_BannerInfoList;

    WZResult			LoadScript(bool bDonwLoad);
};
