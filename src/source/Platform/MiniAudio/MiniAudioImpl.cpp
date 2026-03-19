// Story 5.1.1: MuAudio Abstraction Layer [VS1-AUDIO-ABSTRACT-CORE]
//
// MiniAudioImpl.cpp — miniaudio + stb_vorbis implementation translation unit.
//
// IMPORTANT: This file MUST NOT use the precompiled header.
// It defines MINIAUDIO_IMPLEMENTATION which expands the entire library inline.
// Compiling with PCH would cause duplicate symbol errors.
//
// CMakeLists.txt must have:
//   set_source_files_properties(MiniAudioImpl.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c" // NOLINT — intentional .c include for single-file library
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
