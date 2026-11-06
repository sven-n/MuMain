#include "App/stdafx.h"

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "doctest.h"

#define private public
#include "Core/Platform/Audio/MiniAudioBackend.h"
#undef private

#include "Audio/SoundNumbers.h"
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

// A map object's sound is asked for every frame while the object is drawn
// (maps design, D-MP3): as DirectSound did, the channels start in turn, a
// playing one is left alone, and one that has ended starts again.
TEST_CASE("a sound on a free channel leaves its playing channels alone and starts an ended one [audio][ambient]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.noDevice = MA_TRUE;
    engineConfig.channels = 1;
    engineConfig.sampleRate = 48000;
    REQUIRE(ma_engine_init(&engineConfig, &backend->m_engine) == MA_SUCCESS);
    backend->m_initialized = true;

    constexpr ma_uint64 sourceFrameCount = 1024;
    constexpr int Channels = 3;
    std::array<float, sourceFrameCount> sourceFrames{};
    sourceFrames.fill(0.25f);
    std::array<ma_audio_buffer, Channels> audioBuffers{};
    constexpr ESound soundId = SOUND_CLICK01;
    const int soundIndex = static_cast<int>(soundId);
    for (int channel = 0; channel < Channels; ++channel)
    {
        ma_audio_buffer_config bufferConfig =
            ma_audio_buffer_config_init(ma_format_f32, 1, sourceFrameCount, sourceFrames.data(), nullptr);
        REQUIRE(ma_audio_buffer_init(&bufferConfig, &audioBuffers[channel]) == MA_SUCCESS);
        REQUIRE(ma_sound_init_from_data_source(&backend->m_engine,
                                               reinterpret_cast<ma_data_source*>(&audioBuffers[channel]), 0, nullptr,
                                               &backend->m_sounds[soundIndex][channel]) == MA_SUCCESS);
    }
    backend->m_soundLoaded[soundIndex] = true;
    backend->m_loadedChannels[soundIndex] = Channels;
    backend->m_activeChannel[soundIndex] = 0;
    const auto cursor = [&](int channel)
    {
        ma_uint64 frames = 0;
        REQUIRE(ma_sound_get_cursor_in_pcm_frames(&backend->m_sounds[soundIndex][channel], &frames) == MA_SUCCESS);
        return frames;
    };
    const auto mix = [&](ma_uint64 frames)
    {
        std::array<float, 256> mixed{};
        for (ma_uint64 done = 0; done < frames; done += mixed.size())
        {
            ma_uint64 read = 0;
            REQUIRE(ma_engine_read_pcm_frames(&backend->m_engine, mixed.data(), mixed.size(), &read) == MA_SUCCESS);
        }
    };

    // Three frames start the three channels in turn.
    for (int frame = 0; frame < Channels; ++frame)
    {
        REQUIRE(backend->PlaySoundOnFreeChannel(soundId, 1.0f));
    }
    for (int channel = 0; channel < Channels; ++channel)
    {
        CHECK(ma_sound_is_playing(&backend->m_sounds[soundIndex][channel]));
    }
    mix(256);
    const ma_uint64 before = cursor(0);
    REQUIRE(before > 0);

    // The next frames find every channel playing: nothing starts over.
    for (int frame = 0; frame < Channels; ++frame)
    {
        REQUIRE(backend->PlaySoundOnFreeChannel(soundId, 1.0f));
    }
    CHECK(cursor(0) == before);

    // Once the copies have ended, the next ask starts a channel again.
    mix(sourceFrameCount + 256);
    REQUIRE_FALSE(ma_sound_is_playing(&backend->m_sounds[soundIndex][0]));
    REQUIRE(backend->PlaySoundOnFreeChannel(soundId, 0.5f));
    CHECK(ma_sound_is_playing(&backend->m_sounds[soundIndex][0]));
    CHECK(cursor(0) == 0);
    // At its level of the effect volume (a map object's distance, D-MP7).
    CHECK(ma_sound_get_volume(&backend->m_sounds[soundIndex][0]) == doctest::Approx(0.5f));
    // It keeps the level when the effect volume changes while it plays: the
    // far copy goes down with the near ones, not to the full volume.
    backend->SetSFXVolume(0.8f);
    CHECK(ma_sound_get_volume(&backend->m_sounds[soundIndex][0]) == doctest::Approx(0.4f));
    CHECK(ma_sound_get_volume(&backend->m_sounds[soundIndex][1]) == doctest::Approx(0.8f));

    backend->Shutdown();
    for (ma_audio_buffer& audioBuffer : audioBuffers)
    {
        ma_audio_buffer_uninit(&audioBuffer);
    }
}

