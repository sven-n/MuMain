#pragma once

#include "Data/GameData/BuffData/BuffDefinition.h"

#include <array>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Buffs
{
// What the game shows of a buff in the display language: built when the table is built and when the language changes,
// so drawing never converts a text.
struct BuffDisplayTexts
{
    std::wstring name;
    // The description's lines; none for a buff without one.
    std::vector<std::wstring> descriptionLines;
};

// The buffs in memory (Data/Buffs/Buffs.json, buffs design B1), looked up by the number the server sends.
class BuffDatabase
{
public:
    static BuffDatabase& GetInstance();

    // Replaces all buffs and builds their texts in `locale`.
    void Build(std::span<const BuffDefinition> buffs, std::string_view locale);
    // Builds the texts in another language.
    void SetDisplayLocale(std::string_view locale);

    const std::vector<BuffDefinition>& GetAll() const
    {
        return m_buffs;
    }
    // The buff of that number, or nullptr.
    const BuffDefinition* Find(int number) const;
    // Its texts in the display language, or nullptr.
    const BuffDisplayTexts* FindTexts(int number) const;

    // What the buff strip shows of the `active` buff numbers (buffs design B12): shown[i] is whether active[i] is a
    // known buff that is not hidden and is the highest tier of its family among them (the first of equal tiers).
    // `shown` has as many entries as `active`. No allocation and no state between calls: it runs every frame.
    void PickShownOnStrip(std::span<const int> active, std::span<bool> shown) const;

private:
    static constexpr int NoBuff = -1;

    BuffDatabase();

    std::vector<BuffDefinition> m_buffs;
    std::vector<BuffDisplayTexts> m_texts;
    // The index in m_buffs by number, or NoBuff.
    std::array<int, MaxBuffNumber + 1> m_indices{};
    // Each buff's family as the lowest buff number in it, or NoBuff for none; by index in m_buffs.
    std::vector<int> m_families;
};
} // namespace Data::Buffs

#define g_BuffDatabase Data::Buffs::BuffDatabase::GetInstance()
