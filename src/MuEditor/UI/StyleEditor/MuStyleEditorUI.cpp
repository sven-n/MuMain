#include "stdafx.h"

#ifdef _EDITOR

#include "MuStyleEditorUI.h"

#include "LookEdits.h"
#include "LookFields.h"
#include "LookParts.h"
#include "RenderStyleCheckArea.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorToolWindows.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Common/SourceText.h"
#include "../MuEditor/UI/Common/ToolGroups.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorActions.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorPopups.h"
#include "../MuEditor/UI/ItemEditor/ItemListModel.h"
#include "../MuEditor/UI/ItemTools/ItemEdits.h"
#include "../MuEditor/UI/ItemTools/ItemMeshFocus.h"
#include "../MuEditor/UI/ItemTools/ItemToolWindow.h"
#include "../MuEditor/UI/ItemTools/ItemSelection.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "../MuEditor/UI/ItemTools/ItemUsers.h"
#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/ItemData/ItemTypeUse.h"
#include "Data/GameData/LookData/LookChecks.h"
#include "Data/GameData/LookData/LookEffects.h"
#include "Data/GameData/LookData/LookJson.h"
#include "I18N/All.h"
#include "Render/Items/ItemRenderStyles.h"
#include "Render/Items/LookDrawing.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <cstddef>
#include <functional>
#include <optional>
#include <utility>

using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Looks::EffectKind;
using Data::Looks::Layer;
using Data::Looks::LookDefinition;
using MuEditor::Looks::BasePart;
using MuEditor::Looks::EveryPart;
using MuEditor::Looks::LookPart;

namespace
{
constexpr char FolderPopupId[] = "###LookFolder";
constexpr char NamePopupId[] = "###LookName";
constexpr char ConfirmPopupId[] = "###LookChange";
constexpr char AddEffectPopupId[] = "###AddEffect";
constexpr char DragPayloadType[] = "MU_LOOKS_TREE";
// The roots in the order the tree shows them.
constexpr std::size_t RenderStylesRootIndex = 0;
constexpr std::size_t ItemEffectsRootIndex = 1;
// Pixel widths before the editor UI scale.
constexpr float TreeWidth = 260.0f;
constexpr float NameWidth = 260.0f;
constexpr float UserListLines = 6.0f;
constexpr float LayerListLines = 7.0f;
constexpr float PartListLines = 8.0f;
constexpr float PartWidth = 200.0f;
constexpr float EffectNoteColumn = 170.0f;
constexpr ImVec4 NotFreeColor{1.0f, 0.4f, 0.4f, 1.0f};
constexpr float MaxPreviewSeconds = 60.0f;
// The problems of a look are about "looks[0]", the only one checked.
constexpr std::string_view CheckedLookField = "looks[0].";

MuEditor::NamedTreeTexts TreeTexts()
{
    MuEditor::NamedTreeTexts texts;
    texts.newFolder = I18N::Editor::NewLookFolder;
    texts.renameFolder = I18N::Editor::RenameALookFolder;
    texts.removeFolder = I18N::Editor::RemoveALookFolder;
    texts.moveFolder = I18N::Editor::MoveALookFolder;
    texts.moveEntry = I18N::Editor::MoveALook;
    texts.folderHolds = I18N::Editor::ItHolds0LooksAnd1Folders;
    return texts;
}

// A grey line that wraps at the window's edge.
void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}

ImVec4 SeverityColor(ItemDataIssueSeverity severity)
{
    switch (severity)
    {
    case ItemDataIssueSeverity::Fatal:
        return {1.0f, 0.4f, 0.4f, 1.0f};
    case ItemDataIssueSeverity::Error:
        return {1.0f, 0.65f, 0.3f, 1.0f};
    case ItemDataIssueSeverity::Warning:
    default:
        return {0.95f, 0.85f, 0.4f, 1.0f};
    }
}

// "fatal: layers[1].glowStrength.seconds: must be more than 0", the look's own field without "looks[0].".
void RenderIssue(const ItemDataIssue& issue)
{
    std::string_view field = issue.field;
    if (field.starts_with(CheckedLookField))
    {
        field.remove_prefix(CheckedLookField.size());
    }
    std::string text = Data::Items::GetSeverityName(issue.severity);
    if (Data::Items::IsValidItemId(issue.group, issue.number))
    {
        text += ": " + g_ItemDatabase.GetLogName(Data::Items::MakeItemType(issue.group, issue.number));
    }
    if (!field.empty())
    {
        text += ": " + std::string(field);
    }
    text += ": " + issue.message;
    ImGui::PushStyleColor(ImGuiCol_Text, SeverityColor(issue.severity));
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}

// The layers a row of the parts list shows: the base's, whatever meshes they draw, or the effects on the part.
std::vector<std::size_t> RowLayers(const LookDefinition& look, int part)
{
    std::vector<std::size_t> layers;
    if (part == BasePart)
    {
        for (std::size_t index = 0; index < look.layers.size(); ++index)
        {
            if (Data::Looks::DescribeEffect(look.layers[index]).kind == EffectKind::Base)
            {
                layers.push_back(index);
            }
        }
        return layers;
    }
    for (const std::size_t index : MuEditor::Looks::LayersOn(look, part))
    {
        if (Data::Looks::DescribeEffect(look.layers[index]).kind != EffectKind::Base)
        {
            layers.push_back(index);
        }
    }
    return layers;
}

// The clock of the game for one drawing: the preview's stopped time.
class WorldTimeScope
{
public:
    explicit WorldTimeScope(std::optional<double> milliseconds) : m_previous(WorldTime), m_set(milliseconds.has_value())
    {
        if (m_set)
        {
            WorldTime = *milliseconds;
        }
    }
    ~WorldTimeScope()
    {
        if (m_set)
        {
            WorldTime = m_previous;
        }
    }
    WorldTimeScope(const WorldTimeScope&) = delete;
    WorldTimeScope& operator=(const WorldTimeScope&) = delete;

private:
    double m_previous;
    bool m_set;
};
} // namespace

