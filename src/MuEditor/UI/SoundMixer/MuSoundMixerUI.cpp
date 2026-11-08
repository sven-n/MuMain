#include "stdafx.h"

#ifdef _EDITOR

#include "MuSoundMixerUI.h"

#include "MuSoundOverlay.h"

#include "../MuEditor/UI/SoundEditor/SoundEditorRows.h"
#include "../MuEditor/UI/SoundEditor/SoundSelection.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorToolWindows.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Audio/DSPlaySound.h"
#include "Audio/SoundOverlay.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Core/Text/Utf8.h"
#include "I18N/All.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <iterator>
#include <limits>
#include <string_view>
#include <utility>

MuSoundMixerUI g_MuSoundMixerUI;

namespace
{
using Audio::SoundKind;
using Audio::Mixer::Control;

constexpr std::array<SoundKind, Audio::SoundKindCount> Kinds = {
    SoundKind::Music, SoundKind::MapAmbience, SoundKind::MapObject, SoundKind::Monster,
    SoundKind::Npc,   SoundKind::Hero,        SoundKind::Player,    SoundKind::Effect,
    SoundKind::Item,  SoundKind::Creature,    SoundKind::Interface, SoundKind::Other};
constexpr float FullPercent = 100.0f;
constexpr float SliderWidth = 120.0f;
// The heard table: its columns and at least this many rows of height.
constexpr int HeardColumns = 6;
constexpr float HeardMinimumRows = 6.0f;
constexpr int VolumeColumn = 5;
// The filter bar's widths.
constexpr float FilterTextWidth = 170.0f;
constexpr float FilterChoiceWidth = 140.0f;
constexpr float FilterPlaysWidth = 90.0f;
const ImVec4 MutedColor(0.66f, 0.29f, 0.23f, 1.0f);
const ImVec4 SoloColor(0.23f, 0.48f, 0.29f, 1.0f);
// A row whose sound plays.
const ImVec4 PlayingRowColor(0.23f, 0.48f, 0.29f, 0.35f);
// The row of the sound selected here or in the Sound Editor (Q-SE3).
const ImVec4 SelectedRowColor(0.26f, 0.42f, 0.66f, 0.55f);

// A volume slider, Mute (unless a Hear box is its mute) and Solo for one
// control.
void RenderControl(const char* id, Control& control, bool withMute = true)
{
    using namespace MuEditor::SoundMixer;
    ImGui::PushID(id);
    int percent = static_cast<int>(std::lround(control.volume * FullPercent));
    ImGui::SetNextItemWidth(SliderWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::SliderInt("##Volume", &percent, 0, static_cast<int>(FullPercent), "%d%%"))
    {
        SetVolume(control, static_cast<float>(percent) / FullPercent);
    }
    if (withMute)
    {
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

bool IsPlaying(int number)
{
    return g_platformAudio != nullptr && g_platformAudio->IsSoundPlaying(static_cast<ESound>(number));
}

float LengthOf(int number)
{
    return g_platformAudio != nullptr ? g_platformAudio->GetSoundLength(static_cast<ESound>(number)) : 0.0f;
}

std::string Lowered(std::string text)
{
    for (char& c : text)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return text;
}

// How long ago the row last started: playing first, never last.
double AgoOf(const MuEditor::SoundMixer::HeardRow& row, double now)
{
    if (row.playing)
    {
        return -1.0;
    }
    return row.sound.lastStarted < 0.0 ? std::numeric_limits<double>::max() : now - row.sound.lastStarted;
}

// A combo of fixed choices; true when one is picked.
template <typename Choice, size_t Count>
bool RenderChoice(const char* id, Choice& value, const std::array<std::pair<Choice, const char*>, Count>& choices)
{
    const char* shown = choices[0].second;
    for (const auto& [choice, label] : choices)
    {
        if (choice == value)
        {
            shown = label;
        }
    }
    bool picked = false;
    ImGui::SetNextItemWidth(FilterChoiceWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::BeginCombo(id, shown))
    {
        for (const auto& [choice, label] : choices)
        {
            if (ImGui::Selectable(label, choice == value))
            {
                value = choice;
                picked = true;
            }
        }
        ImGui::EndCombo();
    }
    return picked;
}

void Tooltip(const char* text)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", text);
    }
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
        if (ImGui::SmallButton(I18N::Editor::SoundMixerResetAll))
        {
            Audio::Mixer::ResetAll();
        }
        Tooltip(I18N::Editor::SoundMixerResetAllTip);
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("%s", I18N::Editor::SoundMixerNote);
        ImGui::PopTextWrapPos();
        bool overlay = Audio::Overlay::IsOn();
        if (ImGui::Checkbox(I18N::Editor::SoundMixerShowInWorld, &overlay))
        {
            Audio::Overlay::SetOn(overlay);
        }
        Tooltip(I18N::Editor::SoundMixerShowInWorldTip);
        RenderKinds();
        RenderHeard();
    }
    ImGui::End();
}

