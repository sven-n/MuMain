#include "stdafx.h"

#ifdef _EDITOR

#include "ServerListColorsRoot.h"

#include "NamedColorList.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "../MuEditor/UI/ServerList/MuServerListUI.h"
#include "../MuEditor/UI/ServerList/ServerListColors.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "Data/GameData/ServerListData/ButtonColors.h"
#include "I18N/All.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>

namespace MuEditor::Colors
{
namespace
{
using Data::ServerList::CustomColor;
using Data::ServerList::ServerListData;
namespace ListColors = MuEditor::ServerList::Colors;

constexpr char NewPopupId[] = "###NewServerListColor";
// Pixel widths before the editor UI scale.
constexpr float IdFieldWidth = 180.0f;
constexpr float NameFieldWidth = 320.0f;

const ServerListData& List()
{
    g_MuServerListUI.EnsureLoaded();
    return g_MuServerListUI.GetEdits().GetList();
}

const CustomColor* Find(std::string_view id)
{
    return List().FindCustomColor(id);
}

std::optional<std::size_t> IndexOf(std::string_view id)
{
    const std::vector<CustomColor>& colors = List().colors;
    const auto found = std::find_if(colors.begin(), colors.end(), [id](const CustomColor& color) { return color.id == id; });
    if (found == colors.end())
    {
        return std::nullopt;
    }
    return static_cast<std::size_t>(found - colors.begin());
}

std::string FormatShade(const float rgb[3])
{
    char text[8] = {};
    const auto channel = [](float value)
    { return static_cast<unsigned int>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f); };
    std::snprintf(text, sizeof(text), "#%02X%02X%02X", channel(rgb[0]), channel(rgb[1]), channel(rgb[2]));
    return text;
}

// The themes found, and any other one the color has a shade for.
std::vector<std::string> ThemesOf(const CustomColor& color)
{
    std::vector<std::string> themes = g_MuServerListUI.GetThemes();
    for (const auto& [theme, shade] : color.shades)
    {
        if (std::find(themes.begin(), themes.end(), theme) == themes.end())
        {
            themes.push_back(theme);
        }
    }
    return themes;
}
} // namespace

NamedColorTexts ServerListColorsRoot::GetTexts() const
{
    NamedColorTexts texts;
    texts.root = I18N::Editor::ServerList;
    texts.note = I18N::Editor::ServerListColorsNote;
    texts.removeOne = I18N::Editor::RemoveAColor;
    texts.removeUnused = I18N::Editor::RemoveUnusedColors;
    texts.apply = I18N::Editor::ApplyToTheColor;
    texts.change = I18N::Editor::ChangeAColor;
    texts.usedBy = I18N::Editor::UsedBy0RulesAndLoadParts;
    texts.changeReaches = I18N::Editor::TheChangeReachesThese0RulesAndLoadParts;
    texts.unused = I18N::Editor::NoRuleAndNoLoadPartUsesTheseColors0;
    texts.kept = "";
    return texts;
}

int ServerListColorsRoot::GetVersion() const
{
    return g_MuServerListUI.GetEdits().GetVersion();
}

std::size_t ServerListColorsRoot::GetCount() const
{
    return List().colors.size();
}

const std::string& ServerListColorsRoot::GetName(std::size_t index) const
{
    return List().colors[index].id;
}

bool ServerListColorsRoot::Has(const std::string& name) const
{
    return Find(name) != nullptr;
}

void ServerListColorsRoot::RenderSwatch(const std::string& name) const
{
    const std::string_view theme = ListColors::GetGameTheme();
    ListColors::RenderSwatch(
        ListColors::GetButtonColorSwatch(Data::ServerList::ResolveButtonColor(List(), name, theme), theme));
}

int ServerListColorsRoot::CountUsers(const std::string& name)
{
    const MuEditor::ServerList::ServerListEdits& edits = g_MuServerListUI.GetEdits();
    return static_cast<int>(edits.CountRulesWithColor(name) + edits.CountLoadUsesOfColor(name));
}

std::vector<NamedColorUser> ServerListColorsRoot::FindUsers(const std::string& name) const
{
    std::vector<NamedColorUser> users;
    for (std::string& label : g_MuServerListUI.DescribeColorUsers(name))
    {
        users.push_back({std::move(label), [] { g_MuServerListUI.ShowChannelNames(); }, false});
    }
    return users;
}

bool ServerListColorsRoot::IsKept(const std::string&) const
{
    return false;
}

void ServerListColorsRoot::Remove(const char* step, std::span<const std::string> names)
{
    MuEditor::ServerList::ServerListEdits& edits = g_MuServerListUI.GetEdits();
    for (const std::string& name : names)
    {
        if (const std::optional<std::size_t> index = IndexOf(name))
        {
            edits.RemoveColor(*index);
        }
    }
    // The step's name: the folders stay as the removals left them.
    edits.SetColorFolders(edits.GetList().colorFolders, step);
    g_MuServerListUI.AfterColorEdit();
}

const std::vector<Data::Colors::ColorFolder>& ServerListColorsRoot::GetFolders() const
{
    return List().colorFolders;
}

void ServerListColorsRoot::SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders)
{
    g_MuServerListUI.GetEdits().SetColorFolders(std::move(folders), step);
    g_MuServerListUI.AfterColorEdit();
}

void ServerListColorsRoot::SetOrder(const char* step, std::span<const std::string> names)
{
    g_MuServerListUI.GetEdits().SetColorOrder(names, step);
    g_MuServerListUI.AfterColorEdit();
}

