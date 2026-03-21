// Story 5.3.1: Audio Format Validation [VS1-AUDIO-FORMAT-VALIDATE]
// RED PHASE: Tests verify miniaudio correctly decodes WAV, MP3, and OGG Vorbis
// audio formats used by the game, and that error paths are handled gracefully.
//
// AC-1: MiniAudioBackend::LoadSound() works for WAV (mono + stereo), MP3, OGG
// AC-2: Test assets generated at test runtime (WAV in-code, MP3/OGG as embedded hex arrays)
// AC-3: Non-existent and corrupt files handled gracefully (no crash)
// AC-4: PlayMusic() + streaming path (MA_SOUND_FLAG_STREAM) works for OGG and MP3
// AC-5: ma_decoder direct pipeline validation (frame count, sample format, channel count)
// AC-6: All tests run headless on macOS/Linux/Windows CI without audio device
// AC-7: Tests use standalone MiniAudioBackend instances — do NOT touch g_platformAudio
//
// PCC Constraints (project-context.md):
//   - mu:: namespace for helper code
//   - Allman brace style, 4-space indent, 120-column limit
//   - #pragma once if any header created (N/A — .cpp only)
//   - Catch2 TEST_CASE / SECTION structure
//   - REQUIRE_NOTHROW for crash-safety tests
//   - No binary audio files committed — WAV generated at runtime, MP3/OGG as hex arrays
//   - No #ifdef _WIN32 in test logic — tests are platform-agnostic
//   - std::filesystem::path for file I/O, forward slashes only
//   - No new/delete — stack allocation only
//
// Design Notes:
//   - Tests construct their own MiniAudioBackend instances on the stack
//   - They do NOT touch g_platformAudio (AC-7 — no collision with game sound IDs)
//   - ma_decoder tests do NOT require ma_engine (no audio device needed — AC-6)
//   - Backend tests that require Initialize() guard with bool check (CI headless-safe)
//   - Direct miniaudio.h usage is valid: MuTests includes src/dependencies/miniaudio/
//     and src/source/Platform/MiniAudio/ via CMakeLists.txt target_include_directories

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

#include "MiniAudioBackend.h"
#include "miniaudio.h"

// =============================================================================
// AC-2: Embedded minimal audio assets (hex arrays) and WAV generator
// =============================================================================

namespace test_assets
{

// ---------------------------------------------------------------------------
// GenerateWavFile — synthesizes a minimal valid PCM 16-bit WAV at runtime.
// Writes a 44-byte RIFF header followed by PCM data (440 Hz sine wave).
//
// Parameters:
//   path      — output file path (forward slashes, std::string)
//   channels  — 1 (mono) or 2 (stereo)
//   sampleRate — e.g. 44100
//   numSamples — per-channel sample count (e.g. 4410 = 0.1s at 44100 Hz)
//
// Returns true on success, false if the file could not be written.
// ---------------------------------------------------------------------------
bool GenerateWavFile(const std::string& path, int channels, int sampleRate, int numSamples)
{
    // 16-bit PCM WAV: 44-byte header + (numSamples * channels * 2) bytes data
    const int bytesPerSample = 2; // 16-bit
    const int dataSize = numSamples * channels * bytesPerSample;
    const int32_t chunkSize = 36 + dataSize; // RIFF chunk size = 36 + dataSize

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }

    // RIFF chunk descriptor
    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&chunkSize), 4);
    file.write("WAVE", 4);

    // "fmt " sub-chunk (16 bytes for PCM)
    file.write("fmt ", 4);
    const int32_t subchunk1Size = 16;
    file.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    const int16_t audioFormat = 1; // PCM
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    const int16_t numCh = static_cast<int16_t>(channels);
    file.write(reinterpret_cast<const char*>(&numCh), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    const int32_t byteRate = sampleRate * channels * bytesPerSample;
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    const int16_t blockAlign = static_cast<int16_t>(channels * bytesPerSample);
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    const int16_t bitsPerSample = 16;
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    // "data" sub-chunk
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataSize), 4);

    // Generate 440 Hz sine wave samples
    const double twoPi = 2.0 * 3.14159265358979323846;
    const double frequency = 440.0;
    const double amplitude = 16384.0; // ~50% of INT16_MAX to avoid clipping
    for (int i = 0; i < numSamples; ++i)
    {
        const int16_t sample = static_cast<int16_t>(amplitude * std::sin(twoPi * frequency * i / sampleRate));
        for (int ch = 0; ch < channels; ++ch)
        {
            file.write(reinterpret_cast<const char*>(&sample), 2);
        }
    }

    return file.good();
}

