#include "stdafx.h"

#ifdef _EDITOR

#include "ServerListOfflinePreviewUI.h"

#include "../MuEditor/Config/MuEditorConfig.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "Data/GameData/ServerListData/ServerListData.h"
#include "I18N/All.h"
#include "Network/Server/ServerListManager.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <optional>

namespace
{
// Pixel widths before the editor UI scale.
constexpr float SwitchLoadWidth = 140.0f;
constexpr float ChannelLoadWidth = 90.0f;
// "×" (UTF-8): drops a channel's own load.
constexpr const char* DropOwnLoadLabel = "\xC3\x97##DropOwnLoad";
constexpr const char* LoadFormat = "%d %%";

// The load slider a click last started an edit on, when, and its value before that click: a double-click's first
// click already moved the slider.
struct EditStart
{
    ImGuiID slider = 0;
    double time = 0.0;
    int percent = 0;
};
EditStart s_editStart;

// Opens the slider's text field in the next frame, as Ctrl+click does.
void OpenTextField(ImGuiID id)
{
    ImGui::ActivateItemByID(id);
    ImGui::GetCurrentContext()->NavNextActivateFlags = ImGuiActivateFlags_PreferInput;
}

// Whether the click that just activated the slider is the second of a double-click whose first click started an
// edit on it too: not on its label, which takes no click, nor on an item beside it.
bool IsDoubleClickOnSlider()
{
    return ImGui::IsItemActivated() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) &&
           s_editStart.slider == ImGui::GetItemID() &&
           ImGui::GetTime() - s_editStart.time <= ImGui::GetIO().MouseDoubleClickTime;
}

// A load slider from 0 to the preview's highest load that also takes a typed value: a double-click or Ctrl+click
// turns it into a text field. `greyed`: the value is one it only follows. True when the value changed.
bool LoadSlider(const char* label, int& percent, float width, bool greyed)
{
    const int before = percent;
    ImGui::SetNextItemWidth(width * g_MuEditorCore.GetUIScale());
    if (greyed)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }
    ImGui::SliderInt(label, &percent, 0, CServerListManager::MaxOfflineServerPercent, LoadFormat,
                     ImGuiSliderFlags_AlwaysClamp);
    if (greyed)
    {
        ImGui::PopStyleColor();
    }
    if (IsDoubleClickOnSlider())
    {
        percent = s_editStart.percent;
        OpenTextField(ImGui::GetItemID());
    }
    else if (ImGui::IsItemActivated())
    {
        s_editStart = {ImGui::GetItemID(), ImGui::GetTime(), before};
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
    {
        ImGui::SetTooltip("%s", I18N::Editor::LoadSliderNote);
    }
    return percent != before;
}

// MuEditor.ini keeps the switch and the load once they stop changing, not at each step of the slider.
void KeepSwitch(bool show, int percent)
{
    if (ImGui::IsItemActive() ||
        (show == g_MuEditorConfig.GetShowOfflineServers() && percent == g_MuEditorConfig.GetOfflineServerLoad()))
    {
        return;
    }
    g_MuEditorConfig.SetOfflineServers(show, percent);
    g_MuEditorConfig.Save();
}

bool RenderOwnLoad(int serverId)
{
    const std::optional<int> ownLoad = g_ServerListManager->GetOfflineServerLoad(serverId);
    int percent = ownLoad.value_or(g_ServerListManager->GetOfflineServerPercent());
    bool changed = false;
    if (LoadSlider("##OfflineLoad", percent, ChannelLoadWidth, !ownLoad.has_value()))
    {
        g_ServerListManager->SetOfflineServerLoad(serverId, percent);
        changed = true;
    }
    if (!ownLoad.has_value())
    {
        return changed;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(DropOwnLoadLabel))
    {
        g_ServerListManager->SetOfflineServerLoad(serverId, std::nullopt);
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::UseTheirLoadAgain);
    }
    return changed;
}

void ShowOfflineChannels(int groupId, const std::map<int, int>* sent, bool shown)
{
    for (int channel = 0; channel < Data::ServerList::MaxChannelsPerGroup; ++channel)
    {
        if (sent == nullptr || !sent->contains(channel))
        {
            g_ServerListManager->SetOfflineServerShown(groupId * Data::ServerList::MaxChannelsPerGroup + channel,
                                                       shown);
        }
    }
}
} // namespace

namespace MuEditor::ServerList::OfflinePreview
{
bool RenderSwitch()
{
    bool show = g_ServerListManager->IsShowingOfflineServers();
    int percent = g_ServerListManager->GetOfflineServerPercent();
    bool changed = ImGui::Checkbox(I18N::Editor::ShowOfflineServersInTheGame, &show);
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::OfflineServersNote);
    }
    if (show)
    {
        ImGui::SameLine();
        changed |= LoadSlider(I18N::Editor::TheirLoad, percent, SwitchLoadWidth, false);
    }
    if (changed)
    {
        g_ServerListManager->SetOfflinePreview(show, percent);
    }
    KeepSwitch(show, percent);
    return changed;
}

bool RenderGroupButtons(int groupId, const std::map<int, int>* sent)
{
    if (!g_ServerListManager->IsShowingOfflineServers())
    {
        return false;
    }
    ImGui::TextDisabled("%s", I18N::Editor::OfflineChannels);
    ImGui::SameLine();
    const bool all = ImGui::SmallButton(I18N::Editor::AllOfflineChannels);
    ImGui::SameLine();
    const bool none = ImGui::SmallButton(I18N::Editor::NoOfflineChannel);
    if (!all && !none)
    {
        return false;
    }
    ShowOfflineChannels(groupId, sent, all);
    return true;
}

bool RenderChannel(int serverId)
{
    if (!g_ServerListManager->IsShowingOfflineServers())
    {
        return false;
    }
    bool shown = g_ServerListManager->IsOfflineServerShown(serverId);
    bool changed = false;
    if (ImGui::Checkbox(I18N::Editor::ShowOfflineChannel, &shown))
    {
        g_ServerListManager->SetOfflineServerShown(serverId, shown);
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::OfflineChannelNote);
    }
    if (!shown)
    {
        return changed;
    }
    ImGui::SameLine();
    changed |= RenderOwnLoad(serverId);
    return changed;
}
} // namespace MuEditor::ServerList::OfflinePreview

#endif // _EDITOR
