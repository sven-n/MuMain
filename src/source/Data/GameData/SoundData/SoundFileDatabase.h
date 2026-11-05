#pragma once

#include "Data/GameData/SoundData/SoundFile.h"

#include <span>
#include <unordered_map>
#include <vector>

namespace Data::Sounds
{
// The code's sounds in memory (Data/Audio/SoundFiles.json, Q-SE1), in the
// file's order. Built at start; the Sound Editor replaces it as it edits.
class SoundFileDatabase
{
public:
    static SoundFileDatabase& GetInstance();

    // Replaces all entries.
    void Build(std::span<const SoundFile> sounds);
    const std::vector<SoundFile>& GetAll() const
    {
        return m_sounds;
    }
    // The entry of a sound number, or nullptr.
    const SoundFile* Find(int number) const;
    // Changes with every Build.
    int GetVersion() const
    {
        return m_version;
    }

private:
    SoundFileDatabase() = default;

    std::vector<SoundFile> m_sounds;
    std::unordered_map<int, size_t> m_byNumber;
    int m_version = 0;
};
} // namespace Data::Sounds

#define g_SoundFileDatabase Data::Sounds::SoundFileDatabase::GetInstance()
