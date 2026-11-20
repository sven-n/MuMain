#pragma once

#ifdef _EDITOR

#include "ItemOptionFileChange.h"
#include "OptionTextRows.h"

#include "../MuEditor/UI/Common/NamesGrid.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemOptionDatabase.h"
#include "Data/GameData/ItemData/OptionRule.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

// The item options tool (design O18): the option definitions of Data/Items/Options with their options, the items and
// rules that link each, and the highest level each value list gives (Fill levels makes the marked ones end at another
// level, design O36). The option files are changed on a copy; Apply checks it as Save would write it and lists the
// items the change reaches (D26) before it applies it as one undo step. New, copied, renamed and removed definitions
// (removed only while nothing links them). See docs/item-data.md, "Item options".
class CMuItemOptionsUI
{
public:
    static CMuItemOptionsUI& GetInstance();

    void Render(bool* open);
    // Picks the definition, as the option links of Item Basics do.
    void Show(const std::string& definition);

private:
    CMuItemOptionsUI() = default;

    // What the name dialog adds, copies or renames.
    enum class NamePurpose : uint8_t
    {
        New,
        Copy,
        Rename,
        NewText,
    };

    void RenderDefinitionsTab();
    // The option rules (OptionRules.json) in order, the picked one's options and conditions below.
    void RenderRulesTab();
    // The rules' table: the selected item's rule marked with <, a rule with changes not applied with *.
    void RenderRuleList();
    void RenderRuleRow(size_t index, bool selectedItemsRule);
    // New rule, Move up / down and Remove the rule; whether one changed the rules' order or count.
    bool RenderRuleButtons();
    // The picked rule's definitions and conditions.
    void RenderPickedRule(Data::Items::OptionRule& rule);
    // The option texts (OptionTexts.json) in the names grid.
    void RenderTextsTab();
    void RenderList();
    void RenderListRow(const Data::Items::ItemOptionDefinition& definition, bool unapplied);
    void RenderPicked();
    void RenderDefinition(Data::Items::ItemOptionDefinition& definition);
    void RenderDisplayName(Data::Items::ItemOptionDefinition& definition);
    void RenderServer(Data::Items::ItemOptionDefinition& definition);
    void RenderOption(Data::Items::ItemOptionDefinition& definition, size_t index);
    void RenderValue(Data::Items::ItemOptionValue& value, size_t option);
    void RenderTerms(Data::Items::ItemOptionValue& value);
    void RenderTexts(Data::Items::ItemOption& option);
    void RenderUsers(const std::string& name);
    void RenderApplyButtons();
    // The copy has changes that are not applied: a change made from the stored files (New, Copy, Rename, Remove, Fill
    // levels) would drop them.
    bool HasUnappliedChanges() const;
    void RenderFillLevels();
    void RenderValueFill();
    void RenderConfirm();
    void RenderNameDialog();
    void OpenNameDialog(NamePurpose purpose, std::string from);
    bool IsFreeName(const std::string& name) const;
    void ApplyName();

    // Checks `files` as Save would write them and applies them as one undo step under `label`, after the list of the
    // items they reach when there are any. A rename takes the items that link the definition along.
    void Apply(Data::Items::ItemOptionFiles files, std::string label, std::string renamedFrom = {},
               std::string renamedTo = {});
    void Commit();
    // The copy follows the stored files (an undo, a redo) until it is changed; after that, a change of them marks it
    // stale (UpdateEditedCopy). Marks of names no definition has any more are dropped.
    void UpdateEdit();
    // Builds the files Fill levels would apply, and their lines, when its level or the stored files changed.
    void UpdateFillFiles();
    // How many items link each definition, counted again when the item data changes.
    void UpdateUserCounts();
    int CountUsers(const std::string& name) const;
    // The rule each item takes its list from (the first it matches), counted again when the item data or the rules
    // change.
    void UpdateRuleMatches();
    Data::Items::ItemOptionDefinition* FindEdited(std::string_view name);

    std::string m_picked;
    // The stored files with the changes not applied yet, and as they were taken.
    Data::Items::ItemOptionFiles m_edit;
    Data::Items::ItemOptionFiles m_loaded;
    long long m_editVersion = -1;
    // The stored files changed under the copy's changes (an undo, a redo), so Apply waits for a Revert.
    bool m_stale = false;

    // The change waiting for its confirmation.
    struct PendingChange
    {
        Data::Items::ItemOptionFiles files;
        std::string label;
        std::string renamedFrom;
        std::string renamedTo;
        std::vector<int> reached;
        // The definition to pick once it is applied (a new or renamed one).
        std::string pickAfter;
    };
    PendingChange m_pending;
    bool m_openConfirm = false;
    // The fatal problems of the last change that was refused, and the other problems of the last change applied.
    std::vector<Data::Items::ItemDataIssue> m_fatalIssues;
    std::vector<Data::Items::ItemDataIssue> m_warnings;

    NamePurpose m_namePurpose = NamePurpose::New;
    std::string m_nameFrom;
    std::string m_name;
    Data::Items::ItemOptionType m_newType = Data::Items::ItemOptionType::Option;
    bool m_openName = false;

    std::string m_search;
    // The type the list shows; -1 for all.
    int m_typeFilter = -1;
    // The definitions marked for Fill levels.
    std::set<std::string, std::less<>> m_marked;
    int m_fillLevel = 7;
    bool m_openFill = false;
    // The stored files with the marked lists filled to m_fillFilesLevel (-1: to build), at the item data version
    // m_fillFilesVersion, and a line per filled definition.
    Data::Items::ItemOptionFiles m_fillFiles;
    std::vector<std::string> m_fillLines;
    int m_fillFilesLevel = -1;
    long long m_fillFilesVersion = -1;

    // Fill levels of one option's list.
    size_t m_valueFillOption = 0;
    int m_valueFillFrom = 1;
    int m_valueFillStart = 0;
    int m_valueFillStep = 0;
    int m_valueFillTo = 4;
    bool m_openValueFill = false;

    // The rule picked in the Rules tab; -1 for none.
    int m_pickedRule = -1;
    MuEditor::NamesGrid m_textGrid;
    COptionTextRows m_textRows;

    std::map<std::string, int, std::less<>> m_userCounts;
    long long m_userCountsVersion = -1;
    // The picks m_userCounts counts (MergeCopyPicks).
    MuEditor::Items::AncientBonusPicks m_userCountsPicks{};
    // Whether the copy has changes not applied, taken once a frame.
    bool m_unapplied = false;
    // The items that use the picked definition (Used by), and for which definition and data they were found.
    std::vector<int> m_users;
    std::string m_usersOf;
    long long m_usersVersion = -1;
    // How many items take their list from each rule, the rule of each item slot by item type (-1 for none), and the
    // rules and data they were counted for.
    std::vector<int> m_ruleMatches;
    std::vector<int> m_ruleOfSlot;
    std::vector<Data::Items::OptionRule> m_ruleMatchesOf;
    long long m_ruleMatchesVersion = -1;
};

#define g_MuItemOptionsUI CMuItemOptionsUI::GetInstance()

namespace MuEditor::Items
{
// Opens Item Options at the definition.
void OpenItemOptions(const std::string& definition);
} // namespace MuEditor::Items

#endif // _EDITOR
