#pragma once

#ifdef _EDITOR

#include "SoundEditorRows.h"
#include "SoundFolderTree.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

// The Sound Editor window (Windows menu, sound editor design Q-SE1): every
// sound the game loads in one table; a sound's name, file, channels and 3D
// are edited in its row and reach the game at once.
class MuSoundEditorUI
{
public:
    void Render(bool* open);

private:
    void RenderToolbar();
    void RenderFilterBar(const std::vector<MuEditor::SoundEditor::SoundRow>& rows, size_t shown);
    void RenderTable(const std::vector<const MuEditor::SoundEditor::SoundRow*>& rows);
    void RenderFolderBar();
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
};

extern MuSoundEditorUI g_MuSoundEditorUI;

#endif // _EDITOR
