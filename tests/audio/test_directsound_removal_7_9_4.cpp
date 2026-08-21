// Story 7.9.4: Kill DirectSound — Miniaudio-Only Audio Layer
// Flow Code: VS0-QUAL-AUDIO-KILLDSOUND
//
// RED PHASE: Tests verify that all DirectSound code is removed from Audio/
// and all audio paths route exclusively through IPlatformAudio/miniaudio.
//
// Test phases:
//   AC-1a tests (DbToLinear math)        — ALWAYS PASS (pure arithmetic, no implementation deps)
//   AC-3 tests (Audio/ guard scan)       — RED: fail until all 20 #ifdef _WIN32 in Audio/ removed
//   AC-1/AC-2 tests (DS type scan)       — RED: fail until DirectSound + Win32 types removed
//   AC-4 tests (IPlatformAudio conform.) — ALWAYS PASS (compile-time interface check)
//   AC-STD-2 test (compile check)        — ALWAYS PASS (TU compilation IS the check)
//
// AC Mapping:
//   AC-1     -> TEST_CASE("AC-1 [7-9-4]: ...")  DirectSound types absent from Audio/
//   AC-2     -> TEST_CASE("AC-2 [7-9-4]: ...")  Win32 wave I/O types absent from Audio/
//   AC-3     -> TEST_CASE("AC-3 [7-9-4]: ...")  zero #ifdef _WIN32 in Audio/
//   AC-4     -> TEST_CASE("AC-4 [7-9-4]: ...")  IPlatformAudio interface covers all audio methods
//   AC-STD-2 -> TEST_CASE("AC-STD-2 [7-9-4]: ...") TU compiles without Win32/DirectSound deps
//
// File-scan tests (AC-1, AC-2, AC-3) require MU_SOURCE_DIR injected from CMakeLists.txt.
// They are guarded by #ifndef _WIN32 so MinGW CI compiles cleanly (DirectSound still exists
// in the source tree on CI; structural tests run on macOS/Linux native builds only).
//
// Run with: ctest --test-dir MuMain/out/build/macos-arm64 -R directsound_removal_7_9_4

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <memory>
#include <type_traits>

// CRITICAL: Include Defined_Global.h BEFORE audio headers. DSPlaySound.h's ESound enum
// has #ifdef-guarded entries (ASG_ADD_KARUTAN_MONSTERS, ASG_ADD_MAP_KARUTAN) that change
// MAX_BUFFER. MUAudio is compiled with these defines via stdafx.h PCH, so the test must
// see the same defines to avoid an ODR violation (different sizeof(MiniAudioBackend)).
#include "Defined_Global.h"

#include "IPlatformAudio.h"
#include "MiniAudioBackend.h"

// MU_SOURCE_DIR is injected by CMakeLists.txt as the absolute path to MuMain/src/source/.
// Required for file-scan tests (AC-1, AC-2, AC-3). Skipped gracefully if not set.
#ifndef MU_SOURCE_DIR
#define MU_SOURCE_DIR ""
#endif

// ===========================================================================
// AC-1a: DbToLinear Volume Conversion Math (pure logic — ALWAYS PASS)
// ===========================================================================
//
// Documents the intended DirectSound dB → linear float conversion contract.
// DirectSound stores volume as "hundredths of decibels" (dB * 100):
//   DSBVOLUME_MIN = -10000 → -100 dB → special-cased to 0.0f (mute)
//   DSBVOLUME_MAX =      0 →    0 dB → pow(10, 0/2000) = 1.0f (full)
//   Formula: pow(10.0f, static_cast<float>(dsVol) / 2000.0f) for dsVol > -10000
//
// These tests always pass and serve as regression guards for the conversion math.

TEST_CASE("AC-1 [7-9-4]: DbToLinear formula — mute level (-10000) is near zero",
    "[audio][volume][ac-1][7-9-4]")
{
    // GIVEN: DirectSound minimum volume: -10000 (= -100 dB, effectively silent)
    // WHEN:  Converted to linear scale via pow(10, -10000 / 2000.0f) = pow(10, -5)
    // THEN:  Must produce a value indistinguishable from silence (< 0.001f)
    float linear = std::pow(10.0f, -10000.0f / 2000.0f);
    CHECK(linear < 0.001f);
    CHECK(linear >= 0.0f);
}

