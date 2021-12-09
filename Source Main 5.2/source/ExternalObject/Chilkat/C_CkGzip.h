#ifndef _CkGzip_H
#define _CkGzip_H
#include "Chilkat_C.h"

HCkGzip CkGzip_Create(void);
void CkGzip_Dispose(HCkGzip handle);
void CkGzip_getComment(HCkGzip handle, HCkString retval);
void CkGzip_putComment(HCkGzip handle, const char *newVal);
void CkGzip_getExtraData(HCkGzip handle, HCkByteData retval);
void CkGzip_putExtraData(HCkGzip handle, HCkByteData  newVal);
void CkGzip_getFilename(HCkGzip handle, HCkString retval);
void CkGzip_putFilename(HCkGzip handle, const char *newVal);
void CkGzip_getLastErrorHtml(HCkGzip handle, HCkString retval);
void CkGzip_getLastErrorText(HCkGzip handle, HCkString retval);
void CkGzip_getLastErrorXml(HCkGzip handle, HCkString retval);
void CkGzip_getLastMod(HCkGzip handle, SYSTEMTIME *retval);
void CkGzip_putLastMod(HCkGzip handle, SYSTEMTIME *newVal);
BOOL CkGzip_getUseCurrentDate(HCkGzip handle);
void CkGzip_putUseCurrentDate(HCkGzip handle, BOOL newVal);
BOOL CkGzip_getUtf8(HCkGzip handle);
void CkGzip_putUtf8(HCkGzip handle, BOOL newVal);
void CkGzip_getVersion(HCkGzip handle, HCkString retval);
BOOL CkGzip_CompressFile(HCkGzip handle, const char *inFilename, const char *outFilename);
BOOL CkGzip_CompressFile2(HCkGzip handle, const char *inFilename, const char *embeddedFilename, const char *outFilename);
BOOL CkGzip_CompressFileToMem(HCkGzip handle, const char *inFilename, HCkByteData db);
BOOL CkGzip_CompressMemToFile(HCkGzip handle, HCkByteData db, const char *outFilename);
BOOL CkGzip_CompressMemory(HCkGzip handle, HCkByteData dbIn, HCkByteData dbOut);
BOOL CkGzip_CompressString(HCkGzip handle, const char *inStr, const char *outCharset, HCkByteData outBytes);
BOOL CkGzip_CompressStringToFile(HCkGzip handle, const char *inStr, const char *outCharset, const char *outFilename);
BOOL CkGzip_Decode(HCkGzip handle, const char *str, const char *encoding, HCkByteData outBytes);
void CkGzip_DeflateStringENC(HCkGzip handle, const char *str, const char *charset, const char *encoding, HCkString strOut);
BOOL CkGzip_ExamineFile(HCkGzip handle, const char *inGzFilename);
BOOL CkGzip_ExamineMemory(HCkGzip handle, HCkByteData inGzData);
void CkGzip_InflateStringENC(HCkGzip handle, const char *str, const char *charset, const char *encoding, HCkString strOut);
BOOL CkGzip_IsUnlocked(HCkGzip handle);
BOOL CkGzip_ReadFile(HCkGzip handle, const char *filename, HCkByteData outBytes);
BOOL CkGzip_SaveLastError(HCkGzip handle, const char *filename);
BOOL CkGzip_UnTarGz(HCkGzip handle, const char *gzFilename, const char *destDir, BOOL bNoAbsolute);
BOOL CkGzip_UncompressFile(HCkGzip handle, const char *inFilename, const char *outFilename);
BOOL CkGzip_UncompressFileToMem(HCkGzip handle, const char *inFilename, HCkByteData db);
BOOL CkGzip_UncompressFileToString(HCkGzip handle, const char *inFilename, const char *inCharset, HCkString outStr);
BOOL CkGzip_UncompressMemToFile(HCkGzip handle, HCkByteData db, const char *outFilename);
BOOL CkGzip_UncompressMemory(HCkGzip handle, HCkByteData dbIn, HCkByteData dbOut);
BOOL CkGzip_UncompressString(HCkGzip handle, HCkByteData inData, const char *inCharset, HCkString outStr);
BOOL CkGzip_UnlockComponent(HCkGzip handle, const char *unlockCode);
BOOL CkGzip_WriteFile(HCkGzip handle, const char *filename, HCkByteData binaryData);
BOOL CkGzip_XfdlToXml(HCkGzip handle, const char *xfdl, HCkString outStr);
const char *CkGzip_comment(HCkGzip handle);
const char *CkGzip_deflateStringENC(HCkGzip handle, const char *str, const char *charset, const char *encoding);
const char *CkGzip_filename(HCkGzip handle);
const char *CkGzip_inflateStringENC(HCkGzip handle, const char *str, const char *charset, const char *encoding);
const char *CkGzip_lastErrorHtml(HCkGzip handle);
const char *CkGzip_lastErrorText(HCkGzip handle);
const char *CkGzip_lastErrorXml(HCkGzip handle);
const char *CkGzip_uncompressFileToString(HCkGzip handle, const char *inFilename, const char *inCharset);
const char *CkGzip_uncompressString(HCkGzip handle, HCkByteData inData, const char *inCharset);
const char *CkGzip_version(HCkGzip handle);
const char *CkGzip_xfdlToXml(HCkGzip handle, const char *xfdl);
#endif
