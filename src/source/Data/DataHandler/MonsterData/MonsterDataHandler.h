#pragma once

#include <string>

#ifdef _EDITOR
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <optional>
#include <span>
#include <vector>
#endif

namespace Data::Monsters
{
#ifdef _EDITOR
// A monster number as the monster editor's undo keeps it: its entry, or
// nullopt for no entry.
struct MonsterSnapshot
{
    int number = 0;
    std::optional<MonsterDefinition> monster;

    bool operator==(const MonsterSnapshot&) const = default;
};
#endif

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

#ifdef _EDITOR
    // The monster editor's reads and changes. A change is written into the
    // table in place and the names-changed callback is told its number, or
    // AllNumbers for more than one, so living monsters and NPCs of a number
    // show its new name at once (D55).
    // Their kind is not rewritten: a living character keeps the OBJECT::Kind
    // it appeared with until it appears again.

    // The entry of the number as it is now; nullopt when it has none.
    std::optional<MonsterDefinition> ReadMonster(int number) const;
    // Makes each number what its snapshot says, exactly: the entry (stored at
    // the snapshot's number), or no entry for nullopt. Numbers outside 0-1023
    // are skipped.
    void RestoreMonsters(std::span<const MonsterSnapshot> snapshots);

    // The checks of Load and Save on the table as it is: the names (the
    // separator, language codes, an empty English name, the length the game
    // holds) and names in languages the game has no texts for. Appends the
    // problems to `issues`.
    void Validate(std::vector<Items::ItemDataIssue>& issues) const;
    // Writes the table to the monster list file (GetMonstersFile, relative to
    // the working directory as Load reads it), unless the checks find errors;
    // the old file stays then. The problems found are added to `issues`.
    bool Save(std::vector<Items::ItemDataIssue>& issues);
    // Reads the monster list file again, as Load does: the changes since it
    // was saved are gone. On errors the table stays as it is; the problems
    // found are added to `issues`.
    bool Revert(std::vector<Items::ItemDataIssue>& issues);
#endif

private:
    static void OnLocaleChanged(void* context) noexcept;
    void RegisterLocaleObserver();
#ifdef _EDITOR
    void NotifyNamesChanged(int number) const;
#endif

    NamesChangedCallback m_namesChanged = nullptr;
    bool m_localeObserverRegistered = false;
};
} // namespace Data::Monsters

#define g_MonsterDataHandler Data::Monsters::MonsterDataHandler::GetInstance()
