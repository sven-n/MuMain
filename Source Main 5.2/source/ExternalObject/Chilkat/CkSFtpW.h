#ifndef _CkSFtpW_H
#define _CkSFtpW_H
#pragma once

#include "CkString.h"
#include "CkSFtp.h"


#pragma pack (push, 8) 


class CkSFtpW : public CkSFtp
{
    private:
	unsigned long nextIdxW(void)
	    {
	    m_resultIdxW++;
	    if (m_resultIdxW >= 10)
		{
		m_resultIdxW = 0;
		}
	    return m_resultIdxW;
	    }
	unsigned long m_resultIdxW;
	CkString m_resultStringW[10];

    public:
	CkSFtpW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkSFtpW() { }
	void add64W(const wchar_t *w1,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->Add64(sTemp1.getUtf8(),sTemp2.getUtf8(),outStr);
		}
	bool authenticatePkW(const wchar_t *w1,CkSshKey &privateKey)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AuthenticatePk(sTemp1.getUtf8(),privateKey);
		}
	bool authenticatePwW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AuthenticatePw(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool closeHandleW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CloseHandle(sTemp1.getUtf8());
		}
	bool connectW(const wchar_t *w1,int port)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Connect(sTemp1.getUtf8(),port);
		}
	bool copyFileAttrW(const wchar_t *w1,const wchar_t *w2,bool bIsHandle)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->CopyFileAttr(sTemp1.getUtf8(),sTemp2.getUtf8(),bIsHandle);
		}
	bool createDirW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CreateDir(sTemp1.getUtf8());
		}
	bool downloadFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->DownloadFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool downloadFileByNameW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->DownloadFileByName(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool eofW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Eof(sTemp1.getUtf8());
		}
	bool getFileCreateTimeW(const wchar_t *w1,bool bFollowLinks,bool bIsHandle,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetFileCreateTime(sTemp1.getUtf8(),bFollowLinks,bIsHandle,sysTime);
		}
	bool getFileGroupW(const wchar_t *w1,bool bFollowLinks,bool bIsHandle,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetFileGroup(sTemp1.getUtf8(),bFollowLinks,bIsHandle,outStr);
		}
	bool getFileLastAccessW(const wchar_t *w1,bool bFollowLinks,bool bIsHandle,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetFileLastAccess(sTemp1.getUtf8(),bFollowLinks,bIsHandle,sysTime);
		}
	bool getFileLastModifiedW(const wchar_t *w1,bool bFollowLinks,bool bIsHandle,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetFileLastModified(sTemp1.getUtf8(),bFollowLinks,bIsHandle,sysTime);
		}
	bool getFileOwnerW(const wchar_t *w1,bool bFollowLinks,bool bIsHandle,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetFileOwner(sTemp1.getUtf8(),bFollowLinks,bIsHandle,outStr);
		}
	int getFilePermissionsW(const wchar_t *w1,bool bFollowLinks,bool bIsHandle)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetFilePermissions(sTemp1.getUtf8(),bFollowLinks,bIsHandle);
		}
	unsigned long getFileSize32W(const wchar_t *w1,bool bFollowLinks,bool bIsHandle)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetFileSize32(sTemp1.getUtf8(),bFollowLinks,bIsHandle);
		}
	__int64 getFileSize64W(const wchar_t *w1,bool bFollowLinks,bool bIsHandle)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetFileSize64(sTemp1.getUtf8(),bFollowLinks,bIsHandle);
		}
	bool getFileSizeStrW(const wchar_t *w1,bool bFollowLinks,bool bIsHandle,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetFileSizeStr(sTemp1.getUtf8(),bFollowLinks,bIsHandle,outStr);
		}
	bool lastReadFailedW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LastReadFailed(sTemp1.getUtf8());
		}
	int lastReadNumBytesW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LastReadNumBytes(sTemp1.getUtf8());
		}
	bool openDirW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenDir(sTemp1.getUtf8(),outStr);
		}
	bool openFileW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->OpenFile(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),outStr);
		}
	CkSFtpDir *readDirW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ReadDir(sTemp1.getUtf8());
		}
	bool readFileBytesW(const wchar_t *w1,int numBytes,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ReadFileBytes(sTemp1.getUtf8(),numBytes,outBytes);
		}
	bool readFileBytes32W(const wchar_t *w1,int offset,int numBytes,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ReadFileBytes32(sTemp1.getUtf8(),offset,numBytes,outBytes);
		}
	bool readFileBytes64W(const wchar_t *w1,__int64 offset64,int numBytes,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ReadFileBytes64(sTemp1.getUtf8(),offset64,numBytes,outBytes);
		}
	bool readFileBytes64sW(const wchar_t *w1,const wchar_t *w2,int numBytes,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ReadFileBytes64s(sTemp1.getUtf8(),sTemp2.getUtf8(),numBytes,outBytes);
		}
	bool readFileTextW(const wchar_t *w1,int numBytes,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ReadFileText(sTemp1.getUtf8(),numBytes,sTemp2.getUtf8(),outStr);
		}
	bool readFileText32W(const wchar_t *w1,int offset32,int numBytes,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ReadFileText32(sTemp1.getUtf8(),offset32,numBytes,sTemp2.getUtf8(),outStr);
		}
	bool readFileText64W(const wchar_t *w1,__int64 offset64,int numBytes,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ReadFileText64(sTemp1.getUtf8(),offset64,numBytes,sTemp2.getUtf8(),outStr);
		}
	bool readFileText64sW(const wchar_t *w1,const wchar_t *w2,int numBytes,const wchar_t *w3,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->ReadFileText64s(sTemp1.getUtf8(),sTemp2.getUtf8(),numBytes,sTemp3.getUtf8(),outStr);
		}
	bool realPathW(const wchar_t *w1,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->RealPath(sTemp1.getUtf8(),sTemp2.getUtf8(),outStr);
		}
	bool removeDirW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->RemoveDir(sTemp1.getUtf8());
		}
	bool removeFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->RemoveFile(sTemp1.getUtf8());
		}
	bool renameFileOrDirW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->RenameFileOrDir(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool resumeDownloadFileByNameW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ResumeDownloadFileByName(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool resumeUploadFileByNameW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ResumeUploadFileByName(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool setCreateTimeW(const wchar_t *w1,bool bIsHandle,SYSTEMTIME &createTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetCreateTime(sTemp1.getUtf8(),bIsHandle,createTime);
		}
	bool setLastAccessTimeW(const wchar_t *w1,bool bIsHandle,SYSTEMTIME &createTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetLastAccessTime(sTemp1.getUtf8(),bIsHandle,createTime);
		}
	bool setLastModifiedTimeW(const wchar_t *w1,bool bIsHandle,SYSTEMTIME &createTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetLastModifiedTime(sTemp1.getUtf8(),bIsHandle,createTime);
		}
	bool setOwnerAndGroupW(const wchar_t *w1,bool bIsHandle,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SetOwnerAndGroup(sTemp1.getUtf8(),bIsHandle,sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool setPermissionsW(const wchar_t *w1,bool bIsHandle,int perm)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetPermissions(sTemp1.getUtf8(),bIsHandle,perm);
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	bool uploadFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->UploadFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool uploadFileByNameW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->UploadFileByName(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool writeFileBytesW(const wchar_t *w1,CkByteData &data)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteFileBytes(sTemp1.getUtf8(),data);
		}
	bool writeFileBytes32W(const wchar_t *w1,int offset,CkByteData &data)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteFileBytes32(sTemp1.getUtf8(),offset,data);
		}
	bool writeFileBytes64W(const wchar_t *w1,__int64 offset64,CkByteData &data)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteFileBytes64(sTemp1.getUtf8(),offset64,data);
		}
	bool writeFileBytes64sW(const wchar_t *w1,const wchar_t *w2,CkByteData &data)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->WriteFileBytes64s(sTemp1.getUtf8(),sTemp2.getUtf8(),data);
		}
	bool writeFileTextW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->WriteFileText(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool writeFileText32W(const wchar_t *w1,int offset32,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->WriteFileText32(sTemp1.getUtf8(),offset32,sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool writeFileText64W(const wchar_t *w1,__int64 offset64,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->WriteFileText64(sTemp1.getUtf8(),offset64,sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool writeFileText64sW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		return this->WriteFileText64s(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8());
		}
	const wchar_t *add64W(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->Add64(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *disconnectReasonW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_DisconnectReason(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getFileGroupW(const wchar_t *w1,bool bFollowLinks,bool bIsHandle)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetFileGroup(sTemp1.getUtf8(),bFollowLinks,bIsHandle,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getFileOwnerW(const wchar_t *w1,bool bFollowLinks,bool bIsHandle)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetFileOwner(sTemp1.getUtf8(),bFollowLinks,bIsHandle,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getFileSizeStrW(const wchar_t *w1,bool bFollowLinks,bool bIsHandle)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetFileSizeStr(sTemp1.getUtf8(),bFollowLinks,bIsHandle,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *initializeFailReasonW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_InitializeFailReason(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorHtmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorHtml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorTextW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorText(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorXmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorXml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *openDirW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->OpenDir(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *openFileW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		this->OpenFile(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *readFileTextW(const wchar_t *w1,int numBytes,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->ReadFileText(sTemp1.getUtf8(),numBytes,sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *readFileText32W(const wchar_t *w1,int offset32,int numBytes,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->ReadFileText32(sTemp1.getUtf8(),offset32,numBytes,sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *readFileText64W(const wchar_t *w1,__int64 offset64,int numBytes,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->ReadFileText64(sTemp1.getUtf8(),offset64,numBytes,sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *readFileText64sW(const wchar_t *w1,const wchar_t *w2,int numBytes,const wchar_t *w3)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		this->ReadFileText64s(sTemp1.getUtf8(),sTemp2.getUtf8(),numBytes,sTemp3.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *realPathW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->RealPath(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *sessionLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SessionLog(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *versionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Version(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
