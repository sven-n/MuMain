#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

// The look preset ([Render] Look).
enum class LookPreset : std::uint8_t
{
    Classic,
    Enhanced,
};

namespace GameConfigValidation
{
int ValidateServerPort(int value, int defaultPort);
std::wstring ValidateServerIP(const std::wstring& value, const std::wstring& defaultIP);
// At least 1 (no anisotropy). The upper bound is what the GPU supports and is
// applied by the renderer, which logs when it clamps.
int ValidateAnisotropy(int value);
// 0 (off), 2, 4 or 8: other values round down to the nearest of those, 1 and
// below are off, above 8 is 8. What the GPU supports is applied by the
// renderer, which logs the count it uses.
int ValidateAntialiasing(int value);
// "Classic" or "Enhanced" in any case, surrounding blanks ignored; nothing otherwise.
std::optional<LookPreset> ParseLook(std::wstring_view text);
// The text ParseLook reads back as `preset`.
const wchar_t* FormatLook(LookPreset preset);
// A [Render] Look value, or the default when it is missing or invalid.
LookPreset ValidateLook(std::wstring_view text);

// Trims whitespace, lowercases, and expands the "d3d12" alias to "direct3d12". Does NOT reject
// unrecognized values -- an unknown string is passed through as-is and left for
// the renderer's device creation, which falls back to SDL's own pick when the named driver fails.
std::wstring ValidateRenderBackend(const std::wstring& value, const std::wstring& defaultValue);

// config.ini beside the executable, or `overridePath` when it is set and not empty (developer
// builds pass MU_CONFIG_PATH here, so a scripted client can run with settings of its own).
std::filesystem::path ResolveConfigPath(const std::filesystem::path& executableDirectory, const char* overridePath);
}