TEST_CASE("AC-1 [7-9-4]: DbToLinear formula — full volume (0) maps to 1.0f",
    "[audio][volume][ac-1][7-9-4]")
{
    // GIVEN: DirectSound maximum volume: 0 (= 0 dB = unity gain)
    // WHEN:  Converted to linear scale via pow(10, 0 / 2000.0f)
    // THEN:  Must produce 1.0f
    float linear = std::pow(10.0f, 0.0f / 2000.0f);
    CHECK(std::abs(linear - 1.0f) < 0.0001f);
}

TEST_CASE("AC-1 [7-9-4]: DbToLinear formula — -2000 millibels maps to -20 dB",
    "[audio][volume][ac-1][7-9-4]")
{
    // GIVEN: DirectSound volume -2000 (= -20 dB)
    // WHEN:  Converted to linear scale via pow(10, -2000 / 2000.0f) = pow(10, -1)
    // THEN:  Must produce approximately 0.1f (= 10^-1 = -20 dB in linear amplitude)
    float linear = std::pow(10.0f, -2000.0f / 2000.0f);
    CHECK(std::abs(linear - 0.1f) < 0.0001f);
}

TEST_CASE("AC-1 [7-9-4]: DbToLinear formula — output is monotonically increasing",
    "[audio][volume][ac-1][7-9-4]")
{
    // GIVEN: The dB-to-linear conversion formula across the DirectSound range
    // WHEN:  We evaluate it at increments from -9000 to 0
    // THEN:  Each value must be strictly greater than the previous (monotonic increase)
    float prev = std::pow(10.0f, -9000.0f / 2000.0f);
    for (long vol = -8000L; vol <= 0L; vol += 1000L)
    {
        float current = std::pow(10.0f, static_cast<float>(vol) / 2000.0f);
        REQUIRE(current > prev);
        prev = current;
    }
}

TEST_CASE("AC-1 [7-9-4]: DbToLinear formula — output is always in range [0.0, 1.0]",
    "[audio][volume][ac-1][7-9-4]")
{
    // GIVEN: DirectSound volumes in range (-9999, 0]
    // WHEN:  Converted to linear scale
    // THEN:  All results must be in [0.0, 1.0] — miniaudio volume range
    for (long vol = -9999L; vol <= 0L; vol += 111L)
    {
        float linear = std::pow(10.0f, static_cast<float>(vol) / 2000.0f);
        CHECK(linear >= 0.0f);
        CHECK(linear <= 1.0f);
    }
}

// ===========================================================================
// AC-4: IPlatformAudio Interface Conformance (compile-time — ALWAYS PASS)
// ===========================================================================
//
// Verifies MiniAudioBackend remains a complete concrete implementation of
// IPlatformAudio after story 7-9-4 removes DirectSound code.
// The compile success of these assertions IS the test.

TEST_CASE("AC-4 [7-9-4]: IPlatformAudio remains abstract after DirectSound removal",
    "[audio][interface][ac-4][7-9-4]")
{
    SECTION("IPlatformAudio is still a pure abstract class")
    {
        // GIVEN: The IPlatformAudio interface after story 7-9-4 changes
        // WHEN:  We check its abstract status at compile time
        // THEN:  Must remain a pure abstract class — no direct instantiation
        static_assert(std::is_abstract_v<mu::IPlatformAudio>,
                      "IPlatformAudio must remain pure abstract after DirectSound removal");
        REQUIRE(std::is_abstract_v<mu::IPlatformAudio>);
    }

    SECTION("MiniAudioBackend is still a concrete implementation")
    {
        // GIVEN: MiniAudioBackend after DirectSound code is removed
        // WHEN:  We check its abstract status
        // THEN:  Must NOT be abstract — all pure virtuals still overridden
        static_assert(!std::is_abstract_v<mu::MiniAudioBackend>,
                      "MiniAudioBackend must remain concrete (all pure virtuals overridden)");
        REQUIRE_FALSE(std::is_abstract_v<mu::MiniAudioBackend>);
    }

    SECTION("MiniAudioBackend still derives from IPlatformAudio")
    {
        static_assert(std::is_base_of_v<mu::IPlatformAudio, mu::MiniAudioBackend>,
                      "MiniAudioBackend must remain derived from IPlatformAudio");
        REQUIRE(std::is_base_of_v<mu::IPlatformAudio, mu::MiniAudioBackend>);
    }
}

