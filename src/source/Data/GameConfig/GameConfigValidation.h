#pragma once

#include <string>

namespace GameConfigValidation
{
int ValidateServerPort(int value, int defaultPort);
std::wstring ValidateServerIP(const std::wstring& value, const std::wstring& defaultIP);

// Trims whitespace, lowercases, and expands the "d3d12" alias to "direct3d12". Does NOT reject
// unrecognized values -- an unknown string is passed through as-is and left for
// SDL_CreateGPUDevice's own fail-and-fallback path to handle safely.
std::wstring ValidateRenderBackend(const std::wstring& value, const std::wstring& defaultValue);
}
