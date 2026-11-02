#include "stdafx.h"

#ifdef _EDITOR

#include "MuSoundOverlay.h"

#include "Audio/SoundMixer.h"
#include "Camera/CameraProjection.h"
#include "Camera/CameraState.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iterator>
#include <numbers>
#include <string>
#include <string_view>

namespace
{
using Audio::Overlay::DrawnSource;
using MuEditor::SoundMixer::HeardRow;

// A ring is drawn of pieces about this long (a dash and a gap of a dashed
// one), at least and at most so many, a little above the ground.
constexpr float RingPieceLength = 40.0f;
constexpr int MinRingPieces = 48;
constexpr int MaxRingPieces = 480;
constexpr float GroundLift = 5.0f;
// A start flashes for a second: a ring that grows and fades.
constexpr double FlashSeconds = 1.0;
constexpr float FlashStartRadius = 40.0f;
constexpr float FlashEndRadius = 200.0f;
constexpr float MarkerSize = 5.0f;
constexpr float HighlightGrowth = 1.6f;
constexpr float LineThickness = 1.5f;
constexpr float HighlightThickness = 3.0f;
constexpr float LabelGap = 2.0f;
constexpr float FullAlpha = 255.0f;
const ImU32 MapObjectColor = IM_COL32(110, 180, 240, 230);
// A map object's quietest ring, fainter: many overlap where many objects play.
const ImU32 QuietestRingColor = IM_COL32(110, 180, 240, 110);
const ImU32 HeardColor = IM_COL32(120, 215, 120, 230);
const ImU32 FarColor = IM_COL32(190, 190, 190, 200);
const ImU32 HighlightColor = IM_COL32(255, 210, 90, 255);
// A place of the game's own code's sounds: hollow, as it has no range.
const ImU32 CodeColor = IM_COL32(210, 165, 255, 235);
const ImU32 LabelBackColor = IM_COL32(0, 0, 0, 150);
const ImU32 LabelColor = IM_COL32(255, 255, 255, 255);
// The log sits so far from the game's top left, below the editor's menu bar.
constexpr float LogMargin = 8.0f;

// World positions to ImGui's coordinates: the kept camera's window pixels in
// the game window's viewport.
class Projector
{
public:
    explicit Projector(const CameraState& camera)
        : m_camera(camera), m_origin(ImGui::GetMainViewport()->Pos), m_size(ImGui::GetMainViewport()->Size)
    {
    }

