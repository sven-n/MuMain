#pragma once

#ifdef _EDITOR

#include "Data/GameData/MapData/MapObjectSound.h"

#include <optional>
#include <string>
#include <vector>

// The Map Editor's Sounds tab (maps design, MP1, D-MP5): the object sounds of
// the map the game shows (Data/Maps/ObjectSounds.json). Adds, edits and
// removes entries, plays their sound, takes an object type from the world,
// saves and reverts the file. Edits go into the table at once, so the map
// plays them while you edit.
class CMapSoundsTab
{
public:
    static CMapSoundsTab& GetInstance();

    // Draws the tab for the server map and stage the game shows; `world` is
    // the number of its Data/Object<n> folder, whose models the object types
    // are.
    void Render(int serverMap, int stage, int world);

    // Whether the next click in the world takes an object for the selected
    // entry (the Map Editor then picks as its Objects tab does).
    bool IsPicking() const
    {
        return m_picking;
    }
    // The object clicked in the world: its type joins the selected entry.
    void Picked(int objectType);
    // "Add a sound to an object type": a new entry for the map and stage, no
    // object or file yet, selected.
    void Add(int serverMap, int stage);
    // The table as the file has it now: what Save and Revert compare with.
    void TakeSaved();
    // The tab's Save and Revert: write the file, or read it again over the
    // table. The Sound Editor, which changes the same table, then takes it as
    // saved too.
    void Save();
    void Revert();

    // Whether the table differs from the file as it was last read or saved.
    bool IsModified() const;

private:
    CMapSoundsTab() = default;

    void RenderStatus();
    void RenderEntries(int serverMap, int stage);
    void RenderEntry(size_t index, int world);
    void RenderObjects(Data::Maps::MapObjectSound& sound, int world, bool& changed);
    void RenderFile(Data::Maps::MapObjectSound& sound, size_t index, bool& changed);
    // The table with `sound` at `index`, or without it for nullopt.
    void Store(size_t index, const std::optional<Data::Maps::MapObjectSound>& sound);

    // The table as the file had it when it was last read or saved.
    std::optional<std::vector<Data::Maps::MapObjectSound>> m_saved;
    std::optional<size_t> m_selected;
    bool m_picking = false;
    // The file field being typed in, and the entry it belongs to.
    std::string m_fileEdit;
    std::optional<size_t> m_fileEditOf;
    bool m_fileActive = false;
    // The last Save's or Revert's result.
    std::string m_status;
};

#define g_MapSoundsTab CMapSoundsTab::GetInstance()

#endif // _EDITOR
