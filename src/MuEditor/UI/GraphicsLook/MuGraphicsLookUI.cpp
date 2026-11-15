#include "stdafx.h"

#ifdef _EDITOR

#include "MuGraphicsLookUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Core/Text/Utf8.h"
#include "I18N/All.h"
#include "Render/Look/LookTuning.h"
#include "World/MapInfra/MapManager.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string_view>

namespace
{
using Data::Maps::LookOverrides;
using Data::Maps::LookProfileData;
using Data::Maps::LookScope;
using Data::Maps::LookValue;

// The width of the scope list, before the editor's UI scale.
constexpr float ScopeListWidth = 240.0f;
// The width of a value's slider, in font sizes.
constexpr float SliderWidthInFonts = 16.0f;

std::string FormatNumber(double value)
{
    std::ostringstream text;
    text << value;
    return text.str();
}

std::string JoinMaps(const std::vector<int>& maps)
{
    std::string text;
    for (const int map : maps)
    {
        text += (text.empty() ? "" : ", ") + std::to_string(map);
    }
    return text;
}

std::string MapName(int world)
{
    const wchar_t* name = gMapManager.GetMapName(world);
    return name ? Core::Text::ToUtf8(name) : std::string();
}

// Shown only when the drag ends: the sun shadow's resolution makes a new shadow map. A value the
// light mood reads rebuilds the static map light (a few ms), as Options' Mood strength does while dragged.
bool AppliesOnRelease(LookValue value)
{
    return value == LookValue::SunShadowResolution;
}

// The value table's names grouped by their prefix ("lighting", "mood", ...), each group in the
// order its first value has in the table and its values in table order.
std::vector<std::vector<std::size_t>> ValueGroups()
{
    std::vector<std::vector<std::size_t>> groups;
    std::vector<std::string_view> prefixes;
    const auto& infos = Data::Maps::LookValueInfos();
    for (std::size_t index = 0; index < infos.size(); ++index)
    {
        const std::string_view name = infos[index].name;
        const std::string_view prefix = name.substr(0, name.find('.'));
        const auto found = std::find(prefixes.begin(), prefixes.end(), prefix);
        if (found == prefixes.end())
        {
            prefixes.push_back(prefix);
            groups.push_back({index});
        }
        else
        {
            groups[static_cast<std::size_t>(found - prefixes.begin())].push_back(index);
        }
    }
    return groups;
}

// A slider moves in steps of a thousandth, so the file keeps short numbers.
double Rounded(float number)
{
    return std::round(static_cast<double>(number) * 1000.0) / 1000.0;
}

// The values `scope` sets itself; none for a map without an entry.
const LookOverrides* OwnValues(const LookProfileData& profile, const LookScope& scope)
{
    switch (scope.kind)
    {
    case LookScope::Kind::Environment:
        return scope.environment < profile.environments.size() ? &profile.environments[scope.environment].overrides
                                                               : nullptr;
    case LookScope::Kind::Map:
        if (const Data::Maps::MapLookOverrides* entry = profile.FindMap(scope.world))
        {
            return &entry->overrides;
        }
        return nullptr;
    case LookScope::Kind::Default:
        break;
    }
    return &profile.defaultOverrides;
}

Data::Maps::LookValues ValuesOf(const LookProfileData& profile, const LookScope& scope)
{
    switch (scope.kind)
    {
    case LookScope::Kind::Environment:
        return profile.ForEnvironment(scope.environment);
    case LookScope::Kind::Map:
        return profile.ForWorld(scope.world);
    case LookScope::Kind::Default:
        break;
    }
    Data::Maps::LookValues values = Data::Maps::BuiltInLookValues();
    profile.defaultOverrides.ApplyTo(values);
    return values;
}

// Where a value the scope doesn't set comes from.
std::string InheritedFrom(const LookProfileData& profile, const LookScope& scope, LookValue value)
{
    if (scope.kind == LookScope::Kind::Map)
    {
        const Data::Maps::MapLookOverrides* entry = profile.FindMap(scope.world);
        if (entry && entry->environment && *entry->environment < profile.environments.size() &&
            profile.environments[*entry->environment].overrides[value])
        {
            return I18N::Format(I18N::Editor::LookFromEnvironment0, {profile.environments[*entry->environment].name});
        }
    }
    if (scope.kind != LookScope::Kind::Default && profile.defaultOverrides[value])
    {
        return I18N::Editor::LookFromDefault;
    }
    return I18N::Editor::LookBuiltIn;
}

bool ApplyLive(const LookProfileData& profile)
{
    return Render::Look::ReplaceLive(profile).applied;
}

// Lets ImGui size the std::string it edits to the text, as misc/cpp/imgui_stdlib does.
int ResizeText(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        auto* text = static_cast<std::string*>(data->UserData);
        text->resize(data->BufTextLen);
        data->Buf = text->data();
    }
    return 0;
}

