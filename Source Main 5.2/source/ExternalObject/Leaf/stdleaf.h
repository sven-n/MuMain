 // standard leaf header

#ifndef _STDLEAF_H_
#define _STDLEAF_H_

#pragma once

#include <windows.h>

#pragma warning(disable : 4786)
#include <string>

#ifndef _QWORD_DEFINED_
#define _QWORD_DEFINED_

typedef __int64	QWORD;

#ifndef MAKEQWORD
#define MAKEQWORD(low, high) \
	((QWORD)( ((QWORD) ((DWORD) (high))) << 32 | ((DWORD) (low))))
#endif // !MAKEQWORD

#ifndef LODWORD
#define LODWORD(qw) \
	((DWORD)(qw))
#endif // !LODWORD

#ifndef HIDWORD
#define HIDWORD(qw) \
	((DWORD)(((QWORD)(qw) >> 32) & 0xFFFFFFFF))
#endif // !HIDWORD

#endif // _QWORD_DEFINED

#ifndef MAKEDOWRD
#define MAKEDWORD(low, high) \
	((DWORD)( ((DWORD) ((WORD) (high))) << 16 | ((WORD) (low))))
#endif // !MAKEDWORD

#ifndef MAKE_STRING
#define MAKE_STRING(name) "" #name
#endif // MAKE_STRING

namespace leaf {

	/* File I/O Functions */

	bool CreateDirectoryIncSub(const std::string& path);
	bool DeleteDirectoryIncSub(const std::string& path);

	bool GetFileSizeQW(const HANDLE hFile, QWORD &qwSize);

	void GetAbsolutePath(IN const std::string& path, OUT std::string& abspath);
	void GetAbsoluteFilePath(IN const std::string& path, OUT std::string& abspath, OUT std::string& filename);

	void SplitFileName(IN const std::string& filepath, OUT std::string& filename, bool bIncludeExt = true);
	void SplitDirectoryPath(IN const std::string& filepath, OUT std::string& dir);
	void SplitExt(IN const std::string& filepath, OUT std::string& ext, bool bIncludeDot = false);

	void ExtractDirectoryName(IN const std::string& path, OUT std::string& dirname);
	void ExchangeExt(IN const std::string& in_filepath, IN const std::string& ext, OUT std::string& out_filepath);

	bool CompareFilePath(IN const std::string& path1, IN const std::string& path2, size_t size);
	void AppendFilePath(IN const std::string& dir, IN const std::string& to_append, OUT std::string& out_path);

	/* Time Functions */
	
	bool GetFileCreationTime(IN const std::string& path, OUT SYSTEMTIME& systime, bool bLocalTime = true);
	bool GetFileLastModifiedTime(IN const std::string& path, OUT SYSTEMTIME& systime, bool bLocalTime = true);
	bool GetFileLastAccessedTime(IN const std::string& path, OUT SYSTEMTIME& systime, bool bLocalTime = true);
	
	/* Tools */
	
	const char* FormatString(const char* pFormat, ...);		//. return formatted string
	void GetTimeString(OUT std::string& str);				//. get time string ex) 2004/09/08 21:43:52:001
}

#endif // _STDLEAF_H_
