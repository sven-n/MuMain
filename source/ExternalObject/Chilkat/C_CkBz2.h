#ifndef _CkBz2_H
#define _CkBz2_H
#include "Chilkat_C.h"

HCkBz2 CkBz2_Create(void);
void CkBz2_Dispose(HCkBz2 handle);
void CkBz2_getLastErrorHtml(HCkBz2 handle, HCkString retval);
void CkBz2_getLastErrorText(HCkBz2 handle, HCkString retval);
void CkBz2_getLastErrorXml(HCkBz2 handle, HCkString retval);
BOOL CkBz2_getUtf8(HCkBz2 handle);
void CkBz2_putUtf8(HCkBz2 handle, BOOL newVal);
BOOL CkBz2_CompressFile(HCkBz2 handle, const char *inFilename, const char *outFilename);
BOOL CkBz2_CompressFileToMem(HCkBz2 handle, const char *inFilename, HCkByteData outBytes);
BOOL CkBz2_CompressMemToFile(HCkBz2 handle, HCkByteData inData, const char *outFilename);
BOOL CkBz2_CompressMemory(HCkBz2 handle, HCkByteData inData, HCkByteData outBytes);
BOOL CkBz2_SaveLastError(HCkBz2 handle, const char *filename);
BOOL CkBz2_UncompressFile(HCkBz2 handle, const char *inFilename, const char *outFilename);
BOOL CkBz2_UncompressFileToMem(HCkBz2 handle, const char *inFilename, HCkByteData outBytes);
BOOL CkBz2_UncompressMemToFile(HCkBz2 handle, HCkByteData inData, const char *outFilename);
BOOL CkBz2_UncompressMemory(HCkBz2 handle, HCkByteData inData, HCkByteData outBytes);
BOOL CkBz2_UnlockComponent(HCkBz2 handle, const char *regCode);
const char *CkBz2_lastErrorHtml(HCkBz2 handle);
const char *CkBz2_lastErrorText(HCkBz2 handle);
const char *CkBz2_lastErrorXml(HCkBz2 handle);
#endif
