// standard leaf header

#ifndef _STDLEAF_H_
#define _STDLEAF_H_

#pragma once


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

    bool GetFileSizeQW(const HANDLE hFile, QWORD& qwSize);

}

#endif // _STDLEAF_H_