void MuSoundMixerUI::RenderKinds()
{
    using namespace MuEditor::SoundMixer;
    ImGui::SeparatorText(I18N::Editor::SoundMixerKinds);
    // Each type: Hear (its mute) and Show (the overlay's), its name, volume
    // and Solo (Q-SM14).
    if (!ImGui::BeginTable("##Kinds", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
    {
        return;
    }
    ImGui::TableSetupColumn(I18N::Editor::SoundMixerHear);
    ImGui::TableSetupColumn(I18N::Editor::SoundMixerShow);
    ImGui::TableSetupColumn("##Type", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("##Volume");
    ImGui::TableHeadersRow();
    for (const SoundKind kind : Kinds)
    {
        Control& control = Audio::Mixer::KindControl(kind);
        ImGui::PushID(static_cast<int>(kind));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        RenderHearBox(control);
        ImGui::TableNextColumn();
        RenderShowBox(kind);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(KindName(kind));
        ImGui::TableNextColumn();
        RenderControl(KindName(kind), control, false);
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void MuSoundMixerUI::RenderOverlay()
{
    using namespace MuEditor::SoundMixer;
    std::vector<HeardRow> shown;
    const bool filtered = m_filter.IsActive();
    if (Audio::Overlay::IsOn() && filtered)
    {
        std::vector<HeardRow> rows;
        for (const Audio::Mixer::HeardSound& sound : Audio::Mixer::Heard())
        {
            rows.push_back(MakeRow(sound));
        }
        shown = FilterRows(rows, m_filter);
    }
    const bool hovering = m_hovered.has_value() && m_hoveredFrame == ImGui::GetFrameCount();
    MuEditor::SoundOverlay::Render(shown, filtered, hovering ? &*m_hovered : nullptr);
}

void MuSoundMixerUI::RenderFilterBar()
{
    using namespace MuEditor::SoundMixer;
    const float scale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextItemWidth(FilterTextWidth * scale);
    if (ImGui::InputTextWithHint("##Filter", I18N::Editor::SoundMixerFilterHint, m_filterText.data(),
                                 m_filterText.size()))
    {
        m_filter.text = m_filterText.data();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(FilterChoiceWidth * scale);
    if (ImGui::BeginCombo("##Kind", m_filter.kind ? KindName(*m_filter.kind) : I18N::Editor::SoundMixerAllKinds))
    {
        if (ImGui::Selectable(I18N::Editor::SoundMixerAllKinds, !m_filter.kind))
        {
            m_filter.kind.reset();
        }
        for (const SoundKind kind : Kinds)
        {
            if (ImGui::Selectable(KindName(kind), m_filter.kind == kind))
            {
                m_filter.kind = kind;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    RenderChoice("##Show", m_filter.show,
                 std::array<std::pair<ShowFilter, const char*>, 3>{{
                     {ShowFilter::All, I18N::Editor::SoundMixerShowAll},
                     {ShowFilter::Playing, I18N::Editor::SoundMixerShowPlaying},
                     {ShowFilter::Changed, I18N::Editor::SoundMixerShowChanged},
                 }});
    ImGui::SameLine();
    RenderChoice("##Loops", m_filter.loops,
                 std::array<std::pair<LoopFilter, const char*>, 3>{{
                     {LoopFilter::All, I18N::Editor::SoundMixerLoopsAll},
                     {LoopFilter::Loops, I18N::Editor::SoundMixerLoopsOnly},
                     {LoopFilter::OneShots, I18N::Editor::SoundMixerOneShotsOnly},
                 }});
    ImGui::SameLine();
    ImGui::TextUnformatted(I18N::Editor::SoundMixerMinimumPlays);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(FilterPlaysWidth * scale);
    if (ImGui::InputInt("##MinimumPlays", &m_filter.minimumPlays))
    {
        m_filter.minimumPlays = std::max(m_filter.minimumPlays, 0);
    }
}

void MuSoundMixerUI::RenderHeard()
{
    using namespace MuEditor::SoundMixer;
    ImGui::SeparatorText(I18N::Editor::SoundMixerSoundsHeard);
    RenderFilterBar();
    std::vector<HeardRow> rows;
    for (const Audio::Mixer::HeardSound& sound : Audio::Mixer::Heard())
    {
        rows.push_back(MakeRow(sound));
    }
    std::vector<HeardRow> shown = FilterRows(rows, m_filter);
    if (ImGui::SmallButton(I18N::Editor::SoundMixerClearAll))
    {
        Audio::Mixer::ClearHeard();
    }
    Tooltip(I18N::Editor::SoundMixerClearAllTip);
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::SoundMixerMuteShown))
    {
        SetMuted(shown, true);
    }
    Tooltip(I18N::Editor::SoundMixerMuteShownTip);
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::SoundMixerUnmuteShown))
    {
        SetMuted(shown, false);
    }
    Tooltip(I18N::Editor::SoundMixerUnmuteShownTip);
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::SoundMixerClearFilters))
    {
        m_filter = {};
        m_filterText.fill('\0');
    }
    ImGui::SameLine();
    ImGui::TextDisabled(
        "%s", I18N::Format(I18N::Editor::SoundMixerShown01, {std::to_string(shown.size()), std::to_string(rows.size())})
                  .c_str());
    if (rows.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::SoundMixerNothingHeard);
        return;
    }
    // Rows keep the order of first hearing unless a header sorts them, and
    // the header stays while the rows scroll, so that a sound is easy to find
    // and mute.
    const float height =
        std::max(ImGui::GetContentRegionAvail().y, ImGui::GetFrameHeightWithSpacing() * (HeardMinimumRows + 1.0f));
    constexpr ImGuiTableFlags Flags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortTristate;
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
    ImGui::TableSetupColumn(I18N::Editor::SoundMixerColumnVolume, ImGuiTableColumnFlags_NoSort);
    ImGui::TableHeadersRow();
    if (ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs(); specs != nullptr && specs->SpecsDirty)
    {
        m_sort = {};
        if (specs->SpecsCount > 0 && specs->Specs[0].ColumnIndex < VolumeColumn)
        {
            m_sort.column = static_cast<HeardColumn>(specs->Specs[0].ColumnIndex);
            m_sort.descending = specs->Specs[0].SortDirection == ImGuiSortDirection_Descending;
        }
        specs->SpecsDirty = false;
    }
    const double now = Audio::Mixer::Now();
    SortRows(shown, m_sort, now);
    for (const HeardRow& row : shown)
    {
        ImGui::TableNextRow();
        // Last frame's hovered row: the overlay highlights its sources.
        if (ImGui::TableGetHoveredRow() == ImGui::TableGetRowIndex())
        {
            m_hovered = row;
            m_hoveredFrame = ImGui::GetFrameCount();
        }
        if (row.playing)
        {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(PlayingRowColor));
        }
        if (row.sound.number == MuEditor::SoundSelection::Get())
        {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(SelectedRowColor));
            if (m_seenSelection != MuEditor::SoundSelection::GetVersion())
            {
                ImGui::SetScrollHereY(0.5f);
            }
        }
        ImGui::TableNextColumn();
        MuEditor::SoundMixer::RenderSoundCell(row, m_copied);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.source.c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%d", row.sound.plays);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(LengthText(row.length, row.sound.looped).c_str());
        ImGui::TableNextColumn();
        const std::string last = LastText(row.sound, now, row.playing);
        if (row.playing)
        {
            ImGui::TextUnformatted(last.c_str());
        }
        else
        {
            ImGui::TextDisabled("%s", last.c_str());
        }
        ImGui::TableNextColumn();
        RenderControl(std::to_string(row.sound.number).c_str(), Audio::Mixer::SoundControl(row.sound.number));
    }
    ImGui::EndTable();
    m_seenSelection = MuEditor::SoundSelection::GetVersion();
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
    case SoundKind::Npc:
        return I18N::Editor::SoundKindNpcs;
    case SoundKind::Hero:
        return I18N::Editor::SoundKindHero;
    case SoundKind::Player:
        return I18N::Editor::SoundKindPlayers;
    case SoundKind::Effect:
        return I18N::Editor::SoundKindEffects;
    case SoundKind::Item:
        return I18N::Editor::SoundKindItems;
    case SoundKind::Creature:
        return I18N::Editor::SoundKindCreatures;
    case SoundKind::Interface:
        return I18N::Editor::SoundKindInterface;
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
    else if (sound.kind == SoundKind::MapObject && sound.sourceNumber >= 0)
    {
        text += ": " + I18N::Format(I18N::Editor::SoundMixerObject0, {std::to_string(sound.sourceNumber)});
    }
    else if ((sound.kind == SoundKind::Monster || sound.kind == SoundKind::Npc || sound.kind == SoundKind::Effect ||
              sound.kind == SoundKind::Item || sound.kind == SoundKind::Creature) &&
             sound.sourceNumber >= 0)
    {
        // The monster's or NPC's number, the effect's, item's or creature's type.
        text += ": " + std::to_string(sound.sourceNumber);
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

std::string ShownSoundText(int number)
{
    std::string name = MuEditor::SoundEditor::NameOfNumber(number);
    return name.empty() ? SoundText(number) : name;
}

const std::string& CellText(const HeardRow& row)
{
    return row.name.empty() ? row.file : row.name;
}

std::string SoundCellTip(const HeardRow& row, int copied)
{
    if (copied == row.sound.number)
    {
        return I18N::Format(I18N::Editor::SoundMixerCopied0, {row.file});
    }
    if (Audio::Mixer::IsMusicNumber(row.sound.number))
    {
        return I18N::Editor::SoundMixerCopyMusicTip;
    }
    if (!row.name.empty())
    {
        return I18N::Format(I18N::Editor::SoundMixerCopyNamedTip01, {std::to_string(row.sound.number), row.file});
    }
    return I18N::Format(I18N::Editor::SoundMixerCopyTip0, {std::to_string(row.sound.number)});
}

std::string SecondsText(double seconds)
{
    char text[32];
    std::snprintf(text, sizeof(text), "%.1f s", seconds < 0.0 ? 0.0 : seconds);
    return text;
}

bool HearBoxTicked(const Audio::Mixer::Control& control)
{
    return !control.muted;
}

bool RenderHearBox(Audio::Mixer::Control& control)
{
    bool hear = HearBoxTicked(control);
    const bool clicked = ImGui::Checkbox("##Hear", &hear);
    if (clicked)
    {
        ToggleMute(control);
    }
    Tooltip(I18N::Editor::SoundMixerHearTip);
    return clicked;
}

bool RenderShowBox(Audio::SoundKind kind)
{
    bool shown = Audio::Overlay::IsShown(kind);
    const bool clicked = ImGui::Checkbox("##Show", &shown);
    if (clicked)
    {
        Audio::Overlay::SetShown(kind, shown);
    }
    Tooltip(I18N::Editor::SoundMixerShowTip);
    return clicked;
}

bool RenderSoundCell(const HeardRow& row, int& copied)
{
    // Two numbers can load the same file.
    ImGui::PushID(row.sound.number);
    const bool clicked = ImGui::Selectable(CellText(row).c_str());
    ImGui::PopID();
    if (clicked)
    {
        ImGui::SetClipboardText(row.file.c_str());
        copied = row.sound.number;
        // A music track is no sound of the Sound Editor.
        if (!Audio::Mixer::IsMusicNumber(row.sound.number))
        {
            MuEditor::SoundSelection::Set(row.sound.number);
        }
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", SoundCellTip(row, copied).c_str());
    }
    return clicked;
}

bool HeardFilter::IsActive() const
{
    return !text.empty() || kind.has_value() || show != ShowFilter::All || loops != LoopFilter::All || minimumPlays > 0;
}

HeardRow MakeRow(const Audio::Mixer::HeardSound& sound)
{
    HeardRow row;
    row.sound = sound;
    row.changed = Audio::Mixer::SoundControl(sound.number).IsChanged();
    row.source = SourceText(sound);
    if (Audio::Mixer::IsMusicNumber(sound.number))
    {
        // Q-SM17: the track's file; its length is not known.
        row.file = MuEditor::SoundOverlay::MusicText(Audio::Mixer::MusicOf(sound.number));
        row.playing = Audio::Mixer::IsMusicPlaying(sound.number);
        return row;
    }
    row.file = SoundText(sound.number);
    row.name = MuEditor::SoundEditor::NameOfNumber(sound.number);
    row.length = LengthOf(sound.number);
    row.playing = IsPlaying(sound.number);
    return row;
}

bool Passes(const HeardRow& row, const HeardFilter& filter)
{
    if (!filter.text.empty())
    {
        const std::string text = Lowered(filter.text);
        if (Lowered(row.file).find(text) == std::string::npos && Lowered(row.name).find(text) == std::string::npos &&
            Lowered(row.source).find(text) == std::string::npos)
        {
            return false;
        }
    }
    if (filter.kind && row.sound.kind != *filter.kind)
    {
        return false;
    }
    if ((filter.show == ShowFilter::Playing && !row.playing) || (filter.show == ShowFilter::Changed && !row.changed))
    {
        return false;
    }
    if ((filter.loops == LoopFilter::Loops && !row.sound.looped) ||
        (filter.loops == LoopFilter::OneShots && row.sound.looped))
    {
        return false;
    }
    return row.sound.plays >= filter.minimumPlays;
}

std::vector<HeardRow> FilterRows(const std::vector<HeardRow>& rows, const HeardFilter& filter)
{
    std::vector<HeardRow> shown;
    std::copy_if(rows.begin(), rows.end(), std::back_inserter(shown),
                 [&filter](const HeardRow& row) { return Passes(row, filter); });
    return shown;
}

namespace
{
// By the Sound or Source column's text without case: each row's text lowered
// once, not twice per comparison. Rows alike keep their order.
void SortByText(std::vector<HeardRow>& rows, HeardColumn column, bool descending)
{
    std::vector<std::pair<std::string, size_t>> texts;
    texts.reserve(rows.size());
    for (size_t index = 0; index < rows.size(); ++index)
    {
        texts.emplace_back(Lowered(column == HeardColumn::Sound ? CellText(rows[index]) : rows[index].source), index);
    }
    std::stable_sort(texts.begin(), texts.end(), [descending](const auto& left, const auto& right)
                     { return descending ? right.first < left.first : left.first < right.first; });
    std::vector<HeardRow> sorted;
    sorted.reserve(rows.size());
    for (const auto& text : texts)
    {
        sorted.push_back(std::move(rows[text.second]));
    }
    rows = std::move(sorted);
}
} // namespace

void SortRows(std::vector<HeardRow>& rows, const HeardSort& sort, double now)
{
    if (!sort.column)
    {
        return;
    }
    const HeardColumn column = *sort.column;
    if (column == HeardColumn::Sound || column == HeardColumn::Source)
    {
        SortByText(rows, column, sort.descending);
        return;
    }
    const auto before = [column, now](const HeardRow& left, const HeardRow& right)
    {
        switch (column)
        {
        case HeardColumn::Sound:
        case HeardColumn::Source:
            break;
        case HeardColumn::Plays:
            return left.sound.plays < right.sound.plays;
        case HeardColumn::Length:
            return left.length < right.length;
        case HeardColumn::LastPlayed:
            break;
        }
        return AgoOf(left, now) < AgoOf(right, now);
    };
    std::stable_sort(rows.begin(), rows.end(), [&before, &sort](const HeardRow& left, const HeardRow& right)
                     { return sort.descending ? before(right, left) : before(left, right); });
}

void SetMuted(const std::vector<HeardRow>& rows, bool muted)
{
    for (const HeardRow& row : rows)
    {
        Audio::Mixer::SoundControl(row.sound.number).muted = muted;
    }
    Audio::Mixer::Apply();
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

std::string ReminderText(const Audio::Mixer::Changes& changes, bool overlayOn)
{
    std::string text = ChangesText(changes);
    if (overlayOn)
    {
        text += (text.empty() ? "" : ", ") + std::string(I18N::Editor::SoundMixerOverlayOn);
    }
    return text;
}

void RenderReminder(ToolWindows& windows)
{
    const std::string reminder = ReminderText(Audio::Mixer::CountChanges(), Audio::Overlay::IsOn());
    if (reminder.empty())
    {
        return;
    }
    const std::string label = I18N::Format(I18N::Editor::SoundMixerReminder0, {reminder});
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
