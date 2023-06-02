/*******************************************************************************
*	작 성 자 : 진혜진
*	작 성 일 : 2009.06.10
*	내    용 : FileDownloader
*				File 단위 다운로드 기능 제공
*******************************************************************************/

#pragma once

#include "GameShop\ShopListManager\interface\IConnecter.h"
#include "GameShop\ShopListManager\interface\IDownloaderStateEvent.h"

class FileDownloader
{
public:
    // Constructor, Destructor

    FileDownloader(IDownloaderStateEvent* pStateEvent,
        DownloadServerInfo* pServerInfo,
        DownloadFileInfo* pFileInfo);
    ~FileDownloader();

    // public Function

        //					다운로드 중지
    void				Break();
    //					지정한 파일 다운로드 실행 : 세션, 커낵트, 파일 오픈 모두 처리
    WZResult			DownloadFile();

private:
    // private Function

        //					진행 여부
    BOOL				CanBeContinue();
    //					릴리즈
    void				Release();

    //					커넥터 생성
    IConnecter* CreateConnecter();
    //					접속 처리
    WZResult 			CreateConnection();
    static unsigned int __stdcall RunConnectThread(LPVOID pParam);
    WZResult 			Connection();

    //					전송 처리
    WZResult 			TransferRemoteFile();

    //					로컬 파일 생성
    WZResult 			CreateLocalFile();
    //					다운로드 파일 읽기
    WZResult 			ReadRemoteFile(BYTE* byReadBuffer, DWORD* dwBytesRead);
    //					로컬 파일 쓰기
    WZResult 			WriteLocalFile(BYTE* byReadBuffer, DWORD dwBytesRead);

    //					다운로드 시작 이벤트 보내기
    void				SendStartedDownloadFileEvent(ULONGLONG nFileLength);
    //					다운로드 완료 이벤트 보내기
    void				SendCompletedDownloadFileEvent(WZResult wzResult);
    //					다운로드 진행 상황 이벤트 보내기 : 패킷 단위
    void				SendProgressDownloadFileEvent(ULONGLONG nTotalBytesRead);

    // Member Object

        //							다운로드 중지 플래그
    volatile BOOL				m_bBreak;
    //							결과..
    WZResult 					m_Result;

    //							다운로드 상태 이벤트 받을 객체
    IDownloaderStateEvent* m_pStateEvent;
    //							다운로드 서버 정보 객체
    DownloadServerInfo* m_pServerInfo;
    //							다운로드 파일 정보 객체
    DownloadFileInfo* m_pFileInfo;
    //							커넥터
    IConnecter* m_pConnecter;

    //							WinINet 세션 핸들
    HINTERNET					m_hSession;
    //							WinINet 커낵션 핸들
    HINTERNET					m_hConnection;
    //							서버 파일 핸들
    HINTERNET					m_hRemoteFile;
    //							로컬 파일 핸들
    HANDLE						m_hLocalFile;
    //							파일 사이즈
    ULONGLONG					m_nFileLength;
};
