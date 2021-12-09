#ifndef _CkHttpRequest_H
#define _CkHttpRequest_H
#include "Chilkat_C.h"

HCkHttpRequest CkHttpRequest_Create(void);
void CkHttpRequest_Dispose(HCkHttpRequest handle);
void CkHttpRequest_getCharset(HCkHttpRequest handle, HCkString retval);
void CkHttpRequest_putCharset(HCkHttpRequest handle, const char *newVal);
void CkHttpRequest_getHttpVersion(HCkHttpRequest handle, HCkString retval);
void CkHttpRequest_putHttpVersion(HCkHttpRequest handle, const char *newVal);
void CkHttpRequest_getLastErrorHtml(HCkHttpRequest handle, HCkString retval);
void CkHttpRequest_getLastErrorText(HCkHttpRequest handle, HCkString retval);
void CkHttpRequest_getLastErrorXml(HCkHttpRequest handle, HCkString retval);
int CkHttpRequest_getNumHeaderFields(HCkHttpRequest handle);
int CkHttpRequest_getNumParams(HCkHttpRequest handle);
void CkHttpRequest_getPath(HCkHttpRequest handle, HCkString retval);
void CkHttpRequest_putPath(HCkHttpRequest handle, const char *newVal);
BOOL CkHttpRequest_getSendCharset(HCkHttpRequest handle);
void CkHttpRequest_putSendCharset(HCkHttpRequest handle, BOOL newVal);
BOOL CkHttpRequest_getUtf8(HCkHttpRequest handle);
void CkHttpRequest_putUtf8(HCkHttpRequest handle, BOOL newVal);
BOOL CkHttpRequest_AddFileForUpload(HCkHttpRequest handle, const char *name, const char *filename);
void CkHttpRequest_AddHeader(HCkHttpRequest handle, const char *name, const char *value);
void CkHttpRequest_AddParam(HCkHttpRequest handle, const char *name, const char *value);
void CkHttpRequest_GenerateRequestText(HCkHttpRequest handle, HCkString str);
BOOL CkHttpRequest_GetHeaderField(HCkHttpRequest handle, const char *name, HCkString str);
BOOL CkHttpRequest_GetHeaderName(HCkHttpRequest handle, int index, HCkString str);
BOOL CkHttpRequest_GetHeaderValue(HCkHttpRequest handle, int index, HCkString str);
BOOL CkHttpRequest_GetParam(HCkHttpRequest handle, const char *name, HCkString str);
BOOL CkHttpRequest_GetParamName(HCkHttpRequest handle, int index, HCkString str);
BOOL CkHttpRequest_GetParamValue(HCkHttpRequest handle, int index, HCkString str);
BOOL CkHttpRequest_LoadBodyFromBytes(HCkHttpRequest handle, HCkByteData binaryData);
BOOL CkHttpRequest_LoadBodyFromFile(HCkHttpRequest handle, const char *filename);
BOOL CkHttpRequest_LoadBodyFromString(HCkHttpRequest handle, const char *bodyStr, const char *charset);
void CkHttpRequest_RemoveAllParams(HCkHttpRequest handle);
void CkHttpRequest_RemoveParam(HCkHttpRequest handle, const char *name);
BOOL CkHttpRequest_SaveLastError(HCkHttpRequest handle, const char *filename);
void CkHttpRequest_SetFromUrl(HCkHttpRequest handle, const char *url);
void CkHttpRequest_UseGet(HCkHttpRequest handle);
void CkHttpRequest_UseHead(HCkHttpRequest handle);
void CkHttpRequest_UsePost(HCkHttpRequest handle);
void CkHttpRequest_UsePut(HCkHttpRequest handle);
void CkHttpRequest_UseUpload(HCkHttpRequest handle);
void CkHttpRequest_UseXmlHttp(HCkHttpRequest handle, const char *xmlBody);
const char *CkHttpRequest_charset(HCkHttpRequest handle);
const char *CkHttpRequest_generateRequestText(HCkHttpRequest handle);
const char *CkHttpRequest_getHeaderField(HCkHttpRequest handle, const char *name);
const char *CkHttpRequest_getHeaderName(HCkHttpRequest handle, int index);
const char *CkHttpRequest_getHeaderValue(HCkHttpRequest handle, int index);
const char *CkHttpRequest_getParam(HCkHttpRequest handle, const char *name);
const char *CkHttpRequest_getParamName(HCkHttpRequest handle, int index);
const char *CkHttpRequest_getParamValue(HCkHttpRequest handle, int index);
const char *CkHttpRequest_httpVersion(HCkHttpRequest handle);
const char *CkHttpRequest_lastErrorHtml(HCkHttpRequest handle);
const char *CkHttpRequest_lastErrorText(HCkHttpRequest handle);
const char *CkHttpRequest_lastErrorXml(HCkHttpRequest handle);
const char *CkHttpRequest_path(HCkHttpRequest handle);
#endif
