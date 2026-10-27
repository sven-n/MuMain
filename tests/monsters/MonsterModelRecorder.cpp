#include "stdafx.h"

#include "MonsterModelRecorder.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_TextureIndex.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Core/Text/Utf8.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/EffectData/EffectDataHandler.h"
#include "Data/DataHandler/LoadData.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Sprites/GpuUploadSkip.h"
#include "Render/Textures/TextureFailure.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <cstring>
#include <cwctype>
#include <map>
#include <stdexcept>
#include <string>

static_assert(MonsterRecorder::SoundSlotCount == MAX_MONSTER_SOUND);

namespace MonsterRecorder
{
namespace
{
// Bitmap slots that monster loads fill outside the model slots.
constexpr int WatchedBitmapSlots[] = {
    BITMAP_MONSTER_SKIN,
    BITMAP_MONSTER_SKIN + 1,
    BITMAP_ROBE + 3,
    BITMAP_ROBE + 5,
};

// An audio backend that notes the sound loads and releases, in order.
class RecordingAudio final : public mu::IPlatformAudio
{
public:
    std::vector<SoundEvent> events;
    std::vector<PlayedSound> plays;

    bool Initialize() override
    {
        return true;
    }
    void Shutdown() override {}
    void LoadSound(ESound buffer, const wchar_t* filename, int channels, bool enable3D) override
    {
        events.push_back({false, static_cast<int>(buffer), filename != nullptr ? filename : L"", channels, enable3D});
    }
    bool PlaySound(ESound buffer, const void* object, bool looped) override
    {
        plays.push_back({static_cast<int>(buffer), object, looped});
        return true;
    }
    void StopSound(ESound, bool) override {}
    void AllStopSound() override {}
    void ReleaseSound(ESound buffer) override
    {
        events.push_back({true, static_cast<int>(buffer), L"", 0, false});
    }
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
        return 0.f;
    }
    float GetSFXVolume() const override
    {
        return 0.f;
    }
};

// The shortest text that reads back as the same float (std::to_chars of a
// float is missing before macOS 13.3).
std::string FloatText(float value)
{
    std::array<char, 32> buffer{};
    for (int digits = 1; digits <= std::numeric_limits<float>::max_digits10; ++digits)
    {
        std::snprintf(buffer.data(), buffer.size(), "%.*g", digits, static_cast<double>(value));
        if (std::strtof(buffer.data(), nullptr) == value)
        {
            break;
        }
    }
    return buffer.data();
}

// Paths in the record are compared without case: the game finds files the
// same way on Windows, and the model data names them in their spelling on
// disk ("NPC" for the code's "Npc").
std::string PathText(const std::wstring& path)
{
    std::wstring lower = path;
    for (wchar_t& c : lower)
    {
        c = static_cast<wchar_t>(std::towlower(c));
        if (c == L'/')
            c = L'\\';
    }
    return Core::Text::ToUtf8(lower.c_str());
}

std::wstring BitmapOf(GLuint index)
{
    if (index == BITMAP_HIDE)
        return L"hidden";
    const BITMAP_t* bitmap = Bitmaps.FindTexture(index);
    return bitmap != nullptr ? std::wstring(bitmap->FileName) : std::wstring(L"unknown");
}

bool IsLoaded(const BMD& model)
{
    return model.NumMeshs > 0 || model.NumActions > 0;
}

ModelSlotState ReadSlotState(int slot)
{
    const BMD& model = Models[slot];
    ModelSlotState state;
    state.numMeshs = model.NumMeshs;
    state.numBones = model.NumBones;
    state.numActions = model.NumActions;
    state.boneHead = model.BoneHead;
    state.streamMesh = model.StreamMesh;
    std::copy(std::begin(model.Sounds), std::end(model.Sounds), state.sounds.begin());
    if (!IsLoaded(model))
        return state;

    state.file = gLoadData.GetModelFile(slot);
    for (int i = 0; i < model.NumActions; ++i)
    {
        const Action_t& action = model.Actions[i];
        state.actions.push_back({action.PlaySpeed, action.Loop, action.NumAnimationKeys});
    }
    for (int i = 0; i < model.NumMeshs; ++i)
    {
        ModelSlotState::Texture texture;
        texture.name.assign(model.Textures[i].FileName, strnlen(model.Textures[i].FileName, 32));
        texture.bitmap = BitmapOf(model.IndexTexture[i]);
        texture.noneBlend = model.Meshs[i].NoneBlendMesh;
        state.textures.push_back(texture);
    }
    return state;
}

void AddSlot(Record& record, int slot, const ModelSlotState& state)
{
    const std::string prefix = "slot " + std::to_string(slot) + ".";
    const auto add = [&](const std::string& path, const std::string& value)
    { record.push_back({prefix + path, value}); };
    add("file", PathText(state.file));
    add("meshs", std::to_string(state.numMeshs));
    add("bones", std::to_string(state.numBones));
    add("actions", std::to_string(state.numActions));
    for (size_t i = 0; i < state.actions.size(); ++i)
    {
        const std::string action = "action " + std::to_string(i) + ".";
        add(action + "speed", FloatText(state.actions[i].speed));
        add(action + "hold", state.actions[i].holdsLastFrame ? "1" : "0");
        add(action + "keys", std::to_string(state.actions[i].keys));
    }
    add("boneHead", std::to_string(state.boneHead));
    add("streamMesh", std::to_string(state.streamMesh));
    for (size_t i = 0; i < state.sounds.size(); ++i)
        add("sound " + std::to_string(i), std::to_string(state.sounds[i]));
    for (size_t i = 0; i < state.textures.size(); ++i)
    {
        const std::string texture = "texture " + std::to_string(i) + ".";
        add(texture + "name", state.textures[i].name);
        add(texture + "bitmap", PathText(state.textures[i].bitmap));
        add(texture + "noneBlend", state.textures[i].noneBlend ? "1" : "0");
    }
}

// The file of each watched bitmap slot, and whether it repeats.
std::map<int, std::wstring> ReadWatchedBitmaps()
{
    std::map<int, std::wstring> bitmaps;
    for (const int slot : WatchedBitmapSlots)
    {
        const BITMAP_t* bitmap = Bitmaps.FindTexture(static_cast<GLuint>(slot));
        bitmaps[slot] =
            bitmap != nullptr ? std::wstring(bitmap->FileName) + (bitmap->wrapRepeat ? L" repeat" : L" clamp") : L"";
    }
    return bitmaps;
}

// The texture errors of the running load (the game shows a dialog for each).
std::vector<std::string>& TextureErrors()
{
    static std::vector<std::string> errors;
    return errors;
}

void CollectTextureError(const std::string& message)
{
    TextureErrors().push_back(message);
}

// The model table as the game loads it at startup: the effect catalogue
// first, whose effect types the model list names. Once per test binary; the
// table stays loaded.
void LoadModelTable()
{
    static bool loaded = false;
    if (loaded)
        return;
    std::string error;
    if (!Data::Effects::LoadEffectTypes(error) || !g_MonsterDataHandler.LoadModels(error))
        throw std::runtime_error("the model table does not load: " + error);
    loaded = true;
}

std::string SoundEventText(const SoundEvent& event)
{
    if (event.release)
        return "release " + std::to_string(event.id);
    return "load " + std::to_string(event.id) + " " + PathText(event.file) + " channels " +
           std::to_string(event.channels) + (event.spatial ? " 3d" : "");
}
} // namespace

