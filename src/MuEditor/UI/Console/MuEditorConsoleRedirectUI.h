#pragma once

#ifdef _EDITOR

#include <cstdio>
#include <cstdarg>

// Redirect console output functions to ImGui console
#ifdef __cplusplus
extern "C" {
#endif

// Our replacement functions
int editor_wprintf(const wchar_t* format, ...);
int editor_fwprintf(FILE* stream, const wchar_t* format, ...);

#ifdef __cplusplus
}
#endif

// Replace standard functions with our versions
#define wprintf editor_wprintf
#define fwprintf editor_fwprintf

#endif // _EDITOR
