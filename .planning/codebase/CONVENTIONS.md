# Coding Conventions

**Analysis Date:** 2025-02-08

## Naming Patterns

**C++ Files:**
- PascalCase for classes: `CharacterManager.cpp`, `SceneManager.h`
- Prefix conventions: GM (map), CS (chaos/system), NewUI (UI)
- Global/defines: `_define.h`, `_struct.h`, `_enum.h`

**C# Files:**
- PascalCase: `ConnectionManager.cs`, `ConnectionWrapper.cs`
- Matches class name

**Functions:**
- camelCase in C#: `Connect`, `Send`, `ConnectInner`
- Mixed in C++: often PascalCase for class methods (e.g. `IsConnected`, `GetHandle`)

**Variables:**
- camelCase in C#: `host`, `port`, `Connections`
- C++: `m_` prefix for members in some classes; otherwise varies

**Types:**
- PascalCase for classes, structs, enums
- C#: `Nullable` enabled; avoid `any` (per user rule)

## Code Style

**C# Formatting:**
- `.editorconfig` for C#: tab_width 2, indent 2, crlf
- StyleCop: `stylecop.json` with MUnique company, MIT license header
- Diagnostics: SA1309 (underscore fields) = none, SA1516 (blank lines) = none, SA1615 (return docs) = suggestion

**C++ Formatting:**
- No explicit Prettier/ClangFormat config visible
- Follow existing indentation (appears 4 spaces)

**Linting:**
- C#: StyleCop.Analyzers, Microsoft.VisualStudio.Threading.Analyzers
- Warnings as errors: nullable, CS4014, VSTHRD110, VSTHRD100
- VSTHRD103 (call async in async) = error; VSTHRD111 (ConfigureAwait) = warning

## Import Organization

**C#:**
- Standard ordering: `using System;`, `using System.*`, `using MUnique.*`, `using Pipelines.*`
- Namespace: `MUnique.Client.Library` for ClientLibrary

**C++:**
- `#include "stdafx.h"` first where used
- Then project headers, then system (e.g. `windows.h`, `cwchar`)

## Error Handling

**C#:**
- `ConnectionManager_Connect`: try/catch, returns -1 on exception; `Debug.WriteLine` for errors
- No rethrow of internal details to unmanaged callers

**C++:**
- `Connection`: callbacks for `OnDisconnected`, `ReportDotNetError` for .NET load failures
- Return codes for connection status (`IsConnected`)

## Logging

**C#:**
- `Debug.WriteLine` for errors in ConnectionManager
- `Microsoft.Extensions.Logging.Abstractions` (NullLogger) referenced

**C++:**
- `ReportDotNetError` for .NET symbol load failures
- Limited structured logging in game code

## Comments

**C#:**
- XML doc required: `GenerateDocumentationFile` true
- `<summary>`, `<param>`, `<returns>` on public APIs
- Company/copyright header via StyleCop

**C++:**
- `//` style; no consistent Doxygen/JSDoc standard
- Explain non-obvious logic where needed

## Module Design

**C#:**
- `partial class ConnectionManager` for generated + custom code
- `[UnmanagedCallersOnly]` for C++-callable entry points
- Namespace: `MUnique.Client.Library`

**C++:**
- Header/cpp pairs; `#pragma once` for headers
- Dotnet bridge: `Connection` class wraps LoadLibrary + symbol calls

## Async/Threading

**C#:**
- Async used in ConnectionManager (`ConnectInner`, etc.)
- `ConfigureAwait` recommended (VSTHRD111)
- Async methods must call async (VSTHRD103 = error)

---

*Convention analysis: 2025-02-08*
*Update when patterns change*
