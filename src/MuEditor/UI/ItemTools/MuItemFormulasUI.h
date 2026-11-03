#pragma once

#ifdef _EDITOR

#include "ItemFormulaEntry.h"
#include "ItemStatFileChange.h"
#include "ItemUsers.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemStatFile.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// The item formulas tool: the whole item stat file (Data/Items/Stats/
// ItemStats.json), its drop level bonus, default, profiles, level bonus tables
// and profile rules, each with what uses it. The picked entry is changed on a
// copy; Apply checks it and lists the items the change reaches (D26) before it
// applies it as one undo step. New, copied, renamed and removed profiles and
// tables (removed only while nothing uses them). See docs/item-data.md.
class CMuItemFormulasUI
{
public:
    // An entry of the list.
    using Entry = MuEditor::Items::FormulaEntry;

    static CMuItemFormulasUI& GetInstance();

    void Render(bool* open);
    // Picks the entry, as the stat profile list of the item tools does.
    void Show(const Entry& entry);

private:
    CMuItemFormulasUI() = default;

    // What a name dialog adds, copies or renames.
    enum class NamePurpose : uint8_t
    {
        NewProfile,
        CopyProfile,
        RenameProfile,
        NewTable,
        CopyTable,
        RenameTable,
    };

    void RenderNotes();
    void RenderUnused();
    void RenderList();
    void RenderPicked();
    void RenderDropLevelBonus(Data::Items::ItemStatFile& file);
    void RenderDefault(Data::Items::ItemStatFile& file);
    void RenderProfile(Data::Items::ItemStatFile& file);
    void RenderTable(Data::Items::ItemStatFile& file);
    // The profile rules in order, the picked one's conditions below.
    void RenderRules(Data::Items::ItemStatFile& file);
    void RenderRuleConditions(Data::Items::StatProfileRule& rule);
    void RenderApplyButtons();
    // The picked entry has changes that are not applied: a change made from
    // the stored file (New, Copy, Rename, Remove them) would drop them.
    bool HasUnappliedChanges() const;
    void RenderConfirm();
    void RenderNameDialog();
    void OpenNameDialog(NamePurpose purpose, std::string from);
    bool IsFreeName(NamePurpose purpose, const std::string& name) const;
    void ApplyName(NamePurpose purpose, const std::string& name);

    // Checks `file` and applies it as one undo step under `label`, after the
    // list of the items it reaches when there are any. A rename takes the
    // items that name the profile along, or with `renamedTable` the items
    // whose descriptions name the table.
    void Apply(Data::Items::ItemStatFile file, std::string label, std::string renamedFrom = {},
               std::string renamedTo = {}, bool renamedTable = false);
    void Commit();
    // The edit follows the stored file (an undo, a redo) until it is changed.
    void UpdateEdit();

    Entry m_picked;
    // The stored file with the picked entry as edited, and as it was loaded.
    Data::Items::ItemStatFile m_edit;
    Data::Items::ItemStatFile m_loaded;
    Entry m_editOf;
    long long m_editVersion = -1;

    // The change waiting for its confirmation.
    struct PendingChange
    {
        Data::Items::ItemStatFile file;
        std::string label;
        std::string renamedFrom;
        std::string renamedTo;
        bool renamedTable = false;
        std::vector<int> reached;
        // The entry to pick once it is applied (a new or renamed one).
        Entry pickAfter;
    };
    PendingChange m_pending;
    bool m_openConfirm = false;
    // The fatal problems of the last change that was refused.
    std::vector<Data::Items::ItemDataIssue> m_fatalIssues;

    NamePurpose m_namePurpose = NamePurpose::NewProfile;
    std::string m_nameFrom;
    std::string m_name;
    bool m_openName = false;

    MuEditor::Items::ItemStatProfileUseCounts m_counts;
    // The rule picked in the rules pane; -1 for none.
    int m_pickedRule = -1;
};

#define g_MuItemFormulasUI CMuItemFormulasUI::GetInstance()

namespace MuEditor::Items
{
// Opens Item Formulas at the entry.
void OpenItemFormulas(const FormulaEntry& entry);
} // namespace MuEditor::Items

#endif // _EDITOR
