#pragma once

#ifdef _EDITOR

#include "ItemSyncApply.h"
#include "ItemSyncDiff.h"
#include "ItemUsage.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemExchangeFile.h"

#include <cstdint>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

// The Item Sync window (phase 7, section 9's Item Sync): Import and Export of the item exchange
// file with OpenMU. Import lists how the file differs from the client's items and item formulas
// (ItemSyncDiff); after a confirmation that says what changes, the checked rows are applied as
// one step of the undo history. The known differences (Data/Items/Sync/KnownDifferences.json)
// stay unchecked; a row can be added to them or taken out. Export writes the client's items and
// formulas into a file without a diff (OpenMU's import page shows one); a file that is there is
// replaced after a confirmation that says what changes in it.
class CMuItemSyncUI
{
public:
    static CMuItemSyncUI& GetInstance();

    void Render(bool* open);

    // Compares the client's data with the file at `path`, as Import does after its dialog.
    void Compare(const std::filesystem::path& path);

    // Why the checked rows cannot be applied now.
    enum class ApplyBlocker
    {
        None,
        // The path field holds another file than the one compared.
        OtherPath,
        // The client's items or formulas changed since the comparison.
        DataChanged,
        NothingChecked,
    };
    ApplyBlocker GetApplyBlocker() const;

    // What the checked rows change, for the confirmation before they are applied.
    struct CheckedChanges
    {
        int added = 0;
        int changed = 0;
        int removed = 0;
        int formulaEntries = 0;
    };
    CheckedChanges CountCheckedChanges() const;

    // Applies the checked rows (the confirmation's Apply); false when nothing changed, also while
    // a blocker holds.
    bool Apply();

    // Export after its dialog: writes the client's items and formulas into `path`. A file that is
    // there is replaced only after ConfirmExport (the popup with what changes in it), and not at
    // all when it already has the client's data.
    void Export(const std::filesystem::path& path);
    bool ConfirmExport();
    void CancelExport();
    // The export waiting for its confirmation.
    const std::optional<MuEditor::Items::ItemSyncExportPreview>& GetPendingExport() const
    {
        return m_pendingExport;
    }
    // What the last export did, shown under the buttons.
    const std::string& GetExportResult() const
    {
        return m_exportResult;
    }

    // Takes the path a file dialog chose, on the dialog's thread; the window uses it in its next
    // frame.
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
    // What the last Apply did, shown above the diff it left; empty after any other comparison.
    const std::string& GetResult() const
    {
        return m_result;
    }

private:
    // The button whose file dialog answers.
    enum class FileAction : uint8_t
    {
        Import,
        Export,
    };

    CMuItemSyncUI() = default;

    void TakeChosenFile();
    void SetAllChecked(bool checked);
    void OpenFileDialog(FileAction action);
    void RenderButtons();
    void RenderExportResult();
    void RenderSummary();
    void RenderFieldToggle();
    void RenderItems();
    void RenderItemFields(MuEditor::Items::SyncItem& row);
    void RenderFormulas();
    void RenderApply();
    void RenderApplyPopup();
    void RenderExportPopup();
    void RenderMessages();
    bool WriteExport();
    void SetKnown(int group, int number, const std::string& field, bool known);
    void SetKnownFormula(const std::string& entry, bool known);
    void SaveKnownDifferences();

    // The path field, and the file the diff is of: Apply uses that one.
    std::string m_path;
    std::string m_comparedPath;
    Data::Items::ItemExchangeFile m_file;
    std::optional<MuEditor::Items::ItemSyncDiff> m_diff;
    MuEditor::Items::KnownDifferences m_known;
    bool m_knownLoaded = false;
    // g_ItemDatabase.GetVersion() when the diff was made (a formula change counts there too).
    int m_diffVersion = 0;
    // The row of m_diff->items whose fields are shown, or -1, and what uses the item when the
    // import removes it.
    int m_shownItem = -1;
    MuEditor::Items::ItemUsage m_shownUsage;
    std::string m_toggleField;
    std::vector<std::string> m_messages;
    bool m_messagesAreErrors = false;
    std::string m_result;
    bool m_openApplyPopup = false;

    // The export: the file it writes, what changes in it while the confirmation waits, and what
    // the last one did.
    std::string m_exportPath;
    std::optional<MuEditor::Items::ItemSyncExportPreview> m_pendingExport;
    bool m_openExportPopup = false;
    std::string m_exportResult;
    bool m_exportFailed = false;

    // The file dialogs answer on their own thread: the path waits here for the next frame.
    std::mutex m_chosenMutex;
    std::optional<std::string> m_chosenPath;
    FileAction m_chosenAction = FileAction::Import;
};

#define g_MuItemSyncUI CMuItemSyncUI::GetInstance()

#endif // _EDITOR
