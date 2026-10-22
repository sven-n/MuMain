#pragma once

#ifdef _EDITOR

#include "ItemSyncDiff.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemExchangeFile.h"

#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

// The Item Sync window (phase 7, section 9's Item Sync): Export and Import of the item exchange
// file with OpenMU. Each first lists the differences between the client's items and item
// formulas and the file (ItemSyncDiff); only the checked rows are applied, an import as one step
// of the undo history. The known differences (Data/Items/Sync/KnownDifferences.json) stay
// unchecked; a row can be added to them or taken out.
class CMuItemSyncUI
{
public:
    static CMuItemSyncUI& GetInstance();

    void Render(bool* open);

    // Compares the client's data with the file at `path` for `direction`, as the buttons do
    // after their file dialog. An export into a file that is not there yet compares with nothing.
    void Compare(MuEditor::Items::SyncDirection direction, const std::filesystem::path& path);
    // Applies the checked rows (the Apply button); false when nothing changed.
    bool Apply();
    // Takes the path a file dialog chose, on the dialog's thread; the window compares it in
    // its next frame.
    void ChooseFile(std::string path);
    // The diff shown, for its checkboxes.
    std::optional<MuEditor::Items::ItemSyncDiff>& GetDiff()
    {
        return m_diff;
    }
    const std::vector<std::string>& GetMessages() const
    {
        return m_messages;
    }

private:
    CMuItemSyncUI() = default;

    void TakeChosenFile();
    void SetAllChecked(bool checked);
    void OpenFileDialog(MuEditor::Items::SyncDirection direction);
    void RenderButtons();
    void RenderSummary();
    void RenderFieldToggle();
    void RenderItems();
    void RenderItemFields(MuEditor::Items::SyncItem& row);
    void RenderFormulas();
    void RenderMessages();
    void SetKnown(int group, int number, const std::string& field, bool known);
    void SetKnownFormula(const std::string& entry, bool known);
    void SaveKnownDifferences();

    MuEditor::Items::SyncDirection m_direction = MuEditor::Items::SyncDirection::Import;
    std::string m_path;
    Data::Items::ItemExchangeFile m_file;
    std::optional<MuEditor::Items::ItemSyncDiff> m_diff;
    MuEditor::Items::KnownDifferences m_known;
    bool m_knownLoaded = false;
    // ToolWindow::GetDataVersion() when the diff was made.
    long long m_diffVersion = 0;
    // The row of m_diff->items whose fields are shown, or -1.
    int m_shownItem = -1;
    std::string m_toggleField;
    std::vector<std::string> m_messages;
    bool m_messagesAreErrors = false;

    // The file dialogs answer on their own thread: the path waits here for the next frame.
    std::mutex m_chosenMutex;
    std::optional<std::string> m_chosenPath;
    MuEditor::Items::SyncDirection m_chosenDirection = MuEditor::Items::SyncDirection::Import;
};

#define g_MuItemSyncUI CMuItemSyncUI::GetInstance()

#endif // _EDITOR
