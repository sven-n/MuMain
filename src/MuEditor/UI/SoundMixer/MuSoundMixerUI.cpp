#include "stdafx.h"

#ifdef _EDITOR

#include "MuSoundMixerUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorToolWindows.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Audio/DSPlaySound.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Core/Text/Utf8.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <string_view>

MuSoundMixerUI g_MuSoundMixerUI;

namespace
{
using Audio::SoundKind;
using Audio::Mixer::Control;

constexpr std::array<SoundKind, Audio::SoundKindCount> Kinds = {
    SoundKind::Music, SoundKind::MapAmbience, SoundKind::MapObject, SoundKind::Monster, SoundKind::Other};
constexpr float FullPercent = 100.0f;
constexpr float SliderWidth = 120.0f;
// The heard table: its columns and at least this many rows of height.
constexpr int HeardColumns = 6;
constexpr float HeardMinimumRows = 6.0f;
const ImVec4 MutedColor(0.66f, 0.29f, 0.23f, 1.0f);
const ImVec4 SoloColor(0.23f, 0.48f, 0.29f, 1.0f);
// A row whose sound plays.
const ImVec4 PlayingRowColor(0.23f, 0.48f, 0.29f, 0.35f);

// A volume slider, Mute and Solo for one control.
void RenderControl(const char* id, Control& control)
{
    using namespace MuEditor::SoundMixer;
    ImGui::PushID(id);
    int percent = static_cast<int>(std::lround(control.volume * FullPercent));
    ImGui::SetNextItemWidth(SliderWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::SliderInt("##Volume", &percent, 0, static_cast<int>(FullPercent), "%d%%"))
    {
        SetVolume(control, static_cast<float>(percent) / FullPercent);
    }
    ImGui::SameLine();
    if (control.muted)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, MutedColor);
    }
    if (ImGui::SmallButton(control.muted ? I18N::Editor::SoundMuted : I18N::Editor::SoundMute))
    {
        ToggleMute(control);
    }
    if (control.muted)
    {
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    if (control.solo)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, SoloColor);
    }
    if (ImGui::SmallButton(control.solo ? I18N::Editor::SoundSoloOn : I18N::Editor::SoundSolo))
    {
        ToggleSolo(control);
    }
    if (control.solo)
    {
        ImGui::PopStyleColor();
    }
    ImGui::PopID();
}

std::string SecondsText(double seconds)
{
    char text[32];
    std::snprintf(text, sizeof(text), "%.1f s", seconds < 0.0 ? 0.0 : seconds);
    return text;
}

bool IsPlaying(int number)
{
    return g_platformAudio != nullptr && g_platformAudio->IsSoundPlaying(static_cast<ESound>(number));
}

float LengthOf(int number)
{
    return g_platformAudio != nullptr ? g_platformAudio->GetSoundLength(static_cast<ESound>(number)) : 0.0f;
}
} // namespace

void MuSoundMixerUI::Render(bool* open)
{
    using namespace MuEditor;
    char title[WindowIds::TitleCapacity];
    Layout::PrepareToolWindow();
    if (ImGui::Begin(WindowIds::Title(title, I18N::Editor::SoundMixerTitle, WindowIds::SoundMixer), open,
                     ImGuiWindowFlags_NoCollapse))
    {
        const std::string changes = SoundMixer::ChangesText(Audio::Mixer::CountChanges());
        ImGui::TextUnformatted(changes.empty() ? I18N::Editor::SoundMixerNothingChanged : changes.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::SoundMixerUnmuteAll))
        {
            Audio::Mixer::ResetAll();
        }
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("%s", I18N::Editor::SoundMixerNote);
        ImGui::PopTextWrapPos();
        RenderKinds();
        RenderHeard();
    }
    ImGui::End();
}

void MuSoundMixerUI::RenderKinds()
{
    ImGui::SeparatorText(I18N::Editor::SoundMixerKinds);
    if (!ImGui::BeginTable("##Kinds", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
    {
        return;
    }
    for (const SoundKind kind : Kinds)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(MuEditor::SoundMixer::KindName(kind));
        ImGui::TableNextColumn();
        RenderControl(MuEditor::SoundMixer::KindName(kind), Audio::Mixer::KindControl(kind));
    }
    ImGui::EndTable();
}

void MuSoundMixerUI::RenderHeard()
{
    using namespace MuEditor::SoundMixer;
    ImGui::SeparatorText(I18N::Editor::SoundMixerSoundsHeard);
    if (ImGui::SmallButton(I18N::Editor::SoundMixerClearAll))
    {
        Audio::Mixer::ClearHeard();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::SoundMixerClearAllTip);
    }
    const std::vector<Audio::Mixer::HeardSound> heard = Audio::Mixer::Heard();
    if (heard.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::SoundMixerNothingHeard);
        return;
    }
    // Rows keep their place (the order of first hearing) and the header stays
    // while the rows scroll, so that a sound is easy to find and mute.
    const float height =
        std::max(ImGui::GetContentRegionAvail().y, ImGui::GetFrameHeightWithSpacing() * (HeardMinimumRows + 1.0f));
    constexpr ImGuiTableFlags Flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY |
                                      ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Heard", HeardColumns, Flags, ImVec2(0.0f, height)))
    {
        return;
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn(I18N::Editor::SoundMixerColumnSound, ImGuiTableColumnFlags_WidthStretch, 1.0f);
    ImGui::TableSetupColumn(I18N::Editor::SoundMixerColumnSource, ImGuiTableColumnFlags_WidthStretch, 1.0f);
    ImGui::TableSetupColumn(I18N::Editor::SoundMixerColumnPlays);
    ImGui::TableSetupColumn(I18N::Editor::SoundMixerColumnLength);
    ImGui::TableSetupColumn(I18N::Editor::SoundMixerColumnLast);
    ImGui::TableSetupColumn(I18N::Editor::SoundMixerColumnVolume);
    ImGui::TableHeadersRow();
    const double now = Audio::Mixer::Now();
    for (const Audio::Mixer::HeardSound& sound : heard)
    {
        const bool playing = IsPlaying(sound.number);
        ImGui::TableNextRow();
        if (playing)
        {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(PlayingRowColor));
        }
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(SoundText(sound.number).c_str());
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("#%d", sound.number);
        }
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(SourceText(sound).c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%d", sound.plays);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(LengthText(LengthOf(sound.number), sound.looped).c_str());
        ImGui::TableNextColumn();
        const std::string last = LastText(sound, now, playing);
        if (playing)
        {
            ImGui::TextUnformatted(last.c_str());
        }
        else
        {
            ImGui::TextDisabled("%s", last.c_str());
        }
        ImGui::TableNextColumn();
        RenderControl(std::to_string(sound.number).c_str(), Audio::Mixer::SoundControl(sound.number));
    }
    ImGui::EndTable();
}

