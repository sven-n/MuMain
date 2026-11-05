#pragma once

#ifdef _EDITOR

#include "SoundEditorRows.h"
#include "SoundFolderTree.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/SoundData/SoundEvents.h"

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

// The Sound Editor window (Windows menu, sound editor design Q-SE1): every
// sound the game loads in one table; a sound's name, file, channels and 3D
// are edited in its row and reach the game at once.
class MuSoundEditorUI
{
public:
    void Render(bool* open);

    // The status line's text: the last Save, Revert or refused step, or why
    // a field's text was not taken.
    const std::string& GetStatus() const
    {
        return m_status;
    }

private:
    void RenderToolbar();
    void RenderEvents();
    void RenderEventRules(const std::vector<Data::Sounds::SoundEvent>& events, size_t eventIndex);
    // A field of numbers ("7, 24-29"); the numbers when an edit ends and they
    // read.
    // A field of numbers from 0 to `highest`, the column `label`: the numbers
    // when its text was changed and reads; the status line says why when it
    // does not.
    std::optional<std::vector<int>> RenderNumbersField(const char* id, const char* label,
                                                       const std::vector<int>& numbers, int highest);
    // A text field of a list ("7, 24-29", "confirm, cancel"): its text when an edit ends.
    std::optional<std::string> RenderListField(const char* id, const std::string& shown);
    void RenderSafeZoneCell(const Data::Sounds::SoundEventRule& rule,
                            const std::function<void(const Data::Sounds::SoundEventRule&)>& change);
    void RenderHoldsCell(const Data::Sounds::SoundEventRule& rule,
                         const std::function<void(const Data::Sounds::SoundEventRule&)>& change);
    void RenderPlaysPicker(const Data::Sounds::SoundEventRule& rule,
                           const std::function<void(const Data::Sounds::SoundEventRule&)>& change);
    void RenderFilterBar(const std::vector<MuEditor::SoundEditor::SoundRow>& rows, size_t shown);
    void RenderTable(const std::vector<const MuEditor::SoundEditor::SoundRow*>& rows);
    void RenderFolderBar(const std::vector<MuEditor::SoundEditor::SoundRow>& rows);
    void RenderFolder(const Data::Sounds::SoundFolder& folder, const MuEditor::SoundEditor::FolderPath& path,
                      const std::unordered_map<std::string, const MuEditor::SoundEditor::SoundRow*>& shown);
    void RenderNotSorted(const std::vector<const MuEditor::SoundEditor::SoundRow*>& rows);
    void DropSoundOn(const MuEditor::SoundEditor::FolderPath& path);
    void RenderRow(const MuEditor::SoundEditor::SoundRow& row);
    void RenderNameCell(const MuEditor::SoundEditor::SoundRow& row);
    void RenderTypeCell(const MuEditor::SoundEditor::SoundRow& row);
    void RenderFileCell(const MuEditor::SoundEditor::SoundRow& row);
    void RenderPicker(const MuEditor::SoundEditor::SoundRow& row);
    void SetStatus(bool done, const char* doneText, const std::vector<Data::Items::ItemDataIssue>& issues);

    std::array<char, 128> m_filterText{};
    bool m_onlyProblems = false;
    // The name field being typed in, and the row it belongs to.
    std::string m_nameEdit;
    std::string m_nameEditKey;
    // The row whose picker is open, and its search text.
    std::string m_pickerKey;
    std::array<char, 128> m_pickerText{};
    // The last Save's or Revert's result.
    std::string m_status;
    // The shared selection's version the table last showed (Q-SE3), and in
    // this frame: whether to scroll to it, the folders that hold it.
    int m_seenSelection = -1;
    bool m_follow = false;
    MuEditor::SoundEditor::FolderPath m_openPath;
    // The folder the toolbar acts on ({} for the top), and the rename's text.
    MuEditor::SoundEditor::FolderPath m_selectedFolder;
    std::string m_folderName;
    // The numbers field being typed in, and which one.
    std::string m_numbersEdit;
    std::string m_numbersEditKey;
    // How many folder entries name sounds the rows lack, and what that was
    // counted for: the tables' and the folders' versions, the rows and the
    // sounds played unloaded (RenderFolderBar counts again when one moves).
    using GoneCountKey = std::tuple<int, int, int, int, std::size_t, std::size_t>;
    std::optional<GoneCountKey> m_goneCountKey;
    std::size_t m_goneCount = 0;
};

extern MuSoundEditorUI g_MuSoundEditorUI;

#endif // _EDITOR