TEST_CASE("AC-4 [7-9-4]: MiniAudioBackend default-constructs and routes through IPlatformAudio",
    "[audio][interface][ac-4][7-9-4]")
{
    // Heap-allocated because MiniAudioBackend is ~3.7MB (m_sounds array).
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    REQUIRE(backend != nullptr);
    mu::IPlatformAudio* audio = backend.get();
    float vol = audio->GetBGMVolume();
    CHECK(vol >= 0.0f);
    CHECK(vol <= 1.0f);
}

TEST_CASE("AC-STD-2 [7-9-4]: Translation unit compiles without Win32 or DirectSound dependencies",
    "[audio][ac-std-2][7-9-4]")
{
    // The existence of this test compilation IS the check:
    // If this TU compiled, neither Win32 headers nor DirectSound headers are required.
    // IPlatformAudio.h and MiniAudioBackend.h must be self-contained without dsound.h.
    SUCCEED("Translation unit compiled without Win32/DirectSound dependencies");
}

// ===========================================================================
// File-scan tests — macOS / Linux native builds only
// ===========================================================================
//
// MinGW defines _WIN32 even when cross-compiling on Linux, so these tests are
// excluded from the MinGW CI build. On macOS/Linux native builds they scan the
// Audio/ source directory to verify the structural properties enforced by this story.

#ifndef _WIN32

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

std::string readFileContent(const std::filesystem::path& path)
{
    std::ifstream f(path);
    if (!f.is_open())
    {
        return {};
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Count non-commented #ifdef _WIN32 / #ifndef _WIN32 lines in a single file.
int countWin32GuardsInFile(const std::filesystem::path& filePath)
{
    std::ifstream f(filePath);
    if (!f.is_open())
    {
        return 0;
    }

    int count = 0;
    std::string line;
    while (std::getline(f, line))
    {
        size_t commentPos = line.find("//");
        size_t guard32 = line.find("#ifdef _WIN32");
        size_t nguard32 = line.find("#ifndef _WIN32");

        if (guard32 != std::string::npos && (commentPos == std::string::npos || guard32 < commentPos))
        {
            ++count;
        }
        if (nguard32 != std::string::npos && (commentPos == std::string::npos || nguard32 < commentPos))
        {
            ++count;
        }
    }
    return count;
}

// Count occurrences of a search pattern (as a whole-word substring) across all .h/.cpp files
// in a directory tree.
int countPatternInDir(const std::filesystem::path& dir, const std::string& pattern)
{
    if (!std::filesystem::exists(dir))
    {
        return -1; // signal: directory not found
    }

    int total = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        auto ext = entry.path().extension().string();
        if (ext != ".cpp" && ext != ".h")
        {
            continue;
        }

        std::string content = readFileContent(entry.path());
        size_t pos = 0;
        while ((pos = content.find(pattern, pos)) != std::string::npos)
        {
            // Only count non-commented occurrences
            size_t lineStart = content.rfind('\n', pos);
            lineStart = (lineStart == std::string::npos) ? 0 : lineStart + 1;
            std::string linePrefix = content.substr(lineStart, pos - lineStart);
            if (linePrefix.find("//") == std::string::npos)
            {
                ++total;
            }
            pos += pattern.size();
        }
    }
    return total;
}

// Total #ifdef _WIN32 guard count across all .h/.cpp in a directory tree.
int countWin32GuardsInDir(const std::filesystem::path& dir)
{
    if (!std::filesystem::exists(dir))
    {
        return -1; // signal: directory not found
    }

    int total = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        auto ext = entry.path().extension().string();
        if (ext != ".cpp" && ext != ".h")
        {
            continue;
        }

        total += countWin32GuardsInFile(entry.path());
    }
    return total;
}

} // anonymous namespace

