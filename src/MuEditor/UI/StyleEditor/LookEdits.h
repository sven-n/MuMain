#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/UndoTarget.h"
#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookDefinition.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Looks
{
// The render styles the Style Editor changes (design L18, L31): the looks and folders of Data/Effects/RenderStyles.json
// as the editor has them, what the file has, and an undo history of this session. Each step reaches the game at once
// (g_LookStore), so it draws what the editor shows; Save writes the file.
class LookEdits final : public UndoTarget
{
public:
    static constexpr std::size_t MaxUndoSteps = 256;

    // The looks, sorted by name, and their folders.
    struct State
    {
        std::vector<Data::Looks::LookDefinition> looks;
        std::vector<Data::Colors::ColorFolder> folders;

        bool operator==(const State&) const = default;
    };

    static LookEdits& GetInstance();

    // Reads the file (again): nothing to undo, nothing changed, and the game draws the file's looks. A read with a
    // fatal problem leaves the looks the game has and keeps Save off.
    void Load();
    // Whether the file was read since the start.
    [[nodiscard]] bool IsLoaded() const
    {
        return m_loaded;
    }
    // The problems of the last read or save.
    [[nodiscard]] const std::vector<Data::Items::ItemDataIssue>& GetIssues() const
    {
        return m_issues;
    }
    // Whether the looks are all the file has: after a read with a fatal problem they are not, and a save would lose
    // what the read left out.
    [[nodiscard]] bool IsComplete() const
    {
        return m_complete;
    }

    [[nodiscard]] const State& GetState() const
    {
        return m_state;
    }
    [[nodiscard]] const Data::Looks::LookDefinition* Find(std::string_view name) const;
    // Counts the reads and the steps, also undone and redone, for what the windows keep between frames.
    [[nodiscard]] int GetVersion() const
    {
        return m_version;
    }

    // One undo step each, named `step`. SetLook replaces the look `name` (its new name may differ: the folders follow
    // it); AddLook puts the look into the folder at `folder` (the empty path: under the root).
    void SetLook(const char* step, const std::string& name, Data::Looks::LookDefinition look);
    void AddLook(const char* step, Data::Looks::LookDefinition look, const Data::Colors::ColorFolderPath& folder);
    void RemoveLook(const char* step, const std::string& name);
    void SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders);

    // UndoTarget; IsOpen is the Style Editor's.
    bool IsOpen() const override;
    bool CanUndo() const override
    {
        return !m_undo.empty();
    }
    bool CanRedo() const override
    {
        return !m_redo.empty();
    }
    std::string GetUndoLabel() const override;
    std::string GetRedoLabel() const override;
    void Undo() override;
    void Redo() override;

    // Whether the looks differ from the file.
    [[nodiscard]] bool HasUnsavedChanges() const
    {
        return m_state != m_saved;
    }
    // Writes the file (Data Checks counts it, SavedDataFiles); false, with the problems in GetIssues, when the looks
    // can't be written or read back.
    bool Save();

    // A state for tests, as if read from the file.
    void ResetForTest(State state);

private:
    struct Step
    {
        std::string label;
        State state;
    };

    LookEdits() = default;

    void Push(const char* step, State next);
    // The game draws the looks of the state.
    void ToGame() const;

    State m_state;
    State m_saved;
    std::vector<Step> m_undo;
    std::vector<Step> m_redo;
    std::vector<Data::Items::ItemDataIssue> m_issues;
    bool m_loaded = false;
    bool m_complete = true;
    int m_version = 0;
};
} // namespace MuEditor::Looks

#define g_LookEdits MuEditor::Looks::LookEdits::GetInstance()

#endif // _EDITOR