// A name field of any length; `hint` shows while it is empty.
void InputName(const char* label, const char* hint, std::string& text)
{
    if (hint)
    {
        ImGui::InputTextWithHint(label, hint, text.data(), text.capacity() + 1, ImGuiInputTextFlags_CallbackResize,
                                 ResizeText, &text);
        return;
    }
    ImGui::InputText(label, text.data(), text.capacity() + 1, ImGuiInputTextFlags_CallbackResize, ResizeText, &text);
}
} // namespace

CMuGraphicsLookUI& CMuGraphicsLookUI::GetInstance()
{
    static CMuGraphicsLookUI instance;
    return instance;
}

void CMuGraphicsLookUI::Render(bool* open)
{
    // A drag whose slider went (the window hidden, another item taken) never ends: put the profile back.
    if (m_dragBase && ImGui::GetActiveID() != m_dragId)
    {
        CancelDrag();
    }
    if (!BeginWindow(open))
    {
        return;
    }
    FollowLive();
    KeepScopeValid();
    RenderToolbar();
    RenderStatus();
    ImGui::Separator();
    const ImVec2 listSize(ScopeListWidth * g_MuEditorCore.GetUIScale(), 0.0f);
    if (ImGui::BeginChild("##LookScopes", listSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
    {
        RenderScopes();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    if (ImGui::BeginChild("##LookScope"))
    {
        RenderScope();
    }
    ImGui::EndChild();
    ImGui::End();
}

void CMuGraphicsLookUI::Show()
{
    g_MuEditorCore.GetToolWindows().graphicsLook = true;
    ImGui::SetWindowFocus(MuEditor::WindowIds::GraphicsLook);
}

bool CMuGraphicsLookUI::BeginWindow(bool* open)
{
    char title[MuEditor::WindowIds::TitleCapacity];
    MuEditor::Layout::PrepareToolWindow();
    if (!ImGui::Begin(
            MuEditor::WindowIds::Title(title, I18N::Editor::GraphicsLook, MuEditor::WindowIds::GraphicsLook), open,
            ImGuiWindowFlags_NoCollapse | MuEditor::Layout::ToolWindowBarFlags(MuEditor::WindowIds::GraphicsLook)))
    {
        ImGui::End();
        return false;
    }
    // Before the bar: in this window's own bar it shows the Graphics Look's undo then.
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        MuEditor::UndoTargets::Activate(*this);
    }
    MuEditor::Layout::RenderToolWindowBar();
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
    {
        g_MuEditorCore.SetHoveringUI(true);
    }
    HandleUndoKeys();
    return true;
}

void CMuGraphicsLookUI::FollowLive()
{
    const LookProfileData& live = Render::Look::LiveFiles().lookData;
    const std::uint32_t generation = Render::Look::LiveFilesGeneration();
    if (m_filesGeneration != generation)
    {
        m_history.Clear();
        m_seen = live;
        m_filesGeneration = generation;
        DropDrag();
        m_nameShownFor.reset();
        return;
    }
    if (live != m_seen)
    {
        // A `$look` command (or a change made while the window was closed); a drag shown live starts again from it.
        m_history.Record("$look", m_dragBase ? *m_dragBase : m_seen, live);
        m_seen = live;
        DropDrag();
    }
}

void CMuGraphicsLookUI::RenderToolbar()
{
    const Render::Look::LookFiles& live = Render::Look::LiveFiles();
    const bool canSave = live.lookUsable && !live.saveBlocked;
    ImGui::BeginDisabled(!canSave);
    if (ImGui::Button(I18N::Editor::Save))
    {
        Save();
    }
    ImGui::EndDisabled();
    if (!canSave && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", live.lookUsable ? I18N::Format(I18N::Editor::LookSaveOff0, {*live.saveBlocked}).c_str()
                                                : I18N::Editor::LookFileUnusable);
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Revert))
    {
        Revert();
    }
    if (ImGui::IsItemHovered())
    {
        const std::string& source = Render::Look::LiveSource();
        const std::string file = source == "shipped" ? std::string(Data::Maps::LookProfileFile)
                                                     : "Data/Render/Candidates/" + source + "/Look.json";
        ImGui::SetTooltip("%s", I18N::Format(I18N::Editor::Reads0AgainUnsavedChangesAreLost, {file}).c_str());
    }
    ImGui::SameLine();
    ImGui::Text("%s: %s", I18N::Editor::LookLoadedFrom, Render::Look::LiveSource().c_str());
    if (live.lookData != Render::Look::LoadedFiles().lookData)
    {
        ImGui::SameLine();
        ImGui::TextUnformatted(I18N::Editor::UnsavedChanges);
    }
    RenderPreview();
}

