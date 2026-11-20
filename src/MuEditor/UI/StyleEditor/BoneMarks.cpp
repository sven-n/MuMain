#include "stdafx.h"

#ifdef _EDITOR

#include "BoneMarks.h"

#include "../MuEditor/UI/EffectBrowser/EffectPreviewItems.h"
#include "../MuEditor/UI/EffectBrowser/PreviewModelPose.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <numbers>

namespace MuEditor::Looks::BoneMarks
{
namespace
{
constexpr float DotRadius = 3.0f;
constexpr float RingRadius = 8.0f;
// How far from a dot the mouse may be to point at it, in pixels before the UI scale.
constexpr float PickRadius = 7.0f;

constexpr ImU32 DotColor = IM_COL32(235, 235, 235, 220);
constexpr ImU32 DotOutline = IM_COL32(0, 0, 0, 200);
constexpr ImU32 UsedColor = IM_COL32(255, 170, 40, 255);
constexpr ImU32 RingColor = IM_COL32(255, 230, 60, 255);
constexpr ImU32 FromColor = IM_COL32(90, 200, 255, 255);
constexpr ImU32 TextBack = IM_COL32(0, 0, 0, 170);
constexpr ImU32 LinkColor = IM_COL32(170, 170, 165, 170);
constexpr float NumberGap = 4.0f;

void Label(ImDrawList& list, const ImVec2& at, const std::string& text, ImU32 color)
{
    const ImVec2 size = ImGui::CalcTextSize(text.c_str());
    const ImVec2 corner{at.x + RingRadius, at.y - RingRadius - size.y};
    list.AddRectFilled({corner.x - 2.0f, corner.y - 1.0f}, {corner.x + size.x + 2.0f, corner.y + size.y + 1.0f},
                       TextBack, 2.0f);
    list.AddText(corner, color, text.c_str());
}
// A bone's number right beside its dot.
void Number(ImDrawList& list, const ImVec2& at, int number, float scale)
{
    const std::string text = std::to_string(number);
    const ImVec2 size = ImGui::CalcTextSize(text.c_str());
    const ImVec2 corner{at.x + NumberGap * scale, at.y - NumberGap * scale - size.y};
    list.AddRectFilled({corner.x - 1.0f, corner.y}, {corner.x + size.x + 1.0f, corner.y + size.y}, TextBack, 2.0f);
    list.AddText(corner, UsedColor, text.c_str());
}
} // namespace

void BonesOf(const MuEditor::Effects::EffectPreviewRequest& request, float turnDegrees, std::vector<ShownBone>& shown)
{
    const MuEditor::Effects::PreviewItemModels models = MuEditor::Effects::PreviewItemModelsOf(request);
    if (!models.CanDraw() || models.skeleton != models.model)
    {
        shown.clear();
        return;
    }
    BMD& b = Models[models.model];
    if (b.NumBones <= 0 || b.Bones == nullptr)
    {
        shown.clear();
        return;
    }
    const MuEditor::Effects::ScopedModelState saved(b);
    OBJECT o;
    // The pose, made once: the marks are drawn on the editor's one thread.
    static const std::unique_ptr<vec34_t[]> bones = std::make_unique<vec34_t[]>(MAX_BONES);
    MuEditor::Effects::PosePreviewItem(request, models, turnDegrees, o, bones.get());
    const int count = std::min<int>(b.NumBones, MAX_BONES);
    // Resized, never cleared before, so the bones of the last frame and their names keep their room.
    shown.resize(static_cast<std::size_t>(count));
    for (int bone = 0; bone < count; ++bone)
    {
        // As the game places things on bones (TransformPosition with the model's scale), at the scene's origin.
        ShownBone& entry = shown[static_cast<std::size_t>(bone)];
        entry.number = bone;
        entry.parent = b.Bones[bone].Parent;
        entry.name.assign(b.Bones[bone].Name, strnlen(b.Bones[bone].Name, sizeof(b.Bones[bone].Name)));
        for (int axis = 0; axis < 3; ++axis)
        {
            entry.place[static_cast<std::size_t>(axis)] = bones[bone][axis][3] * o.Scale;
        }
    }
}

std::optional<ImVec2> ToScreen(const MuEditor::Effects::EffectPreviewCamera& camera, const ImVec2& min,
                               const ImVec2& max, const MuEditor::Effects::PreviewVector& place)
{
    const float width = max.x - min.x;
    const float height = max.y - min.y;
    if (width <= 0.0f || height <= 0.0f)
    {
        return std::nullopt;
    }
    const MuEditor::Effects::EffectPreviewCamera::Basis basis = camera.GetBasis();
    const MuEditor::Effects::PreviewVector eye = camera.Eye();
    float right = 0.0f;
    float up = 0.0f;
    float forward = 0.0f;
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
        const float d = place[axis] - eye[axis];
        right += d * basis.right[axis];
        up += d * basis.up[axis];
        forward += d * basis.forward[axis];
    }
    if (forward <= camera.Near())
    {
        return std::nullopt;
    }
    // As MeshPicking::RayThroughView casts its rays, the other way.
    const float halfHeight = std::tan(MuEditor::Effects::EffectPreviewCamera::FieldOfViewDegrees * 0.5f *
                                      std::numbers::pi_v<float> / 180.0f);
    const float viewX = right / (forward * halfHeight * (width / height));
    const float viewY = up / (forward * halfHeight);
    return ImVec2{min.x + (viewX + 1.0f) * 0.5f * width, min.y + (1.0f - viewY) * 0.5f * height};
}

