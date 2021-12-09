#ifndef _CkUnixCompress_H
#define _CkUnixCompress_H
#include "Chilkat_C.h"

HCkUnixCompress CkUnixCompress_Create(void);
void CkUnixCompress_Dispose(HCkUnixCompress handle);
void CkUnixCompress_getLastErrorHtml(HCkUnixCompress handle, HCkString retval);
void CkUnixCompress_getLastErrorText(HCkUnixCompress handle, HCkString retval);
void CkUnixCompress_getLastErrorXml(HCkUnixCompress handle, HCkString retval);
BOOL CkUnixCompress_getUtf8(HCkUnixCompress handle);
void CkUnixCompress_putUtf8(HCkUnixCompress handle, BOOL newVal);
BOOL CkUnixCompress_CompressFile(HCkUnixCompress handle, const char *inFilename, const char *outFilename);
BOOL CkUnixCompress_CompressFileToMem(HCkUnixCompress handle, const char *inFilename, HCkByteData db);
BOOL CkUnixCompress_CompressMemToFile(HCkUnixCompress handle, HCkByteData db, const char *outFilename);
BOOL CkUnixCompress_CompressMemory(HCkUnixCompress handle, HCkByteData dbIn, HCkByteData dbOut);
BOOL CkUnixCompress_CompressString(HCkUnixCompress handle, const char *inStr, const char *outCharset, HCkByteData outBytes);
BOOL CkUnixCompress_CompressStringToFile(HCkUnixCompress handle, const char *inStr, const char *outCharset, const char *outFilename);
BOOL CkUnixCompress_IsUnlocked(HCkUnixCompress handle);
BOOL CkUnixCompress_SaveLastError(HCkUnixCompress handle, const char *filename);
BOOL CkUnixCompress_UnTarZ(HCkUnixCompress handle, const char *zFilename, const char *destDir, BOOL bNoAbsolute);
BOOL CkUnixCompress_UncompressFile(HCkUnixCompress handle, const char *inFilename, const char *outFilename);
BOOL CkUnixCompress_UncompressFileToMem(HCkUnixCompress handle, const char *inFilename, HCkByteData db);
BOOL CkUnixCompress_UncompressFileToString(HCkUnixCompress handle, const char *inFilename, const char *inCharset, HCkString outStr);
BOOL CkUnixCompress_UncompressMemToFile(HCkUnixCompress handle, HCkByteData db, const char *outFilename);
BOOL CkUnixCompress_UncompressMemory(HCkUnixCompress handle, HCkByteData dbIn, HCkByteData dbOut);
BOOL CkUnixCompress_UncompressString(HCkUnixCompress handle, HCkByteData inData, const char *inCharset, HCkString outStr);
BOOL CkUnixCompress_UnlockComponent(HCkUnixCompress handle, const char *unlockCode);
const char *CkUnixCompress_lastErrorHtml(HCkUnixCompress handle);
const char *CkUnixCompress_lastErrorText(HCkUnixCompress handle);
const char *CkUnixCompress_lastErrorXml(HCkUnixCompress handle);
const char *CkUnixCompress_uncompressFileToString(HCkUnixCompress handle, const char *inFilename, const char *inCharset);
const char *CkUnixCompress_uncompressString(HCkUnixCompress handle, HCkByteData inData, const char *inCharset);
#endif