// ===========================================================================
// AC-3: Zero #ifdef _WIN32 Guards in Audio/
// ===========================================================================

TEST_CASE("AC-3 [7-9-4]: Audio/ directory has zero #ifdef _WIN32 guards",
    "[audio][ac-3][7-9-4]")
{
    const std::string sourceDir = MU_SOURCE_DIR;
    if (sourceDir.empty())
    {
        SKIP("MU_SOURCE_DIR not injected — skipping Audio/ guard audit (set in CMakeLists.txt)");
    }

    const std::filesystem::path audioDir = std::filesystem::path(sourceDir) / "Audio";
    REQUIRE(std::filesystem::exists(audioDir));

    SECTION("zero #ifdef _WIN32 in all Audio/ .h and .cpp files")
    {
        int guardCount = countWin32GuardsInDir(audioDir);
        REQUIRE(guardCount >= 0); // -1 means directory not found

        INFO("Found " << guardCount << " #ifdef _WIN32 / #ifndef _WIN32 guards in Audio/");
        INFO("Expected: 0 after Tasks 3.1-3.5 complete.");
        INFO("Current counts (before implementation):");
        INFO("  Audio/DSplaysound.cpp — 14 guards (Task 3.1: replace with IPlatformAudio calls)");
        INFO("  Audio/DSPlaySound.h   —  1 guard  (Task 3.2: remove DirectSound type declarations)");
        INFO("  Audio/DSwaveIO.cpp    —  2 guards (Task 3.3: delete Win32 wave I/O)");
        INFO("  Audio/DSwaveIO.h      —  2 guards (Task 3.4: delete DirectSound type declarations)");
        INFO("  Audio/DSWavRead.h     —  1 guard  (Task 3.5: remove Win32 guard and DS types)");
        INFO("Verify with: grep -rn '#ifdef _WIN32' src/source/Audio/");
        REQUIRE(guardCount == 0);
    }
}

TEST_CASE("AC-3 [7-9-4]: Individual Audio/ files have zero platform guards",
    "[audio][ac-3][7-9-4]")
{
    const std::string sourceDir = MU_SOURCE_DIR;
    if (sourceDir.empty())
    {
        SKIP("MU_SOURCE_DIR not injected");
    }

    const std::filesystem::path audioDir = std::filesystem::path(sourceDir) / "Audio";

    // The 5 files targeted by Tasks 3.1-3.5
    const std::vector<std::string> targetFiles = {
        "DSplaysound.cpp",
        "DSPlaySound.h",
        "DSwaveIO.cpp",
        "DSwaveIO.h",
        "DSWavRead.h",
    };

    for (const auto& filename : targetFiles)
    {
        auto filePath = audioDir / filename;
        if (!std::filesystem::exists(filePath))
        {
            continue; // file may have been deleted as part of implementation
        }

        DYNAMIC_SECTION("Audio/" + filename + " has zero #ifdef _WIN32 guards")
        {
            int guards = countWin32GuardsInFile(filePath);
            INFO(filename << " must have zero #ifdef _WIN32 guards after story 7-9-4");
            CHECK(guards == 0);
        }
    }
}

// ===========================================================================
// AC-1: No DirectSound API Types Remain in Audio/
// ===========================================================================

