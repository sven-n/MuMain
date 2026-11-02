#include "stdafx.h"

#ifdef _EDITOR

#include "ServerListOfflinePreviewUI.h"

#include "../MuEditor/Config/MuEditorConfig.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "Data/GameData/ServerListData/ServerListData.h"
#include "I18N/All.h"
#include "Network/Server/ServerListManager.h"
#include "imgui.h"

#include <optional>

namespace
{
using MuEditor::ServerList::OfflinePreview::ChannelLoadLabel;
using MuEditor::ServerList::OfflinePreview::DropOwnLoadLabel;

// Pixel widths before the editor UI scale.
constexpr float SwitchLoadWidth = 140.0f;
constexpr float ChannelLoadWidth = 90.0f;
constexpr const char* LoadFormat = "%d %%";

// The load slider a click last started an edit on, when, and its value before that click (nothing: it followed
// another load). A double-click's first click already moved the slider; an edit that ends at the value it started
// from leaves the slider as it was.
struct EditStart
{
    ImGuiID slider = 0;
    double time = 0.0;
    std::optional<int> load;
};
EditStart s_editStart;

// Opens the text field of the slider just drawn in the next frame, as Ctrl+click does (and Tab to it).
void OpenTextField()
{
    ImGui::SetKeyboardFocusHere(-1);
}

// Whether the click that just activated the slider is the second of a double-click whose first click started an
// edit on it too: not on its label, which takes no click, nor on an item beside it.
bool IsDoubleClickOnSlider()
{
    return ImGui::IsItemActivated() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) &&
           s_editStart.slider == ImGui::GetItemID() &&
           ImGui::GetTime() - s_editStart.time <= ImGui::GetIO().MouseDoubleClickTime;
}

// After a load slider: a double-click on it gives back its value from before the first click and opens its text
// field; an edit that ends at the value it started from leaves a slider that followed `follows` following it again.
void TrackEdit(std::optional<int>& load, const std::optional<int>& before, int follows)
{
    const ImGuiID id = ImGui::GetItemID();
    if (IsDoubleClickOnSlider())
    {
        load = s_editStart.load;
        OpenTextField();
    }
    else if (ImGui::IsItemActivated())
    {
        s_editStart = {id, ImGui::GetTime(), before};
    }
    else if (ImGui::IsItemDeactivated() && s_editStart.slider == id && !s_editStart.load && load == follows)
    {
        load.reset();
    }
}

// A load slider from 0 to the preview's highest load that also takes a typed value: a double-click or Ctrl+click
// turns it into a text field. `load`: its own value, or nothing while it follows `follows` (shown greyed). True when
// `load` changed.
bool LoadSlider(const char* label, std::optional<int>& load, int follows, float width)
{
    const std::optional<int> before = load;
    int percent = load.value_or(follows);
    ImGui::SetNextItemWidth(width * g_MuEditorCore.GetUIScale());
    if (!before)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }
    ImGui::SliderInt(label, &percent, 0, CServerListManager::MaxOfflineServerPercent, LoadFormat,
                     ImGuiSliderFlags_AlwaysClamp);
    if (!before)
    {
        ImGui::PopStyleColor();
    }
    if (percent != before.value_or(follows))
    {
        load = percent;
    }
    TrackEdit(load, before, follows);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
    {
        ImGui::SetTooltip("%s", I18N::Editor::LoadSliderNote);
    }
    return load != before;
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
    std::optional<int> ownLoad = g_ServerListManager->GetOfflineServerLoad(serverId);
    bool changed = false;
    if (LoadSlider(ChannelLoadLabel, ownLoad, g_ServerListManager->GetOfflineServerPercent(), ChannelLoadWidth))
    {
        g_ServerListManager->SetOfflineServerLoad(serverId, ownLoad);
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
            g_ServerListManager->SetOfflineServerShown(Data::ServerList::ChannelServerId(groupId, channel), shown);
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
        // Their load follows no other load: it always has a value of its own.
        std::optional<int> load = percent;
        changed |= LoadSlider(I18N::Editor::TheirLoad, load, percent, SwitchLoadWidth);
        percent = load.value_or(percent);
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
