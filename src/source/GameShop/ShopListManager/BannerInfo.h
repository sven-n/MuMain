#pragma once

#include "Include.h"
#include <map>

class CBannerInfo
{
public:
    CBannerInfo();
    virtual ~CBannerInfo();

    bool SetBanner(std::wstring strdata, std::wstring strDirPath, bool bDonwLoad);

public:
    int BannerSeq;                                   //  1. ��� �׷� ����
    wchar_t BannerName[BANNER_LENGTH_NAME];          //  2. ��� �׷� ��
    wchar_t BannerImageURL[INTERNET_MAX_URL_LENGTH]; //  3. ��� �̹��� URL
    int BannerOrder;                                 //  4. ��� ���� ����
    int BannerDirection;                             //  5. ��� ���� ���� * ���� �ڵ� ���� ����
    tm BannerStartDate;                              //  6. ��� ���� ������
    tm BannerEndDate;                                //  7. ��� ���� ������
    wchar_t BannerLinkURL[INTERNET_MAX_URL_LENGTH];  //  8. ��� ��ũ URL

    wchar_t BannerImagePath[MAX_PATH]; // ��� �̹��� ���� ���� ���
};
