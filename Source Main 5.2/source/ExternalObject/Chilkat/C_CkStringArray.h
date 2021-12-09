#ifndef _CkStringArray_H
#define _CkStringArray_H
#include "Chilkat_C.h"

HCkStringArray CkStringArray_Create(void);
void CkStringArray_Dispose(HCkStringArray handle);
long CkStringArray_getCount(HCkStringArray handle);
BOOL CkStringArray_getCrlf(HCkStringArray handle);
void CkStringArray_putCrlf(HCkStringArray handle, BOOL newVal);
BOOL CkStringArray_getTrim(HCkStringArray handle);
void CkStringArray_putTrim(HCkStringArray handle, BOOL newVal);
BOOL CkStringArray_getUnique(HCkStringArray handle);
void CkStringArray_putUnique(HCkStringArray handle, BOOL newVal);
BOOL CkStringArray_getUtf8(HCkStringArray handle);
void CkStringArray_putUtf8(HCkStringArray handle, BOOL newVal);
void CkStringArray_Append(HCkStringArray handle, const char *str);
BOOL CkStringArray_AppendSerialized(HCkStringArray handle, const char *encodedStr);
void CkStringArray_Clear(HCkStringArray handle);
BOOL CkStringArray_Contains(HCkStringArray handle, const char *str);
long CkStringArray_Find(HCkStringArray handle, const char *str, long firstIndex);
BOOL CkStringArray_GetString(HCkStringArray handle, long index, HCkString str);
const char *CkStringArray_GetStringByIndex(HCkStringArray handle, long index);
void CkStringArray_InsertAt(HCkStringArray handle, long index, const char *str);
BOOL CkStringArray_LastString(HCkStringArray handle, HCkString str);
BOOL CkStringArray_LoadFromFile(HCkStringArray handle, const char *filename);
void CkStringArray_LoadFromText(HCkStringArray handle, const char *str);
BOOL CkStringArray_Pop(HCkStringArray handle, HCkString str);
void CkStringArray_Prepend(HCkStringArray handle, const char *str);
void CkStringArray_Remove(HCkStringArray handle, const char *str);
BOOL CkStringArray_RemoveAt(HCkStringArray handle, long index);
BOOL CkStringArray_SaveNthToFile(HCkStringArray handle, int index, const char *filename);
BOOL CkStringArray_SaveToFile(HCkStringArray handle, const char *filename);
void CkStringArray_SaveToText(HCkStringArray handle, HCkString outStr);
void CkStringArray_Serialize(HCkStringArray handle, HCkString encodedStr);
void CkStringArray_Sort(HCkStringArray handle, BOOL ascending);
void CkStringArray_SplitAndAppend(HCkStringArray handle, const char *str, const char *boundary);
const char *CkStringArray_getString(HCkStringArray handle, long index);
const char *CkStringArray_lastStr(HCkStringArray handle);
const char *CkStringArray_pop(HCkStringArray handle);
const char *CkStringArray_saveToText(HCkStringArray handle);
const char *CkStringArray_serialize(HCkStringArray handle);
const char *CkStringArray_strAt(HCkStringArray handle, long index);
#endif
