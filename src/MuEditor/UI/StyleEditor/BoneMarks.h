#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/EffectBrowser/EffectPreviewCamera.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewScene.h"
#include "Data/GameData/LookData/LookDefinition.h"
#include "imgui.h"

#include <optional>
#include <string>
#include <vector>

// The bones of the item a preview view shows, marked over the view (design L58): every bone as a dot joined to the
// bone it hangs on by a line, with its number and name on hover ("Show bones"), the bones of the picked entry of an
// effect's things on bones, the bone the mouse points at in the entry's fields ringed with its number, and the
// pick mode that adds or removes a bone by clicking its dot.
namespace MuEditor::Looks::BoneMarks
{
// A bone of the shown item: its number and name, and where the view's scene has it.
struct ShownBone
{
    int number = 0;
    // The bone it hangs on, -1 for a root: the marks join them with a line (design L58).
    int parent = -1;
    std::string name;
    MuEditor::Effects::PreviewVector place{};
};

// The bones of the item of `request` as its view poses it, turned by `turnDegrees`, at the scene's origin, into
// `shown` (whose room and names are reused each frame); none for an item drawn on another skeleton (armor), whose
// things on bones the game creates nowhere.
void BonesOf(const MuEditor::Effects::EffectPreviewRequest& request, float turnDegrees, std::vector<ShownBone>& shown);

// Where `place` shows in a view standing at `min` to `max` in the window, as `camera` sees it; nullopt behind the eye.
std::optional<ImVec2> ToScreen(const MuEditor::Effects::EffectPreviewCamera& camera, const ImVec2& min,
                               const ImVec2& max, const MuEditor::Effects::PreviewVector& place);

// The index in `points` of the one nearest `mouse` within `radius` pixels; nullopt for none.
std::optional<std::size_t> NearestWithin(const std::vector<ImVec2>& points, const ImVec2& mouse, float radius);

// A click on bone `bone` in pick mode: at bones (`between` false) it adds the bone or takes it out; between bones the
// first click is the lightning's start (`from`), the second its end, which adds the pair or, when the entry has it,
// takes it out; a second click on the start lets it go. True when the places changed.
bool PickBone(Data::Looks::BonePlaces& places, bool between, std::optional<int>& from, int bone);

// The pick mode's clicks, which a double click (it puts the view back) takes back as a mesh pick's
// (CItemMeshFocus::TakeBackPick): each pick keeps the places and pair start it changed, and when.
class PickTakeBack
{
public:
    // PickBone at `time`, keeping what was before it.
    bool Pick(Data::Looks::BonePlaces& places, bool between, std::optional<int>& from, int bone, double time);
    // A double click at `time`: puts back what the last pick changed when it came at most `doubleClickTime` before,
    // once. True when it put something back.
    bool TakeBack(Data::Looks::BonePlaces& places, std::optional<int>& from, double time, double doubleClickTime);

private:
    std::optional<Data::Looks::BonePlaces> m_places;
    std::optional<int> m_from;
    double m_time = 0.0;
};

// What the marks of a view show.
struct Marks
{
    // Every bone as a dot ("Show bones" or the pick mode).
    bool dots = false;
    // The bones the picked entry names, and its pairs, drawn as lines from start to end; each with its number.
    std::vector<int> bones;
    std::vector<std::pair<int, int>> pairs;
    // The bone the entry's fields point at, ringed with its number.
    std::optional<int> ringed;
    // The start of a pair being picked.
    std::optional<int> from;
};

// Draws the marks over the view standing at `min` to `max`; returns the bone a click (`click`) hit, if any. With
// `hovered`, the dot under the mouse shows its number and name.
std::optional<int> Draw(const std::vector<ShownBone>& bones, const Marks& marks,
                        const MuEditor::Effects::EffectPreviewCamera& camera, const ImVec2& min, const ImVec2& max,
                        const std::optional<ImVec2>& click, bool hovered);
} // namespace MuEditor::Looks::BoneMarks

#endif // _EDITOR