std::span<const int> WatchedBitmaps()
{
    return WatchedBitmapSlots;
}

std::filesystem::path ClientFolder()
{
    return std::filesystem::path(MU_TEST_DATA_DIR).parent_path();
}

struct LoadScope::State
{
    RecordingAudio audio;
    std::unique_ptr<BMD[]> slots;
    BMD* previousModels = nullptr;
    mu::IPlatformAudio* previousAudio = nullptr;
    int previousWorld = 0;
    std::filesystem::path previousFolder;
};

std::vector<PlayedSound> LoadScope::TakePlayedSounds()
{
    std::vector<PlayedSound> plays = std::move(m_state->audio.plays);
    m_state->audio.plays.clear();
    return plays;
}

LoadScope::LoadScope(int world) : m_state(std::make_unique<State>())
{
    // The log file is set up before switching folders, so it is not written
    // into the client folder.
    mu::log::Init();
    m_state->previousFolder = std::filesystem::current_path();
    std::filesystem::current_path(ClientFolder());
    LoadModelTable();
    Render::Sprites::SetGpuUploadsSkipped(true);
    Render::Textures::SetTextureErrorCollector(&CollectTextureError);

    m_state->previousAudio = g_platformAudio;
    g_platformAudio = &m_state->audio;
    // The recorder holds no sound yet.
    ForgetLoadedSounds();

    // OpenPlayers zeroes the slots right after making them; the loads rely on
    // that (a model whose code sets no sound slot keeps zeros).
    m_state->slots.reset(new BMD[MAX_MODELS]);
    std::memset(static_cast<void*>(m_state->slots.get()), 0, sizeof(BMD) * MAX_MODELS);
    m_state->previousModels = Models;
    Models = m_state->slots.get();

    m_state->previousWorld = gMapManager.WorldActive;
    gMapManager.WorldActive = world;
    // Without a bitmap of another scope: a model's texture that its folders
    // lack could be taken from one (OpenTexture uses a loaded texture of the
    // same name).
    Bitmaps.UnloadAllImages();
}