void CMuGraphicsLookUI::RenderPreview()
{
    using Render::Look::LookPreview;
    const bool authoring = Render::Look::ActivePreview() == LookPreview::Authoring;
    ImGui::Text("%s:", I18N::Editor::LookPreview);
    ImGui::SameLine();
    if (ImGui::RadioButton(I18N::Editor::LookPreviewAuthoring, authoring) && !authoring)
    {
        Render::Look::SetLivePreview(LookPreview::Authoring);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::LookPreviewAuthoringHelp);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton(I18N::Editor::LookPreviewPlayer, !authoring) && authoring)
    {
        Render::Look::SetLivePreview(LookPreview::Player);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::LookPreviewPlayerHelp);
    }
    const int world = gMapManager.WorldActive;
    if (world >= 0)
    {
        const std::string name = MapName(world);
        ImGui::SameLine();
        ImGui::TextDisabled(
            "%s", I18N::Format(I18N::Editor::LookPreviewing0,
                               {(name.empty() ? std::string() : name + " ") + "[" + std::to_string(world) + "]"})
                      .c_str());
    }
}

void CMuGraphicsLookUI::RenderStatus()
{
    if (!Render::Look::LiveFiles().lookUsable)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.3f, 1.0f), "%s", I18N::Editor::LookFileUnusable);
    }
    if (!m_status.empty())
    {
        if (m_statusIsError)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.45f, 0.45f, 1.0f));
        }
        ImGui::TextWrapped("%s", m_status.c_str());
        if (m_statusIsError)
        {
            ImGui::PopStyleColor();
        }
    }
}