void ServerListColorsRoot::OpenNew(const std::string& from)
{
    m_newFrom = from;
    m_newName.clear();
    ImGui::OpenPopup(NewPopupId);
}

std::optional<std::string> ServerListColorsRoot::RenderNew(const Data::Colors::ColorFolderPath& folder)
{
    const std::string title = std::string(I18N::Editor::NewColor) + NewPopupId;
    if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return std::nullopt;
    }
    std::optional<std::string> created;
    ImGui::SetNextItemWidth(NameFieldWidth * g_MuEditorCore.GetUIScale());
    MuEditor::Items::InputString(I18N::Editor::Name, m_newName);
    Core::Text::CutToUtf16Length(m_newName, Data::ServerList::MaxCustomColorNameLength);
    ImGui::BeginDisabled(m_newName.empty());
    if (ImGui::Button(I18N::Editor::OK))
    {
        MuEditor::ServerList::ServerListEdits& edits = g_MuServerListUI.GetEdits();
        const std::size_t index = edits.AddColor(m_newName);
        if (const CustomColor* from = Find(m_newFrom))
        {
            edits.EditList().colors[index].shades = from->shades;
        }
        created = edits.GetList().colors[index].id;
        // Into the picked folder in the same step.
        Data::Colors::MoveColorInto(edits.EditList().colorFolders, *created, folder);
        g_MuServerListUI.AfterColorEdit();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
    return created;
}

void ServerListColorsRoot::RenderEdit(const std::string& name)
{
    const CustomColor* color = Find(name);
    if (color == nullptr)
    {
        return;
    }
    UpdateEdit(*color);
    const float scale = g_MuEditorCore.GetUIScale();

    ImGui::SetNextItemWidth(IdFieldWidth * scale);
    MuEditor::Items::InputString(I18N::Editor::VIPTypeID, m_edit.id);
    if (!IsEditedIdUsable(name))
    {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", I18N::Editor::NotAFreeColorID);
        ImGui::PopTextWrapPos();
    }
    RenderColorNote(I18N::Editor::ColorIDNote);

    ImGui::SetNextItemWidth(NameFieldWidth * scale);
    MuEditor::Items::InputString(I18N::Editor::Name, m_edit.name);
    Core::Text::CutToUtf16Length(m_edit.name, Data::ServerList::MaxCustomColorNameLength);

    for (const std::string& theme : ThemesOf(m_edit))
    {
        ImGui::PushID(theme.c_str());
        const auto shade = m_edit.shades.find(theme);
        const bool hasShade = shade != m_edit.shades.end();
        const ImVec4 fallback = ListColors::GetThemeColorSwatch(Data::ServerList::ThemeColor::Default, theme);
        const ImVec4 current = hasShade ? ListColors::ParseShade(shade->second).value_or(fallback) : fallback;
        float rgb[3] = {current.x, current.y, current.z};
        const std::string label = UI::RmlBridge::GetThemeDisplayName(theme);
        if (ImGui::ColorEdit3(label.c_str(), rgb, ImGuiColorEditFlags_Uint8))
        {
            m_edit.shades[theme] = FormatShade(rgb);
        }
        ImGui::SameLine();
        if (hasShade)
        {
            if (ImGui::SmallButton(I18N::Editor::Remove))
            {
                m_edit.shades.erase(theme);
            }
        }
        else
        {
            ImGui::TextDisabled("(%s)", I18N::Editor::Default);
        }
        ImGui::PopID();
    }
}

bool ServerListColorsRoot::IsEdited(const std::string& name) const
{
    const CustomColor* color = Find(name);
    return color != nullptr && m_editOf == name && m_edit != *color && IsEditedIdUsable(name);
}

void ServerListColorsRoot::ApplyEdit(const std::string& name)
{
    const std::optional<std::size_t> index = IndexOf(name);
    if (!index || !IsEditedIdUsable(name))
    {
        return;
    }
    MuEditor::ServerList::ServerListEdits& edits = g_MuServerListUI.GetEdits();
    edits.ChangeColorId(*index, m_edit.id);
    CustomColor& color = edits.EditList().colors[*index];
    color.name = m_edit.name;
    color.shades = m_edit.shades;
    m_loaded = color;
    m_editOf = color.id;
    g_MuServerListUI.AfterColorEdit();
}

void ServerListColorsRoot::RevertEdit(const std::string& name)
{
    if (const CustomColor* color = Find(name))
    {
        m_edit = *color;
        m_loaded = *color;
    }
}

std::string ServerListColorsRoot::GetNameAfterEdit(const std::string& name) const
{
    return IsEditedIdUsable(name) ? m_edit.id : name;
}

MuEditor::UndoTarget& ServerListColorsRoot::GetUndoTarget()
{
    return g_MuServerListUI;
}

void ServerListColorsRoot::UpdateEdit(const CustomColor& color)
{
    const bool unchanged = m_edit == m_loaded;
    if (m_editOf != color.id || (color != m_loaded && unchanged))
    {
        m_edit = color;
        m_loaded = color;
        m_editOf = color.id;
    }
}

bool ServerListColorsRoot::IsEditedIdUsable(const std::string& name) const
{
    return m_edit.id == name || g_MuServerListUI.GetEdits().IsColorIdFree(m_edit.id);
}
} // namespace MuEditor::Colors

#endif // _EDITOR
