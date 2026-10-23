#pragma once

#include <string>

namespace Data::Monsters
{
// Loads the monster list (Data/Monsters/Monsters.json) into the monster table
// (g_MonsterDatabase), and keeps its names in the UI language.
class MonsterDataHandler
{
public:
    // Told when the shown names changed: the number of the monster, or -1 for
    // all. The game sets it to write the names into the living monsters.
    using NamesChangedCallback = void (*)(int number);
    static constexpr int AllNumbers = -1;

    static MonsterDataHandler& GetInstance();

    // Reads the file, logs its problems and the load time, and fills the table
    // with the names in the UI language (I18N::GetCurrentLocale), which it
    // follows from then on. On errors the table stays as it was and
    // `errorMessage` describes them; the game must not start then.
    bool Load(std::string& errorMessage);

    void SetNamesChangedCallback(NamesChangedCallback callback)
    {
        m_namesChanged = callback;
    }

private:
    static void OnLocaleChanged(void* context) noexcept;
    void RegisterLocaleObserver();

    NamesChangedCallback m_namesChanged = nullptr;
    bool m_localeObserverRegistered = false;
};
} // namespace Data::Monsters

#define g_MonsterDataHandler Data::Monsters::MonsterDataHandler::GetInstance()
