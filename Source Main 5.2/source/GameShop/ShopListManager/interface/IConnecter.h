/*******************************************************************************
*	작 성 자 : 진혜진
*	작 성 일 : 2009.07.07
*	내    용 : Connecter Interface
*******************************************************************************/

#pragma once

#include "GameShop\ShopListManager\interface\WZResult\WZResult.h"
#include "GameShop\ShopListManager\interface\DownloadInfo.h"

class IConnecter
{
public:
    // Constructor, Destructor

    IConnecter(DownloadServerInfo* pServerInfo,
        DownloadFileInfo* pFileInfo)
        : m_pServerInfo(pServerInfo),
        m_pFileInfo(pFileInfo)
    {};
    ~IConnecter() {};

    // abstract Function

        //						세션
    virtual WZResult		CreateSession(HINTERNET& hSession) = 0;
    //						커낵트
    virtual WZResult		CreateConnection(HINTERNET& hSession,
        HINTERNET& hConnection) = 0;
    //						다운로드 파일 오픈 & 사이즈 가져오기
    virtual WZResult		OpenRemoteFile(HINTERNET& hConnection,
        HINTERNET& hRemoteFile,
        ULONGLONG& nFileLength) = 0;
    //						리모트 파일 읽기
    virtual WZResult		ReadRemoteFile(HINTERNET& hRemoteFile,
        BYTE* byReadBuffer,
        DWORD* dwBytesRead) = 0;

protected:
    // Member Object

        //						결과..
    WZResult 				m_Result;
    //						다운로드 서버 정보 객체
    DownloadServerInfo* m_pServerInfo;
    //						다운로드 파일 정보 객체
    DownloadFileInfo* m_pFileInfo;
};
