#pragma once

#include "include.h"
#include <map>

class CBannerInfo
{
public:
    CBannerInfo();
    virtual ~CBannerInfo();

    bool	SetBanner(std::wstring strdata, std::wstring strDirPath, bool bDonwLoad);

public:
    int		BannerSeq;									//  1. 배너 그룹 순번
    wchar_t	BannerName[BANNER_LENGTH_NAME];				//  2. 배너 그룹 명
    wchar_t	BannerImageURL[INTERNET_MAX_URL_LENGTH];	//  3. 배너 이미지 URL
    int		BannerOrder;								//  4. 배너 노출 순서
    int		BannerDirection;							//  5. 배너 노출 방향 * 공통 코드 정의 참고
    tm 		BannerStartDate;							//  6. 배너 노출 시작일
    tm 		BannerEndDate;								//  7. 배너 노출 종료일
    wchar_t	BannerLinkURL[INTERNET_MAX_URL_LENGTH];		//  8. 배너 링크 URL

    wchar_t	BannerImagePath[MAX_PATH];					// 배너 이미지 파일 로컬 경로
};
