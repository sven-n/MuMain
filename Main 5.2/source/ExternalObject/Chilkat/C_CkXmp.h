#ifndef _CkXmp_H
#define _CkXmp_H
#include "Chilkat_C.h"

HCkXmp CkXmp_Create(void);
void CkXmp_Dispose(HCkXmp handle);
void CkXmp_getLastErrorHtml(HCkXmp cHandle, HCkString retval);
void CkXmp_getLastErrorText(HCkXmp cHandle, HCkString retval);
void CkXmp_getLastErrorXml(HCkXmp cHandle, HCkString retval);
long CkXmp_getNumEmbedded(HCkXmp cHandle);
BOOL CkXmp_getStructInnerDescrip(HCkXmp cHandle);
void CkXmp_putStructInnerDescrip(HCkXmp cHandle, BOOL newVal);
BOOL CkXmp_getUtf8(HCkXmp cHandle);
void CkXmp_putUtf8(HCkXmp cHandle, BOOL newVal);
void CkXmp_getVersion(HCkXmp cHandle, HCkString retval);
BOOL CkXmp_AddArray(HCkXmp cHandle, HCkXml xml, const char *arrType, const char *propName, HCkStringArray values);
void CkXmp_AddNsMapping(HCkXmp cHandle, const char *ns, const char *uri);
BOOL CkXmp_AddSimpleDate(HCkXmp cHandle, HCkXml xml, const char *propName, SYSTEMTIME *sysTime);
BOOL CkXmp_AddSimpleInt(HCkXmp cHandle, HCkXml xml, const char *propName, int propVal);
BOOL CkXmp_AddSimpleStr(HCkXmp cHandle, HCkXml xml, const char *propName, const char *propVal);
BOOL CkXmp_AddStructProp(HCkXmp cHandle, HCkXml xml, const char *structName, const char *propName, const char *propVal);
BOOL CkXmp_Append(HCkXmp cHandle, HCkXml xml);
BOOL CkXmp_DateToString(HCkXmp cHandle, SYSTEMTIME *sysTime, HCkString strOut);
BOOL CkXmp_GetArray(HCkXmp cHandle, HCkXml xml, const char *propName, HCkStringArray array);
HCkXml CkXmp_GetEmbedded(HCkXmp cHandle, int index);
BOOL CkXmp_GetSimpleDate(HCkXmp cHandle, HCkXml xml, const char *propName, SYSTEMTIME *sysTime);
int CkXmp_GetSimpleInt(HCkXmp cHandle, HCkXml xml, const char *propName);
BOOL CkXmp_GetSimpleStr(HCkXmp cHandle, HCkXml xml, const char *propName, HCkString strOut);
BOOL CkXmp_GetStructPropNames(HCkXmp cHandle, HCkXml xml, const char *structName, HCkStringArray array);
BOOL CkXmp_GetStructValue(HCkXmp cHandle, HCkXml xml, const char *structName, const char *propName, HCkString strOut);
BOOL CkXmp_LoadAppFile(HCkXmp cHandle, const char *filename);
BOOL CkXmp_LoadFromBuffer(HCkXmp cHandle, HCkByteData byteData, const char *ext);
void CkXmp_NewXmp(HCkXmp cHandle, HCkXml xmlOut);
void CkXmp_RemoveAllEmbedded(HCkXmp cHandle);
BOOL CkXmp_RemoveArray(HCkXmp cHandle, HCkXml xml, const char *propName);
void CkXmp_RemoveEmbedded(HCkXmp cHandle, int index);
void CkXmp_RemoveNsMapping(HCkXmp cHandle, const char *ns);
BOOL CkXmp_RemoveSimple(HCkXmp cHandle, HCkXml xml, const char *propName);
BOOL CkXmp_RemoveStruct(HCkXmp cHandle, HCkXml xml, const char *structName);
BOOL CkXmp_RemoveStructProp(HCkXmp cHandle, HCkXml xml, const char *structName, const char *propName);
BOOL CkXmp_SaveAppFile(HCkXmp cHandle, const char *filename);
BOOL CkXmp_SaveLastError(HCkXmp cHandle, const char *filename);
BOOL CkXmp_SaveToBuffer(HCkXmp cHandle, HCkByteData byteData);
BOOL CkXmp_StringToDate(HCkXmp cHandle, const char *str, SYSTEMTIME *sysTime);
BOOL CkXmp_UnlockComponent(HCkXmp cHandle, const char *unlockCode);
const char *CkXmp_dateToString(HCkXmp cHandle, SYSTEMTIME *sysTime);
const char *CkXmp_getSimpleStr(HCkXmp cHandle, HCkXml xml, const char *propName);
const char *CkXmp_getStructValue(HCkXmp cHandle, HCkXml xml, const char *structName, const char *propName);
const char *CkXmp_lastErrorHtml(HCkXmp cHandle);
const char *CkXmp_lastErrorText(HCkXmp cHandle);
const char *CkXmp_lastErrorXml(HCkXmp cHandle);
const char *CkXmp_simpleStr(HCkXmp cHandle, HCkXml xml, const char *propName);
const char *CkXmp_structValue(HCkXmp cHandle, HCkXml xml, const char *structName, const char *propName);
const char *CkXmp_version(HCkXmp cHandle);
#endif