CMuStyleEditorUI& CMuStyleEditorUI::GetInstance()
{
    static CMuStyleEditorUI instance;
    return instance;
}

CMuStyleEditorUI::CMuStyleEditorUI() : m_tree({&m_renderStyles, &m_itemEffects}, DragPayloadType, FolderPopupId)
{
    m_renderStyles.SetUnappliedCheck([this](const std::string& name) { return HasUnappliedChanges(name); });
}

void CMuStyleEditorUI::Render(bool* open)
{
    m_tree.KeepPickValid();
    // One of the item tools (design L39): their docking, undo history and Save Items.
    if (MuEditor::Items::ToolWindow::Begin(I18N::Editor::StyleEditor, MuEditor::WindowIds::StyleEditor, open))
    {
        // The parts it points at show in the previews, the Item Model window's too.
        g_ItemMeshFocus.BeginFrame(g_ItemSelection.GetItemType(), ImGui::GetFrameCount());
        const MuEditor::NamedTreeTexts texts = TreeTexts();
        RenderFileBar();
        RenderToolbar();
        // In the toolbar's ID scope, where the buttons opened them.
        RenderNameDialog();
        m_tree.RenderFolderDialog(texts, I18N::Editor::NewFolder, I18N::Editor::Rename, I18N::Editor::FolderName,
                                  I18N::Editor::AFolderNameOf1To32CharactersThatNoOtherFolderHereHas, I18N::Editor::OK,
                                  I18N::Editor::Cancel);
        RenderNote(I18N::Editor::StyleEditorTreeNote);
        ImGui::Separator();

        const ImVec2 treeSize(TreeWidth * g_MuEditorCore.GetUIScale(), 0.0f);
        if (ImGui::BeginChild("##Tree", treeSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
        {
            m_tree.Render(texts);
        }
        ImGui::EndChild();
        ImGui::SameLine();
        if (ImGui::BeginChild("##Picked"))
        {
            RenderPicked();
        }
        ImGui::EndChild();

        m_tree.RunAction();
    }
    MuEditor::Items::ToolWindow::End();
}

void CMuStyleEditorUI::ShowRenderStyle(const std::string& name)
{
    g_MuEditorCore.GetToolWindows().styleEditor = true;
    if (m_renderStyles.Has(name))
    {
        m_tree.PickEntry(RenderStylesRootIndex, name);
    }
    else
    {
        m_tree.SetPick(RenderStylesRootIndex, {});
    }
    m_tree.OpenRoot(RenderStylesRootIndex);
    ImGui::SetWindowFocus(MuEditor::WindowIds::StyleEditor);
}

bool CMuStyleEditorUI::IsRenderStylesPicked() const
{
    return m_tree.GetPick().root == RenderStylesRootIndex;
}

const LookDefinition* CMuStyleEditorUI::GetPickedFileLook() const
{
    return IsRenderStylesPicked() && !GetPickedLook().empty() ? g_LookEdits.Find(GetPickedLook()) : nullptr;
}

void CMuStyleEditorUI::RenderFileBar()
{
    // The looks are item data (design L39): Save Items writes their file with the items.
    CItemEditorActions::RenderSaveButton();
    CItemEditorPopups::RenderAll();
    if (g_ItemEdits.GetHistory().IsModifiedSinceSave())
    {
        ImGui::SameLine();
        ImGui::TextUnformatted(I18N::Editor::UnsavedChanges);
    }
    if (!g_LookEdits.IsComplete())
    {
        RenderNote(I18N::Editor::TheLookFileHasAFatalProblem);
    }
}

void CMuStyleEditorUI::RenderToolbar()
{
    const MuEditor::NamedTreeTexts texts = TreeTexts();
    const LookDefinition* look = GetPickedFileLook();
    const bool folderPicked = m_tree.IsFolderPicked();
    const bool unused = look != nullptr && m_renderStyles.CountUsers(look->name) == 0;

    m_tree.RenderNewFolderButton(I18N::Editor::NewFolder);
    ImGui::SameLine();
    ImGui::BeginDisabled(!IsRenderStylesPicked());
    if (ImGui::Button(I18N::Editor::NewLook))
    {
        OpenNameDialog(NameDialog::New, "");
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(look == nullptr);
    if (ImGui::Button(I18N::Editor::Copy))
    {
        OpenNameDialog(NameDialog::Copy, look->name + "Copy");
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    // D26: a look goes or gets another name only while no item names it.
    ImGui::BeginDisabled(!folderPicked && !unused);
    if (ImGui::Button(I18N::Editor::Rename))
    {
        if (folderPicked)
        {
            m_tree.OpenRenameFolder();
        }
        else
        {
            OpenNameDialog(NameDialog::Rename, look->name);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Remove))
    {
        if (folderPicked)
        {
            m_tree.Later([this, texts] { m_tree.RemovePickedFolder(texts); });
        }
        else
        {
            const std::string name = look->name;
            m_tree.Later(
                [this, name]
                {
                    g_LookEdits.RemoveLook(I18N::Editor::RemoveALook, name);
                    m_tree.SetPick(m_tree.GetPick().root, m_tree.GetPick().folder);
                });
        }
    }
    ImGui::EndDisabled();
    if (look != nullptr && !unused && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::OnlyALookNoItemUsesCanBeRenamedOrRemoved);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_tree.CanMovePicked());
    if (ImGui::Button(I18N::Editor::MoveUp))
    {
        m_tree.Later([this, texts] { m_tree.MovePicked(texts, -1); });
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::MoveDown))
    {
        m_tree.Later([this, texts] { m_tree.MovePicked(texts, 1); });
    }
    ImGui::EndDisabled();
}

void CMuStyleEditorUI::OpenNameDialog(NameDialog dialog, std::string name)
{
    m_nameDialog = dialog;
    m_nameFrom = GetPickedLook();
    m_name = std::move(name);
    ImGui::OpenPopup(NamePopupId);
}

void CMuStyleEditorUI::RenderNameDialog()
{
    const char* title = m_nameDialog == NameDialog::Rename ? I18N::Editor::RenameALook
                        : m_nameDialog == NameDialog::Copy ? I18N::Editor::CopyALook
                                                           : I18N::Editor::NewLook;
    const std::string popup = std::string(title) + NamePopupId;
    if (!ImGui::BeginPopupModal(popup.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ImGui::SetNextItemWidth(NameWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::IsWindowAppearing())
    {
        ImGui::SetKeyboardFocusHere();
    }
    MuEditor::Items::InputString(I18N::Editor::LookName, m_name);
    const std::string name = MuEditor::SourceText::Trimmed(m_name);
    // The render styles' names: a look of the file or one in code may not have it.
    const bool usable = Data::Items::Json::IsName(name) && !m_renderStyles.Has(name);
    RenderNote(I18N::Editor::ALookNameOfLettersAndDigitsThatNoOtherRenderStyleHas);
    ImGui::BeginDisabled(!usable);
    if (ImGui::Button(I18N::Editor::OK))
    {
        const NameDialog dialog = m_nameDialog;
        const std::string from = m_nameFrom;
        const Data::Colors::ColorFolderPath folder = m_tree.GetPick().folder;
        m_tree.Later(
            [this, dialog, from, name, folder]
            {
                const LookDefinition* source = g_LookEdits.Find(from);
                if (dialog == NameDialog::New)
                {
                    // A new look draws the item plainly, as one texture layer.
                    g_LookEdits.AddLook(I18N::Editor::NewLook, {name, {Layer{}}, {}}, folder);
                }
                else if (source != nullptr && dialog == NameDialog::Copy)
                {
                    LookDefinition copy = *source;
                    copy.name = name;
                    g_LookEdits.AddLook(I18N::Editor::CopyALook, std::move(copy), folder);
                }
                else if (source != nullptr && dialog == NameDialog::Rename)
                {
                    LookDefinition renamed = *source;
                    renamed.name = name;
                    g_LookEdits.SetLook(I18N::Editor::RenameALook, from, std::move(renamed));
                    RenameDraft(from, name);
                }
                if (g_LookEdits.Find(name) != nullptr)
                {
                    m_tree.PickEntry(RenderStylesRootIndex, name);
                    m_tree.OpenRoot(RenderStylesRootIndex);
                }
            });
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void CMuStyleEditorUI::RenderPicked()
{
    const MuEditor::NamedTree::Pick& pick = m_tree.GetPick();
    if (!pick.entry.empty())
    {
        if (const LookDefinition* look = GetPickedFileLook())
        {
            RenderLook(*look);
            return;
        }
        RenderCodeLook(pick.entry, pick.root == ItemEffectsRootIndex);
        return;
    }
    if (!pick.folder.empty())
    {
        m_tree.RenderPickedFolder(TreeTexts(), I18N::Editor::FolderNote);
        return;
    }
    RenderRoot();
}

void CMuStyleEditorUI::RenderRoot()
{
    if (!IsRenderStylesPicked())
    {
        ImGui::TextUnformatted(I18N::Editor::ItemEffects);
        RenderNote(I18N::Editor::ItemEffectsRootNote);
        return;
    }
    const std::string file =
        (Data::Effects::GetEffectDataDirectory() / Data::Looks::GetRenderStylesFileName()).generic_string();
    ImGui::TextUnformatted(I18N::Editor::RenderStyles);
    RenderNote(I18N::Format(I18N::Editor::RenderStylesRootNote, {file}).c_str());
    const std::vector<ItemDataIssue>& issues = g_LookEdits.GetIssues();
    if (!issues.empty())
    {
        ImGui::Separator();
        for (const ItemDataIssue& issue : issues)
        {
            RenderIssue(issue);
        }
    }
}

void CMuStyleEditorUI::RenderCodeLook(const std::string& name, bool itemEffect)
{
    ImGui::TextUnformatted((name + " " + I18N::Editor::CodeMark).c_str());
    if (!itemEffect)
    {
        RenderPreview(nullptr);
    }
    // Below the preview, which stays where it is.
    if (ImGui::BeginChild("##Look"))
    {
        RenderNote(itemEffect ? I18N::Editor::ItemEffectsRootNote : I18N::Editor::CodeLookNote);
        RenderUsers(UsersOf(name, itemEffect));
    }
    ImGui::EndChild();
}

void CMuStyleEditorUI::RenderLook(const LookDefinition& look)
{
    UpdateDraft(look);
    CheckDraft();
    const std::string name = look.name;
    ImGui::TextUnformatted(name.c_str());
    RenderApply(look);
    RenderPreview(&m_draft);
    // The curves of the fields mark the preview's time (design L43).
    MuEditor::Looks::Fields::SetCurveTime(m_stopTime ? static_cast<double>(m_seconds) : WorldTime / 1000.0);
    // The look's problems, parts or layers and fields scroll below the preview, which stays where it is.
    if (ImGui::BeginChild("##Look"))
    {
        RenderChecks();
        ImGui::Checkbox(I18N::Editor::ShowTheLayers, &m_showLayers);
        if (m_showLayers)
        {
            RenderLayers();
        }
        else
        {
            RenderParts();
        }
        ImGui::Separator();
        RenderUsers(UsersOf(name, false));
    }
    ImGui::EndChild();
    RenderConfirm();
}

void CMuStyleEditorUI::RenderApply(const LookDefinition& look)
{
    const bool edited = m_draft != look;
    const bool fatal = Data::Items::HasFatal(m_draftIssues);
    ImGui::BeginDisabled(!edited || fatal);
    if (ImGui::Button(I18N::Editor::Apply))
    {
        OpenApply(look);
    }
    ImGui::SameLine();
    ImGui::EndDisabled();
    ImGui::BeginDisabled(!edited);
    if (ImGui::Button(I18N::Editor::Revert))
    {
        LoadDraft(look);
    }
    ImGui::EndDisabled();
    if (edited && fatal)
    {
        ImGui::SameLine();
        ImGui::TextColored(SeverityColor(ItemDataIssueSeverity::Fatal), "%s",
                           I18N::Editor::ApplyIsOffWhileAFatalProblemIsLeft);
    }
}

void CMuStyleEditorUI::OpenApply(const LookDefinition& look)
{
    // D26: a change to a look reaches all its items; they are listed before it is applied.
    m_confirmUsers = UsersOf(look.name, false);
    const int itemType = g_ItemSelection.GetItemType();
    m_applyItem =
        g_ItemDatabase.Find(itemType) != nullptr && g_ItemModelDatabase.Find(itemType) != nullptr ? itemType : -1;
    if (m_confirmUsers.empty() && m_applyItem < 0)
    {
        // After the window is drawn: the look's place changes with it.
        m_tree.Later([this] { Apply(); });
        return;
    }
    m_applyTo = ApplyTo::AllItems;
    m_copyName = UnusedLookName(look.name + "Copy");
    if (m_applyItem >= 0)
    {
        m_newNumber =
            GetNextFreeItemNumber(Data::Items::GetItemGroup(m_applyItem), Data::Items::GetItemNumber(m_applyItem));
        m_newItemName = g_ItemDatabase.Find(m_applyItem)->names.GetNeutral();
    }
    ImGui::OpenPopup(ConfirmPopupId);
}

std::string CMuStyleEditorUI::UnusedLookName(const std::string& base) const
{
    std::string name = base;
    for (int suffix = 2; m_renderStyles.Has(name); ++suffix)
    {
        name = base + std::to_string(suffix);
    }
    return name;
}

void CMuStyleEditorUI::RenderConfirm()
{
    const std::string title = std::string(I18N::Editor::ApplyTheChange) + ConfirmPopupId;
    if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    const auto choice = [this](ApplyTo to, const std::string& label)
    {
        if (ImGui::RadioButton(label.c_str(), m_applyTo == to))
        {
            m_applyTo = to;
        }
    };
    choice(ApplyTo::AllItems,
           I18N::Format(I18N::Editor::ForAll0ItemsThatUseIt, {std::to_string(m_confirmUsers.size())}));
    if (m_applyTo == ApplyTo::AllItems && !m_confirmUsers.empty())
    {
        ImGui::Indent();
        RenderNote(I18N::Format(I18N::Editor::ThisChanges0ForTheseItemsCtrlZUndoesIt, {m_draftOf}).c_str());
        if (ImGui::BeginListBox("##ConfirmUsers", ImVec2(NameWidth * g_MuEditorCore.GetUIScale(),
                                                         UserListLines * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (const int user : m_confirmUsers)
            {
                ImGui::TextUnformatted(g_ItemDatabase.GetLogName(user).c_str());
            }
            ImGui::EndListBox();
        }
        ImGui::Unindent();
    }
    const bool item = m_applyItem >= 0;
    const std::string itemName = item ? g_ItemDatabase.GetLogName(m_applyItem) : "";
    ImGui::BeginDisabled(!item);
    choice(ApplyTo::ThisItem, I18N::Format(I18N::Editor::Only0AsACopyOfTheLook, {itemName}));
    choice(ApplyTo::NewItem, I18N::Format(I18N::Editor::ANewItemFrom0, {itemName}));
    ImGui::EndDisabled();
    if (!item)
    {
        RenderNote(I18N::Editor::PickAnItemToApplyToIt);
    }
    const bool usable = m_applyTo == ApplyTo::AllItems || (item && RenderApplyCopy());

    ImGui::BeginDisabled(!usable);
    if (ImGui::Button(I18N::Editor::Apply))
    {
        const ApplyTo to = m_applyTo;
        const std::string copyName = MuEditor::SourceText::Trimmed(m_copyName);
        const int itemType = m_applyItem;
        const int newItemType = item ? Data::Items::MakeItemType(Data::Items::GetItemGroup(itemType), m_newNumber) : -1;
        const std::string newItemName = m_newItemName;
        m_tree.Later(
            [this, to, copyName, itemType, newItemType, newItemName]
            {
                if (to == ApplyTo::AllItems)
                {
                    Apply();
                }
                else
                {
                    ApplyCopy(to, copyName, itemType, newItemType, newItemName);
                }
            });
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

bool CMuStyleEditorUI::RenderApplyCopy()
{
    if (m_applyTo == ApplyTo::AllItems)
    {
        return true;
    }
    ImGui::Indent();
    const float width = NameWidth * g_MuEditorCore.GetUIScale();
    RenderNote(m_applyTo == ApplyTo::ThisItem ? I18N::Format(I18N::Editor::TheOtherItemsKeep0, {m_draftOf}).c_str()
                                              : I18N::Editor::NewItemNote);
    ImGui::SetNextItemWidth(width);
    MuEditor::Items::InputString(I18N::Editor::LookName, m_copyName);
    const std::string copy = MuEditor::SourceText::Trimmed(m_copyName);
    bool usable = Data::Items::Json::IsName(copy) && !m_renderStyles.Has(copy);
    if (!usable)
    {
        RenderNote(I18N::Editor::ALookNameOfLettersAndDigitsThatNoOtherRenderStyleHas);
    }
    if (m_applyTo == ApplyTo::NewItem)
    {
        const int group = Data::Items::GetItemGroup(m_applyItem);
        ImGui::SetNextItemWidth(width);
        ImGui::InputInt(I18N::Editor::Number, &m_newNumber);
        m_newNumber = std::clamp(m_newNumber, 0, MAX_ITEM_INDEX - 1);
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::NextFree))
        {
            m_newNumber = GetNextFreeItemNumber(group, m_newNumber + 1);
        }
        if (!Data::Items::IsFreeItemType(Data::Items::MakeItemType(group, m_newNumber)))
        {
            ImGui::TextColored(NotFreeColor, "%s", I18N::Editor::ThisNumberIsNotFree);
            usable = false;
        }
        ImGui::SetNextItemWidth(width);
        MuEditor::Items::InputString(I18N::Editor::EnglishName, m_newItemName);
        usable = usable && !m_newItemName.empty() &&
                 m_newItemName.find(Data::LocalizedString::Separator) == std::string::npos;
    }
    ImGui::Unindent();
    return usable;
}

void CMuStyleEditorUI::Apply()
{
    if (g_LookEdits.Find(m_draftOf) == nullptr)
    {
        return;
    }
    g_LookEdits.SetLook(I18N::Editor::ChangeALook, m_draftOf, m_draft);
    m_draftLoaded = m_draft;
}

void CMuStyleEditorUI::ApplyCopy(ApplyTo to, const std::string& copyName, int itemType, int newItemType,
                                 const std::string& itemName)
{
    if (g_LookEdits.Find(m_draftOf) == nullptr)
    {
        return;
    }
    LookDefinition copy = m_draft;
    copy.name = copyName;
    if (to == ApplyTo::ThisItem)
    {
        g_LookEdits.ApplyToItemOnly(I18N::Editor::OnlyThisItemACopyOfTheLook, m_draftOf, std::move(copy), itemType);
    }
    else
    {
        g_LookEdits.ApplyToNewItem(I18N::Editor::ANewItemWithTheLook, m_draftOf, std::move(copy), itemType, newItemType,
                                   itemName);
        if (g_ItemDatabase.Find(newItemType) != nullptr)
        {
            g_ItemSelection.Select(newItemType);
        }
    }
    // The look keeps what it had; the editor goes on with the copy.
    m_draft = m_draftLoaded;
    if (g_LookEdits.Find(copyName) != nullptr)
    {
        m_tree.PickEntry(RenderStylesRootIndex, copyName);
    }
}

void CMuStyleEditorUI::RenderLayers()
{
    ImGui::TextUnformatted(I18N::Editor::Layers);
    ImGui::SameLine();
    RenderLayerToolbar();
    const float height = LayerListLines * ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("##Layers", ImVec2(0.0f, height), ImGuiChildFlags_Borders))
    {
        for (std::size_t index = 0; index < m_draft.layers.size(); ++index)
        {
            ImGui::PushID(static_cast<int>(index));
            bool shown = m_hiddenLayers[index] == 0;
            if (ImGui::Checkbox("##Shown", &shown))
            {
                m_hiddenLayers[index] = static_cast<std::uint8_t>(shown ? 0 : 1);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", I18N::Editor::ShowThisLayerInThePreview);
            }
            ImGui::SameLine();
            const std::string line =
                std::to_string(index + 1) + ". " + MuEditor::Looks::Fields::DescribeLayer(m_draft.layers[index]);
            if (ImGui::Selectable(line.c_str(), m_line == index))
            {
                m_line = index;
            }
            ImGui::PopID();
        }
        const std::string values = MuEditor::Looks::Fields::DescribeItemValues(m_draft.itemValuesAfter);
        const std::string line = std::string(I18N::Editor::ItemValuesAfterDrawing) + ": " +
                                 (values.empty() ? std::string(I18N::Editor::None) : values);
        if (ImGui::Selectable(line.c_str(), m_line == m_draft.layers.size()))
        {
            m_line = m_draft.layers.size();
        }
    }
    ImGui::EndChild();

    ImGui::PushID("Line");
    // Each line of each look keeps its own field state, such as "Each channel apart".
    ImGui::PushID(m_draftOf.c_str());
    ImGui::PushID(static_cast<int>(m_line));
    // The mesh fields offer the picked item's parts by name (design L42).
    const std::vector<LookPart> parts = MuEditor::Looks::PartsOf(g_ItemSelection.GetItemType());
    if (m_line < m_draft.layers.size())
    {
        MuEditor::Looks::Fields::Layer(m_draft.layers[m_line], parts);
    }
    else
    {
        MuEditor::Looks::Fields::ItemValues(m_draft.itemValuesAfter, parts);
    }
    ImGui::PopID();
    ImGui::PopID();
    ImGui::PopID();
}

void CMuStyleEditorUI::RenderLayerToolbar()
{
    const std::size_t count = m_draft.layers.size();
    const bool layerPicked = m_line < count;
    if (ImGui::SmallButton(I18N::Editor::Add))
    {
        // After the picked layer, as a plain texture layer.
        InsertLayer(layerPicked ? m_line + 1 : count, Layer{});
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!layerPicked);
    if (ImGui::SmallButton(I18N::Editor::Copy))
    {
        InsertLayer(m_line + 1, m_draft.layers[m_line]);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!layerPicked || m_line == 0);
    if (ImGui::SmallButton(I18N::Editor::MoveUp))
    {
        SwapLayers(m_line, m_line - 1);
        --m_line;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!layerPicked || m_line + 1 >= count);
    if (ImGui::SmallButton(I18N::Editor::MoveDown))
    {
        SwapLayers(m_line, m_line + 1);
        ++m_line;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::Remove))
    {
        RemoveLayer(m_line);
        m_line = std::min(m_line, m_draft.layers.size());
    }
    ImGui::EndDisabled();
}

std::vector<LookPart> CMuStyleEditorUI::PartRows() const
{
    std::vector<LookPart> rows = MuEditor::Looks::PartsOf(g_ItemSelection.GetItemType());
    for (const Layer& layer : m_draft.layers)
    {
        if (layer.meshes.allBut)
        {
            continue;
        }
        for (const int mesh : layer.meshes.meshes)
        {
            if (std::ranges::find(rows, mesh, &LookPart::mesh) == rows.end())
            {
                rows.push_back({.mesh = mesh});
            }
        }
    }
    std::ranges::sort(rows, {}, &LookPart::mesh);
    return rows;
}

void CMuStyleEditorUI::PickPart(int part)
{
    m_part = part;
    const std::optional<int> kept = g_ItemMeshFocus.Kept();
    if (part >= 0 ? kept != part : kept.has_value())
    {
        g_ItemMeshFocus.ToggleKept(part >= 0 ? part : *kept);
    }
    m_keptMesh = g_ItemMeshFocus.Kept();
}

void CMuStyleEditorUI::AddEffect(EffectKind kind)
{
    if (Data::Looks::NeedsOnePart(kind) && m_part < 0)
    {
        return;
    }
    std::vector<int> meshes;
    if (m_part >= 0)
    {
        meshes.push_back(m_part);
    }
    InsertLayer(m_draft.layers.size(), Data::Looks::MakeEffectLayer(kind, std::move(meshes)));
}

void CMuStyleEditorUI::RenderParts()
{
    // A click on a part in the preview picks its row.
    const std::optional<int> kept = g_ItemMeshFocus.Kept();
    if (kept != m_keptMesh)
    {
        m_keptMesh = kept;
        if (kept)
        {
            m_part = *kept;
        }
    }
    const std::vector<LookPart> parts = PartRows();
    if (m_part >= 0 && std::ranges::find(parts, m_part, &LookPart::mesh) == parts.end())
    {
        m_part = EveryPart;
    }

    ImGui::TextUnformatted(I18N::Editor::PartsAndTheirEffects);
    RenderNote(I18N::Editor::PartsNote);
    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;
    const float height = PartListLines * ImGui::GetFrameHeightWithSpacing();
    std::string pickedLabel = I18N::Editor::EveryPart;
    if (ImGui::BeginTable("##Parts", 2, flags, ImVec2(0.0f, height)))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn(I18N::Editor::Part, ImGuiTableColumnFlags_WidthFixed,
                                PartWidth * g_MuEditorCore.GetUIScale());
        ImGui::TableSetupColumn(I18N::Editor::Effects, ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        RenderPartRow(I18N::Editor::BaseOwnTexture, BasePart, RowLayers(m_draft, BasePart));
        RenderPartRow(I18N::Editor::EveryPart, EveryPart, RowLayers(m_draft, EveryPart));
        for (const LookPart& part : parts)
        {
            const std::string label = MuEditor::Looks::PartLabel(part);
            RenderPartRow(label.c_str(), part.mesh, RowLayers(m_draft, part.mesh));
            if (part.mesh == m_part)
            {
                pickedLabel = label;
            }
        }
        ImGui::EndTable();
    }

    // The base is the item's own texture, no effect of the gallery.
    ImGui::BeginDisabled(m_part == BasePart);
    const std::string add = I18N::Format(I18N::Editor::AddEffectTo0, {pickedLabel}) + AddEffectPopupId;
    if (ImGui::Button(add.c_str()))
    {
        ImGui::OpenPopup(AddEffectPopupId);
    }
    ImGui::EndDisabled();
    RenderAddEffect();
    ImGui::Separator();
    RenderEffectSettings();
}

void CMuStyleEditorUI::RenderPartRow(const char* label, int part, const std::vector<std::size_t>& layers)
{
    ImGui::PushID(part);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    if (ImGui::Selectable(label, m_part == part,
                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
    {
        PickPart(part);
    }
    if (part >= 0 && ImGui::IsItemHovered())
    {
        g_ItemMeshFocus.Point(part);
    }
    ImGui::TableSetColumnIndex(1);
    if (layers.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::NoEffect);
    }
    for (std::size_t index = 0; index < layers.size(); ++index)
    {
        if (index > 0)
        {
            ImGui::SameLine();
        }
        RenderEffectChip(layers[index], part);
    }
    ImGui::PopID();
}

void CMuStyleEditorUI::RenderEffectChip(std::size_t layer, int part)
{
    ImGui::PushID(static_cast<int>(layer));
    bool shown = m_hiddenLayers[layer] == 0;
    if (ImGui::Checkbox("##Shown", &shown))
    {
        m_hiddenLayers[layer] = static_cast<std::uint8_t>(shown ? 0 : 1);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::ShowThisEffectInThePreview);
    }
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    const Layer& drawn = m_draft.layers[layer];
    std::string label = MuEditor::Looks::Fields::EffectLabel(Data::Looks::DescribeEffect(drawn));
    // The rows of the base and of every part say which meshes, when not all.
    if (part == BasePart || (part == EveryPart && !drawn.meshes.IsAll()))
    {
        label += ": " + MuEditor::Looks::Fields::DescribeMeshes(drawn.meshes);
    }
    const bool picked = m_line == layer;
    if (picked)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    }
    if (ImGui::SmallButton(label.c_str()))
    {
        PickPart(part);
        m_line = layer;
    }
    if (picked)
    {
        ImGui::PopStyleColor();
    }
    ImGui::PopID();
}

void CMuStyleEditorUI::RenderAddEffect()
{
    if (!ImGui::BeginPopup(AddEffectPopupId))
    {
        return;
    }
    for (const EffectKind kind : Data::Looks::GetGalleryEffects())
    {
        ImGui::PushID(static_cast<int>(kind));
        // A flowing texture and frames move on one part.
        const bool needsPart = Data::Looks::NeedsOnePart(kind) && m_part < 0;
        ImGui::BeginDisabled(needsPart);
        if (ImGui::Selectable(MuEditor::Looks::Fields::EffectName(kind)))
        {
            AddEffect(kind);
        }
        ImGui::EndDisabled();
        if (needsPart && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("%s", I18N::Editor::MovesOnOnePart);
        }
        ImGui::SameLine(EffectNoteColumn * g_MuEditorCore.GetUIScale());
        ImGui::TextDisabled("%s", MuEditor::Looks::Fields::EffectNote(kind));
        ImGui::PopID();
    }
    ImGui::EndPopup();
}

void CMuStyleEditorUI::RenderEffectSettings()
{
    std::vector<Layer>& layers = m_draft.layers;
    if (m_line >= layers.size())
    {
        RenderNote(I18N::Editor::PickAnEffectOrAddOne);
        return;
    }
    Layer& layer = layers[m_line];
    const Data::Looks::LayerEffect effect = Data::Looks::DescribeEffect(layer);
    ImGui::SeparatorText(MuEditor::Looks::Fields::EffectLabel(effect).c_str());
    ImGui::Checkbox(I18N::Editor::AllFields, &m_allFields);
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::RemoveTheEffect))
    {
        RemoveLayer(m_line);
        m_line = layers.size();
        return;
    }
    ImGui::PushID("Effect");
    // Each effect of each look keeps its own field state, such as "Each channel apart".
    ImGui::PushID(m_draftOf.c_str());
    ImGui::PushID(static_cast<int>(m_line));
    const std::vector<LookPart> parts = MuEditor::Looks::PartsOf(g_ItemSelection.GetItemType());
    if (m_allFields)
    {
        MuEditor::Looks::Fields::Layer(layer, parts);
    }
    else
    {
        MuEditor::Looks::Fields::Effect(layer, effect, parts);
    }
    ImGui::PopID();
    ImGui::PopID();
    ImGui::PopID();
}

void CMuStyleEditorUI::RenderChecks()
{
    if (m_draftIssues.empty())
    {
        return;
    }
    if (ImGui::TreeNodeEx("##Problems", ImGuiTreeNodeFlags_DefaultOpen, "%s (%d)", I18N::Editor::ProblemsOfThisLook,
                          static_cast<int>(m_draftIssues.size())))
    {
        for (const ItemDataIssue& issue : m_draftIssues)
        {
            RenderIssue(issue);
        }
        ImGui::TreePop();
    }
}

const std::vector<int>& CMuStyleEditorUI::UsersOf(const std::string& name, bool itemEffect)
{
    const int modelsVersion = g_ItemModelDatabase.GetVersion();
    if (m_users.name != name || m_users.itemEffect != itemEffect || m_users.modelsVersion != modelsVersion)
    {
        m_users = {name, itemEffect, modelsVersion,
                   itemEffect ? MuEditor::Items::FindItemEffectUsers(name)
                              : MuEditor::Items::FindRenderStyleUsers(name)};
    }
    return m_users.items;
}

void CMuStyleEditorUI::RenderUsers(const std::vector<int>& users)
{
    const std::string label = I18N::Format(I18N::Editor::UsedBy0Items, {std::to_string(users.size())});
    if (users.empty())
    {
        ImGui::TextUnformatted(label.c_str());
        return;
    }
    if (!ImGui::TreeNodeEx("##Users", ImGuiTreeNodeFlags_DefaultOpen, "%s", label.c_str()))
    {
        return;
    }
    if (ImGui::BeginListBox("##UserList", ImVec2(-FLT_MIN, UserListLines * ImGui::GetTextLineHeightWithSpacing())))
    {
        for (const int user : users)
        {
            ImGui::PushID(user);
            if (ImGui::Selectable(g_ItemDatabase.GetLogName(user).c_str(), user == g_ItemSelection.GetItemType()))
            {
                g_ItemSelection.Select(user);
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
    ImGui::TreePop();
}

void CMuStyleEditorUI::RenderPreview(const LookDefinition* look)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::Preview, ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    ImGui::Checkbox(I18N::Editor::StopTime, &m_stopTime);
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_stopTime);
    ImGui::SetNextItemWidth(NameWidth * g_MuEditorCore.GetUIScale());
    ImGui::SliderFloat(I18N::Editor::Time, &m_seconds, 0.0f, MaxPreviewSeconds, "%.2f s");
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::DrawAsADoppelganger, &m_asDoppelganger);
    const int itemType = g_ItemSelection.GetItemType();
    if (g_ItemDatabase.Find(itemType) == nullptr || g_ItemModelDatabase.Find(itemType) == nullptr)
    {
        RenderNote(I18N::Editor::PickAnItemToSeeTheLookOnIt);
        return;
    }
    RenderNote(I18N::Format(I18N::Editor::ThePreviewDraws0, {g_ItemDatabase.GetLogName(itemType)}).c_str());
    m_preview.SetDrawAsDoppelganger(m_asDoppelganger);
    // The look as the fields have it, without the layers whose eye is off.
    LookDefinition shown;
    if (look != nullptr)
    {
        shown.name = look->name;
        shown.itemValuesAfter = look->itemValuesAfter;
        for (std::size_t index = 0; index < look->layers.size(); ++index)
        {
            if (index >= m_hiddenLayers.size() || m_hiddenLayers[index] == 0)
            {
                shown.layers.push_back(look->layers[index]);
            }
        }
    }
    // All views at the same moment.
    const WorldTimeScope time(m_stopTime ? std::optional<double>(m_seconds * 1000.0) : std::nullopt);
    if (look == nullptr)
    {
        // A look in code: the item as the game draws it.
        m_preview.Render(itemType);
        return;
    }
    // The item as it is (with its own look or none), then the look as applied and as the fields have it on the picked
    // item, whatever look it names (design L41).
    const LookDefinition* applied = g_LookEdits.Find(look->name);
    const int slot = Data::Items::ToModelSlot(itemType);
    m_preview.RenderSideBySide(
        itemType, 3, [this](int view, float width) { RenderViewHeader(view, width); },
        [&](int view, const std::function<void()>& draw)
        {
            if (view == 0)
            {
                if (m_originalWithoutLook)
                {
                    const Render::Items::Styles::PreviewLook withoutLook(slot);
                    draw();
                }
                else
                {
                    draw();
                }
                return;
            }
            const LookDefinition* drawn = view == 1 ? applied : &shown;
            if (drawn == nullptr)
            {
                draw();
                return;
            }
            const Render::Items::Styles::PreviewLook preview(slot, *drawn);
            draw();
        });
}

void CMuStyleEditorUI::RenderViewHeader(int view, float width)
{
    if (view != 0)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%s", view == 1 ? I18N::Editor::ChosenLook : I18N::Editor::AsEdited);
        return;
    }
    // The original item: as the game draws it, or without a look, to see what a look adds.
    const char* const shows[] = {I18N::Editor::OriginalItemAsInTheGame, I18N::Editor::OriginalItemWithoutALook};
    int shown = m_originalWithoutLook ? 1 : 0;
    ImGui::SetNextItemWidth(width);
    if (ImGui::Combo("##OriginalItem", &shown, shows, IM_ARRAYSIZE(shows)))
    {
        m_originalWithoutLook = shown == 1;
    }
}

void CMuStyleEditorUI::UpdateDraft(const LookDefinition& look)
{
    if (m_draftOf != look.name)
    {
        // The changes not applied stay with their look while another one is picked.
        KeepDraft();
        if (!TakeKeptDraft(look.name))
        {
            m_line = 0;
            LoadDraft(look);
        }
    }
    else if (m_draftVersion != g_LookEdits.GetVersion() && m_draft == m_draftLoaded)
    {
        LoadDraft(look);
    }
    m_draftVersion = g_LookEdits.GetVersion();
}

bool CMuStyleEditorUI::HasUnappliedChanges(const std::string& name) const
{
    return (name == m_draftOf && m_draft != m_draftLoaded) || m_keptDrafts.contains(name);
}

void CMuStyleEditorUI::ForgetDraftsForTest()
{
    m_draft = {};
    m_draftLoaded = {};
    m_draftOf.clear();
    m_draftVersion = -1;
    m_keptDrafts.clear();
    m_hiddenLayers.clear();
    m_line = 0;
}

void CMuStyleEditorUI::KeepDraft()
{
    if (!m_draftOf.empty() && m_draft != m_draftLoaded)
    {
        m_keptDrafts[m_draftOf] = {m_draft, m_draftLoaded, m_hiddenLayers, m_line};
    }
}

bool CMuStyleEditorUI::TakeKeptDraft(const std::string& name)
{
    const auto kept = m_keptDrafts.find(name);
    if (kept == m_keptDrafts.end())
    {
        return false;
    }
    m_draft = std::move(kept->second.draft);
    m_draftLoaded = std::move(kept->second.loaded);
    m_hiddenLayers = std::move(kept->second.hiddenLayers);
    m_line = kept->second.line;
    m_draftOf = name;
    m_keptDrafts.erase(kept);
    return true;
}

void CMuStyleEditorUI::RenameDraft(const std::string& from, const std::string& to)
{
    if (m_draftOf == from)
    {
        m_draftOf = to;
        m_draft.name = to;
        m_draftLoaded.name = to;
    }
    if (auto kept = m_keptDrafts.extract(from))
    {
        kept.key() = to;
        kept.mapped().draft.name = to;
        kept.mapped().loaded.name = to;
        m_keptDrafts.insert(std::move(kept));
    }
}

void CMuStyleEditorUI::LoadDraft(const LookDefinition& look)
{
    m_draft = look;
    m_draftLoaded = look;
    m_draftOf = look.name;
    m_hiddenLayers.assign(m_draft.layers.size(), 0);
    m_line = std::min(m_line, m_draft.layers.size());
}

void CMuStyleEditorUI::InsertLayer(std::size_t at, Layer layer)
{
    m_draft.layers.insert(m_draft.layers.begin() + static_cast<std::ptrdiff_t>(at), std::move(layer));
    m_hiddenLayers.insert(m_hiddenLayers.begin() + static_cast<std::ptrdiff_t>(at), std::uint8_t{0});
    m_line = at;
}

void CMuStyleEditorUI::RemoveLayer(std::size_t at)
{
    m_draft.layers.erase(m_draft.layers.begin() + static_cast<std::ptrdiff_t>(at));
    m_hiddenLayers.erase(m_hiddenLayers.begin() + static_cast<std::ptrdiff_t>(at));
}

void CMuStyleEditorUI::SwapLayers(std::size_t first, std::size_t second)
{
    std::swap(m_draft.layers[first], m_draft.layers[second]);
    std::swap(m_hiddenLayers[first], m_hiddenLayers[second]);
}

void CMuStyleEditorUI::CheckDraft()
{
    if (m_checked && m_checkedDraft == m_draft && m_checkedModels == g_ItemModelDatabase.GetVersion())
    {
        return;
    }
    // While a field is dragged or typed into, the draft changes each frame: it is checked once the field is let go,
    // and Apply, which needs the click, keeps the last check until then. Another look is checked at once.
    if (m_checked && m_checkedDraft.name == m_draft.name && ImGui::IsAnyItemActive())
    {
        return;
    }
    m_checked = true;
    m_checkedDraft = m_draft;
    m_checkedModels = g_ItemModelDatabase.GetVersion();
    // The textures the edited look names, for its preview: looked for when it changes, not each frame.
    Render::Items::Looks::PrepareDraftTextures(m_draft);
    m_draftIssues.clear();
    // The look as the file would have it, read back: what a save would refuse is fatal.
    const std::string source(Data::Looks::GetRenderStylesFileName());
    Data::Looks::RenderStylesFile file;
    Data::Looks::ReadRenderStylesFile(Data::Looks::WriteRenderStylesJson({m_draft}), source, file, m_draftIssues);
    if (!Data::Items::HasFatal(m_draftIssues))
    {
        Data::Looks::CheckRenderStyles(file.looks, source, MuEditor::Looks::MakeEditorCheckInput(), m_draftIssues);
    }
    // The source is the file for all of them.
    for (ItemDataIssue& issue : m_draftIssues)
    {
        issue.source.clear();
    }
}

#endif // _EDITOR