std::optional<std::size_t> NearestWithin(const std::vector<ImVec2>& points, const ImVec2& mouse, float radius)
{
    std::optional<std::size_t> nearest;
    float best = radius * radius;
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        const float dx = points[i].x - mouse.x;
        const float dy = points[i].y - mouse.y;
        const float distance = dx * dx + dy * dy;
        if (distance <= best)
        {
            best = distance;
            nearest = i;
        }
    }
    return nearest;
}

bool PickBone(Data::Looks::BonePlaces& places, bool between, std::optional<int>& from, int bone)
{
    if (!between)
    {
        from.reset();
        if (const auto at = std::ranges::find(places.bones, bone); at != places.bones.end())
        {
            places.bones.erase(at);
        }
        else
        {
            places.bones.push_back(bone);
        }
        return true;
    }
    if (!from)
    {
        from = bone;
        return false;
    }
    const int start = *from;
    from.reset();
    if (start == bone)
    {
        return false;
    }
    const std::pair<int, int> pair{start, bone};
    if (const auto at = std::ranges::find(places.between, pair); at != places.between.end())
    {
        places.between.erase(at);
    }
    else
    {
        places.between.push_back(pair);
    }
    return true;
}

bool PickTakeBack::Pick(Data::Looks::BonePlaces& places, bool between, std::optional<int>& from, int bone, double time)
{
    m_places = places;
    m_from = from;
    m_time = time;
    return PickBone(places, between, from, bone);
}

bool PickTakeBack::TakeBack(Data::Looks::BonePlaces& places, std::optional<int>& from, double time,
                            double doubleClickTime)
{
    if (!m_places || time - m_time > doubleClickTime)
    {
        return false;
    }
    places = *m_places;
    from = m_from;
    m_places.reset();
    return true;
}

