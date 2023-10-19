#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "stdafx.h"

#include "stdleaf.h"

// GetTimeString
#include <time.h>
#include <sys/timeb.h>

bool leaf::GetFileSizeQW(const HANDLE hFile, QWORD& qwSize)
{
    if (hFile == NULL)
        return false;

    DWORD dwLo = 0, dwHi = 0;
    dwLo = GetFileSize(hFile, &dwHi);

    if (dwLo == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
        qwSize = 0;
        return false;
    }
    qwSize = MAKEQWORD(dwLo, dwHi);
    return true;
}