namespace MuEditor::SoundMixer
{
std::string ChangesText(const Audio::Mixer::Changes& changes)
{
    std::string text;
    const auto add = [&text](int count, const char* format)
    {
        if (count > 0)
        {
            text += (text.empty() ? "" : ", ") + I18N::Format(format, {std::to_string(count)});
        }
    };
    add(changes.muted, I18N::Editor::SoundMixerMuted0);
    add(changes.solo, I18N::Editor::SoundMixerSolo0);
    add(changes.lowered, I18N::Editor::SoundMixerLowered0);
    return text;
}

const char* KindName(Audio::SoundKind kind)
{
    switch (kind)
    {
    case SoundKind::Music:
        return I18N::Editor::SoundKindMusic;
    case SoundKind::MapAmbience:
        return I18N::Editor::SoundKindMapAmbience;
    case SoundKind::MapObject:
        return I18N::Editor::SoundKindMapObjects;
    case SoundKind::Monster:
        return I18N::Editor::SoundKindMonsters;
    case SoundKind::Other:
        break;
    }
    return I18N::Editor::SoundKindEverythingElse;
}

std::string SourceText(const Audio::Mixer::HeardSound& sound)
{
    std::string text = KindName(sound.kind);
    if (sound.kind == SoundKind::Monster && !sound.name.empty())
    {
        text += ": " + sound.name;
        if (sound.sourceNumber >= 0)
        {
            text += " (" + std::to_string(sound.sourceNumber) + ")";
        }
    }
    else if (sound.kind == SoundKind::Monster && sound.sourceNumber >= 0)
    {
        text += ": " + std::to_string(sound.sourceNumber);
    }
    else if (sound.kind == SoundKind::MapObject && sound.sourceNumber >= 0)
    {
        text += ": " + I18N::Format(I18N::Editor::SoundMixerObject0, {std::to_string(sound.sourceNumber)});
    }
    return text;
}

std::string SoundText(int number)
{
    std::string file = Core::Text::ToUtf8(GetLoadedSoundFile(static_cast<ESound>(number)).c_str());
    if (file.empty())
    {
        return "#" + std::to_string(number);
    }
    for (char& c : file)
    {
        if (c == '\\')
        {
            c = '/';
        }
    }
    constexpr std::string_view SoundFolder = "data/sound/";
    if (file.size() > SoundFolder.size())
    {
        std::string start = file.substr(0, SoundFolder.size());
        for (char& c : start)
        {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (start == SoundFolder)
        {
            file.erase(0, SoundFolder.size());
        }
    }
    return file;
}

std::string LengthText(float seconds, bool looped)
{
    if (seconds <= 0.0f)
    {
        return {};
    }
    const std::string length = SecondsText(seconds);
    return looped ? I18N::Format(I18N::Editor::SoundMixerLoop0, {length}) : length;
}

std::string LastText(const Audio::Mixer::HeardSound& sound, double now, bool playing)
{
    if (playing)
    {
        return I18N::Editor::SoundMixerPlaying;
    }
    if (sound.lastStarted < 0.0)
    {
        return {};
    }
    return I18N::Format(I18N::Editor::SoundMixerAgo0, {SecondsText(now - sound.lastStarted)});
}

void RenderReminder(ToolWindows& windows)
{
    const std::string changes = ChangesText(Audio::Mixer::CountChanges());
    if (changes.empty())
    {
        return;
    }
    const std::string label = I18N::Format(I18N::Editor::SoundMixerReminder0, {changes});
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.78f, 0.45f, 1.0f));
    if (ImGui::MenuItem(label.c_str()))
    {
        windows.soundMixer = true;
    }
    ImGui::PopStyleColor();
}

void ToggleMute(Audio::Mixer::Control& control)
{
    control.muted = !control.muted;
    Audio::Mixer::Apply();
}

void ToggleSolo(Audio::Mixer::Control& control)
{
    control.solo = !control.solo;
    Audio::Mixer::Apply();
}

void SetVolume(Audio::Mixer::Control& control, float volume)
{
    control.volume = std::clamp(volume, 0.0f, 1.0f);
    Audio::Mixer::Apply();
}
} // namespace MuEditor::SoundMixer

#endif // _EDITOR