void CMuGraphicsLookUI::RenderScopes()
{
    if (ImGui::Selectable(I18N::Editor::LookScopeDefault, m_scope.kind == LookScope::Kind::Default))
    {
        SelectScope(LookScope::Default());
    }
    if (ImGui::Button(I18N::Editor::LookScopeCurrentMap) && gMapManager.WorldActive >= 0)
    {
        SelectScope(LookScope::Map(gMapManager.WorldActive));
    }

    ImGui::SeparatorText(I18N::Editor::LookEnvironments);
    for (std::size_t index = 0; index < m_seen.environments.size(); ++index)
    {
        const std::size_t users = Data::Maps::MapsUsingEnvironment(m_seen, index).size();
        const std::string label = m_seen.environments[index].name + "  (" +
                                  I18N::Format(I18N::Editor::LookUsedBy0Maps, {std::to_string(users)}) + ")##Env" +
                                  std::to_string(index);
        if (ImGui::Selectable(label.c_str(), m_scope == LookScope::Environment(index)))
        {
            SelectScope(LookScope::Environment(index));
        }
    }
    ImGui::SetNextItemWidth(-ImGui::CalcTextSize(I18N::Editor::Add).x - ImGui::GetStyle().FramePadding.x * 4.0f);
    InputName("##NewEnvironment", I18N::Editor::LookNewEnvironment, m_newEnvironment);
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Add))
    {
        LookProfileData edited = m_seen;
        const std::string name = m_newEnvironment;
        const Data::Maps::LookEditOutcome outcome = Data::Maps::AddEnvironment(edited, name);
        Change(std::string(I18N::Editor::Add) + " " + name, outcome, edited);
        if (outcome.applied && m_seen == edited)
        {
            SelectScope(LookScope::Environment(m_seen.environments.size() - 1));
            m_newEnvironment.clear();
        }
    }

    ImGui::SeparatorText(I18N::Editor::LookMapEntries);
    for (const Data::Maps::MapLookOverrides& entry : m_seen.maps)
    {
        if (entry.maps.empty())
        {
            continue;
        }
        const std::string label = (entry.name.empty() ? std::string() : entry.name + "  ") + "[" +
                                  JoinMaps(entry.maps) + "]##Entry" + std::to_string(entry.maps.front());
        const bool selected = m_scope.kind == LookScope::Kind::Map && entry.Has(m_scope.world);
        if (ImGui::Selectable(label.c_str(), selected))
        {
            SelectScope(LookScope::Map(entry.maps.front()));
        }
    }
}

void CMuGraphicsLookUI::RenderScope()
{
    ImGui::SeparatorText(ScopeName().c_str());
    const bool editable = RenderSharedWarning(ScopeMaps());
    if (m_scope.kind == LookScope::Kind::Environment)
    {
        RenderEnvironment(m_scope.environment);
    }
    ImGui::BeginDisabled(!editable || !Render::Look::LiveFiles().lookUsable);
    if (m_scope.kind == LookScope::Kind::Map)
    {
        RenderEntry(m_scope.world);
    }
    RenderValues();
    ImGui::EndDisabled();
}

void CMuGraphicsLookUI::RenderEnvironment(std::size_t environment)
{
    // A copy: a change below replaces the profile.
    const std::string name = m_seen.environments[environment].name;
    if (m_nameShownFor != environment)
    {
        m_environmentName = name;
        m_nameShownFor = environment;
    }
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * SliderWidthInFonts);
    InputName("##EnvironmentName", nullptr, m_environmentName);
    ImGui::SameLine();
    const std::string newName = m_environmentName;
    ImGui::BeginDisabled(newName == name);
    if (ImGui::Button(I18N::Editor::Rename))
    {
        LookProfileData edited = m_seen;
        const Data::Maps::LookEditOutcome outcome = Data::Maps::RenameEnvironment(edited, environment, newName);
        Change(std::string(I18N::Editor::Rename) + " " + name + " -> " + newName, outcome, edited);
        m_nameShownFor.reset();
    }
    ImGui::EndDisabled();

    const std::vector<int> users = Data::Maps::MapsUsingEnvironment(m_seen, environment);
    ImGui::SameLine();
    ImGui::BeginDisabled(!users.empty());
    if (ImGui::Button(I18N::Editor::Remove))
    {
        LookProfileData edited = m_seen;
        const std::string label = std::string(I18N::Editor::Remove) + " " + name;
        const Data::Maps::LookEditOutcome outcome = Data::Maps::RemoveEnvironment(edited, environment);
        Change(label, outcome, edited);
        // The environments after it moved down: none of them takes its place in the selection.
        if (m_seen == edited)
        {
            SelectScope(LookScope::Default());
        }
    }
    ImGui::EndDisabled();
    if (!users.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::LookRemoveOffWhileUsed);
    }
    if (users.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::LookNoMapUsesIt);
    }
}