    // False behind the camera.
    bool Project(float x, float y, float z, ImVec2& point) const
    {
        vec3_t world{x, y, z};
        float windowX = 0.0f;
        float windowY = 0.0f;
        if (!CameraProjection::WorldToWindowPixel(m_camera, world, &windowX, &windowY))
        {
            return false;
        }
        point = MuEditor::SoundOverlay::ToDisplay(windowX, windowY, m_origin, m_size, WindowWidth, WindowHeight);
        return true;
    }

private:
    const CameraState& m_camera;
    ImVec2 m_origin;
    ImVec2 m_size;
};

// A ring on the ground around (x, y); a dashed one draws every other piece.
void DrawRing(ImDrawList& list, const Projector& projector, float centerX, float centerY, float radius, ImU32 color,
              float thickness, bool dashed)
{
    const float circumference = 2.0f * std::numbers::pi_v<float> * radius;
    const int pieces = std::clamp(static_cast<int>(circumference / RingPieceLength), MinRingPieces, MaxRingPieces);
    ImVec2 previous;
    bool hasPrevious = false;
    for (int i = 0; i <= pieces; ++i)
    {
        const float angle = 2.0f * std::numbers::pi_v<float> * static_cast<float>(i) / static_cast<float>(pieces);
        const float x = centerX + radius * std::cos(angle);
        const float y = centerY + radius * std::sin(angle);
        ImVec2 point;
        const bool visible = projector.Project(x, y, RequestTerrainHeight(x, y) + GroundLift, point);
        if (visible && hasPrevious && (!dashed || i % 2 == 1))
        {
            list.AddLine(previous, point, color, thickness);
        }
        previous = point;
        hasPrevious = visible;
    }
}

void DrawLabel(ImDrawList& list, const ImVec2& marker, const std::string& text,
               MuEditor::SoundOverlay::LabelLine line = MuEditor::SoundOverlay::LabelLine::Beside)
{
    const ImVec2 size = ImGui::CalcTextSize(text.c_str());
    const ImVec2 start(marker.x + MarkerSize * 2.0f, MuEditor::SoundOverlay::LabelTop(marker.y, size.y, line));
    list.AddRectFilled(ImVec2(start.x - LabelGap, start.y - LabelGap),
                       ImVec2(start.x + size.x + LabelGap, start.y + size.y + LabelGap), LabelBackColor);
    list.AddText(start, LabelColor, text.c_str());
}

// A map object: a square, its file and its volume rings (full volume solid,
// quietest dashed). A monster: a dot, its set and its hearing ring, solid
// while the hero is inside it.
void DrawSource(ImDrawList& list, const Projector& projector, const DrawnSource& source, bool highlighted)
{
    const float x = source.position[0];
    const float y = source.position[1];
    const float thickness = highlighted ? HighlightThickness : LineThickness;
    const bool mapObject = source.kind == Audio::SoundKind::MapObject;
    ImU32 color = MapObjectColor;
    if (mapObject)
    {
        color = highlighted ? HighlightColor : MapObjectColor;
        DrawRing(list, projector, x, y, source.inner, color, thickness, false);
        if (source.outer > source.inner)
        {
            DrawRing(list, projector, x, y, source.outer, highlighted ? HighlightColor : QuietestRingColor,
                     highlighted ? HighlightThickness : 1.0f, true);
        }
    }
    else
    {
        const bool heard = Hero != nullptr &&
                           MuEditor::SoundOverlay::IsInside(source, Hero->Object.Position[0], Hero->Object.Position[1]);
        color = highlighted ? HighlightColor : (heard ? HeardColor : FarColor);
        if (source.inner > 0.0f)
        {
            DrawRing(list, projector, x, y, source.inner, color, thickness, !heard);
        }
    }
    ImVec2 marker;
    if (!projector.Project(x, y, source.position[2], marker))
    {
        return;
    }
    const float size = highlighted ? MarkerSize * HighlightGrowth : MarkerSize;
    if (mapObject)
    {
        list.AddRectFilled(ImVec2(marker.x - size, marker.y - size), ImVec2(marker.x + size, marker.y + size), color);
    }
    else
    {
        list.AddCircleFilled(marker, size, color);
    }
    std::string label = source.name;
    if (mapObject)
    {
        label = MuEditor::SoundMixer::ShownSoundText(source.soundNumber);
    }
    else if (source.inner <= 0.0f)
    {
        label = I18N::Format(I18N::Editor::SoundMixerHeardAnywhere0, {source.name});
    }
    DrawLabel(list, marker, label);
}

// A place of the game's own code's sounds: a hollow square (a map object) or
// circle (a monster) and its sounds' files; no ring, as the code has no range.
void DrawCodeMarker(ImDrawList& list, const Projector& projector, const MuEditor::SoundOverlay::CodeMarker& marker,
                    bool highlighted)
{
    ImVec2 point;
    if (!projector.Project(marker.position[0], marker.position[1], marker.position[2], point))
    {
        return;
    }
    const ImU32 color = highlighted ? HighlightColor : CodeColor;
    const float size = highlighted ? MarkerSize * HighlightGrowth : MarkerSize;
    const float thickness = highlighted ? HighlightThickness : LineThickness;
    if (marker.kind == Audio::SoundKind::MapObject)
    {
        list.AddRect(ImVec2(point.x - size, point.y - size), ImVec2(point.x + size, point.y + size), color, 0.0f, 0,
                     thickness);
    }
    else
    {
        list.AddCircle(point, size, color, 0, thickness);
    }
    std::string label;
    for (const int sound : marker.sounds)
    {
        label += (label.empty() ? "" : ", ") + MuEditor::SoundMixer::ShownSoundText(sound);
    }
    DrawLabel(list, point, label, MuEditor::SoundOverlay::LabelLine::Below);
}

void DrawFlash(ImDrawList& list, const Projector& projector, const Audio::Mixer::PlacedStart& start, double now)
{
    const float age = std::clamp(static_cast<float>((now - start.time) / FlashSeconds), 0.0f, 1.0f);
    const ImU32 color =
        (HighlightColor & ~IM_COL32_A_MASK) | (static_cast<ImU32>(FullAlpha * (1.0f - age)) << IM_COL32_A_SHIFT);
    DrawRing(list, projector, start.position[0], start.position[1],
             FlashStartRadius + (FlashEndRadius - FlashStartRadius) * age, color, HighlightThickness, false);
}

bool AnyNames(const std::vector<HeardRow>& rows, Audio::SoundKind kind, const std::string& name, int sourceNumber)
{
    return std::any_of(rows.begin(), rows.end(), [&](const HeardRow& row)
                       { return MuEditor::SoundOverlay::Names(row, kind, name, sourceNumber); });
}

// A line of the log, newest on top: yellow when it starts, white a second
// later, yellow while its row is hovered.
void DrawLog(ImDrawList& list, const std::vector<Audio::Mixer::PlacelessStart>& starts, double now,
             const HeardRow* hovered)
{
    const ImVec4 fresh = ImGui::ColorConvertU32ToFloat4(HighlightColor);
    const ImVec4 settled = ImGui::ColorConvertU32ToFloat4(LabelColor);
    ImVec2 at(ImGui::GetMainViewport()->WorkPos.x + LogMargin, ImGui::GetMainViewport()->WorkPos.y + LogMargin);
    for (const Audio::Mixer::PlacelessStart& start : starts)
    {
        const std::string text = MuEditor::SoundOverlay::LogText(start, now);
        const ImVec2 size = ImGui::CalcTextSize(text.c_str());
        const float age = std::clamp(static_cast<float>((now - start.time) / FlashSeconds), 0.0f, 1.0f);
        ImU32 color = ImGui::ColorConvertFloat4ToU32(ImLerp(fresh, settled, age));
        if (hovered != nullptr && hovered->sound.number == start.number)
        {
            color = HighlightColor;
        }
        list.AddRectFilled(ImVec2(at.x - LabelGap, at.y - LabelGap),
                           ImVec2(at.x + size.x + LabelGap, at.y + size.y + LabelGap), LabelBackColor);
        list.AddText(at, color, text.c_str());
        at.y += size.y + 2.0f * LabelGap + 1.0f;
    }
}
} // namespace

