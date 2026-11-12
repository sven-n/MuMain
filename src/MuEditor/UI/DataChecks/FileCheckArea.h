#pragma once

#ifdef _EDITOR

#include "DataCheckArea.h"

#include "../MuEditor/UI/Common/MuEditorToolWindows.h"

#include <functional>
#include <memory>

namespace MuEditor::DataChecks
{
// An area whose data is files the game reads at start: the checks read them
// as the game does, and a problem is about a file and a field. A click opens
// the area's tool, if it has one.
class FileCheckArea final : public DataCheckArea
{
public:
    // Reads the area's files as the game does and gives their problems.
    using CheckFiles = std::function<std::vector<Data::Items::ItemDataIssue>()>;

    // `name` is a translated text, read when drawn; `tool` and `toolWindowId`
    // are the window a click opens, nullptr for none.
    FileCheckArea(const char* const* name, CheckFiles check, bool ToolWindows::* tool, const char* toolWindowId);

    const char* GetName() const override;
    void Check(std::vector<Data::Items::ItemDataIssue>& issues) override;
    void RenderSubject(const Data::Items::ItemDataIssue& issue) const override;
    void Open(const Data::Items::ItemDataIssue& issue, bool clicked) override;

private:
    const char* const* m_name;
    CheckFiles m_check;
    bool ToolWindows::* m_tool;
    const char* m_toolWindowId;
};

// Data/Local/ServerList.json; a click opens the Server List.
std::unique_ptr<DataCheckArea> CreateServerListCheckArea();
// The effect catalogue in Data/Effects; a click opens the Effect Browser.
std::unique_ptr<DataCheckArea> CreateEffectCheckArea();
// The glTF model file table (Data/Models/ModelFiles.json, the developer
// overlay MU_DATA_OVERLAY first); it has no tool to open.
std::unique_ptr<DataCheckArea> CreateModelFileCheckArea();
// The buffs (Data/Buffs/Buffs.json) as the game reads them at start, with
// their items checked against the item list as the item tools have it
// (unsaved changes too); no tool edits them yet.
std::unique_ptr<DataCheckArea> CreateBuffCheckArea();
// The sounds of map objects (Data/Maps/ObjectSounds.json) as the Map Editor
// has them; a click opens the Map Editor.
std::unique_ptr<DataCheckArea> CreateMapSoundCheckArea();
// The look profile and the light moods (Data/Render/Look.json; an old
// Data/Maps/LightMood.json beside it is reported); a click opens the Dev Editor.
std::unique_ptr<DataCheckArea> CreateLookCheckArea();
// The item options (Data/Items/Options, the option texts and rules) with the
// links of the items as the item tools have them; no tool edits them yet.
std::unique_ptr<DataCheckArea> CreateItemOptionCheckArea();
} // namespace MuEditor::DataChecks

#endif // _EDITOR
