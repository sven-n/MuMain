/*******************************************************************************
*	�� �� �� : ������
*	�� �� �� : 2009.07.07
*	��    �� : FTP Connecter
*******************************************************************************/

#pragma once

#include "GameShop/ShopListManager/interface/IConnecter.h"

class FTPConnecter : public IConnecter
{
public:
    // Constructor, Destructor

    FTPConnecter(DownloadServerInfo* pServerInfo,
        DownloadFileInfo* pFileInfo);
    ~FTPConnecter();

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