void CMuGraphicsLookUI::RenderEntry(int world)
{
    const Data::Maps::MapLookOverrides* entry = m_seen.FindMap(world);
    if (!entry)
    {
        ImGui::TextWrapped(
            "%s",
            I18N::Format(I18N::Editor::LookMap0HasNoEntry, {std::to_string(world) + " " + MapName(world)}).c_str());
    }
    const std::optional<std::size_t> current = entry ? entry->environment : std::nullopt;
    const char* preview = current && *current < m_seen.environments.size() ? m_seen.environments[*current].name.c_str()
                                                                           : I18N::Editor::None;
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * SliderWidthInFonts);
    if (!ImGui::BeginCombo(I18N::Editor::LookEnvironment, preview))
    {
        return;
    }
    std::optional<std::optional<std::size_t>> picked;
    if (ImGui::Selectable(I18N::Editor::None, !current))
    {
        picked = std::optional<std::size_t>();
    }
    for (std::size_t index = 0; index < m_seen.environments.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        if (ImGui::Selectable(m_seen.environments[index].name.c_str(), current == index))
        {
            picked = index;
        }
        ImGui::PopID();
    }
    ImGui::EndCombo();
    if (picked && *picked != current)
    {
        LookProfileData edited = m_seen;
        const Data::Maps::LookEditOutcome outcome =
            Data::Maps::SetMapEnvironment(edited, world, *picked, MapName(world));
        const std::string name = *picked ? m_seen.environments[**picked].name : std::string(I18N::Editor::None);
        Change(ScopeName() + ": " + I18N::Editor::LookEnvironment + " " + name, outcome, edited);
    }
}

bool CMuGraphicsLookUI::RenderSharedWarning(const std::vector<int>& maps)
{
    if (maps.size() <= 1)
    {
        return true;
    }
    const std::string list = I18N::Format(I18N::Editor::LookMaps0, {JoinMaps(maps)});
    if (m_confirmed.contains(maps))
    {
        ImGui::TextDisabled("%s", list.c_str());
        return true;
    }
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "%s",
                       I18N::Format(I18N::Editor::LookChangeReaches0Maps, {std::to_string(maps.size())}).c_str());
    ImGui::TextWrapped("%s", list.c_str());
    if (ImGui::Button(I18N::Editor::LookEditForAll))
    {
        m_confirmed.insert(maps);
        return true;
    }
    return false;
}

void CMuGraphicsLookUI::RenderValues()
{
    // A copy: a row's change replaces m_seen while the rows below still read it.
    const LookOverrides* found = OwnValues(m_seen, m_scope);
    const std::optional<LookOverrides> own = found ? std::optional<LookOverrides>(*found) : std::nullopt;
    const Data::Maps::LookValues values = ValuesOf(m_seen, m_scope);
    const std::string mapName = m_scope.kind == LookScope::Kind::Map ? MapName(m_scope.world) : std::string();
    const auto& infos = Data::Maps::LookValueInfos();
    static const std::vector<std::vector<std::size_t>> groups = ValueGroups();
    for (const std::vector<std::size_t>& group : groups)
    {
        const std::string_view first = infos[group.front()].name;
        ImGui::SeparatorText(std::string(first.substr(0, first.find('.'))).c_str());
        for (const std::size_t index : group)
        {
            RenderValue(index, own ? &*own : nullptr, values, mapName);
        }
    }
}