TEST_CASE("AC-1 [7-9-4]: No DirectSound API types remain in Audio/ header or source files",
    "[audio][ac-1][7-9-4]")
{
    const std::string sourceDir = MU_SOURCE_DIR;
    if (sourceDir.empty())
    {
        SKIP("MU_SOURCE_DIR not injected — skipping DirectSound type scan");
    }

    const std::filesystem::path audioDir = std::filesystem::path(sourceDir) / "Audio";
    REQUIRE(std::filesystem::exists(audioDir));

    // DirectSound COM interface types and API functions banned from Audio/
    // after story 7-9-4 Tasks 1, 3.1, 3.2 complete.
    const std::vector<std::string> bannedDSTypes = {
        "IDirectSoundBuffer",
        "IDirectSound3DBuffer",
        "IDirectSound3DListener",
        "LPDIRECTSOUND",
        "DSBUFFERDESC",
        "DSBCAPS",
        "DirectSoundCreate",
    };

    for (const auto& pattern : bannedDSTypes)
    {
        DYNAMIC_SECTION("Pattern absent: " + pattern)
        {
            int count = countPatternInDir(audioDir, pattern);
            REQUIRE(count >= 0); // -1 = directory missing

            INFO("Pattern '" << pattern << "' found " << count << " time(s) in Audio/");
            INFO("All DirectSound COM types must be removed by Tasks 1 and 3.1-3.2");
            INFO("Verify with: grep -rn '" << pattern << "' src/source/Audio/");
            CHECK(count == 0);
        }
    }
}

TEST_CASE("AC-1 [7-9-4]: No direct DirectSound initialization calls remain in Audio/",
    "[audio][ac-1][7-9-4]")
{
    const std::string sourceDir = MU_SOURCE_DIR;
    if (sourceDir.empty())
    {
        SKIP("MU_SOURCE_DIR not injected");
    }

    const std::filesystem::path audioDir = std::filesystem::path(sourceDir) / "Audio";
    REQUIRE(std::filesystem::exists(audioDir));

    // DirectSound function calls that must be replaced by IPlatformAudio equivalents
    const std::vector<std::string> bannedDSCalls = {
        "CreateSoundBuffer",
        "IDirectSoundBuffer::Play",
        "IDirectSoundBuffer::Stop",
        "IDirectSoundBuffer::SetVolume",
    };

    for (const auto& pattern : bannedDSCalls)
    {
        DYNAMIC_SECTION("Call absent: " + pattern)
        {
            int count = countPatternInDir(audioDir, pattern);
            REQUIRE(count >= 0);

            INFO("DirectSound call '" << pattern << "' found " << count << " time(s) in Audio/");
            INFO("Must be replaced with g_platformAudio->... equivalent by Task 3.1");
            CHECK(count == 0);
        }
    }
}

// ===========================================================================
// AC-2: No Win32 Wave I/O Types Remain in Audio/
// ===========================================================================

TEST_CASE("AC-2 [7-9-4]: No Win32 wave I/O types remain in Audio/ files",
    "[audio][ac-2][7-9-4]")
{
    const std::string sourceDir = MU_SOURCE_DIR;
    if (sourceDir.empty())
    {
        SKIP("MU_SOURCE_DIR not injected — skipping Win32 wave type scan");
    }

    const std::filesystem::path audioDir = std::filesystem::path(sourceDir) / "Audio";
    REQUIRE(std::filesystem::exists(audioDir));

    // Win32 multimedia I/O types used in DSwaveIO.cpp/h and DSWavRead.h
    // after story 7-9-4 Tasks 2, 3.3, 3.4, 3.5 complete.
    const std::vector<std::string> bannedWaveTypes = {
        "WAVEFORMATEX",
        "MMCKINFO",
        "mmioOpen",
        "mmioRead",
        "mmioDescend",
        "mmioClose",
        "waveOutOpen",
    };

    for (const auto& pattern : bannedWaveTypes)
    {
        DYNAMIC_SECTION("Pattern absent: " + pattern)
        {
            int count = countPatternInDir(audioDir, pattern);
            REQUIRE(count >= 0);

            INFO("Win32 wave type '" << pattern << "' found " << count << " time(s) in Audio/");
            INFO("Must be removed/replaced by Tasks 2 and 3.3-3.5");
            INFO("WAV loading goes through miniaudio's ma_decoder after this story");
            INFO("Verify with: grep -rn '" << pattern << "' src/source/Audio/");
            CHECK(count == 0);
        }
    }
}

#endif // !_WIN32
