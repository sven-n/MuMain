#ifndef _CkHtmlToText_H
#define _CkHtmlToText_H
#include "Chilkat_C.h"

HCkHtmlToText CkHtmlToText_Create(void);
void CkHtmlToText_Dispose(HCkHtmlToText handle);
void CkHtmlToText_getLastErrorHtml(HCkHtmlToText handle, HCkString retval);
void CkHtmlToText_getLastErrorText(HCkHtmlToText handle, HCkString retval);
void CkHtmlToText_getLastErrorXml(HCkHtmlToText handle, HCkString retval);
BOOL CkHtmlToText_getUtf8(HCkHtmlToText handle);
void CkHtmlToText_putUtf8(HCkHtmlToText handle, BOOL newVal);
BOOL CkHtmlToText_IsUnlocked(HCkHtmlToText handle);
BOOL CkHtmlToText_ReadFileToString(HCkHtmlToText handle, const char *filename, const char *srcCharset, HCkString str);
BOOL CkHtmlToText_SaveLastError(HCkHtmlToText handle, const char *filename);
BOOL CkHtmlToText_ToText(HCkHtmlToText handle, const char *html, HCkString outStr);
BOOL CkHtmlToText_UnlockComponent(HCkHtmlToText handle, const char *code);
BOOL CkHtmlToText_WriteStringToFile(HCkHtmlToText handle, const char *str, const char *filename, const char *charset);
const char *CkHtmlToText_lastErrorHtml(HCkHtmlToText handle);
const char *CkHtmlToText_lastErrorText(HCkHtmlToText handle);
const char *CkHtmlToText_lastErrorXml(HCkHtmlToText handle);
const char *CkHtmlToText_readFileToString(HCkHtmlToText handle, const char *filename, const char *srcCharset);
const char *CkHtmlToText_toText(HCkHtmlToText handle, const char *html);
#endif