void CMuGraphicsLookUI::RenderValue(std::size_t index, const LookOverrides* own, const Data::Maps::LookValues& values,
                                    const std::string& mapName)
{
    const Data::Maps::LookValueInfo& info = Data::Maps::LookValueInfos()[index];
    ImGui::PushID(static_cast<int>(index));
    bool setHere = own && (*own)[info.value].has_value();
    if (ImGui::Checkbox("##SetHere", &setHere))
    {
        LookProfileData edited = m_seen;
        if (setHere)
        {
            const Data::Maps::LookValueEdit edit{info.value, values[info.value]};
            const Data::Maps::LookEditOutcome outcome =
                Data::Maps::ApplyLookEdits(edited, {&edit, 1}, m_scope, mapName);
            Change(ScopeName() + ": " + info.name + " " + FormatNumber(values[info.value]), outcome, edited);
        }
        else
        {
            const Data::Maps::LookEditOutcome outcome = Data::Maps::ClearLookValue(edited, info.value, m_scope);
            Change(ScopeName() + ": " + info.name + " -", outcome, edited);
        }
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::LookSetHere);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!setHere);
    float number = m_draggedValue == index ? m_draggedNumber : static_cast<float>(values[info.value]);
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * SliderWidthInFonts);
    if (ImGui::SliderFloat(info.name, &number, static_cast<float>(info.min), static_cast<float>(info.max), "%.3f",
                           ImGuiSliderFlags_AlwaysClamp))
    {
        m_draggedValue = index;
        m_draggedNumber = number;
        m_dragId = ImGui::GetItemID();
        PreviewDrag(info.value, mapName);
    }
    // One undo step per drag.
    if (ImGui::IsItemDeactivated())
    {
        if (ImGui::IsItemDeactivatedAfterEdit() && m_draggedValue == index)
        {
            EndDrag(info.value, mapName);
        }
        else
        {
            CancelDrag();
        }
    }
    ImGui::EndDisabled();
    if (m_draggedValue == index && AppliesOnRelease(info.value))
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", I18N::Editor::LookAppliesOnRelease);
    }
    if (!setHere)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", InheritedFrom(m_seen, m_scope, info.value).c_str());
    }
    ImGui::PopID();
}

void CMuGraphicsLookUI::PreviewDrag(LookValue value, const std::string& mapName)
{
    if (!m_dragBase)
    {
        m_dragBase = m_seen;
    }
    if (AppliesOnRelease(value))
    {
        return;
    }
    LookProfileData edited = *m_dragBase;
    const Data::Maps::LookValueEdit edit{value, Rounded(m_draggedNumber)};
    if (Data::Maps::ApplyLookEdits(edited, {&edit, 1}, m_scope, mapName).applied &&
        Render::Look::ReplaceLive(edited).applied)
    {
        // Followed here, so FollowLive doesn't take the drag for a `$look` change.
        m_seen = edited;
    }
}

void CMuGraphicsLookUI::EndDrag(LookValue value, const std::string& mapName)
{
    // The step starts where the drag did.
    if (m_dragBase)
    {
        m_seen = *m_dragBase;
    }
    DropDrag();
    LookProfileData edited = m_seen;
    const Data::Maps::LookValueEdit edit{value, Rounded(m_draggedNumber)};
    const Data::Maps::LookEditOutcome outcome = Data::Maps::ApplyLookEdits(edited, {&edit, 1}, m_scope, mapName);
    Change(ScopeName() + ": " + Data::Maps::Info(value).name + " " + FormatNumber(edit.number), outcome, edited);
    // A refused end leaves the live profile as the drag showed it: put back the one before.
    if (Render::Look::LiveFiles().lookData != m_seen)
    {
        (void)Render::Look::ReplaceLive(m_seen);
    }
}

void CMuGraphicsLookUI::CancelDrag()
{
    if (m_dragBase)
    {
        m_seen = *m_dragBase;
        if (Render::Look::LiveFiles().lookData != m_seen)
        {
            (void)Render::Look::ReplaceLive(m_seen);
        }
    }
    DropDrag();
}

void CMuGraphicsLookUI::DropDrag()
{
    m_dragBase.reset();
    m_draggedValue.reset();
}

void CMuGraphicsLookUI::Change(const std::string& label, const Data::Maps::LookEditOutcome& outcome,
                               const LookProfileData& edited)
{
    if (!outcome.applied)
    {
        m_status = outcome.error;
        m_statusIsError = true;
        return;
    }
    if (edited == m_seen)
    {
        return;
    }
    const Render::Look::LookEditResult result = Render::Look::ReplaceLive(edited);
    if (!result.applied)
    {
        m_status = result.error;
        m_statusIsError = true;
        return;
    }
    m_history.Record(label, m_seen, edited);
    m_seen = edited;
    m_status.clear();
    m_statusIsError = false;
}

