#ifndef _CkCgi_H
#define _CkCgi_H
#include "Chilkat_C.h"

HCkCgi CkCgi_Create(void);
void CkCgi_Dispose(HCkCgi handle);
unsigned long CkCgi_getAsyncBytesRead(HCkCgi handle);
BOOL CkCgi_getAsyncInProgress(HCkCgi handle);
unsigned long CkCgi_getAsyncPostSize(HCkCgi handle);
BOOL CkCgi_getAsyncSuccess(HCkCgi handle);
void CkCgi_getDebugLogFilename(HCkCgi handle, HCkString retval);
void CkCgi_putDebugLogFilename(HCkCgi handle, const char *newVal);
int CkCgi_getHeartbeatMs(HCkCgi handle);
void CkCgi_putHeartbeatMs(HCkCgi handle, int newVal);
int CkCgi_getIdleTimeoutMs(HCkCgi handle);
void CkCgi_putIdleTimeoutMs(HCkCgi handle, int newVal);
void CkCgi_getLastErrorHtml(HCkCgi handle, HCkString retval);
void CkCgi_getLastErrorText(HCkCgi handle, HCkString retval);
void CkCgi_getLastErrorXml(HCkCgi handle, HCkString retval);
int CkCgi_getNumParams(HCkCgi handle);
int CkCgi_getNumUploadFiles(HCkCgi handle);
int CkCgi_getReadChunkSize(HCkCgi handle);
void CkCgi_putReadChunkSize(HCkCgi handle, int newVal);
unsigned long CkCgi_getSizeLimitKB(HCkCgi handle);
void CkCgi_putSizeLimitKB(HCkCgi handle, unsigned long newVal);
BOOL CkCgi_getStreamToUploadDir(HCkCgi handle);
void CkCgi_putStreamToUploadDir(HCkCgi handle, BOOL newVal);
void CkCgi_getUploadDir(HCkCgi handle, HCkString retval);
void CkCgi_putUploadDir(HCkCgi handle, const char *newVal);
BOOL CkCgi_getUtf8(HCkCgi handle);
void CkCgi_putUtf8(HCkCgi handle, BOOL newVal);
void CkCgi_getVersion(HCkCgi handle, HCkString retval);
void CkCgi_AbortAsync(HCkCgi handle);
BOOL CkCgi_GetEnv(HCkCgi handle, const char *varName, HCkString str);
BOOL CkCgi_GetParam(HCkCgi handle, const char *paramName, HCkString str);
BOOL CkCgi_GetParamName(HCkCgi handle, int idx, HCkString str);
BOOL CkCgi_GetParamValue(HCkCgi handle, int idx, HCkString str);
void CkCgi_GetRawPostData(HCkCgi handle, HCkByteData byteData);
BOOL CkCgi_GetUploadData(HCkCgi handle, int idx, HCkByteData byteData);
BOOL CkCgi_GetUploadFilename(HCkCgi handle, int idx, HCkString str);
unsigned long CkCgi_GetUploadSize(HCkCgi handle, int idx);
BOOL CkCgi_IsGet(HCkCgi handle);
BOOL CkCgi_IsHead(HCkCgi handle);
BOOL CkCgi_IsPost(HCkCgi handle);
BOOL CkCgi_IsUpload(HCkCgi handle);
BOOL CkCgi_ReadRequest(HCkCgi handle);
BOOL CkCgi_SaveLastError(HCkCgi handle, const char *filename);
void CkCgi_SleepMs(HCkCgi handle, int millisec);
const char *CkCgi_getEnv(HCkCgi handle, const char *varName);
const char *CkCgi_getParam(HCkCgi handle, const char *paramName);
const char *CkCgi_getParamName(HCkCgi handle, int idx);
const char *CkCgi_getParamValue(HCkCgi handle, int idx);
const char *CkCgi_getUploadFilename(HCkCgi handle, int idx);
const char *CkCgi_lastErrorHtml(HCkCgi handle);
const char *CkCgi_lastErrorText(HCkCgi handle);
const char *CkCgi_lastErrorXml(HCkCgi handle);
const char *CkCgi_version(HCkCgi handle);
#endif
