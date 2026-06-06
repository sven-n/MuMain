/*******************************************************************************
*	작 성 자 : 진혜진
*	작 성 일 : 2009.06.10
*	내    용 : Interface - Download State Event 가상 클래스
*				다운로드 상태를 전달 받을 객체
*				라이브러리를 사용할 프로그램에서 재정의하여 사용
*******************************************************************************/

#pragma once

class IDownloaderStateEvent
{
public:
    // Constructor, Destructor

    IDownloaderStateEvent() {};
    virtual ~IDownloaderStateEvent() {};

    // abstract Function

        //				다운로드 시작 이벤트 핸들러
    virtual void	OnStartedDownloadFile(TCHAR* szFileName, ULONGLONG uFileLength) = 0;
    //				다운로드 진행 상황 이벤트 핸들러 : 패킷 단위
    virtual void	OnProgressDownloadFile(TCHAR* szFileName, ULONGLONG uDownloadFileLength) = 0;
    //				다운로드 종료 이벤트 핸들러
    virtual void	OnCompletedDownloadFile(TCHAR* szFileName, WZResult wzResult) = 0;
};
