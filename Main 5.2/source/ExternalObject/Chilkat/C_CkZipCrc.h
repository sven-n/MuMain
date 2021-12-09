#ifndef _CkZipCrc_H
#define _CkZipCrc_H
#include "Chilkat_C.h"

HCkZipCrc CkZipCrc_Create(void);
void CkZipCrc_Dispose(HCkZipCrc handle);
BOOL CkZipCrc_getUtf8(HCkZipCrc handle);
void CkZipCrc_putUtf8(HCkZipCrc handle, BOOL newVal);
void CkZipCrc_BeginStream(HCkZipCrc handle);
unsigned long CkZipCrc_CalculateCrc(HCkZipCrc handle, HCkByteData byteData);
unsigned long CkZipCrc_EndStream(HCkZipCrc handle);
unsigned long CkZipCrc_FileCrc(HCkZipCrc handle, const char *filename);
void CkZipCrc_MoreData(HCkZipCrc handle, HCkByteData byteData);
void CkZipCrc_ToHex(HCkZipCrc handle, unsigned long crc, HCkString strHex);
const char *CkZipCrc_toHex(HCkZipCrc handle, unsigned long crc);
#endif
