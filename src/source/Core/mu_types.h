#pragma once

#include <map>
#include <string>
#include "mu_enum.h"                // eBuffState
#include "Platform/PlatformTypes.h" // DWORD (no-op on Windows)

typedef char* PCHAR;
typedef char CHAR;
typedef std::string STRING;

typedef wchar_t* PWCHAR;
typedef wchar_t WCHAR;
typedef std::wstring WSTRING;
typedef std::map<eBuffState, DWORD> BuffStateMap;
