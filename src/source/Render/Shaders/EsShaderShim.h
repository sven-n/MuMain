#pragma once
// AH-1118: on Android the engine's `#version 330 core` shaders are transposed
// to GLSL ES (`#version 300 es`, with explicit default precision) at
// glShaderSource time, so the modern core-profile pipeline runs on a native
// OpenGL ES 3 context. Desktop builds resolve the real driver entry unchanged.
#ifdef __ANDROID__
#include <SDL3/SDL.h>
#include <cstring>
#include <string>

typedef void (*MU_PFNGLSHADERSOURCE)(unsigned int, int, const char* const*, const int*);

inline void MU_EsShaderSource(unsigned int shader, int count, const char* const* strings, const int* lengths)
{
    static MU_PFNGLSHADERSOURCE real =
        (MU_PFNGLSHADERSOURCE)SDL_GL_GetProcAddress("glShaderSource");

    std::string src;
    for (int i = 0; i < count; ++i)
    {
        if (strings[i] == nullptr)
        {
            continue;
        }
        if (lengths != nullptr && lengths[i] >= 0)
        {
            src.append(strings[i], static_cast<size_t>(lengths[i]));
        }
        else
        {
            src.append(strings[i]);
        }
    }

    // ES compilers require #version on the very first line; the engine's raw
    // string literals open with a newline that desktop drivers tolerated.
    const size_t firstContent = src.find_first_not_of(" \t\r\n");
    if (firstContent != std::string::npos && firstContent > 0)
    {
        src.erase(0, firstContent);
    }

    static const char kDesktop[] = "#version 330 core";
    const size_t pos = src.find(kDesktop);
    if (pos != std::string::npos)
    {
        src.replace(pos, sizeof(kDesktop) - 1,
                    "#version 300 es\n"
                    "precision highp float;\n"
                    "precision highp int;\n");
    }

    const char* one = src.c_str();
    real(shader, 1, &one, nullptr);
}

#define MU_GLSHADERSOURCE_PROC ((void*)&MU_EsShaderSource)
#else
#define MU_GLSHADERSOURCE_PROC SDL_GL_GetProcAddress("glShaderSource")
#endif
