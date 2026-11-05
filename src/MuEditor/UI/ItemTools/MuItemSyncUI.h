#pragma once

#ifdef _EDITOR

#include "ItemSyncApply.h"
#include "ItemSyncDiff.h"
#include "ItemSyncServer.h"
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
// replaced after a confirmation that says what changes in it. Compare two files lists, read-only,
// what changes from one exchange file to another (e.g. a client export and a server export); its
// rows can be marked as known differences too. Update from server downloads OpenMU's file from its
// admin panel (ItemSyncServer.h) and goes on as Import.
class CMuItemSyncUI
{
public:
    static CMuItemSyncUI& GetInstance();

    void Render(bool* open);

    // Compares the client's data with the file at `path`, as Import does after its dialog.
    void Compare(const std::filesystem::path& path);
    // Compares two exchange files (Compare the files): what changes from `a` to `b`, in every
    // language, read-only.
    void CompareFiles(const std::filesystem::path& a, const std::filesystem::path& b);
    // Update from server: starts the download from the admin panel's address in the settings;
    // its answer is compared in a later frame. False with a message when it cannot start.
    bool UpdateFromServer();
    // Compares the client's data with a file `url` sent, as Update from server does with the
    // downloaded one; Apply compares it again afterwards without downloading it again.
    void CompareServerFile(const std::string& text, const std::string& url);
    // What the last Update from server did not get, or empty.
    const std::string& GetServerMessage() const
    {
        return m_serverMessage;
    }
    bool IsComparingFiles() const
    {
        return m_comparingFiles;
    }

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

    // The button whose file dialog answers.
    enum class FileAction : uint8_t
    {
        Import,
        Export,
        // The two files of Compare two files.
        CompareA,
        CompareB,
    };
    // Takes the path a file dialog chose, on the dialog's thread; the window uses it in its next
    // frame without a confirmation open.
    void ChooseFile(std::string path, FileAction action);
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
    // A list of the diff's items: the ones that differ, and in an import the ones only the client
    // has (in a section of their own).
    enum class ItemList : uint8_t
    {
        None,
        Differ,
        OnlyInClient,
    };

    CMuItemSyncUI() = default;

    void TakeChosenFile();
    void TakeServerDownload();
    void RenderServer();
    void CompareText(const std::optional<std::string>& text, const std::string& name);
    void SetAllChecked(bool checked);
    void OpenFileDialog(FileAction action);
    void RenderButtons();
    void RenderCompareFiles();
    void RenderExportResult();
    void RenderSummary();
    void RenderFieldToggle();
    void RenderItems();
    // The list's header with its Copy as Markdown button (on the header, so a folded list can be
    // copied too); true while the list is open.
    bool RenderItemListHeader(ItemList list, const std::string& header, const std::vector<int>& rows);
    // The table of the diff's items at `rows`, and the fields of the shown one when it is there.
    void RenderItemTable(const char* id, const std::vector<int>& rows);
    void CopyItemList(ItemList list, const std::vector<int>& rows);
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

    // The path field, and the file the diff is of: Apply uses that one. From the server, the
    // address of its file.
    std::string m_path;
    std::string m_comparedPath;
    // The diff is of a file from the server: its text, which Apply compares again.
    bool m_fromServer = false;
    const char* FromTheSource() const;
    void RenderUnsaved();
    std::string m_serverText;
    // The file an import compares with the client, or file B of a comparison of two files.
    Data::Items::ItemExchangeFile m_file;
    // A comparison of two files is shown instead of an import: its path fields, the files it is
    // of, and file A.
    bool m_comparingFiles = false;
    std::string m_pathA;
    std::string m_pathB;
    std::string m_comparedA;
    std::string m_comparedB;
    Data::Items::ItemExchangeFile m_fileA;
    std::optional<MuEditor::Items::ItemSyncDiff> m_diff;
    MuEditor::Items::KnownDifferences m_known;
    bool m_knownLoaded = false;
    // g_ItemDatabase.GetVersion() when the diff was made (a formula change counts there too).
    int m_diffVersion = 0;
    // The row of m_diff->items whose fields are shown, or -1, and what uses the item when the
    // import removes it.
    int m_shownItem = -1;
    MuEditor::Items::ItemUsage m_shownUsage;
    // The list Copy as Markdown copied last for this diff, which its button says, and how many rows.
    ItemList m_copiedList = ItemList::None;
    int m_copiedRows = 0;
    // The fields of the changed items, for "A field in all items" (built with the diff), and the
    // one picked there.
    std::vector<std::string> m_toggleFields;
    std::string m_toggleField;
    std::vector<std::string> m_messages;
    bool m_messagesAreErrors = false;
    std::string m_result;
    // Apply was clicked: the confirmation is open (again, should ImGui close it otherwise) until
    // it is answered.
    bool m_confirmApply = false;

    // The export: the file it writes, what changes in it while the confirmation waits (which is
    // open while it does), and what the last one did.
    std::string m_exportPath;
    std::optional<MuEditor::Items::ItemSyncExportPreview> m_pendingExport;
    std::string m_exportResult;
    bool m_exportFailed = false;

    // Update from server: the address field (MuEditor.ini's [OpenMU] AdminPanelUrl), the running
    // download and its address, and why the last one has no file.
    std::string m_adminPanelUrl;
    bool m_adminPanelUrlLoaded = false;
    // The API key field; Remember keeps it in the system's key store for the address.
    std::string m_apiKey;
    bool m_rememberApiKey = false;
    // Why the key store did not take the key; empty when it did.
    std::string m_apiKeyMessage;
    void LoadServerSettings();
    void LoadApiKey();
    void StoreApiKey();
    void RenderApiKey();
    MuEditor::Items::CItemSyncDownload m_download;
    std::string m_downloadUrl;
    std::string m_serverMessage;

    // The file dialogs answer on their own thread: the path waits here, with the action of the
    // dialog that chose it.
    std::mutex m_chosenMutex;
    std::optional<std::string> m_chosenPath;
    FileAction m_chosenAction = FileAction::Import;
};

#define g_MuItemSyncUI CMuItemSyncUI::GetInstance()

#endif // _EDITOR
