#pragma once

#ifdef _EDITOR

#include "SoundEditorRows.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <array>
#include <string>
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
    void RenderRow(const MuEditor::SoundEditor::SoundRow& row);
    void RenderNameCell(const MuEditor::SoundEditor::SoundRow& row);
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
};

extern MuSoundEditorUI g_MuSoundEditorUI;

#endif // _EDITOR
