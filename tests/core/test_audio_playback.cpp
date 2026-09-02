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