LoadScope::~LoadScope()
{
    for (int slot = 0; slot < MAX_MODELS; ++slot)
    {
        if (IsLoaded(Models[slot]))
            Models[slot].Release();
    }
    // Also the bitmaps the models did not take along (the fixed slots).
    Bitmaps.UnloadAllImages();
    gMapManager.WorldActive = m_state->previousWorld;
    Models = m_state->previousModels;
    g_platformAudio = m_state->previousAudio;
    ForgetLoadedSounds();
    Render::Textures::SetTextureErrorCollector(nullptr);
    Render::Sprites::SetGpuUploadsSkipped(false);
    std::filesystem::current_path(m_state->previousFolder);
}

LoadChanges LoadScope::Run(const std::function<void()>& load)
{
    std::vector<ModelSlotState> before;
    before.reserve(MAX_MODELS);
    for (int slot = 0; slot < MAX_MODELS; ++slot)
        before.push_back(ReadSlotState(slot));
    const std::map<int, std::wstring> bitmapsBefore = ReadWatchedBitmaps();
    m_state->audio.events.clear();
    TextureErrors().clear();

    load();

    LoadChanges changes;
    for (int slot = 0; slot < MAX_MODELS; ++slot)
    {
        ModelSlotState after = ReadSlotState(slot);
        if (!(after == before[static_cast<size_t>(slot)]))
            changes.slots.emplace_back(slot, std::move(after));
    }
    for (const auto& [slot, file] : ReadWatchedBitmaps())
    {
        if (file != bitmapsBefore.at(slot))
            changes.bitmaps.emplace_back(slot, file);
    }
    changes.sounds.swap(m_state->audio.events);
    changes.textureErrors.swap(TextureErrors());
    return changes;
}

Record LoadScope::RecordLoad(const std::function<void()>& load)
{
    return ToRecord(Run(load));
}

Record ToRecord(const LoadChanges& changes)
{
    Record record;
    for (const auto& [slot, state] : changes.slots)
        AddSlot(record, slot, state);
    for (const auto& [slot, file] : changes.bitmaps)
        record.push_back({"bitmap " + std::to_string(slot), PathText(file)});
    for (size_t i = 0; i < changes.sounds.size(); ++i)
        record.push_back({"sound event " + std::to_string(i), SoundEventText(changes.sounds[i])});
    for (size_t i = 0; i < changes.textureErrors.size(); ++i)
        record.push_back({"texture error " + std::to_string(i), changes.textureErrors[i]});
    return record;
}

ModelSlotState LoadScope::ReadSlot(int slot) const
{
    return ReadSlotState(slot);
}

std::vector<std::string> Compare(const Record& old, const Record& now)
{
    std::map<std::string, std::pair<std::string, std::string>> values;
    constexpr const char* Unset = "(not changed)";
    for (const RecordedValue& value : old)
        values[value.path] = {value.value, Unset};
    for (const RecordedValue& value : now)
    {
        auto [entry, added] = values.try_emplace(value.path, Unset, value.value);
        if (!added)
            entry->second.second = value.value;
    }
    std::vector<std::string> differences;
    for (const auto& [path, pair] : values)
    {
        if (pair.first != pair.second)
            differences.push_back(path + ": " + pair.first + " -> " + pair.second);
    }
    return differences;
}
} // namespace MonsterRecorder
