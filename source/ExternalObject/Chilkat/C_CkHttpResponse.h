#ifndef _CkHttpResponse_H
#define _CkHttpResponse_H
#include "Chilkat_C.h"

HCkHttpResponse CkHttpResponse_Create(void);
void CkHttpResponse_Dispose(HCkHttpResponse handle);
void CkHttpResponse_getBody(HCkHttpResponse handle, HCkByteData retval);
void CkHttpResponse_getBodyStr(HCkHttpResponse handle, HCkString retval);
void CkHttpResponse_getCharset(HCkHttpResponse handle, HCkString retval);
unsigned long CkHttpResponse_getContentLength(HCkHttpResponse handle);
void CkHttpResponse_getDate(HCkHttpResponse handle, SYSTEMTIME *retval);
void CkHttpResponse_getDomain(HCkHttpResponse handle, HCkString retval);
void CkHttpResponse_getHeader(HCkHttpResponse handle, HCkString retval);
int CkHttpResponse_getNumCookies(HCkHttpResponse handle);
int CkHttpResponse_getNumHeaderFields(HCkHttpResponse handle);
int CkHttpResponse_getStatusCode(HCkHttpResponse handle);
void CkHttpResponse_getStatusLine(HCkHttpResponse handle, HCkString retval);
BOOL CkHttpResponse_getUtf8(HCkHttpResponse handle);
void CkHttpResponse_putUtf8(HCkHttpResponse handle, BOOL newVal);
BOOL CkHttpResponse_GetCookieDomain(HCkHttpResponse handle, int index, HCkString str);
BOOL CkHttpResponse_GetCookieExpires(HCkHttpResponse handle, int index, SYSTEMTIME *sysTime);
BOOL CkHttpResponse_GetCookieExpiresStr(HCkHttpResponse handle, int index, HCkString str);
BOOL CkHttpResponse_GetCookieName(HCkHttpResponse handle, int index, HCkString str);
BOOL CkHttpResponse_GetCookiePath(HCkHttpResponse handle, int index, HCkString str);
BOOL CkHttpResponse_GetCookieValue(HCkHttpResponse handle, int index, HCkString str);
BOOL CkHttpResponse_GetHeaderField(HCkHttpResponse handle, const char *fieldName, HCkString str);
BOOL CkHttpResponse_GetHeaderFieldAttr(HCkHttpResponse handle, const char *fieldName, const char *attrName, HCkString str);
BOOL CkHttpResponse_GetHeaderName(HCkHttpResponse handle, int index, HCkString str);
BOOL CkHttpResponse_GetHeaderValue(HCkHttpResponse handle, int index, HCkString str);
const char *CkHttpResponse_bodyStr(HCkHttpResponse handle);
const char *CkHttpResponse_charset(HCkHttpResponse handle);
const char *CkHttpResponse_domain(HCkHttpResponse handle);
const char *CkHttpResponse_getCookieDomain(HCkHttpResponse handle, int index);
const char *CkHttpResponse_getCookieExpiresStr(HCkHttpResponse handle, int index);
const char *CkHttpResponse_getCookieName(HCkHttpResponse handle, int index);
const char *CkHttpResponse_getCookiePath(HCkHttpResponse handle, int index);
const char *CkHttpResponse_getCookieValue(HCkHttpResponse handle, int index);
const char *CkHttpResponse_getHeaderField(HCkHttpResponse handle, const char *fieldName);
const char *CkHttpResponse_getHeaderFieldAttr(HCkHttpResponse handle, const char *fieldName, const char *attrName);
const char *CkHttpResponse_getHeaderName(HCkHttpResponse handle, int index);
const char *CkHttpResponse_getHeaderValue(HCkHttpResponse handle, int index);
const char *CkHttpResponse_header(HCkHttpResponse handle);
const char *CkHttpResponse_statusLine(HCkHttpResponse handle);
#endif
