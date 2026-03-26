#pragma once

// Lightweight header for mu::GetAudioDeviceNames() — avoids pulling in
// miniaudio.h (95k lines) via MiniAudioBackend.h.
// Single source of truth for this declaration (implementation in MiniAudioBackend.cpp).
// [Story 7-6-7: AC-6]

#include <string>
#include <vector>

namespace mu
{
std::vector<std::string> GetAudioDeviceNames();
} // namespace mu
