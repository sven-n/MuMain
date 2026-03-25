// GL/gl.h — Compat header for macOS/Linux native builds
// On macOS, <GL/gl.h> doesn't exist — Apple uses <OpenGL/gl.h>.
// However, stdafx.h already provides GL type stubs and function stubs,
// so this header is intentionally empty to avoid redefinition conflicts.
// [VS0-QUAL-BUILDCOMP-MACOS]
#pragma once