// The editor's sound previews have their own level: they are heard while the
// game's effect volume is 0.
TEST_CASE("a sound played at a level ignores the effect volume [audio][volume]")
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
    ma_sound& sound = backend->m_sounds[soundIndex][0];
    REQUIRE(ma_sound_init_from_data_source(&backend->m_engine, reinterpret_cast<ma_data_source*>(&audioBuffer), 0,
                                           nullptr, &sound) == MA_SUCCESS);
    backend->m_soundLoaded[soundIndex] = true;
    backend->m_loadedChannels[soundIndex] = 1;
    backend->m_activeChannel[soundIndex] = 0;
    backend->SetSFXVolume(0.0f);

    REQUIRE(backend->PlaySoundAtLevel(soundId, 0.6f));
    CHECK(ma_sound_get_volume(&sound) == doctest::Approx(0.6f));
    // Also when the effect volume changes while it plays (not 0.5 of it).
    backend->SetSFXVolume(0.5f);
    CHECK(ma_sound_get_volume(&sound) == doctest::Approx(0.6f));
    backend->SetSFXVolume(0.0f);

    // The game's sounds keep the effect volume.
    ma_sound_stop(&sound);
    REQUIRE(backend->PlaySound(soundId, nullptr, false));
    CHECK(ma_sound_get_volume(&sound) == doctest::Approx(0.0f));

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

// Writes Data/Music/corrupt.wav, which no decoder accepts, next to CreateMusicAsset()'s track.
std::filesystem::path CreateCorruptMusicFile(const std::filesystem::path& directory)
{
    const std::filesystem::path corrupt = directory / "Data" / "Music" / "corrupt.wav";
    std::ofstream file(corrupt, std::ios::binary);
    REQUIRE((file << "invalid audio").good());
    return corrupt;
}
} // namespace

