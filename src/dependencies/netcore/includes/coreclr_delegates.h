// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#ifndef __CORECLR_DELEGATES_H__
#define __CORECLR_DELEGATES_H__

#include <stdint.h>

// __stdcall is a no-op calling convention outside Windows and not a valid
// keyword on non-Windows GCC/Clang, so guard it the way upstream .NET does.
#if defined(_WIN32)
#define CORECLR_DELEGATE_CALLTYPE __stdcall
#else
#define CORECLR_DELEGATE_CALLTYPE
#endif
#endif // __CORECLR_DELEGATES_H__
