/*******************************************************************************
*	작 성 자 : 진혜진
*	작 성 일 : 2009.07.07
*	내    용 : HTTP Connecter
*******************************************************************************/

#pragma once

#include "GameShop\ShopListManager\interface\IConnecter.h"

class HTTPConnecter : public IConnecter
{
public:
    // Constructor, Destructor

    HTTPConnecter(DownloadServerInfo* pServerInfo,
        DownloadFileInfo* pFileInfo);
    ~HTTPConnecter();

    // abstract Function

        //						세션
    virtual WZResult		CreateSession(HINTERNET& hSession);
    //						커낵트
    virtual WZResult		CreateConnection(HINTERNET& hSession,
        HINTERNET& hConnection);
    //						다운로드 파일 오픈 & 사이즈 가져오기
    virtual WZResult		OpenRemoteFile(HINTERNET& hConnection,
        HINTERNET& hRemoteFile,
        ULONGLONG& nFileLength);
    //						리모트 파일 읽기
    virtual WZResult		ReadRemoteFile(HINTERNET& hRemoteFile,
        BYTE* byReadBuffer,
        DWORD* dwBytesRead);
};
