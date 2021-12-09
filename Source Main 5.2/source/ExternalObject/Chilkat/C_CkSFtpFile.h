#ifndef _CkSFtpFile_H
#define _CkSFtpFile_H
#include "Chilkat_C.h"

HCkSFtpFile CkSFtpFile_Create(void);
void CkSFtpFile_Dispose(HCkSFtpFile handle);
void CkSFtpFile_getCreateTime(HCkSFtpFile handle, SYSTEMTIME *retval);
void CkSFtpFile_getFileType(HCkSFtpFile handle, HCkString retval);
void CkSFtpFile_getFilename(HCkSFtpFile handle, HCkString retval);
int CkSFtpFile_getGid(HCkSFtpFile handle);
void CkSFtpFile_getGroup(HCkSFtpFile handle, HCkString retval);
BOOL CkSFtpFile_getIsAppendOnly(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsArchive(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsCaseInsensitive(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsCompressed(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsDirectory(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsEncrypted(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsHidden(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsImmutable(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsReadOnly(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsRegular(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsSparse(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsSymLink(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsSync(HCkSFtpFile handle);
BOOL CkSFtpFile_getIsSystem(HCkSFtpFile handle);
void CkSFtpFile_getLastAccessTime(HCkSFtpFile handle, SYSTEMTIME *retval);
void CkSFtpFile_getLastErrorHtml(HCkSFtpFile handle, HCkString retval);
void CkSFtpFile_getLastErrorText(HCkSFtpFile handle, HCkString retval);
void CkSFtpFile_getLastErrorXml(HCkSFtpFile handle, HCkString retval);
void CkSFtpFile_getLastModifiedTime(HCkSFtpFile handle, SYSTEMTIME *retval);
void CkSFtpFile_getOwner(HCkSFtpFile handle, HCkString retval);
int CkSFtpFile_getPermissions(HCkSFtpFile handle);
int CkSFtpFile_getSize32(HCkSFtpFile handle);
void CkSFtpFile_getSizeStr(HCkSFtpFile handle, HCkString retval);
int CkSFtpFile_getUid(HCkSFtpFile handle);
BOOL CkSFtpFile_getUtf8(HCkSFtpFile handle);
void CkSFtpFile_putUtf8(HCkSFtpFile handle, BOOL newVal);
BOOL CkSFtpFile_SaveLastError(HCkSFtpFile handle, const char *filename);
const char *CkSFtpFile_fileType(HCkSFtpFile handle);
const char *CkSFtpFile_filename(HCkSFtpFile handle);
const char *CkSFtpFile_group(HCkSFtpFile handle);
const char *CkSFtpFile_lastErrorHtml(HCkSFtpFile handle);
const char *CkSFtpFile_lastErrorText(HCkSFtpFile handle);
const char *CkSFtpFile_lastErrorXml(HCkSFtpFile handle);
const char *CkSFtpFile_owner(HCkSFtpFile handle);
const char *CkSFtpFile_sizeStr(HCkSFtpFile handle);
#endif
