#pragma once

namespace mu::platform
{

/// Opaque handle to a dynamically loaded library.
using LibraryHandle = void*;

/// Loads a dynamic library from the given filesystem path.
/// @param path Null-terminated library path (platform-native format).
/// @return Non-null handle on success; nullptr on failure (error is logged).
[[nodiscard]] LibraryHandle Load(const char* path);

/// Resolves a symbol (function or variable) from a loaded library.
/// @param handle Valid library handle obtained from Load().
/// @param name Null-terminated symbol name.
/// @return Pointer to the symbol on success; nullptr on failure (error is logged).
[[nodiscard]] void* GetSymbol(LibraryHandle handle, const char* name);

/// Resolves a symbol the library may lack, such as an export newer than the library: unlike GetSymbol, a
/// missing symbol is no error and is not logged.
/// @param handle Library handle obtained from Load() (may be nullptr).
/// @param name Null-terminated symbol name.
/// @return Pointer to the symbol, or nullptr when the library has no such symbol.
[[nodiscard]] void* FindSymbol(LibraryHandle handle, const char* name);

/// Unloads a previously loaded library. Passing nullptr is a safe no-op.
/// @param handle Library handle to unload (may be nullptr).
void Unload(LibraryHandle handle);

} // namespace mu::platform