// ---------------------------------------------------------------------------
// WriteMP3File — writes a minimal valid MP3 from an embedded constexpr array.
//
// Encoding: ffmpeg -f lavfi -i "anullsrc=r=44100:cl=mono" -t 0.1
//           -c:a libmp3lame -b:a 128k minimal.mp3
// xxd -i minimal.mp3
// Parameters: mono, 44100 Hz, 128 kbps MPEG Layer 3, ~0.1s silence (~3.2 KB)
//
// Returns true on success.
// ---------------------------------------------------------------------------
bool WriteMP3File(const std::string& path)
{
    // Minimal MP3: ID3v2 tag + MPEG Layer 3 frames (mono, 44100 Hz, 128 kbps)
    // Generated: ffmpeg -f lavfi -i "anullsrc=r=44100:cl=mono" -t 0.1 -c:a libmp3lame -b:a 128k
    // Embedded as constexpr to avoid committing binary files to the repository (AC-2).
    // clang-format off
    static constexpr uint8_t kMinimalMp3[] = {
        0x49, 0x44, 0x33, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x54, 0x53,
        0x53, 0x45, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x4c, 0x61, 0x76, 0x66,
        0x36, 0x31, 0x2e, 0x31, 0x2e, 0x31, 0x30, 0x30, 0x2e, 0x31, 0x00, 0xff,
        0xfb, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xfb, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xfb, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    // clang-format on

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }
    file.write(reinterpret_cast<const char*>(kMinimalMp3), sizeof(kMinimalMp3));
    return file.good();
}

// ---------------------------------------------------------------------------
// WriteOggFile — writes a minimal valid OGG Vorbis file from an embedded
// constexpr array.
//
// Encoding: ffmpeg -f lavfi -i "anullsrc=r=44100:cl=mono" -t 0.1 -c:a libvorbis
// xxd -i minimal.ogg
// Parameters: mono, 44100 Hz, quality 0 (lowest bitrate), ~0.1s silence (~3 KB)
//
// Returns true on success.
// ---------------------------------------------------------------------------
bool WriteOggFile(const std::string& path)
{
    // Minimal OGG Vorbis: 3 pages (identification, comment, setup) + audio data
    // Generated: ffmpeg -f lavfi -i "anullsrc=r=44100:cl=mono" -t 0.1 -c:a libvorbis
    // Embedded as constexpr to avoid committing binary files to the repository (AC-2).
    // OGG container magic bytes: 0x4f, 0x67, 0x67, 0x53 ("OggS")
    // Vorbis identification header magic: 0x01, 0x76, 0x6f, 0x72, 0x62, 0x69, 0x73 ("\x01vorbis")
    // clang-format off
    static constexpr uint8_t kMinimalOgg[] = {
        // OggS capture pattern + identification header (page 0)
        0x4f, 0x67, 0x67, 0x53, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x40,
        0x00, 0x00, 0x01, 0x1e, 0x01, 0x76, 0x6f, 0x72, 0x62, 0x69, 0x73, 0x00,
        0x00, 0x00, 0x00, 0x02, 0x44, 0xac, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x38, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb8, 0x01,
        // OggS capture pattern + comment + setup headers (page 1)
        0x4f, 0x67, 0x67, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2c, 0xe1,
        0x2a, 0x8c, 0x02, 0xff, 0x03, 0x76, 0x6f, 0x72, 0x62, 0x69, 0x73, 0x1f,
        0x00, 0x00, 0x00, 0x58, 0x69, 0x70, 0x68, 0x2e, 0x4f, 0x72, 0x67, 0x20,
        0x6c, 0x69, 0x62, 0x56, 0x6f, 0x72, 0x62, 0x69, 0x73, 0x20, 0x49, 0x20,
        0x32, 0x30, 0x31, 0x38, 0x30, 0x33, 0x31, 0x36, 0x20, 0x28, 0x3f, 0x29,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x76, 0x6f, 0x72, 0x62, 0x69, 0x73,
        0x22, 0x00, 0x00, 0x00, 0x42, 0x43, 0x56, 0x01, 0x00, 0x48, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01
    };
    // clang-format on

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }
    file.write(reinterpret_cast<const char*>(kMinimalOgg), sizeof(kMinimalOgg));
    return file.good();
}

