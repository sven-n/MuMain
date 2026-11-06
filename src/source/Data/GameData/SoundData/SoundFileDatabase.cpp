#include "stdafx.h"

#include "SoundFileDatabase.h"

namespace Data::Sounds
{
SoundFileDatabase& SoundFileDatabase::GetInstance()
{
    static SoundFileDatabase instance;
    return instance;
}

void SoundFileDatabase::Build(std::span<const SoundFile> sounds)
{
    m_sounds.assign(sounds.begin(), sounds.end());
    m_byNumber.clear();
    for (size_t index = 0; index < m_sounds.size(); ++index)
    {
        // The first entry of a number wins, as the checks allow only one.
        m_byNumber.emplace(m_sounds[index].number, index);
    }
    ++m_version;
}

const SoundFile* SoundFileDatabase::Find(int number) const
{
    const auto found = m_byNumber.find(number);
    return found != m_byNumber.end() ? &m_sounds[found->second] : nullptr;
}
} // namespace Data::Sounds
