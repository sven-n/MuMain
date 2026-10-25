#pragma once

#include <array>
#include <filesystem>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

class BMD;

// The monster recorder's model part (monsters design, section 10): what
// opening monster and NPC models changes, recorded so that the old code and
// the model data can be compared case by case in one build.
//
// A test binary has no GPU: textures load without an upload and are recorded
// by file name; sounds go to an audio backend that records the loads.
namespace MonsterRecorder
{
// One recorded value: "slot 2134.action 3.speed" = "0.33".
struct RecordedValue
{
    std::string path;
    std::string value;
};
using Record = std::vector<RecordedValue>;

// The sound slots of a model (MAX_MONSTER_SOUND).
constexpr int SoundSlotCount = 10;

// One LoadWaveFile, or a release of a sound, in the order they came.
struct SoundEvent
{
    bool release = false;
    int id = 0;
    std::wstring file;
    int channels = 0;
    bool spatial = false;
};

// What one model slot holds.
struct ModelSlotState
{
    std::wstring file;
    int numMeshs = 0;
    int numBones = 0;
    int numActions = 0;
    struct Action
    {
        float speed = 0.0f;
        bool holdsLastFrame = false;
        int keys = 0;

        bool operator==(const Action&) const = default;
    };
    std::vector<Action> actions;
    int boneHead = 0;
    int streamMesh = 0;
    std::array<int, SoundSlotCount> sounds{};
    struct Texture
    {
        std::string name;
        // The file of the bitmap the mesh draws with, or "hidden" / "unknown".
        std::wstring bitmap;
        bool noneBlend = false;

        bool operator==(const Texture&) const = default;
    };
    std::vector<Texture> textures;

    bool operator==(const ModelSlotState&) const = default;
};

// The bitmap slots that monster loads fill (Tantallos, Crust, map loads).
std::span<const int> WatchedBitmaps();

// What a load needs for the life of the scope, like the game sets it up at
// startup: the client folder as the working folder, model slots that start
// zeroed, uploads skipped, the recording audio backend and the active world.
// Each scope has slots of its own, so two loads compared start the same.
class LoadScope
{
public:
    explicit LoadScope(int world);
    ~LoadScope();

    LoadScope(const LoadScope&) = delete;
    LoadScope& operator=(const LoadScope&) = delete;

    // Runs `load` and records what it changed: every model slot that differs
    // afterwards, the sounds loaded and released in order, and the watched
    // bitmap slots.
    Record RecordLoad(const std::function<void()>& load);

    // What a slot holds now, and the sounds since the last RecordLoad or
    // TakeSoundEvents.
    ModelSlotState ReadSlot(int slot) const;
    std::vector<SoundEvent> TakeSoundEvents();

private:
    struct State;
    std::unique_ptr<State> m_state;
};

// The client folder of the tests (the parent of src/bin/Data).
std::filesystem::path ClientFolder();

// Lines "path: old -> new" for each path whose value differs.
std::vector<std::string> Compare(const Record& old, const Record& now);
} // namespace MonsterRecorder