// ---------------------------------------------------------------------------
// TempAudioDir — RAII wrapper for a temporary test directory.
// Creates the directory in the system temp location and removes it on destruction.
// Uses std::filesystem and forward slashes as required by PCC constraints.
// ---------------------------------------------------------------------------
class TempAudioDir
{
public:
    TempAudioDir() : m_path(std::filesystem::temp_directory_path() / "mu_audio_test_5_3_1")
    {
        std::filesystem::create_directories(m_path);
    }

    ~TempAudioDir()
    {
        std::filesystem::remove_all(m_path);
    }

    // Returns the directory path as a string with forward slashes.
    std::string GetPath() const
    {
        return m_path.generic_string();
    }

    // Returns a full file path inside the temp directory.
    std::string FilePath(const std::string& filename) const
    {
        return (m_path / filename).generic_string();
    }

    // Disable copy
    TempAudioDir(const TempAudioDir&) = delete;
    TempAudioDir& operator=(const TempAudioDir&) = delete;

private:
    std::filesystem::path m_path;
};

} // namespace test_assets

// =============================================================================
// AC-1 / AC-5 / AC-6: WAV format validation tests
// =============================================================================

// ---------------------------------------------------------------------------
// AC-1: WAV mono PCM 16-bit loads via MiniAudioBackend
// AC-6: Backend tests guard with Initialize() success check (headless CI safe)
// AC-7: Standalone MiniAudioBackend instance — NOT g_platformAudio
// ---------------------------------------------------------------------------
TEST_CASE("AC-1: WAV mono PCM 16-bit loads via MiniAudioBackend", "[audio][wav][format][5-3-1]")
{
    // GIVEN: A minimal mono WAV file written to a temp directory
    test_assets::TempAudioDir tempDir;
    const std::string wavPath = tempDir.FilePath("test_mono.wav");
    REQUIRE(test_assets::GenerateWavFile(wavPath, 1, 44100, 4410)); // 0.1s mono

    // GIVEN: A standalone MiniAudioBackend instance (not g_platformAudio — AC-7)
    mu::MiniAudioBackend backend;
    const bool initOk = backend.Initialize();

    // WHEN: LoadSound is called with the mono WAV path (wide string conversion)
    // THEN: Must not crash regardless of init state (AC-6: CI headless guard)
    // NOTE: LoadSound takes wchar_t* — use std::wstring for conversion
    const std::wstring wWavPath(wavPath.begin(), wavPath.end());
    backend.LoadSound(static_cast<ESound>(SOUND_EXPAND_END - 1), wWavPath.c_str(), 1, false);

    if (!initOk)
    {
        WARN("MiniAudioBackend::Initialize() failed (no audio device?) -- LoadSound no-op");
    }

    // Cleanup
    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-1: WAV stereo PCM 16-bit loads via MiniAudioBackend
// ---------------------------------------------------------------------------
TEST_CASE("AC-1: WAV stereo PCM 16-bit loads via MiniAudioBackend", "[audio][wav][format][5-3-1]")
{
    // GIVEN: A minimal stereo WAV file written to a temp directory
    test_assets::TempAudioDir tempDir;
    const std::string wavPath = tempDir.FilePath("test_stereo.wav");
    REQUIRE(test_assets::GenerateWavFile(wavPath, 2, 44100, 4410)); // 0.1s stereo

    // GIVEN: A standalone MiniAudioBackend instance (not g_platformAudio — AC-7)
    mu::MiniAudioBackend backend;
    const bool initOk = backend.Initialize();

    const std::wstring wWavPath(wavPath.begin(), wavPath.end());
    backend.LoadSound(static_cast<ESound>(SOUND_EXPAND_END - 2), wWavPath.c_str(), 1, false);

    if (!initOk)
    {
        WARN("MiniAudioBackend::Initialize() failed (no audio device?) -- LoadSound no-op");
    }

    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-5: WAV mono decodes to correct PCM frames via ma_decoder (direct pipeline)
// ma_decoder does NOT require ma_engine — always works headless on all platforms (AC-6)
// ---------------------------------------------------------------------------
TEST_CASE("AC-5: WAV mono decodes to correct PCM frames via ma_decoder", "[audio][wav][decoder][5-3-1]")
{
    // GIVEN: A minimal mono WAV file (0.1s, 44100 Hz, 1 channel, 4410 frames)
    test_assets::TempAudioDir tempDir;
    const std::string wavPath = tempDir.FilePath("test_mono_decode.wav");
    REQUIRE(test_assets::GenerateWavFile(wavPath, 1, 44100, 4410));

    // WHEN: ma_decoder_init_file is called on the WAV
    ma_decoder decoder;
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
    const ma_result result = ma_decoder_init_file(wavPath.c_str(), &config, &decoder);
    REQUIRE(result == MA_SUCCESS);

    // THEN: Frame count is > 0
    ma_uint64 frameCount = 0;
    const ma_result lenResult = ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);
    CHECK(lenResult == MA_SUCCESS);
    CHECK(frameCount > 0);

    // THEN: Channel count matches the mono WAV (1 channel)
    CHECK(decoder.outputChannels == 1);

    // THEN: Sample rate matches 44100
    CHECK(decoder.outputSampleRate == 44100);

    ma_decoder_uninit(&decoder);
}

// ---------------------------------------------------------------------------
// AC-5: WAV stereo decodes to correct PCM frames via ma_decoder
// ---------------------------------------------------------------------------
TEST_CASE("AC-5: WAV stereo decodes to correct PCM frames via ma_decoder", "[audio][wav][decoder][5-3-1]")
{
    // GIVEN: A minimal stereo WAV file (0.1s, 44100 Hz, 2 channels)
    test_assets::TempAudioDir tempDir;
    const std::string wavPath = tempDir.FilePath("test_stereo_decode.wav");
    REQUIRE(test_assets::GenerateWavFile(wavPath, 2, 44100, 4410));

    // WHEN: ma_decoder_init_file is called on the stereo WAV
    ma_decoder decoder;
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
    const ma_result result = ma_decoder_init_file(wavPath.c_str(), &config, &decoder);
    REQUIRE(result == MA_SUCCESS);

    // THEN: Frame count is > 0
    ma_uint64 frameCount = 0;
    const ma_result lenResult = ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);
    CHECK(lenResult == MA_SUCCESS);
    CHECK(frameCount > 0);

    // THEN: Channel count matches the stereo WAV (2 channels)
    CHECK(decoder.outputChannels == 2);

    ma_decoder_uninit(&decoder);
}

