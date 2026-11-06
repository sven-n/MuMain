#pragma once

#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <array>
#include <bitset>
#include <span>
#include <string>
#include <string_view>

namespace Data::Monsters
{
// The monster list in memory: one entry per monster number in a flat table, so
// a lookup costs one array read. Built once at startup; only a language switch
// and the monster editor change it afterwards. The table and its names never
// move, so pointers to them stay valid (the quest window keeps a name).
class MonsterDatabase
{
public:
    static MonsterDatabase& GetInstance();

    // Replaces all monsters, in place. Monsters with a number outside 0-1023 are
    // skipped (the file's reader reports them). The names are shown in `locale`.
    void Build(std::span<const MonsterDefinition> monsters, std::string_view locale);

    // Shows the names in another language: a missing translation shows English.
    void SetDisplayLocale(std::string_view locale);
    const std::string& GetDisplayLocale() const
    {
        return m_displayLocale;
    }

    // The monster with this number; nullptr outside 0-1023 and for a number the
    // list has no entry for. Defined here so it can be inlined.
    const MonsterDefinition* Find(int number) const
    {
        if (number < 0 || number >= MonsterNumberCount || !m_exists[static_cast<std::size_t>(number)])
        {
            return nullptr;
        }
        return &m_monsters[static_cast<std::size_t>(number)];
    }

    // The monster's name in the display language, cut to what the game holds
    // (MAX_MONSTER_NAME); nullptr like Find.
    const wchar_t* FindDisplayName(int number) const;

    // The English name, for logs ("<English name> (<number>)"); nullptr like Find.
    const wchar_t* FindEnglishName(int number) const;

#ifdef _EDITOR
    // The monster editor's changes of one number, in place like Build: its
    // names are filled again and the version counts the change. A number
    // outside 0-1023 changes nothing.
    void Set(const MonsterDefinition& monster);
    void Remove(int number);
#endif

    // Counts the changes of the monsters (Build, and the editor's Set and
    // Remove), so a cache can tell it is out of date. A language switch is not
    // one: the shown names are read where they are (FindDisplayName).
    int GetVersion() const
    {
        return m_version;
    }

private:
    using NameBuffer = std::array<wchar_t, MaxMonsterNameLength + 1>;

    void FillNames(std::size_t number);

    std::array<MonsterDefinition, MonsterNumberCount> m_monsters{};
    std::bitset<MonsterNumberCount> m_exists;
    std::array<NameBuffer, MonsterNumberCount> m_displayNames{};
    std::array<NameBuffer, MonsterNumberCount> m_englishNames{};
    std::string m_displayLocale = std::string(LocalizedString::NeutralLocale);
    int m_version = 0;
};

// What the game uses of the table, with the answers it gave before the table
// for a number it has no entry for (monsters design, D44). Such a number is
// logged once.

// The name the game shows; "()" for a number without an entry.
const wchar_t* GetMonsterName(int number);

// The OBJECT::Kind of a character of this number; the old kind rules for a
// number without an entry.
int GetMonsterObjectKind(int number);
} // namespace Data::Monsters

#define g_MonsterDatabase Data::Monsters::MonsterDatabase::GetInstance()
