// Story 7.6.1: Stub definitions for globals from Win32-only source files.
//
// ZzzOpenglUtil.cpp (WGL/Windows OpenGL) and ZzzLodTerrain.cpp (GL/glu.h)
// are excluded from non-Win32 builds. These files define globals that
// other compiled TUs reference via extern declarations.
// [VS0-QUAL-BUILDCOMP-MACOS]

#ifndef _WIN32

// From ZzzOpenglUtil.cpp — referenced by Core/CameraUtility.cpp
float g_fCameraCustomDistance = 0.f;

// From ZzzLodTerrain.cpp — referenced by Core/CameraUtility.cpp, Scenes/SceneCore.cpp
float g_fSpecialHeight = 1200.f;

#endif