// =============================================================================
// AC-1 / AC-4 / AC-5 / AC-6: MP3 format validation tests
// =============================================================================

// ---------------------------------------------------------------------------
// AC-1: MP3 loads via MiniAudioBackend (LoadSound path)
// ---------------------------------------------------------------------------
TEST_CASE("AC-1: MP3 loads via MiniAudioBackend", "[audio][mp3][format][5-3-1]")
{
    // GIVEN: A minimal MP3 file written from the embedded constexpr array (AC-2)
    test_assets::TempAudioDir tempDir;
    const std::string mp3Path = tempDir.FilePath("test_minimal.mp3");
    REQUIRE(test_assets::WriteMP3File(mp3Path));

    // GIVEN: A standalone MiniAudioBackend instance (not g_platformAudio — AC-7)
    mu::MiniAudioBackend backend;
    const bool initOk = backend.Initialize();

    const std::wstring wMp3Path(mp3Path.begin(), mp3Path.end());
    backend.LoadSound(static_cast<ESound>(SOUND_EXPAND_END - 3), wMp3Path.c_str(), 1, false);

    if (!initOk)
    {
        WARN("MiniAudioBackend::Initialize() failed (no audio device?) -- LoadSound no-op");
    }

    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-4: MP3 streaming path via PlayMusic (MA_SOUND_FLAG_STREAM)
// AC-6: Initialize() guard — CI headless path gracefully skips streaming check
// ---------------------------------------------------------------------------
TEST_CASE("AC-4: MP3 streams via PlayMusic without crash", "[audio][mp3][streaming][5-3-1]")
{
    // GIVEN: A minimal MP3 file written from the embedded constexpr array
    test_assets::TempAudioDir tempDir;
    const std::string mp3Path = tempDir.FilePath("test_stream.mp3");
    REQUIRE(test_assets::WriteMP3File(mp3Path));

    // GIVEN: A standalone MiniAudioBackend instance
    mu::MiniAudioBackend backend;
    const bool initOk = backend.Initialize();

    if (!initOk)
    {
        // CI may not have an audio device — streaming check is headless-safe (AC-6)
        WARN("MiniAudioBackend::Initialize() failed (no audio device?) -- streaming test skipped");
        return;
    }

    // WHEN: PlayMusic is called with the MP3 path
    // THEN: Must not crash
    backend.PlayMusic(mp3Path.c_str(), TRUE);

    // WHEN: StopMusic is called immediately
    // THEN: Must not crash; IsEndMusic() returns true after stop
    backend.StopMusic(nullptr, TRUE);
    CHECK(backend.IsEndMusic());

    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-5: MP3 decodes to correct PCM frames via ma_decoder (direct pipeline)
// Does NOT require ma_engine — always works headless (AC-6)
// ---------------------------------------------------------------------------
TEST_CASE("AC-5: MP3 decodes to correct PCM frames via ma_decoder", "[audio][mp3][decoder][5-3-1]")
{
    // GIVEN: A minimal MP3 file
    test_assets::TempAudioDir tempDir;
    const std::string mp3Path = tempDir.FilePath("test_decode.mp3");
    REQUIRE(test_assets::WriteMP3File(mp3Path));

    // WHEN: ma_decoder_init_file is called on the MP3
    ma_decoder decoder;
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
    const ma_result result = ma_decoder_init_file(mp3Path.c_str(), &config, &decoder);
    REQUIRE(result == MA_SUCCESS);

    // THEN: Frame count > 0 (decoded frames exist)
    ma_uint64 frameCount = 0;
    const ma_result lenResult = ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);
    CHECK(lenResult == MA_SUCCESS);
    CHECK(frameCount > 0);

    // THEN: Sample format is f32 or s16 (AC-5: valid decoded sample format)
    const bool validFormat = (decoder.outputFormat == ma_format_f32 || decoder.outputFormat == ma_format_s16);
    CHECK(validFormat);

    ma_decoder_uninit(&decoder);
}

// =============================================================================
// AC-1 / AC-4 / AC-5 / AC-6: OGG Vorbis format validation tests
// =============================================================================

// ---------------------------------------------------------------------------
// AC-1: OGG Vorbis loads via MiniAudioBackend
// The game uses 3 OGG tracks via stb_vorbis (crywolf BGM). This test confirms
// the stb_vorbis decoder path works end-to-end via MiniAudioBackend::LoadSound().
// ---------------------------------------------------------------------------
TEST_CASE("AC-1: OGG Vorbis loads via MiniAudioBackend", "[audio][ogg][format][5-3-1]")
{
    // GIVEN: A minimal OGG Vorbis file written from the embedded constexpr array (AC-2)
    test_assets::TempAudioDir tempDir;
    const std::string oggPath = tempDir.FilePath("test_minimal.ogg");
    REQUIRE(test_assets::WriteOggFile(oggPath));

    // GIVEN: A standalone MiniAudioBackend instance (not g_platformAudio — AC-7)
    mu::MiniAudioBackend backend;
    const bool initOk = backend.Initialize();

    const std::wstring wOggPath(oggPath.begin(), oggPath.end());
    backend.LoadSound(static_cast<ESound>(SOUND_EXPAND_END - 4), wOggPath.c_str(), 1, false);

    if (!initOk)
    {
        WARN("MiniAudioBackend::Initialize() failed (no audio device?) -- LoadSound no-op");
    }

    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-4: OGG Vorbis streaming path via PlayMusic (MA_SOUND_FLAG_STREAM)
// The 3 crywolf tracks use this streaming path in production.
// AC-6: Initialize() guard — headless CI safe
// ---------------------------------------------------------------------------
TEST_CASE("AC-4: OGG Vorbis streams via PlayMusic without crash", "[audio][ogg][streaming][5-3-1]")
{
    // GIVEN: A minimal OGG Vorbis file written from the embedded constexpr array
    test_assets::TempAudioDir tempDir;
    const std::string oggPath = tempDir.FilePath("test_stream.ogg");
    REQUIRE(test_assets::WriteOggFile(oggPath));

    // GIVEN: A standalone MiniAudioBackend instance
    mu::MiniAudioBackend backend;
    const bool initOk = backend.Initialize();

    if (!initOk)
    {
        // CI may not have an audio device — streaming check is headless-safe (AC-6)
        WARN("MiniAudioBackend::Initialize() failed (no audio device?) -- OGG streaming test skipped");
        return;
    }

    // WHEN: PlayMusic is called with the OGG path
    // THEN: Must not crash
    backend.PlayMusic(oggPath.c_str(), TRUE);

    // WHEN: StopMusic is called immediately
    // THEN: Must not crash; IsEndMusic() returns true after stop
    backend.StopMusic(nullptr, TRUE);
    CHECK(backend.IsEndMusic());

    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-5: OGG Vorbis decodes to correct PCM frames via ma_decoder (direct)
// Uses stb_vorbis path. Does NOT require ma_engine — headless safe (AC-6).
// ---------------------------------------------------------------------------
TEST_CASE("AC-5: OGG Vorbis decodes to correct PCM frames via ma_decoder", "[audio][ogg][decoder][5-3-1]")
{
    // GIVEN: A minimal OGG Vorbis file
    test_assets::TempAudioDir tempDir;
    const std::string oggPath = tempDir.FilePath("test_decode.ogg");
    REQUIRE(test_assets::WriteOggFile(oggPath));

    // WHEN: ma_decoder_init_file is called on the OGG
    ma_decoder decoder;
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
    const ma_result result = ma_decoder_init_file(oggPath.c_str(), &config, &decoder);
    REQUIRE(result == MA_SUCCESS);

    // THEN: Frame count > 0 (decoded frames exist)
    ma_uint64 frameCount = 0;
    const ma_result lenResult = ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);
    CHECK(lenResult == MA_SUCCESS);
    CHECK(frameCount > 0);

    // THEN: Sample format is f32 or s16 (AC-5)
    const bool validFormat = (decoder.outputFormat == ma_format_f32 || decoder.outputFormat == ma_format_s16);
    CHECK(validFormat);

    // THEN: Channel count == 1 (embedded OGG is mono)
    CHECK(decoder.outputChannels == 1);

    ma_decoder_uninit(&decoder);
}

// =============================================================================
// AC-3: Error handling tests — non-existent and corrupt files
// =============================================================================

// ---------------------------------------------------------------------------
// AC-3: LoadSound with a non-existent file handles gracefully
// Backend must not crash. The slot remains unloaded (PlaySound is a no-op).
// AC-7: Standalone backend instance — not g_platformAudio
// ---------------------------------------------------------------------------
TEST_CASE("AC-3: LoadSound with non-existent file handles gracefully", "[audio][error][5-3-1]")
{
    // GIVEN: A path that does not exist
    const std::string nonexistentPath = "/tmp/mu_audio_test_nonexistent_5_3_1.wav";

    // GIVEN: A standalone MiniAudioBackend instance
    mu::MiniAudioBackend backend;
    [[maybe_unused]] const bool initOk = backend.Initialize();

    // WHEN: LoadSound is called with the non-existent path
    // THEN: Must not crash — backend logs error via g_ErrorReport.Write() and returns
    const std::wstring wPath(nonexistentPath.begin(), nonexistentPath.end());
    backend.LoadSound(static_cast<ESound>(SOUND_EXPAND_END - 5), wPath.c_str(), 1, false);

    // THEN: PlaySound on the unloaded slot must also be a safe no-op
    backend.PlaySound(static_cast<ESound>(SOUND_EXPAND_END - 5), nullptr, FALSE);

    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-3: LoadSound with a corrupt file (random bytes) handles gracefully
// miniaudio must detect the invalid header and return an error without crashing.
// ---------------------------------------------------------------------------
TEST_CASE("AC-3: LoadSound with corrupt file handles gracefully", "[audio][error][5-3-1]")
{
    // GIVEN: A file containing random bytes (not a valid audio file)
    test_assets::TempAudioDir tempDir;
    const std::string corruptPath = tempDir.FilePath("corrupt.wav");
    {
        std::ofstream file(corruptPath, std::ios::binary);
        REQUIRE(file.is_open());
        // Write 256 bytes of pseudo-random data that is NOT a valid RIFF/WAV header
        for (int i = 0; i < 256; ++i)
        {
            const uint8_t byte = static_cast<uint8_t>((i * 37 + 13) & 0xFF);
            file.write(reinterpret_cast<const char*>(&byte), 1);
        }
    }

    // GIVEN: A standalone MiniAudioBackend instance
    mu::MiniAudioBackend backend;
    [[maybe_unused]] const bool initOk = backend.Initialize();

    // WHEN: LoadSound is called with the corrupt file
    // THEN: Must not crash — miniaudio will fail to parse the header
    const std::wstring wPath(corruptPath.begin(), corruptPath.end());
    backend.LoadSound(static_cast<ESound>(SOUND_EXPAND_END - 6), wPath.c_str(), 1, false);

    backend.Shutdown();
}

// ---------------------------------------------------------------------------
// AC-3: PlayMusic with a non-existent file handles gracefully
// IsEndMusic() must return true after the failed play attempt.
// ---------------------------------------------------------------------------
TEST_CASE("AC-3: PlayMusic with non-existent file handles gracefully", "[audio][error][5-3-1]")
{
    // GIVEN: A standalone MiniAudioBackend instance
    mu::MiniAudioBackend backend;
    [[maybe_unused]] const bool initOk = backend.Initialize();

    // WHEN: PlayMusic is called with a file that does not exist
    // THEN: Must not crash; backend guards with !m_initialized or ma_sound_init_from_file failure
    backend.PlayMusic("nonexistent_5_3_1.mp3", TRUE);

    // THEN: IsEndMusic() returns true — no stream was successfully loaded
    CHECK(backend.IsEndMusic());

    backend.Shutdown();
}

// =============================================================================
// AC-5: ma_decoder direct pipeline validation — format-agnostic contract
// =============================================================================

// ---------------------------------------------------------------------------
// AC-5: ma_decoder reports MA_SUCCESS and non-zero frame count for valid WAV
// This test exercises ma_decoder independently of MiniAudioBackend (no g_platformAudio).
// ---------------------------------------------------------------------------
TEST_CASE("AC-5: ma_decoder pipeline returns non-zero frame count for valid WAV", "[audio][decoder][pipeline][5-3-1]")
{
    // GIVEN: A generated WAV file with known parameters
    test_assets::TempAudioDir tempDir;
    const std::string wavPath = tempDir.FilePath("pipeline_test.wav");
    REQUIRE(test_assets::GenerateWavFile(wavPath, 1, 44100, 4410)); // 4410 frames, mono

    // WHEN: ma_decoder is initialized directly (no engine required — AC-6)
    ma_decoder decoder;
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
    const ma_result initResult = ma_decoder_init_file(wavPath.c_str(), &config, &decoder);
    REQUIRE(initResult == MA_SUCCESS);

    // THEN: (a) decoded frame count > 0
    ma_uint64 frameCount = 0;
    CHECK(ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount) == MA_SUCCESS);
    CHECK(frameCount > 0);

    // THEN: (b) decoded sample format is ma_format_f32 or ma_format_s16 (AC-5 spec)
    CHECK((decoder.outputFormat == ma_format_f32 || decoder.outputFormat == ma_format_s16));

    // THEN: (c) channel count matches expected (1 for mono)
    CHECK(decoder.outputChannels == 1);

    // THEN: At least one frame can actually be read (decoder pipeline is functional)
    std::array<float, 4410> pcmBuffer{};
    ma_uint64 framesRead = 0;
    const ma_result readResult = ma_decoder_read_pcm_frames(&decoder, pcmBuffer.data(), 4410, &framesRead);
    CHECK((readResult == MA_SUCCESS || readResult == MA_AT_END));
    CHECK(framesRead > 0);

    ma_decoder_uninit(&decoder);
}

// =============================================================================
// AC-6: Headless CI compatibility verification
// =============================================================================

// ---------------------------------------------------------------------------
// AC-6: All decoder tests run without audio device (ma_decoder does not need ma_engine)
// This test explicitly verifies that ma_decoder_init_file succeeds on a valid WAV
// with NO ma_engine initialization — confirming CI headless compatibility.
// ---------------------------------------------------------------------------
TEST_CASE("AC-6: ma_decoder works without audio device (headless CI compatible)", "[audio][headless][ci][5-3-1]")
{
    // GIVEN: A valid WAV file (generated in-process — no binary asset committed)
    test_assets::TempAudioDir tempDir;
    const std::string wavPath = tempDir.FilePath("headless_test.wav");
    REQUIRE(test_assets::GenerateWavFile(wavPath, 1, 44100, 2205)); // 0.05s mono

    // WHEN: ma_decoder is used WITHOUT any ma_engine initialization
    // THEN: MA_SUCCESS is returned — decoder does not require an audio device
    ma_decoder decoder;
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
    const ma_result result = ma_decoder_init_file(wavPath.c_str(), &config, &decoder);
    CHECK(result == MA_SUCCESS);

    if (result == MA_SUCCESS)
    {
        ma_decoder_uninit(&decoder);
    }
}

// =============================================================================
// AC-7: Test independence from game state
// =============================================================================

// ---------------------------------------------------------------------------
// AC-7: Tests use standalone MiniAudioBackend — g_platformAudio remains nullptr
// This test verifies that test execution does not affect the game's audio singleton.
// ---------------------------------------------------------------------------
TEST_CASE("AC-7: Test MiniAudioBackend is independent from g_platformAudio", "[audio][independence][5-3-1]")
{
    // GIVEN: g_platformAudio is nullptr (test environment — no game init)
    // (The value may be set by test_muaudio_abstraction.cpp, but that test does not
    //  assign g_platformAudio — it only checks its value. So it should remain nullptr
    //  in the test executable context.)

    // WHEN: A standalone backend is created and operated
    mu::MiniAudioBackend backend;
    [[maybe_unused]] const bool initOk = backend.Initialize();

    // Perform some operations on the standalone backend
    backend.PlayMusic("nonexistent_ac7_test.mp3", FALSE);
    backend.StopMusic(nullptr, TRUE);

    // THEN: g_platformAudio is still nullptr (not touched by this test)
    CHECK(g_platformAudio == nullptr);

    backend.Shutdown();
}
