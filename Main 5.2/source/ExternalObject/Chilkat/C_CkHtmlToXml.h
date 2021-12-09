#ifndef _CkHtmlToXml_H
#define _CkHtmlToXml_H
#include "Chilkat_C.h"

HCkHtmlToXml CkHtmlToXml_Create(void);
void CkHtmlToXml_Dispose(HCkHtmlToXml handle);
BOOL CkHtmlToXml_getDropCustomTags(HCkHtmlToXml handle);
void CkHtmlToXml_putDropCustomTags(HCkHtmlToXml handle, BOOL newVal);
void CkHtmlToXml_getHtml(HCkHtmlToXml handle, HCkString retval);
void CkHtmlToXml_putHtml(HCkHtmlToXml handle, const char *newVal);
void CkHtmlToXml_getLastErrorHtml(HCkHtmlToXml handle, HCkString retval);
void CkHtmlToXml_getLastErrorText(HCkHtmlToXml handle, HCkString retval);
void CkHtmlToXml_getLastErrorXml(HCkHtmlToXml handle, HCkString retval);
long CkHtmlToXml_getNbsp(HCkHtmlToXml handle);
void CkHtmlToXml_putNbsp(HCkHtmlToXml handle, long newVal);
BOOL CkHtmlToXml_getUtf8(HCkHtmlToXml handle);
void CkHtmlToXml_putUtf8(HCkHtmlToXml handle, BOOL newVal);
void CkHtmlToXml_getVersion(HCkHtmlToXml handle, HCkString retval);
void CkHtmlToXml_getXmlCharset(HCkHtmlToXml handle, HCkString retval);
void CkHtmlToXml_putXmlCharset(HCkHtmlToXml handle, const char *newVal);
BOOL CkHtmlToXml_ConvertFile(HCkHtmlToXml handle, const char *inHtmlFilename, const char *outXmlFilename);
void CkHtmlToXml_DropTagType(HCkHtmlToXml handle, const char *tagName);
void CkHtmlToXml_DropTextFormattingTags(HCkHtmlToXml handle);
BOOL CkHtmlToXml_IsUnlocked(HCkHtmlToXml handle);
BOOL CkHtmlToXml_ReadFileToString(HCkHtmlToXml handle, const char *filename, const char *srcCharset, HCkString str);
BOOL CkHtmlToXml_SaveLastError(HCkHtmlToXml handle, const char *filename);
BOOL CkHtmlToXml_SetHtmlFromFile(HCkHtmlToXml handle, const char *filename);
BOOL CkHtmlToXml_ToXml(HCkHtmlToXml handle, HCkString str);
void CkHtmlToXml_UndropTagType(HCkHtmlToXml handle, const char *tagName);
void CkHtmlToXml_UndropTextFormattingTags(HCkHtmlToXml handle);
BOOL CkHtmlToXml_UnlockComponent(HCkHtmlToXml handle, const char *code);
BOOL CkHtmlToXml_WriteStringToFile(HCkHtmlToXml handle, const char *str, const char *filename, const char *charset);
const char *CkHtmlToXml_html(HCkHtmlToXml handle);
const char *CkHtmlToXml_lastErrorHtml(HCkHtmlToXml handle);
const char *CkHtmlToXml_lastErrorText(HCkHtmlToXml handle);
const char *CkHtmlToXml_lastErrorXml(HCkHtmlToXml handle);
const char *CkHtmlToXml_readFileToString(HCkHtmlToXml handle, const char *filename, const char *srcCharset);
const char *CkHtmlToXml_toXml(HCkHtmlToXml handle);
const char *CkHtmlToXml_version(HCkHtmlToXml handle);
const char *CkHtmlToXml_xml(HCkHtmlToXml handle);
const char *CkHtmlToXml_xmlCharset(HCkHtmlToXml handle);
#endif
