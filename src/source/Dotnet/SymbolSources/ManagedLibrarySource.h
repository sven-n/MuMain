#pragma once

// The adapter for targets that ship the network library as a NativeAOT library: desktop and Android.

#include <string>

#include "Core/Platform/PlatformLibrary.h"

// The library file; by bare name where the loader searches the executable's folder or the app's libraries (Windows,
// Android), otherwise beside the executable, which POSIX loaders don't search.
std::string ManagedLibraryPath();

// Null until LoadManagedLibrary, and after it when the library is missing.
extern mu::platform::LibraryHandle munique_client_library_handle;

// Loads the network library once, at startup after the client has chosen its folders; later calls do nothing.
void LoadManagedLibrary();
