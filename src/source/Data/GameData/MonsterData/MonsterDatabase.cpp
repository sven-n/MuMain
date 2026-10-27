#include "stdafx.h"

#include "MonsterDatabase.h"

#include "Core/Text/Utf8.h"
#include "Core/Text/WideString.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/GameData/MonsterData/LegacyMonsterKind.h"

#include <set>

namespace Data::Monsters
{
namespace
{
// getMonsterName's answer for a number the client has no name for.
constexpr const wchar_t* UnknownMonsterName = L"()";

// The text cut to what the game holds, counted in UTF-16 units on every
// platform, into a wide name buffer.
template <std::size_t Size> void CopyName(std::array<wchar_t, Size>& buffer, std::string text)
{
    Core::Text::CutToUtf16Length(text, Size - 1);
    Core::Text::CopyWideString(buffer.data(), buffer.size(), Core::Text::FromUtf8(text).c_str());
}

// Each number the game asks for without an entry, once in the log.
void LogMissingMonster(int number)
{
    static std::set<int> logged;
    if (!logged.insert(number).second)
    {
        return;
    }
    MU_LOG_WARN(mu::log::Get("data"), "The monster list has no monster {}; the game uses its old defaults", number);
}
} // namespace

MonsterDatabase& MonsterDatabase::GetInstance()
{
    static MonsterDatabase instance;
    return instance;
}

void MonsterDatabase::Build(std::span<const MonsterDefinition> monsters, std::string_view locale)
{
    m_exists.reset();
    m_monsters.fill(MonsterDefinition{});
    for (const MonsterDefinition& monster : monsters)
    {
        if (monster.number < 0 || monster.number >= MonsterNumberCount)
        {
            continue;
        }
        const auto number = static_cast<std::size_t>(monster.number);
        m_monsters[number] = monster;
        m_exists.set(number);
    }
    m_displayLocale = std::string(locale);
    for (std::size_t number = 0; number < m_monsters.size(); ++number)
    {
        FillNames(number);
    }
    ++m_version;
}

void MonsterDatabase::SetDisplayLocale(std::string_view locale)
{
    // Not a change of the data: the version stays (GetVersion).
    m_displayLocale = std::string(locale);
    for (std::size_t number = 0; number < m_monsters.size(); ++number)
    {
        FillNames(number);
    }
}

void MonsterDatabase::FillNames(std::size_t number)
{
    if (!m_exists[number])
    {
        m_displayNames[number].fill(L'\0');
        m_englishNames[number].fill(L'\0');
        return;
    }
    const LocalizedString& names = m_monsters[number].names;
    CopyName(m_displayNames[number], names.Get(m_displayLocale));
    CopyName(m_englishNames[number], names.GetNeutral());
}

#ifdef _EDITOR
void MonsterDatabase::Set(const MonsterDefinition& monster)
{
    if (monster.number < 0 || monster.number >= MonsterNumberCount)
    {
        return;
    }
    const auto number = static_cast<std::size_t>(monster.number);
    m_monsters[number] = monster;
    m_exists.set(number);
    FillNames(number);
    ++m_version;
}

void MonsterDatabase::Remove(int number)
{
    if (number < 0 || number >= MonsterNumberCount)
    {
        return;
    }
    const auto slot = static_cast<std::size_t>(number);
    m_monsters[slot] = MonsterDefinition{};
    m_exists.reset(slot);
    FillNames(slot);
    ++m_version;
}
#endif

const wchar_t* MonsterDatabase::FindDisplayName(int number) const
{
    return Find(number) != nullptr ? m_displayNames[static_cast<std::size_t>(number)].data() : nullptr;
}

const wchar_t* MonsterDatabase::FindEnglishName(int number) const
{
    return Find(number) != nullptr ? m_englishNames[static_cast<std::size_t>(number)].data() : nullptr;
}

const wchar_t* GetMonsterName(int number)
{
    const wchar_t* name = MonsterDatabase::GetInstance().FindDisplayName(number);
    if (name == nullptr)
    {
        LogMissingMonster(number);
        return UnknownMonsterName;
    }
    return name;
}

int GetMonsterObjectKind(int number)
{
    const MonsterDefinition* monster = MonsterDatabase::GetInstance().Find(number);
    if (monster == nullptr)
    {
        LogMissingMonster(number);
        return GetLegacyObjectKind(number);
    }
    return ToObjectKind(monster->kind);
}
} // namespace Data::Monsters
