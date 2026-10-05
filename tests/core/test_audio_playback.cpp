#include "App/stdafx.h"

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "doctest.h"

#define private public
#include "Core/Platform/Audio/MiniAudioBackend.h"
#undef private

#include "Scenes/SceneCore.h"
#ifdef _EDITOR
#include "Audio/DSPlaySound.h"
#include "Audio/EditorSoundMute.h"
#include "Core/Platform/IPlatformAudio.h"
#endif

TEST_CASE("effect volume changes only the SFX gain [audio][volume]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    mu::IPlatformAudio* previousBackend = g_platformAudio;
    g_platformAudio = backend.get();

    SetEffectVolumeLevel(5);
    const float sfxVolume = backend->GetSFXVolume();
    const float bgmVolume = backend->GetBGMVolume();

    g_platformAudio = previousBackend;

    CHECK(sfxVolume == doctest::Approx(0.5f));
    CHECK(bgmVolume == doctest::Approx(1.0f));
}

TEST_CASE("maximum effect volume reaches full SFX gain [audio][volume]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    mu::IPlatformAudio* previousBackend = g_platformAudio;
    g_platformAudio = backend.get();

    SetEffectVolumeLevel(10);
    const float sfxVolume = backend->GetSFXVolume();

    g_platformAudio = previousBackend;

    CHECK(sfxVolume == doctest::Approx(1.0f));
}

TEST_CASE("an active one-channel sound is not restarted [audio][ambient]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.noDevice = MA_TRUE;
    engineConfig.channels = 1;
    engineConfig.sampleRate = 48000;
    REQUIRE(ma_engine_init(&engineConfig, &backend->m_engine) == MA_SUCCESS);
    backend->m_initialized = true;

    constexpr ma_uint64 sourceFrameCount = 1024;
    std::array<float, sourceFrameCount> sourceFrames{};
    sourceFrames.fill(0.25f);

    ma_audio_buffer_config bufferConfig =
        ma_audio_buffer_config_init(ma_format_f32, 1, sourceFrameCount, sourceFrames.data(), nullptr);
    ma_audio_buffer audioBuffer{};
    REQUIRE(ma_audio_buffer_init(&bufferConfig, &audioBuffer) == MA_SUCCESS);

    constexpr ESound soundId = SOUND_CLICK01;
    const int soundIndex = static_cast<int>(soundId);
    REQUIRE(ma_sound_init_from_data_source(&backend->m_engine, reinterpret_cast<ma_data_source*>(&audioBuffer), 0,
                                           nullptr, &backend->m_sounds[soundIndex][0]) == MA_SUCCESS);

    backend->m_soundLoaded[soundIndex] = true;
    backend->m_loadedChannels[soundIndex] = 1;
    backend->m_activeChannel[soundIndex] = 0;

    REQUIRE(backend->PlaySound(soundId, nullptr, false));

    std::array<float, 128> mixedFrames{};
    ma_uint64 framesRead = 0;
    REQUIRE(ma_engine_read_pcm_frames(&backend->m_engine, mixedFrames.data(), mixedFrames.size(), &framesRead) ==
            MA_SUCCESS);
    REQUIRE(framesRead == mixedFrames.size());

    ma_uint64 cursorBeforeReplay = 0;
    REQUIRE(ma_sound_get_cursor_in_pcm_frames(&backend->m_sounds[soundIndex][0], &cursorBeforeReplay) == MA_SUCCESS);
    REQUIRE(cursorBeforeReplay > 0);

    REQUIRE(backend->PlaySound(soundId, nullptr, false));

    ma_uint64 cursorAfterReplay = 0;
    REQUIRE(ma_sound_get_cursor_in_pcm_frames(&backend->m_sounds[soundIndex][0], &cursorAfterReplay) == MA_SUCCESS);
    CHECK(cursorAfterReplay == cursorBeforeReplay);

    backend->Shutdown();
    ma_audio_buffer_uninit(&audioBuffer);
}

