#include "stdafx.h"

#ifdef _EDITOR

#include "MuSoundMixerUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorToolWindows.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Audio/DSPlaySound.h"
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
const ImVec4 MutedColor(0.66f, 0.29f, 0.23f, 1.0f);
const ImVec4 SoloColor(0.23f, 0.48f, 0.29f, 1.0f);

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

std::string AgoText(double seconds)
{
    char text[32];
    std::snprintf(text, sizeof(text), "%.1f s", seconds < 0.0 ? 0.0 : seconds);
    return text;
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
    ImGui::SeparatorText(I18N::Editor::SoundMixerHeardJustNow);
    const double now = Audio::Mixer::Now();
    const std::vector<Audio::Mixer::HeardSound> heard = Audio::Mixer::Heard(now);
    if (heard.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::SoundMixerNothingHeard);
        return;
    }
    if (!ImGui::BeginTable("##Heard", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
    {
        return;
    }
    for (const Audio::Mixer::HeardSound& sound : heard)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextDisabled("%s", AgoText(now - sound.lastHeard).c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(SoundText(sound.number).c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(SourceText(sound).c_str());
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