TEST_CASE("sound effects requested as 3D load without spatialization [audio][3d]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    InitNoDeviceEngine(*backend);

    constexpr std::array<unsigned char, 46> wav = {
        'R',  'I',  'F',  'F',  0x26, 0x00, 0x00, 0x00, 'W',  'A',  'V',  'E',  'f',  'm',  't',  ' ',
        0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x80, 0x3e, 0x00, 0x00,
        0x02, 0x00, 0x10, 0x00, 'd',  'a',  't',  'a',  0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path testDirectory =
        std::filesystem::temp_directory_path() / ("mu_audio_3d_" + std::to_string(timestamp));
    const std::filesystem::path assetPath = testDirectory / "Data" / "Sound" / "iButtonClick.wav";
    REQUIRE(std::filesystem::create_directories(assetPath.parent_path()));
    {
        std::ofstream file(assetPath, std::ios::binary);
        REQUIRE(file.write(reinterpret_cast<const char*>(wav.data()), wav.size()).good());
    }

    // Monster models request 3D; a spatialized sound at a world position would be inaudible.
    const int soundIndex = static_cast<int>(SOUND_CLICK01);
    backend->LoadSound(SOUND_CLICK01, assetPath.wstring().c_str(), 2, true);

    REQUIRE(backend->m_soundLoaded[soundIndex]);
    for (int ch = 0; ch < backend->m_loadedChannels[soundIndex]; ++ch)
    {
        CHECK_FALSE(ma_sound_is_spatialization_enabled(&backend->m_sounds[soundIndex][ch]));
    }
    backend->Shutdown();
    std::filesystem::remove_all(testDirectory);
}

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

TEST_CASE("the music start count rises once for each track started, for the Sound Mixer's log [audio][music]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    InitNoDeviceEngine(*backend);
    const std::filesystem::path testDirectory = CreateMusicAsset();

    const std::string track = testDirectory.string() + "\\data\\music\\track.wav";
    CHECK(backend->GetMusicStarts() == 0);
    backend->PlayMusic(track.c_str(), false);
    CHECK(backend->GetMusicStarts() == 1);
    // The track that plays starts nothing.
    backend->PlayMusic(track.c_str(), false);
    CHECK(backend->GetMusicStarts() == 1);
    // A missing track neither.
    const std::string missing = testDirectory.string() + "\\data\\music\\missing.wav";
    backend->PlayMusic(missing.c_str(), false);
    CHECK(backend->GetMusicStarts() == 1);
    backend->PlayMusic(track.c_str(), false);
    CHECK(backend->GetMusicStarts() == 2);

    backend->Shutdown();
    std::filesystem::remove_all(testDirectory);
}

TEST_CASE("missing tracks stay suppressed across frame requests and stops [audio][music]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    InitNoDeviceEngine(*backend);
    const auto directory = CreateMusicAsset();
    const auto source = directory / "Data" / "Music" / "track.wav";
    const auto first = directory / "Data" / "Music" / "missing-one.wav";
    const auto second = directory / "Data" / "Music" / "missing-two.wav";
    const std::string firstName = first.generic_string();
    const std::string secondName = second.generic_string();
    const std::string firstWindowsName = directory.string() + "\\Data\\Music\\missing-one.wav";
    backend->PlayMusic(firstWindowsName.c_str(), false);
    backend->PlayMusic(secondName.c_str(), false);
    REQUIRE(backend->m_failedMusicNames.size() == 2);
    CHECK(backend->IsEndMusic());
    CHECK(backend->GetMusicPosition() == 0);

    // Make both files playable: any accidental retry now starts music instead of failing again.
    REQUIRE(std::filesystem::copy_file(source, first));
    REQUIRE(std::filesystem::copy_file(source, second));
    constexpr int frameRequests = 50000;
    for (int frame = 0; frame < frameRequests; ++frame)
    {
        const char* name = frame % 2 == 0 ? firstName.c_str() : secondName.c_str();
        backend->StopMusic(name, false);
        backend->StopMusic(nullptr, true);
        backend->PlayMusic(name, false);
    }
    CHECK(backend->IsEndMusic());
    CHECK(backend->m_currentMusicName.empty());
    CHECK(backend->m_failedMusicNames.size() == 2);

    backend->SetBGMVolume(0.25f);
    backend->PlayMusic(firstWindowsName.c_str(), true);
    CHECK_FALSE(backend->IsEndMusic());
    CHECK(backend->m_failedMusicNames.size() == 1);
    CHECK(ma_sound_get_volume(&backend->m_musicSound) == doctest::Approx(0.25f));
    backend->StopMusic(firstName.c_str(), false);
    backend->PlayMusic(firstName.c_str(), false);
    CHECK_FALSE(backend->IsEndMusic());
    backend->PlayMusic(secondName.c_str(), false);
    CHECK(backend->IsEndMusic());
    CHECK(backend->m_failedMusicNames.size() == 1);

    backend->Shutdown();
    std::filesystem::remove_all(directory);
}

TEST_CASE("decoder failures are suppressed until explicitly retried [audio][music]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    InitNoDeviceEngine(*backend);
    const auto directory = CreateMusicAsset();
    const auto source = directory / "Data" / "Music" / "track.wav";
    const auto corrupt = CreateCorruptMusicFile(directory);
    const std::string name = corrupt.generic_string();
    backend->PlayMusic(name.c_str(), false);
    CHECK(backend->IsEndMusic());
    REQUIRE(backend->m_failedMusicNames.size() == 1);
    backend->PlayMusic(name.c_str(), true);
    CHECK(backend->m_failedMusicNames.size() == 1);

    REQUIRE(std::filesystem::copy_file(source, corrupt, std::filesystem::copy_options::overwrite_existing));
    backend->PlayMusic(name.c_str(), false);
    CHECK(backend->IsEndMusic());
    backend->PlayMusic(name.c_str(), true);
    CHECK_FALSE(backend->IsEndMusic());
    CHECK(backend->m_failedMusicNames.empty());

    backend->Shutdown();
    std::filesystem::remove_all(directory);
}

// Unpatched miniaudio wrote to a failed stream after waking the caller, which had already freed it.
// The next stream often reused that memory, and its load then waited forever. The write only lands
// late when the job thread is preempted, so this is a smoke test: a regression shows up as a ctest
// timeout some of the time. MiniAudioImpl.cpp's build check is what guarantees the patch.
TEST_CASE("a failed stream load never stalls the next one [audio][music]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    InitNoDeviceEngine(*backend);
    const auto directory = CreateMusicAsset();
    const std::string trackName = (directory / "Data" / "Music" / "track.wav").generic_string();
    const std::string corruptName = CreateCorruptMusicFile(directory).generic_string();

    constexpr int loadPairs = 200;
    int startedTracks = 0;
    for (int pair = 0; pair < loadPairs; ++pair)
    {
        backend->PlayMusic(corruptName.c_str(), true);
        backend->PlayMusic(trackName.c_str(), true);
        startedTracks += backend->IsEndMusic() ? 0 : 1;
    }
    CHECK(startedTracks == loadPairs);
    CHECK(backend->m_failedMusicNames.size() == 1);

    backend->Shutdown();
    std::filesystem::remove_all(directory);
}

TEST_CASE("audio shutdown clears failed tracks even without a loaded stream [audio][music]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    InitNoDeviceEngine(*backend);
    const auto directory = CreateMusicAsset();
    const auto source = directory / "Data" / "Music" / "track.wav";
    const auto missing = directory / "Data" / "Music" / "missing.wav";
    const std::string name = missing.generic_string();
    backend->PlayMusic(name.c_str(), false);
    REQUIRE(backend->m_failedMusicNames.size() == 1);
    backend->Shutdown();
    CHECK(backend->m_failedMusicNames.empty());

    REQUIRE(std::filesystem::copy_file(source, missing));
    InitNoDeviceEngine(*backend);
    backend->PlayMusic(name.c_str(), false);
    CHECK_FALSE(backend->IsEndMusic());
    backend->Shutdown();
    std::filesystem::remove_all(directory);
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

    // A preview at its own level is muted too.
    PlayBufferAtLevel(SOUND_CLICK01, 1.0f);
    CHECK(audio.played == 0);

    Audio::EditorMute::SetMuted(false);
    PlayBuffer(SOUND_CLICK01, nullptr, FALSE);
    CHECK(audio.played == 1);
    PlayBufferAtLevel(SOUND_CLICK01, 0.5f);
    CHECK(audio.played == 2);

    g_platformAudio = previous;
}
#endif // _EDITOR

namespace
{
// Writes Data/Sound/<name>.wav under a fresh temp directory and returns the file.
std::filesystem::path CreateSoundAsset(const std::string& name)
{
    constexpr std::array<unsigned char, 46> wav = {
        'R',  'I',  'F',  'F',  0x26, 0x00, 0x00, 0x00, 'W',  'A',  'V',  'E',  'f',  'm',  't',  ' ',
        0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x80, 0x3e, 0x00, 0x00,
        0x02, 0x00, 0x10, 0x00, 'd',  'a',  't',  'a',  0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path assetPath = std::filesystem::temp_directory_path() /
                                            ("mu_audio_sound_" + std::to_string(timestamp)) / "Data" / "Sound" /
                                            (name + ".wav");
    REQUIRE(std::filesystem::create_directories(assetPath.parent_path()));
    std::ofstream file(assetPath, std::ios::binary);
    REQUIRE(file.write(reinterpret_cast<const char*>(wav.data()), wav.size()).good());
    return assetPath;
}
} // namespace

TEST_CASE("a sound with a number after the SOUND_* list loads, plays and is released [audio]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    InitNoDeviceEngine(*backend);
    const std::filesystem::path asset = CreateSoundAsset("numbered");
    const int number = static_cast<int>(MAX_BUFFER) + 3;

    backend->LoadSound(static_cast<ESound>(number), asset.wstring().c_str(), 2, true);

    CHECK(backend->m_soundLoaded.size() == static_cast<size_t>(number) + 1);
    CHECK(backend->m_soundLoaded[static_cast<size_t>(number)]);
    CHECK(backend->m_loadedChannels[static_cast<size_t>(number)] == 2);
    CHECK(backend->PlaySound(static_cast<ESound>(number), nullptr, false));
    backend->ReleaseSound(static_cast<ESound>(number));
    CHECK_FALSE(backend->m_soundLoaded[static_cast<size_t>(number)]);
    // The SOUND_* ids before it keep their slots.
    CHECK_FALSE(backend->m_soundLoaded[static_cast<size_t>(SOUND_CLICK01)]);

    // No number beyond the sounds that can be given numbers: what ForFile
    // gives when all are taken.
    backend->LoadSound(Audio::SoundNumbers::NoNumber, asset.wstring().c_str(), 1, false);
    CHECK(backend->m_soundLoaded.size() == static_cast<size_t>(number) + 1);

    backend->Shutdown();
    std::filesystem::remove_all(asset.parent_path().parent_path().parent_path());
}

TEST_CASE("sound files get one number each after the SOUND_* list [audio]")
{
    const ESound first = Audio::SoundNumbers::ForFile(L"Data/Sound/TestNumbersOne.wav");
    const ESound second = Audio::SoundNumbers::ForFile(L"Data/Sound/TestNumbersTwo.wav");

    CHECK(static_cast<int>(first) >= static_cast<int>(MAX_BUFFER));
    CHECK(second != first);
    // The same file, as the game opens it: without case, \ and / alike.
    CHECK(Audio::SoundNumbers::ForFile(L"data\\sound\\testnumbersone.wav") == first);
}

TEST_CASE("the Sound Mixer's gain changes a sound's playing copies of its group and new copies at once, not a "
          "preview's level nor another group's copy, and the music's too [audio][volume][mixer]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.noDevice = MA_TRUE;
    engineConfig.channels = 1;
    engineConfig.sampleRate = 48000;
    REQUIRE(ma_engine_init(&engineConfig, &backend->m_engine) == MA_SUCCESS);
    backend->m_initialized = true;

    constexpr ma_uint64 sourceFrameCount = 48000;
    std::vector<float> sourceFrames(sourceFrameCount, 0.25f);
    ma_audio_buffer_config bufferConfig =
        ma_audio_buffer_config_init(ma_format_f32, 1, sourceFrameCount, sourceFrames.data(), nullptr);
    std::array<ma_audio_buffer, 3> buffers{};
    for (ma_audio_buffer& buffer : buffers)
    {
        REQUIRE(ma_audio_buffer_init(&bufferConfig, &buffer) == MA_SUCCESS);
    }
    constexpr ESound soundId = SOUND_CLICK01;
    const int soundIndex = static_cast<int>(soundId);
    for (int channel = 0; channel < 2; ++channel)
    {
        REQUIRE(ma_sound_init_from_data_source(&backend->m_engine, reinterpret_cast<ma_data_source*>(&buffers[channel]),
                                               0, nullptr, &backend->m_sounds[soundIndex][channel]) == MA_SUCCESS);
    }
    backend->m_soundLoaded[soundIndex] = true;
    backend->m_loadedChannels[soundIndex] = 2;
    backend->m_activeChannel[soundIndex] = 0;
    ma_sound& first = backend->m_sounds[soundIndex][0];
    ma_sound& second = backend->m_sounds[soundIndex][1];
    backend->SetSFXVolume(0.8f);

    // A playing copy follows the gain at once, and keeps running muted.
    REQUIRE(backend->PlaySound(soundId, nullptr, false));
    CHECK(ma_sound_get_volume(&first) == doctest::Approx(0.8f));
    backend->SetSoundMixGain(soundId, 0, 0.0f);
    CHECK(ma_sound_get_volume(&first) == doctest::Approx(0.0f));
    CHECK(ma_sound_is_playing(&first));
    backend->SetSoundMixGain(soundId, 0, 0.5f);
    CHECK(ma_sound_get_volume(&first) == doctest::Approx(0.4f));
    // A new copy starts at it.
    REQUIRE(backend->PlaySound(soundId, nullptr, false));
    CHECK(ma_sound_get_volume(&second) == doctest::Approx(0.4f));
    // The effect volume keeps it.
    backend->SetSFXVolume(1.0f);
    CHECK(ma_sound_get_volume(&first) == doctest::Approx(0.5f));
    CHECK(ma_sound_get_volume(&second) == doctest::Approx(0.5f));
    // A preview's level is its own, also when the gain changes after.
    REQUIRE(backend->PlaySoundAtLevel(soundId, 0.6f));
    CHECK(ma_sound_get_volume(&first) == doctest::Approx(0.6f));
    backend->SetSoundMixGain(soundId, 0, 0.0f);
    CHECK(ma_sound_get_volume(&first) == doctest::Approx(0.6f));
    CHECK(ma_sound_get_volume(&second) == doctest::Approx(0.0f));
    // A copy on a free channel too.
    ma_sound_stop(&second);
    backend->m_activeChannel[soundIndex] = 1;
    backend->SetSoundMixGain(soundId, 0, 0.5f);
    REQUIRE(backend->PlaySoundOnFreeChannel(soundId, 0.5f));
    CHECK(ma_sound_get_volume(&second) == doctest::Approx(0.25f));

    // A copy another group (type) started keeps its gain: the hero's swing
    // while another player's swing of the same sound starts muted.
    ma_sound_stop(&first);
    ma_sound_stop(&second);
    backend->m_activeChannel[soundIndex] = 0;
    backend->SetSoundMixGain(soundId, 1, 1.0f);
    REQUIRE(backend->PlaySound(soundId, nullptr, false));
    CHECK(ma_sound_get_volume(&first) == doctest::Approx(1.0f));
    backend->SetSoundMixGain(soundId, 2, 0.0f);
    CHECK(ma_sound_get_volume(&first) == doctest::Approx(1.0f));
    REQUIRE(backend->PlaySound(soundId, nullptr, false));
    CHECK(ma_sound_get_volume(&second) == doctest::Approx(0.0f));
    // Each group's change reaches its own copy.
    backend->SetSoundMixGain(soundId, 1, 0.5f);
    CHECK(ma_sound_get_volume(&first) == doctest::Approx(0.5f));
    CHECK(ma_sound_get_volume(&second) == doctest::Approx(0.0f));
    backend->SetSoundMixGain(soundId, 2, 0.25f);
    CHECK(ma_sound_get_volume(&first) == doctest::Approx(0.5f));
    CHECK(ma_sound_get_volume(&second) == doctest::Approx(0.25f));

    // The music: its volume times its gain.
    REQUIRE(ma_sound_init_from_data_source(&backend->m_engine, reinterpret_cast<ma_data_source*>(&buffers[2]), 0,
                                           nullptr, &backend->m_musicSound) == MA_SUCCESS);
    backend->m_musicLoaded = true;
    backend->SetBGMVolume(0.8f);
    CHECK(ma_sound_get_volume(&backend->m_musicSound) == doctest::Approx(0.8f));
    backend->SetMusicMixGain(0.5f);
    CHECK(ma_sound_get_volume(&backend->m_musicSound) == doctest::Approx(0.4f));
    backend->SetBGMVolume(0.6f);
    CHECK(ma_sound_get_volume(&backend->m_musicSound) == doctest::Approx(0.3f));

    backend->Shutdown();
    for (ma_audio_buffer& buffer : buffers)
    {
        ma_audio_buffer_uninit(&buffer);
    }
}

TEST_CASE("the backend counts the starts the Sound Mixer lists, not a call while it plays nor a preview, and it says "
          "whether they play and how long the sound is [audio][mixer]")
{
    auto backend = std::make_unique<mu::MiniAudioBackend>();
    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.noDevice = MA_TRUE;
    engineConfig.channels = 1;
    engineConfig.sampleRate = 48000;
    REQUIRE(ma_engine_init(&engineConfig, &backend->m_engine) == MA_SUCCESS);
    backend->m_initialized = true;

    // Two channels of a sound of a second and a half.
    constexpr ma_uint64 sourceFrameCount = 72000;
    std::vector<float> sourceFrames(sourceFrameCount, 0.25f);
    ma_audio_buffer_config bufferConfig =
        ma_audio_buffer_config_init(ma_format_f32, 1, sourceFrameCount, sourceFrames.data(), nullptr);
    bufferConfig.sampleRate = 48000;
    std::array<ma_audio_buffer, 2> buffers{};
    for (ma_audio_buffer& buffer : buffers)
    {
        REQUIRE(ma_audio_buffer_init(&bufferConfig, &buffer) == MA_SUCCESS);
    }
    constexpr ESound soundId = SOUND_CLICK01;
    const int soundIndex = static_cast<int>(soundId);
    for (int channel = 0; channel < 2; ++channel)
    {
        REQUIRE(ma_sound_init_from_data_source(&backend->m_engine, reinterpret_cast<ma_data_source*>(&buffers[channel]),
                                               0, nullptr, &backend->m_sounds[soundIndex][channel]) == MA_SUCCESS);
    }
    backend->m_soundLoaded[soundIndex] = true;
    backend->m_loadedChannels[soundIndex] = 2;
    backend->m_activeChannel[soundIndex] = 0;

    CHECK(backend->GetSoundStarts(soundId) == 0);
    CHECK_FALSE(backend->IsSoundPlaying(soundId));
    CHECK(backend->GetSoundLength(soundId) == doctest::Approx(1.5f));
    // A loop starts once; asked for again while it plays, it is no start.
    REQUIRE(backend->PlaySound(soundId, nullptr, true));
    CHECK(backend->GetSoundStarts(soundId) == 1);
    CHECK(backend->IsSoundPlaying(soundId));
    REQUIRE(backend->PlaySound(soundId, nullptr, true));
    CHECK(backend->GetSoundStarts(soundId) == 1);
    backend->StopSound(soundId, true);
    CHECK_FALSE(backend->IsSoundPlaying(soundId));
    // A preview is neither counted nor playing for the mixer.
    REQUIRE(backend->PlaySoundAtLevel(soundId, 0.5f));
    CHECK(backend->GetSoundStarts(soundId) == 1);
    CHECK_FALSE(backend->IsSoundPlaying(soundId));
    backend->StopSound(soundId, true);
    // A copy on a free channel counts.
    REQUIRE(backend->PlaySoundOnFreeChannel(soundId, 1.0f));
    CHECK(backend->GetSoundStarts(soundId) == 2);
    CHECK(backend->IsSoundPlaying(soundId));
    // A sound that is not loaded: none, not playing, no length.
    constexpr ESound otherId = static_cast<ESound>(SOUND_CLICK01 + 1);
    CHECK(backend->GetSoundStarts(otherId) == 0);
    CHECK_FALSE(backend->IsSoundPlaying(otherId));
    CHECK(backend->GetSoundLength(otherId) == 0.0f);

    backend->Shutdown();
    for (ma_audio_buffer& buffer : buffers)
    {
        ma_audio_buffer_uninit(&buffer);
    }
}
