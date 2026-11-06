#include "stdafx.h"

#ifdef _EDITOR

#include "MuStyleEditorUI.h"

#include "LookEdits.h"
#include "LookFields.h"
#include "RenderStyleCheckArea.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorToolWindows.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Common/SourceText.h"
#include "../MuEditor/UI/Common/ToolGroups.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorActions.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorPopups.h"
#include "../MuEditor/UI/ItemTools/ItemEdits.h"
#include "../MuEditor/UI/ItemTools/ItemToolWindow.h"
#include "../MuEditor/UI/ItemTools/ItemSelection.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "../MuEditor/UI/ItemTools/ItemUsers.h"
#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/LookData/LookChecks.h"
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
using Data::Looks::Layer;
using Data::Looks::LookDefinition;

namespace
{
constexpr char FolderPopupId[] = "###LookFolder";
constexpr char NamePopupId[] = "###LookName";
constexpr char ConfirmPopupId[] = "###LookChange";
constexpr char DragPayloadType[] = "MU_LOOKS_TREE";
// The roots in the order the tree shows them.
constexpr std::size_t RenderStylesRootIndex = 0;
constexpr std::size_t ItemEffectsRootIndex = 1;
// Pixel widths before the editor UI scale.
constexpr float TreeWidth = 260.0f;
constexpr float NameWidth = 260.0f;
constexpr float UserListLines = 6.0f;
constexpr float LayerListLines = 7.0f;
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

CMuStyleEditorUI::CMuStyleEditorUI() : m_tree({&m_renderStyles, &m_itemEffects}, DragPayloadType, FolderPopupId) {}

void CMuStyleEditorUI::Render(bool* open)
{
    m_tree.KeepPickValid();
    // One of the item tools (design L39): their docking, undo history and Save Items.
    if (MuEditor::Items::ToolWindow::Begin(I18N::Editor::StyleEditor, MuEditor::WindowIds::StyleEditor, open))
    {
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
        RenderUsers(itemEffect ? MuEditor::Items::FindItemEffectUsers(name)
                               : MuEditor::Items::FindRenderStyleUsers(name));
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
    // The look's problems, layers and fields scroll below the preview, which stays where it is.
    if (ImGui::BeginChild("##Look"))
    {
        RenderChecks();
        RenderLayers();
        ImGui::Separator();
        RenderUsers(MuEditor::Items::FindRenderStyleUsers(name));
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
        // D26: a change to a look reaches all its items; they are listed before it is applied.
        m_confirmUsers = MuEditor::Items::FindRenderStyleUsers(look.name);
        if (m_confirmUsers.empty())
        {
            // After the window is drawn: the look's place changes with it.
            m_tree.Later([this] { Apply(); });
        }
        else
        {
            ImGui::OpenPopup(ConfirmPopupId);
        }
    }
    ImGui::SameLine();
    ImGui::EndDisabled();
    ImGui::BeginDisabled(!edited);
    if (ImGui::Button(I18N::Editor::Revert))
    {
        m_draft = look;
        m_draftLoaded = look;
        m_hiddenLayers.assign(m_draft.layers.size(), 0);
        m_line = std::min(m_line, m_draft.layers.size());
    }
    ImGui::EndDisabled();
    if (edited && fatal)
    {
        ImGui::SameLine();
        ImGui::TextColored(SeverityColor(ItemDataIssueSeverity::Fatal), "%s",
                           I18N::Editor::ApplyIsOffWhileAFatalProblemIsLeft);
    }
}

void CMuStyleEditorUI::RenderConfirm()
{
    const std::string title = std::string(I18N::Editor::ChangeALook) + ConfirmPopupId;
    if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ImGui::TextUnformatted(
        I18N::Format(I18N::Editor::TheChangeReachesThese0Items, {std::to_string(m_confirmUsers.size())}).c_str());
    if (ImGui::BeginListBox("##ConfirmUsers", ImVec2(NameWidth * g_MuEditorCore.GetUIScale(),
                                                     UserListLines * ImGui::GetTextLineHeightWithSpacing())))
    {
        for (const int user : m_confirmUsers)
        {
            ImGui::TextUnformatted(g_ItemDatabase.GetLogName(user).c_str());
        }
        ImGui::EndListBox();
    }
    if (ImGui::Button(I18N::Editor::Apply))
    {
        m_tree.Later([this] { Apply(); });
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
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

void CMuStyleEditorUI::RenderLayers()
{
    ImGui::TextUnformatted(I18N::Editor::Layers);
    ImGui::SameLine();
    RenderLayerToolbar();
    m_hiddenLayers.resize(m_draft.layers.size(), 0);
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
    if (m_line < m_draft.layers.size())
    {
        MuEditor::Looks::Fields::Layer(m_draft.layers[m_line]);
    }
    else
    {
        MuEditor::Looks::Fields::ItemValues(m_draft.itemValuesAfter);
    }
    ImGui::PopID();
}

void CMuStyleEditorUI::RenderLayerToolbar()
{
    std::vector<Layer>& layers = m_draft.layers;
    const bool layerPicked = m_line < layers.size();
    if (ImGui::SmallButton(I18N::Editor::Add))
    {
        // After the picked layer, as a plain texture layer.
        const std::size_t at = layerPicked ? m_line + 1 : layers.size();
        layers.insert(layers.begin() + static_cast<std::ptrdiff_t>(at), Layer{});
        m_hiddenLayers.insert(m_hiddenLayers.begin() + static_cast<std::ptrdiff_t>(at), std::uint8_t{0});
        m_line = at;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!layerPicked);
    if (ImGui::SmallButton(I18N::Editor::Copy))
    {
        const Layer copy = layers[m_line];
        layers.insert(layers.begin() + static_cast<std::ptrdiff_t>(m_line + 1), copy);
        m_hiddenLayers.insert(m_hiddenLayers.begin() + static_cast<std::ptrdiff_t>(m_line + 1), std::uint8_t{0});
        ++m_line;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!layerPicked || m_line == 0);
    if (ImGui::SmallButton(I18N::Editor::MoveUp))
    {
        std::swap(layers[m_line], layers[m_line - 1]);
        std::swap(m_hiddenLayers[m_line], m_hiddenLayers[m_line - 1]);
        --m_line;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!layerPicked || m_line + 1 >= layers.size());
    if (ImGui::SmallButton(I18N::Editor::MoveDown))
    {
        std::swap(layers[m_line], layers[m_line + 1]);
        std::swap(m_hiddenLayers[m_line], m_hiddenLayers[m_line + 1]);
        ++m_line;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::Remove))
    {
        layers.erase(layers.begin() + static_cast<std::ptrdiff_t>(m_line));
        m_hiddenLayers.erase(m_hiddenLayers.begin() + static_cast<std::ptrdiff_t>(m_line));
        m_line = std::min(m_line, layers.size());
    }
    ImGui::EndDisabled();
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
        Render::Items::Looks::PrepareTextures(shown);
    }
    // Both views at the same moment.
    const WorldTimeScope time(m_stopTime ? std::optional<double>(m_seconds * 1000.0) : std::nullopt);
    if (look == nullptr)
    {
        // A look in code: the item as the game draws it.
        m_preview.Render(itemType);
        return;
    }
    // The look as applied beside it as the fields have it, both on the picked item, whatever look it names.
    const LookDefinition* applied = g_LookEdits.Find(look->name);
    const int slot = Data::Items::ToModelSlot(itemType);
    m_preview.RenderSideBySide(itemType, I18N::Editor::AsApplied, I18N::Editor::AsEdited,
                               [&](int view, const std::function<void()>& draw)
                               {
                                   const LookDefinition* drawn = view == 0 ? applied : &shown;
                                   if (drawn == nullptr)
                                   {
                                       draw();
                                       return;
                                   }
                                   const Render::Items::Styles::PreviewLook preview(slot, *drawn);
                                   draw();
                               });
}

void CMuStyleEditorUI::UpdateDraft(const LookDefinition& look)
{
    const bool unchanged = m_draft == m_draftLoaded;
    if (m_draftOf != look.name || (m_draftVersion != g_LookEdits.GetVersion() && unchanged))
    {
        if (m_draftOf != look.name)
        {
            m_line = 0;
            m_hiddenLayers.assign(look.layers.size(), 0);
        }
        m_draft = look;
        m_draftLoaded = look;
        m_draftOf = look.name;
        m_line = std::min(m_line, m_draft.layers.size());
    }
    m_draftVersion = g_LookEdits.GetVersion();
}

void CMuStyleEditorUI::CheckDraft()
{
    if (m_checked && m_checkedDraft == m_draft && m_checkedModels == g_ItemModelDatabase.GetVersion())
    {
        return;
    }
    m_checked = true;
    m_checkedDraft = m_draft;
    m_checkedModels = g_ItemModelDatabase.GetVersion();
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