TEST_CASE("sound effects resolve Windows-spelled asset paths [audio][paths]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.noDevice = MA_TRUE;
    engineConfig.channels = 1;
    engineConfig.sampleRate = 48000;
    REQUIRE(ma_engine_init(&engineConfig, &backend->m_engine) == MA_SUCCESS);
    backend->m_initialized = true;

    constexpr std::array<unsigned char, 46> wav = {
        'R',  'I',  'F',  'F',  0x26, 0x00, 0x00, 0x00, 'W',  'A',  'V',  'E',  'f',  'm',  't',  ' ',
        0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x80, 0x3e, 0x00, 0x00,
        0x02, 0x00, 0x10, 0x00, 'd',  'a',  't',  'a',  0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path testDirectory =
        std::filesystem::temp_directory_path() / ("mu_audio_path_" + std::to_string(timestamp));
    const std::filesystem::path assetPath = testDirectory / "Data" / "Sound" / "iButtonClick.wav";
    REQUIRE(std::filesystem::create_directories(assetPath.parent_path()));
    {
        std::ofstream file(assetPath, std::ios::binary);
        REQUIRE(file.write(reinterpret_cast<const char*>(wav.data()), wav.size()).good());
    }

    const std::wstring wideAssetPath = (testDirectory / "data" / "sound" / "ibuttonclick.wav").wstring();
    backend->LoadSound(SOUND_CLICK01, wideAssetPath.c_str(), 1, false);

    CHECK(backend->m_soundLoaded[static_cast<int>(SOUND_CLICK01)]);
    backend->Shutdown();
    std::filesystem::remove_all(testDirectory);
}

namespace
{
// Initializes the backend on a device-less engine so music can be streamed without audio hardware.
void InitNoDeviceEngine(mu::MiniAudioBackend& backend)
{
    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.noDevice = MA_TRUE;
    engineConfig.channels = 1;
    engineConfig.sampleRate = 48000;
    REQUIRE(ma_engine_init(&engineConfig, &backend.m_engine) == MA_SUCCESS);
    backend.m_initialized = true;
}

// Writes Data/Music/track.wav under a fresh temp directory and returns that directory.
std::filesystem::path CreateMusicAsset()
{
    constexpr std::array<unsigned char, 46> wav = {
        'R',  'I',  'F',  'F',  0x26, 0x00, 0x00, 0x00, 'W',  'A',  'V',  'E',  'f',  'm',  't',  ' ',
        0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x80, 0x3e, 0x00, 0x00,
        0x02, 0x00, 0x10, 0x00, 'd',  'a',  't',  'a',  0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path testDirectory =
        std::filesystem::temp_directory_path() / ("mu_audio_music_" + std::to_string(timestamp));
    const std::filesystem::path assetPath = testDirectory / "Data" / "Music" / "track.wav";
    REQUIRE(std::filesystem::create_directories(assetPath.parent_path()));
    std::ofstream file(assetPath, std::ios::binary);
    REQUIRE(file.write(reinterpret_cast<const char*>(wav.data()), wav.size()).good());
    return testDirectory;
}
} // namespace

TEST_CASE("a named music stop matches a Windows-spelled track name [audio][music]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    InitNoDeviceEngine(*backend);
    const std::filesystem::path testDirectory = CreateMusicAsset();

    // Spelled like the MUSIC_* constants: backslashes and a case that differs from the file on disk.
    const std::string track = testDirectory.string() + "\\data\\music\\track.wav";
    backend->PlayMusic(track.c_str(), false);
    REQUIRE_FALSE(backend->IsEndMusic());

    backend->StopMusic(track.c_str(), false);
    CHECK(backend->IsEndMusic());

    backend->Shutdown();
    std::filesystem::remove_all(testDirectory);
}

TEST_CASE("a track stopped by name plays again when replayed [audio][music]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    InitNoDeviceEngine(*backend);
    const std::filesystem::path testDirectory = CreateMusicAsset();

    const std::string track = testDirectory.string() + "\\data\\music\\track.wav";
    backend->PlayMusic(track.c_str(), false);
    backend->StopMusic(track.c_str(), false);
    backend->PlayMusic(track.c_str(), false);
    CHECK_FALSE(backend->IsEndMusic());

    backend->Shutdown();
    std::filesystem::remove_all(testDirectory);
}

#ifdef _EDITOR
namespace
{
// An audio backend that counts the sound effects started and stopped.
class CountingAudio final : public mu::IPlatformAudio
{
public:
    int played = 0;
    int playedLooped = 0;
    int stopped = 0;

    bool Initialize() override
    {
        return true;
    }
    void Shutdown() override {}
    void LoadSound(ESound, const wchar_t*, int, bool) override {}
    bool PlaySound(ESound, const void*, bool looped) override
    {
        ++(looped ? playedLooped : played);
        return true;
    }
    void StopSound(ESound, bool) override
    {
        ++stopped;
    }
    void AllStopSound() override {}
    void ReleaseSound(ESound) override {}
    void Set3DSoundPosition() override {}
    void SetVolume(ESound, long) override {}
    void SetMasterVolume(long) override {}
    void PlayMusic(const char*, bool) override {}
    void StopMusic(const char*, bool) override {}
    bool IsEndMusic() override
    {
        return true;
    }
    int GetMusicPosition() override
    {
        return 0;
    }
    void SetBGMVolume(float) override {}
    void SetSFXVolume(float) override {}
    float GetBGMVolume() const override
    {
        return 1.0f;
    }
    float GetSFXVolume() const override
    {
        return 1.0f;
    }
};
} // namespace

TEST_CASE("the editor's mute starts no sound effect played once, and lets looped ones and stops through "
          "[audio][editor]")
{
    CountingAudio audio;
    mu::IPlatformAudio* const previous = g_platformAudio;
    g_platformAudio = &audio;

    Audio::EditorMute::SetMuted(true);
    PlayBuffer(SOUND_CLICK01, nullptr, FALSE);
    PlayBuffer(SOUND_CLICK01, nullptr, TRUE);
    StopBuffer(SOUND_CLICK01, TRUE);
    CHECK(audio.played == 0);
    CHECK(audio.playedLooped == 1);
    CHECK(audio.stopped == 1);

    Audio::EditorMute::SetMuted(false);
    PlayBuffer(SOUND_CLICK01, nullptr, FALSE);
    CHECK(audio.played == 1);

    g_platformAudio = previous;
}
#endif // _EDITOR