std::vector<int> CMuGraphicsLookUI::ScopeMaps() const
{
    switch (m_scope.kind)
    {
    case LookScope::Kind::Environment:
        return Data::Maps::MapsUsingEnvironment(m_seen, m_scope.environment);
    case LookScope::Kind::Map:
        if (const Data::Maps::MapLookOverrides* entry = m_seen.FindMap(m_scope.world))
        {
            return entry->maps;
        }
        return {m_scope.world};
    case LookScope::Kind::Default:
        break;
    }
    return {};
}

std::string CMuGraphicsLookUI::ScopeName() const
{
    switch (m_scope.kind)
    {
    case LookScope::Kind::Environment:
        return m_scope.environment < m_seen.environments.size() ? m_seen.environments[m_scope.environment].name
                                                                : std::string();
    case LookScope::Kind::Map:
    {
        const Data::Maps::MapLookOverrides* entry = m_seen.FindMap(m_scope.world);
        if (entry && !entry->name.empty())
        {
            return entry->name;
        }
        const std::string name = MapName(m_scope.world);
        return name.empty() ? std::to_string(m_scope.world) : name;
    }
    case LookScope::Kind::Default:
        break;
    }
    return I18N::Editor::LookScopeDefault;
}

void CMuGraphicsLookUI::SelectScope(const LookScope& scope)
{
    // A slider of the same value in the new scope has the same ID: its end must not apply the old edit.
    CancelDrag();
    m_scope = scope;
    m_nameShownFor.reset();
}

void CMuGraphicsLookUI::KeepScopeValid()
{
    if (m_scope.kind == LookScope::Kind::Environment && m_scope.environment >= m_seen.environments.size())
    {
        SelectScope(LookScope::Default());
    }
}

void CMuGraphicsLookUI::Save()
{
    if (Render::Look::SaveLive())
    {
        m_status = I18N::Format(I18N::Editor::SavedTo0, {Data::Maps::LookProfileFile});
        m_statusIsError = false;
        return;
    }
    m_status = I18N::Editor::LookNotSaved;
    m_statusIsError = true;
}

void CMuGraphicsLookUI::Revert()
{
    const std::string source = Render::Look::LiveSource();
    if (!Render::Look::LoadLive(source == "shipped" ? std::string_view() : std::string_view(source)))
    {
        m_status = I18N::Editor::LookNotReadAgain;
        m_statusIsError = true;
        return;
    }
    m_status.clear();
    m_statusIsError = false;
}

bool CMuGraphicsLookUI::IsOpen() const
{
    return g_MuEditorCore.GetToolWindows().graphicsLook;
}

bool CMuGraphicsLookUI::CanUndo() const
{
    return m_history.CanUndo();
}

bool CMuGraphicsLookUI::CanRedo() const
{
    return m_history.CanRedo();
}

std::string CMuGraphicsLookUI::GetUndoLabel() const
{
    return m_history.GetUndoLabel();
}

std::string CMuGraphicsLookUI::GetRedoLabel() const
{
    return m_history.GetRedoLabel();
}

void CMuGraphicsLookUI::Undo()
{
    // The bar and Data Checks' keys undo while the window is hidden: first what changed meanwhile.
    FollowLive();
    if (m_history.Undo(ApplyLive))
    {
        AfterHistoryStep();
    }
}

void CMuGraphicsLookUI::Redo()
{
    FollowLive();
    if (m_history.Redo(ApplyLive))
    {
        AfterHistoryStep();
    }
}

void CMuGraphicsLookUI::AfterHistoryStep()
{
    m_seen = Render::Look::LiveFiles().lookData;
    DropDrag();
    m_nameShownFor.reset();
    KeepScopeValid();
}

void CMuGraphicsLookUI::HandleUndoKeys()
{
    // Only while the keyboard is the editor's; a text field being edited takes the keys first.
    if (!g_MuEditorCore.IsKeyboardWithEditor())
    {
        return;
    }
    if (CanUndo() && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z))
    {
        Undo();
    }
    if (CanRedo() &&
        (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y) || ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z)))
    {
        Redo();
    }
}

#endif // _EDITOR
