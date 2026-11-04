#pragma once

#ifdef _EDITOR

#include "Audio/SoundMixer.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace MuEditor
{
struct ToolWindows;
}

namespace MuEditor::SoundMixer
{
enum class ShowFilter
{
    All,
    Playing,
    Changed,
};

enum class LoopFilter
{
    All,
    Loops,
    OneShots,
};

// The filter bar above the Sounds heard table (Q-SM7).
struct HeardFilter
{
    // Contained in the sound's file or its source, whatever the case.
    std::string text;
    // Every kind, or one.
    std::optional<Audio::SoundKind> kind;
    ShowFilter show = ShowFilter::All;
    LoopFilter loops = LoopFilter::All;
    int minimumPlays = 0;

    // Anything set.
    bool IsActive() const;
};

// The columns a header click sorts by (Q-SM7).
enum class HeardColumn
{
    Sound,
    Source,
    Plays,
    Length,
    LastPlayed,
};

struct HeardSort
{
    // None: the order of first hearing.
    std::optional<HeardColumn> column;
    bool descending = false;
};

// A row of the table: the sound, its texts and what the backend and its
// control say of it.
struct HeardRow
{
    Audio::Mixer::HeardSound sound;
    std::string file;
    // The name the Sound Editor gave the sound (Q-SE1); "" for none.
    std::string name;
    std::string source;
    float length = 0.0f;
    bool playing = false;
    // Muted, solo or lowered.
    bool changed = false;
};
} // namespace MuEditor::SoundMixer

// The Sound Mixer window (Windows menu, docs/superpowers/specs/2026-11-01-sound-mixer-design.md):
// mute, solo and a volume for each kind of sound and each sound heard, for the
// session; the editor's previews are not touched.
class MuSoundMixerUI
{
public:
    void Render(bool* open);

    // The overlay over the game, once a frame, also while the window is
    // closed (Q-SM10): the sources of the rows the filters show. After Render
    // in the frame: it marks the row the mouse is over in this frame.
    void RenderOverlay();

private:
    void RenderKinds();
    void RenderFilterBar();
    void RenderHeard();

    // The filters and the sort, for the session.
    MuEditor::SoundMixer::HeardFilter m_filter;
    std::array<char, 128> m_filterText{};
    MuEditor::SoundMixer::HeardSort m_sort;
    // The row under the mouse and the frame it was, for the overlay.
    std::optional<MuEditor::SoundMixer::HeardRow> m_hovered;
    int m_hoveredFrame = -1;
    // The sound whose file was copied last, for its cell's tooltip.
    int m_copied = -1;
    // The shared selection's version the table last showed (Q-SE3).
    int m_seenSelection = -1;
};

extern MuSoundMixerUI g_MuSoundMixerUI;

namespace MuEditor::SoundMixer
{
// "2 muted, 1 solo, 1 lowered"; "" for nothing changed.
std::string ChangesText(const Audio::Mixer::Changes& changes);

// The menu bar's reminder: the changes and whether the overlay is on
// ("2 muted, overlay on"); "" for neither.
std::string ReminderText(const Audio::Mixer::Changes& changes, bool overlayOn);

// The type's name in the window.
const char* KindName(Audio::SoundKind kind);

// "1.4 s".
std::string SecondsText(double seconds);

// Whether a type's Hear box is ticked: while the type is not muted.
bool HearBoxTicked(const Audio::Mixer::Control& control);

// A type's Hear box (Q-SM14): HearBoxTicked; a click mutes or unmutes the
// type at once. Whether this frame's click did.
bool RenderHearBox(Audio::Mixer::Control& control);

// A type's Show box (Q-SM14): whether the overlay draws the type. Whether
// this frame's click changed it.
bool RenderShowBox(Audio::SoundKind kind);

// Where a heard sound came from: its kind, and the set and monster or the map
// object type.
std::string SourceText(const Audio::Mixer::HeardSound& sound);

// The file a heard sound plays below Data/Sound, or its number.
std::string SoundText(int number);

// What the mixer and the overlay show for a sound: the name the Sound Editor
// gave it (Q-SE1), else SoundText.
std::string ShownSoundText(int number);

// A row's Sound cell text: its name, else its file.
const std::string& CellText(const HeardRow& row);

// The tooltip of a row's Sound cell: the sound's number and that a click
// copies the file, or, once `copied` is the row's sound, that it was.
std::string SoundCellTip(const HeardRow& row, int copied);

// A row's Sound cell: its name or file, which a click copies (to quote it
// elsewhere) and selects in the Sound Editor too (Q-SE3), and its tooltip.
// `copied`: the sound copied last. Whether this frame's click copied it.
bool RenderSoundCell(const HeardRow& row, int& copied);

// The sound's row: its texts, length, whether it plays and is changed.
HeardRow MakeRow(const Audio::Mixer::HeardSound& sound);

// Whether the row passes every filter set.
bool Passes(const HeardRow& row, const HeardFilter& filter);

// The rows the filter leaves, in their order.
std::vector<HeardRow> FilterRows(const std::vector<HeardRow>& rows, const HeardFilter& filter);

// Sorts by the column, keeping equal rows in their order; no column keeps
// the order. Last played goes from the most recent (playing) to never.
void SortRows(std::vector<HeardRow>& rows, const HeardSort& sort, double now);

// Mute all / Unmute all (Q-SM8): the rows given, then applies once.
void SetMuted(const std::vector<HeardRow>& rows, bool muted);

// A sound's length: "1.4 s", "6.0 s loop"; "" when it is not known (0).
std::string LengthText(float seconds, bool looped);

// When a listed sound last started: "playing" while it plays, "12.0 s ago",
// "" for no start since the list was cleared.
std::string LastText(const Audio::Mixer::HeardSound& sound, double now, bool playing);

// The menu bar's reminder (Q-SM5): while anything is changed, "Sound Mixer:
// …", which opens the window.
void RenderReminder(ToolWindows& windows);

// The mixer's controls as the window's buttons change them: they apply at once.
void ToggleMute(Audio::Mixer::Control& control);
void ToggleSolo(Audio::Mixer::Control& control);
void SetVolume(Audio::Mixer::Control& control, float volume);
} // namespace MuEditor::SoundMixer

#endif // _EDITOR
