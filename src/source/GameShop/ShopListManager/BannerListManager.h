/**************************************************************************************************

�ۼ���: 2009.10.06
�ۼ���: ������

**************************************************************************************************/

#pragma once

#include "Include.h"
#include "BannerInfoList.h"
#include "ListManager.h"

class CBannerListManager : public CListManager
{
public:
    CBannerListManager();
    virtual ~CBannerListManager();

    CBannerInfoList* GetListPtr()
    {
        return m_BannerInfoList;
    };

private:
    CBannerInfoList* m_BannerInfoList;

    WZResult LoadScript(bool bDonwLoad);
};
