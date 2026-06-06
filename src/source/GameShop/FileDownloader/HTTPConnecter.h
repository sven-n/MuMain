/*******************************************************************************
*	�� �� �� : ������
*	�� �� �� : 2009.07.07
*	��    �� : HTTP Connecter
*******************************************************************************/

#pragma once

#include "GameShop/ShopListManager/interface/IConnecter.h"

class HTTPConnecter : public IConnecter
{
public:
    // Constructor, Destructor

    HTTPConnecter(DownloadServerInfo* pServerInfo,
        DownloadFileInfo* pFileInfo);
    ~HTTPConnecter();

    // abstract Function

        //						����
    virtual WZResult		CreateSession(HINTERNET& hSession);
    //						Ŀ��Ʈ
    virtual WZResult		CreateConnection(HINTERNET& hSession,
        HINTERNET& hConnection);
    //						�ٿ�ε� ���� ���� & ������ ��������
    virtual WZResult		OpenRemoteFile(HINTERNET& hConnection,
        HINTERNET& hRemoteFile,
        ULONGLONG& nFileLength);
    //						����Ʈ ���� �б�
    virtual WZResult		ReadRemoteFile(HINTERNET& hRemoteFile,
        BYTE* byReadBuffer,
        DWORD* dwBytesRead);
};