std::optional<int> Draw(const std::vector<ShownBone>& bones, const Marks& marks,
                        const MuEditor::Effects::EffectPreviewCamera& camera, const ImVec2& min, const ImVec2& max,
                        const std::optional<ImVec2>& click, bool hovered)
{
    // Reused each frame (the marks are drawn on the editor's one thread), so drawing allocates nothing once they grew.
    static std::vector<ImVec2> points;
    static std::vector<bool> seen;
    static std::vector<ImVec2> seenPoints;
    static std::vector<std::size_t> seenBones;
    static std::vector<int> used;
    points.assign(bones.size(), ImVec2{});
    seen.assign(bones.size(), false);
    seenPoints.clear();
    seenBones.clear();
    for (std::size_t i = 0; i < bones.size(); ++i)
    {
        if (const std::optional<ImVec2> point = ToScreen(camera, min, max, bones[i].place))
        {
            points[i] = *point;
            seen[i] = true;
            seenPoints.push_back(*point);
            seenBones.push_back(i);
        }
    }
    const auto placeOf = [&](int bone) -> std::optional<ImVec2>
    {
        if (bone < 0 || static_cast<std::size_t>(bone) >= bones.size() || !seen[static_cast<std::size_t>(bone)])
        {
            return std::nullopt;
        }
        return points[static_cast<std::size_t>(bone)];
    };

    ImDrawList& list = *ImGui::GetWindowDrawList();
    list.PushClipRect(min, max, true);
    // The marks grow with the editor's font.
    const float scale = std::max(1.0f, ImGui::GetFontSize() / 13.0f);
    // The bones the picked entry names, at bones or in its pairs.
    used.assign(marks.bones.begin(), marks.bones.end());
    for (const auto& [start, end] : marks.pairs)
    {
        used.push_back(start);
        used.push_back(end);
    }
    std::ranges::sort(used);
    const auto [last, end] = std::ranges::unique(used);
    used.erase(last, end);
    const auto isUsed = [&](int bone) { return std::ranges::binary_search(used, bone); };
    if (marks.dots)
    {
        // The skeleton: each bone joined to the one it hangs on, the picked entry's in its color.
        for (const ShownBone& bone : bones)
        {
            const std::optional<ImVec2> child = placeOf(bone.number);
            const std::optional<ImVec2> parent = placeOf(bone.parent);
            if (child && parent)
            {
                const bool picked = isUsed(bone.number);
                list.AddLine(*parent, *child, picked ? UsedColor : LinkColor, picked ? 1.6f : 1.0f);
            }
        }
        for (const ImVec2& point : seenPoints)
        {
            list.AddCircleFilled(point, DotRadius * scale + 1.0f, DotOutline);
            list.AddCircleFilled(point, DotRadius * scale, DotColor);
        }
    }
    for (const auto& [start, end] : marks.pairs)
    {
        const std::optional<ImVec2> a = placeOf(start);
        const std::optional<ImVec2> b = placeOf(end);
        if (a && b)
        {
            list.AddLine(*a, *b, UsedColor, 1.5f);
            // The end: where the lightning goes.
            list.AddCircle(*b, DotRadius * scale + 2.0f, UsedColor, 0, 1.5f);
        }
    }
    for (const int bone : marks.bones)
    {
        if (const std::optional<ImVec2> point = placeOf(bone))
        {
            list.AddCircleFilled(*point, DotRadius * scale + 1.0f, UsedColor);
        }
    }
    for (const auto& [start, end] : marks.pairs)
    {
        for (const int bone : {start, end})
        {
            if (const std::optional<ImVec2> point = placeOf(bone))
            {
                list.AddCircleFilled(*point, DotRadius * scale + 1.0f, UsedColor);
            }
        }
    }
    // The picked entry's bones with their numbers; a ringed one gets its own.
    for (const int bone : used)
    {
        if (bone == marks.ringed || bone == marks.from)
        {
            continue;
        }
        if (const std::optional<ImVec2> point = placeOf(bone))
        {
            Number(list, *point, bone, scale);
        }
    }
    if (marks.from)
    {
        if (const std::optional<ImVec2> point = placeOf(*marks.from))
        {
            list.AddCircle(*point, RingRadius * scale, FromColor, 0, 2.0f);
            Label(list, *point, std::to_string(*marks.from), FromColor);
        }
    }
    if (marks.ringed)
    {
        if (const std::optional<ImVec2> point = placeOf(*marks.ringed))
        {
            list.AddCircle(*point, RingRadius * scale, RingColor, 0, 2.0f);
            Label(list, *point, std::to_string(*marks.ringed), RingColor);
        }
    }
    std::optional<int> hit;
    if (marks.dots)
    {
        const float radius = PickRadius * scale;
        if (hovered)
        {
            if (const std::optional<std::size_t> under = NearestWithin(seenPoints, ImGui::GetIO().MousePos, radius))
            {
                const ShownBone& bone = bones[seenBones[*under]];
                list.AddCircle(seenPoints[*under], RingRadius * scale, DotColor, 0, 1.5f);
                Label(list, seenPoints[*under],
                      bone.name.empty() ? std::to_string(bone.number) : std::to_string(bone.number) + " " + bone.name,
                      DotColor);
            }
        }
        if (click)
        {
            if (const std::optional<std::size_t> under = NearestWithin(seenPoints, *click, radius))
            {
                hit = bones[seenBones[*under]].number;
            }
        }
    }
    list.PopClipRect();
    return hit;
}
} // namespace MuEditor::Looks::BoneMarks

#endif // _EDITOR
