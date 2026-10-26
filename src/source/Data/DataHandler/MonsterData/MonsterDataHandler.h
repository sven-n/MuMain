#pragma once

#include <string>

#ifdef _EDITOR
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

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

// A model slot as the monster editor's undo keeps it: its entry, or nullopt
// for no entry.
struct MonsterModelSnapshot
{
    int slot = 0;
    std::optional<MonsterModelDefinition> model;

    bool operator==(const MonsterModelSnapshot&) const = default;
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

    // Reads the model list (Data/Monsters/Models.json) into the model table
    // (g_MonsterModelDatabase), after the effect catalogue, whose effect
    // types it names. Like Load, it logs the problems; on errors the table
    // stays as it was and `errorMessage` describes them.
    bool LoadModels(std::string& errorMessage);

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
    // The checks of LoadModels on the model table as it is: the names it
    // holds (sound ids and slots, bitmap and part slots, part effects) and
    // the model and part files that are not in the client folder, as
    // problems of the model list file. Appends them to `issues`.
    void ValidateModels(std::vector<Items::ItemDataIssue>& issues) const;
    // Writes the table to the monster list file (GetMonstersFile, relative to
    // the working directory as Load reads it), unless the checks find errors;
    // the old file stays then. The problems found are added to `issues`.
    bool Save(std::vector<Items::ItemDataIssue>& issues);
    // Reads the monster list file again, as Load does: the changes since it
    // was saved are gone. On errors the table stays as it is; the problems
    // found are added to `issues`.
    bool Revert(std::vector<Items::ItemDataIssue>& issues);

    // The monster editor's reads and changes of the model table. A change
    // is written into the table in place; the slots it changed that the
    // game has open are opened again by ReopenChangedModels, between frames
    // (D55).

    // The entry of the model slot as it is now; nullopt when it has none.
    std::optional<MonsterModelDefinition> ReadModel(int slot) const;
    // Makes each slot what its snapshot says, exactly: the entry (stored at
    // the snapshot's slot), or no entry for nullopt. Slots outside the
    // monster and NPC ranges are skipped.
    void RestoreModels(std::span<const MonsterModelSnapshot> snapshots);
    // Writes the model table to the model list file (GetMonsterModelsFile)
    // unless the checks of LoadModels find errors; the old file stays then.
    // The problems found are added to `issues`.
    bool SaveModels(std::vector<Items::ItemDataIssue>& issues);
    // Reads the model list file again, as LoadModels does; the models that
    // changed are opened again. On errors the table stays as it is; the
    // problems found are added to `issues`.
    bool RevertModels(std::vector<Items::ItemDataIssue>& issues);

    // Save and SaveModels together: neither file is written when the checks
    // of one find errors.
    bool SaveAll(std::vector<Items::ItemDataIssue>& issues);
    // Revert and RevertModels together: neither table changes when one file
    // has errors.
    bool RevertAll(std::vector<Items::ItemDataIssue>& issues);

    // Told after ReopenChangedModels with the slots it opened again; the
    // game clamps the animations of the characters that use them.
    using ModelsReopenedCallback = void (*)(std::span<const int> slots);
    void SetModelsReopenedCallback(ModelsReopenedCallback callback)
    {
        m_modelsReopened = callback;
    }
    bool HasModelsToReopen() const
    {
        return !m_modelsToReopen.empty() || !m_modelsToOpen.empty();
    }
    // Opens the changed models that the game has open again (ModelLoader::
    // Reopen): the file, its textures from the folders of the loaded map,
    // the parts, the speeds and the sounds. Also opens the models the tools
    // asked for.
    void ReopenChangedModels();
    // Opens the slot's model with ReopenChangedModels when the game does not
    // have it open, for the monster tools, which show the animations of an
    // open model. It stays open as if a monster of it had appeared.
    void OpenModelForTools(int slot);
#endif

private:
    static void OnLocaleChanged(void* context) noexcept;
    void RegisterLocaleObserver();
#ifdef _EDITOR
    void NotifyNamesChanged(int number) const;
    // The tables become what the files read: the monsters with their names
    // in the language shown, the models with the changed slots to reopen.
    void UseMonsters(std::span<const MonsterDefinition> monsters);
    void UseModels(std::span<const MonsterModelDefinition> models);
#endif

    NamesChangedCallback m_namesChanged = nullptr;
    bool m_localeObserverRegistered = false;
#ifdef _EDITOR
    ModelsReopenedCallback m_modelsReopened = nullptr;
    std::vector<int> m_modelsToReopen;
    std::vector<int> m_modelsToOpen;
#endif
};
} // namespace Data::Monsters

#define g_MonsterDataHandler Data::Monsters::MonsterDataHandler::GetInstance()
