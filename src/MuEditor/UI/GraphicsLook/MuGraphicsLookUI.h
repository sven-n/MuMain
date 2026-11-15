#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/UndoTarget.h"
#include "Data/GameData/MapData/LookProfileEdits.h"
#include "LookEditHistory.h"

#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <vector>

// The Graphics Look window: the look profile (Data/Render/Look.json) on every
// scope, the default, its environments and its map entries. Each change goes
// through the profile's edit operations and shows in the game from the next
// frame; Save writes the file. The editor bar's Undo and Redo act on its
// changes while it is the tool worked in last.
class CMuGraphicsLookUI final : public MuEditor::UndoTarget
{
public:
    static CMuGraphicsLookUI& GetInstance();

    void Render(bool* open);
    // Opens the window and brings it to the front (Data Checks).
    void Show();

    // MuEditor::UndoTarget
    bool IsOpen() const override;
    bool CanUndo() const override;
    bool CanRedo() const override;
    std::string GetUndoLabel() const override;
    std::string GetRedoLabel() const override;
    void Undo() override;
    void Redo() override;

private:
    using LookScope = Data::Maps::LookScope;

    // Begins the window with the editor's bar; false (and ended) while hidden.
    bool BeginWindow(bool* open);
    // Follows the live profile: other files end the history, and a change made
    // elsewhere (`$look`) becomes one undo step.
    void FollowLive();

    void RenderToolbar();
    void RenderStatus();
    void RenderScopes();
    void RenderScope();
    void RenderEnvironment(std::size_t environment);
    void RenderEntry(int world);
    // The warning before a change many maps share; true when the scope can be edited.
    bool RenderSharedWarning(const std::vector<int>& maps);
    void RenderValues();

    // Makes `edited` live as one undo step, or shows why the edit was refused.
    void Change(const std::string& label, const Data::Maps::LookEditOutcome& outcome,
                const Data::Maps::LookProfileData& edited);
    // The maps a change on the selected scope reaches.
    [[nodiscard]] std::vector<int> ScopeMaps() const;
    // The scope's name in undo labels and the header.
    [[nodiscard]] std::string ScopeName() const;
    // Selects `scope`; a slider edit or a name typed for the scope before goes.
    void SelectScope(const LookScope& scope);
    // The selected scope still exists (an undo can remove an environment).
    void KeepScopeValid();
    void Save();
    void Revert();
    void AfterHistoryStep();
    void HandleUndoKeys();

    MuEditor::GraphicsLook::LookEditHistory m_history;
    // The live profile as the history last saw it.
    Data::Maps::LookProfileData m_seen;
    std::optional<std::uint32_t> m_filesGeneration;
    LookScope m_scope = LookScope::Default();
    // The scopes whose shared change was confirmed this session, by their maps.
    std::set<std::vector<int>> m_confirmed;
    // The value a slider is being dragged to; applied when the drag ends.
    std::optional<std::size_t> m_draggedValue;
    float m_draggedNumber = 0.f;
    std::string m_newEnvironment;
    std::string m_environmentName;
    std::optional<std::size_t> m_nameShownFor;
    std::string m_status;
    bool m_statusIsError = false;
};

#define g_MuGraphicsLookUI CMuGraphicsLookUI::GetInstance()

#endif // _EDITOR