namespace MuEditor::SoundOverlay
{
bool Names(const SoundMixer::HeardRow& row, Audio::SoundKind kind, const std::string& name, int sourceNumber)
{
    if (row.sound.kind != kind || row.sound.sourceNumber != sourceNumber)
    {
        return false;
    }
    return kind != Audio::SoundKind::Monster || row.sound.name.empty() || row.sound.name == name;
}

std::vector<Audio::Overlay::DrawnSource> Followed(const std::vector<Audio::Overlay::DrawnSource>& drawn,
                                                  const std::vector<SoundMixer::HeardRow>& shown, bool filtered)
{
    std::vector<Audio::Overlay::DrawnSource> followed;
    std::copy_if(drawn.begin(), drawn.end(), std::back_inserter(followed),
                 [&shown, filtered](const DrawnSource& source)
                 {
                     return Audio::Overlay::IsShown(source.kind) &&
                            (!filtered || AnyNames(shown, source.kind, source.name, source.sourceNumber));
                 });
    return followed;
}

ImVec2 ToDisplay(float windowX, float windowY, const ImVec2& origin, const ImVec2& size, unsigned int windowWidth,
                 unsigned int windowHeight)
{
    const float scaleX = windowWidth > 0 ? size.x / static_cast<float>(windowWidth) : 1.0f;
    const float scaleY = windowHeight > 0 ? size.y / static_cast<float>(windowHeight) : 1.0f;
    return ImVec2(origin.x + windowX * scaleX, origin.y + windowY * scaleY);
}

float LabelTop(float markerY, float textHeight, LabelLine line)
{
    const float top = markerY - textHeight * 0.5f;
    return line == LabelLine::Below ? top + textHeight + 2.0f * LabelGap : top;
}

bool IsInside(const Audio::Overlay::DrawnSource& source, float x, float y)
{
    // No ring: heard anywhere.
    if (source.inner <= 0.0f)
    {
        return true;
    }
    const float dx = x - source.position[0];
    const float dy = y - source.position[1];
    return dx * dx + dy * dy <= source.inner * source.inner;
}

bool Follows(const Audio::Mixer::PlacedStart& start, const std::vector<SoundMixer::HeardRow>& shown)
{
    if (!start.byCode)
    {
        return AnyNames(shown, start.kind, start.name, start.sourceNumber);
    }
    return std::any_of(shown.begin(), shown.end(),
                       [&start](const HeardRow& row) { return row.sound.number == start.number; });
}

bool Draws(const Audio::Mixer::PlacedStart& start, const std::vector<SoundMixer::HeardRow>& shown, bool filtered)
{
    return Audio::Overlay::IsShown(start.kind) && (!filtered || Follows(start, shown));
}

std::vector<CodeMarker> GroupCodeSounds(const std::vector<Audio::Mixer::PlacedStart>& starts)
{
    std::vector<CodeMarker> markers;
    for (const Audio::Mixer::PlacedStart& start : starts)
    {
        const auto same = std::find_if(markers.begin(), markers.end(),
                                       [&start](const CodeMarker& marker)
                                       {
                                           return marker.kind == start.kind &&
                                                  marker.sourceNumber == start.sourceNumber &&
                                                  marker.position == start.position;
                                       });
        if (same != markers.end())
        {
            same->sounds.push_back(start.number);
        }
        else
        {
            markers.push_back(CodeMarker{start.kind, start.sourceNumber, start.position, {start.number}});
        }
    }
    return markers;
}

bool Highlights(const SoundMixer::HeardRow& hovered, const CodeMarker& marker)
{
    return std::find(marker.sounds.begin(), marker.sounds.end(), hovered.sound.number) != marker.sounds.end();
}

std::vector<Audio::Mixer::PlacelessStart> Logged(const std::vector<Audio::Mixer::PlacelessStart>& starts,
                                                 const std::vector<SoundMixer::HeardRow>& shown, bool filtered)
{
    std::vector<Audio::Mixer::PlacelessStart> logged;
    for (const Audio::Mixer::PlacelessStart& start : starts)
    {
        if (logged.size() == static_cast<size_t>(LogLines))
        {
            break;
        }
        const bool followed = !filtered || std::any_of(shown.begin(), shown.end(), [&start](const HeardRow& row)
                                                       { return row.sound.number == start.number; });
        if (Audio::Overlay::IsShown(start.kind) && followed)
        {
            logged.push_back(start);
        }
    }
    return logged;
}

std::string MusicText(const std::string& music)
{
    std::string file = music;
    std::replace(file.begin(), file.end(), '\\', '/');
    constexpr std::string_view MusicFolder = "data/music/";
    if (file.size() > MusicFolder.size())
    {
        std::string start = file.substr(0, MusicFolder.size());
        std::transform(start.begin(), start.end(), start.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (start == MusicFolder)
        {
            file.erase(0, MusicFolder.size());
        }
    }
    return file;
}

std::string LogText(const Audio::Mixer::PlacelessStart& start, double now)
{
    const std::string ago = SoundMixer::SecondsText(now - start.time);
    if (start.kind == Audio::SoundKind::Music && start.number < 0)
    {
        return I18N::Format(I18N::Editor::SoundMixerLogMusic01, {ago, MusicText(start.music)});
    }
    return I18N::Format(start.looped ? I18N::Editor::SoundMixerLogLoop012 : I18N::Editor::SoundMixerLog012,
                        {ago, SoundMixer::ShownSoundText(start.number), SoundMixer::KindName(start.kind)});
}

void Render(const std::vector<SoundMixer::HeardRow>& shown, bool filtered, const SoundMixer::HeardRow* hovered)
{
    const std::vector<DrawnSource> drawn = Audio::Overlay::TakeDrawn();
    if (!Audio::Overlay::IsOn())
    {
        return;
    }
    ImDrawList& list = *ImGui::GetBackgroundDrawList();
    DrawLog(list, Logged(Audio::Mixer::PlacelessStarts(), shown, filtered), Audio::Mixer::Now(), hovered);
    const CameraState* camera = Audio::Overlay::KeptCamera();
    if (camera == nullptr)
    {
        return;
    }
    const Projector projector(*camera);
    for (const DrawnSource& source : Followed(drawn, shown, filtered))
    {
        DrawSource(list, projector, source,
                   hovered != nullptr && Names(*hovered, source.kind, source.name, source.sourceNumber));
    }
    const double now = Audio::Mixer::Now();
    std::vector<Audio::Mixer::PlacedStart> codeSounds;
    for (const Audio::Mixer::PlacedStart& start : Audio::Mixer::CodeSounds(now))
    {
        if (Draws(start, shown, filtered))
        {
            codeSounds.push_back(start);
        }
    }
    for (const CodeMarker& marker : GroupCodeSounds(codeSounds))
    {
        DrawCodeMarker(list, projector, marker, hovered != nullptr && Highlights(*hovered, marker));
    }
    for (const Audio::Mixer::PlacedStart& start : Audio::Mixer::RecentStarts(now, FlashSeconds))
    {
        if (Draws(start, shown, filtered))
        {
            DrawFlash(list, projector, start, now);
        }
    }
}
} // namespace MuEditor::SoundOverlay

#endif // _EDITOR
