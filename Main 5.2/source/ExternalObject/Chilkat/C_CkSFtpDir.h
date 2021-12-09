#ifndef _CkSFtpDir_H
#define _CkSFtpDir_H
#include "Chilkat_C.h"

HCkSFtpDir CkSFtpDir_Create(void);
void CkSFtpDir_Dispose(HCkSFtpDir handle);
void CkSFtpDir_getLastErrorHtml(HCkSFtpDir handle, HCkString retval);
void CkSFtpDir_getLastErrorText(HCkSFtpDir handle, HCkString retval);
void CkSFtpDir_getLastErrorXml(HCkSFtpDir handle, HCkString retval);
int CkSFtpDir_getNumFilesAndDirs(HCkSFtpDir handle);
void CkSFtpDir_getOriginalPath(HCkSFtpDir handle, HCkString retval);
BOOL CkSFtpDir_getUtf8(HCkSFtpDir handle);
void CkSFtpDir_putUtf8(HCkSFtpDir handle, BOOL newVal);
HCkSFtpFile CkSFtpDir_GetFileObject(HCkSFtpDir handle, int index);
BOOL CkSFtpDir_GetFilename(HCkSFtpDir handle, int index, HCkString outStr);
BOOL CkSFtpDir_SaveLastError(HCkSFtpDir handle, const char *filename);
const char *CkSFtpDir_getFilename(HCkSFtpDir handle, int index);
const char *CkSFtpDir_lastErrorHtml(HCkSFtpDir handle);
const char *CkSFtpDir_lastErrorText(HCkSFtpDir handle);
const char *CkSFtpDir_lastErrorXml(HCkSFtpDir handle);
const char *CkSFtpDir_originalPath(HCkSFtpDir handle);
#endif
