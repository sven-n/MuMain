#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/EffectBrowser/EffectPreviewCamera.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewScene.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewTracker.h"
#include "Data/GameData/LookData/LookDefinition.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "Render/Items/LookDrawing.h"

#include <array>
#include <memory>
#include <optional>
#include <vector>

namespace MuEditor::Looks
{
// What a view of the Style Editor's preview drew, for its things on bones.
struct BoneThingsShown
{
    MuEditor::Effects::EffectPreviewRequest request;
    // The effect the view's item runs (Render::Items::ItemEffects::EffectOf while the view draws); nullptr: none.
    const Data::Looks::LookDefinition* effect = nullptr;
    // How far the view's turn has turned the item.
    float turnDegrees = 0.0f;
    // The preview's stopped time, in milliseconds of WorldTime; nullopt while time runs.
    std::optional<double> stoppedAt;
};

// The things on bones of the items the Style Editor's preview and the Item Model window's preview show (FX1.7c, design
// section 7a, L51 to L54). Each view
// keeps its own copy of the item object with its own bones at the player's character (L52, L53); in the game's update
// it creates the things its effect creates there, with the game's own calls into the game's effect lists, which
// move them as in the game. Their slots carry the view's mark (Render::Effects::PreviewSlots): the world doesn't draw
// them, and the view draws only its own, right after its item, with its camera. While the preview's time stops the
// game moves none of a view's things and the view creates only its sprites and light, made from the stopped clock
// (L54). A view's things are removed when its item or effect changes, when it is no longer drawn, and when the world
// is cleared. The game comes first: a view creates nothing while a list has less than a quarter of its slots free,
// and the views together keep to a quarter of each list.
class BoneThingsPreview
{
public:
    // The Style Editor's three views, then the Item Model window's.
    static constexpr int MaxViews = 4;
    static constexpr int ItemModelView = 3;
    // How many of the game's updates a view's things live on without the view being drawn: the editor's windows skip
    // frames the GPU is behind on.
    static constexpr int UpdatesWithoutDraw = 10;

    // In view `view`'s draw, right after its item, with the view's matrices: keeps what the view shows for the next
    // update and draws the view's things with `camera`.
    void Draw(int view, const BoneThingsShown& shown, const MuEditor::Effects::EffectPreviewCamera& camera);
    // Once a frame in the game's update (Core::EffectSpawnHook): removes what is no longer shown, takes in what the
    // views' things created, and creates this frame's things.
    void Update();
    // The game is about to clear its effect lists.
    void OnWorldClearing();
    // Removes every view's things.
    void RemoveAll();

    MuEditor::Effects::EffectPreviewCounts GetCounts(int view) const;

private:
    // What a view's things were made for: when it changes they are removed.
    struct Made
    {
        int model = -1;
        MuEditor::Effects::PreviewItemPose pose = MuEditor::Effects::PreviewItemPose::Inventory;
        int effectVersion = 0;

        bool operator==(const Made&) const = default;
    };

    struct ViewThings
    {
        MuEditor::Effects::EffectPreviewTracker tracker;
        // The view's copy of the item: the owner and target of its things.
        OBJECT object;
        std::unique_ptr<vec34_t[]> bones;
        // What the view showed last.
        MuEditor::Effects::EffectPreviewRequest request;
        float turnDegrees = 0.0f;
        std::optional<double> stoppedAt;
        Made shown;
        // What its things were made for, while it has any.
        std::optional<Made> made;
        // The effect it draws, and the same with its sprites only (time stopped), each with its catalogue types.
        std::optional<Data::Looks::LookDefinition> effect;
        Render::Items::Looks::BoneThingTypes types;
        Data::Looks::LookDefinition spritesOnly;
        Render::Items::Looks::BoneThingTypes spriteTypes;
        int effectVersion = 0;
        // The sprite slots the last update created: a sprite lives one frame.
        std::vector<int> sprites;
        // Where the item stood in the last update: the view draws its things around it.
        MuEditor::Effects::PreviewVector place{};
        int updatesSinceDraw = UpdatesWithoutDraw;
    };

    void Remove(ViewThings& things, int view);
    void Create(ViewThings& things, const MuEditor::Effects::EffectPools& pools);
    static void Capture(int view, const ViewThings& things, const MuEditor::Effects::EffectPreviewCamera& camera);
    bool HasRoom() const;

    std::array<ViewThings, MaxViews> m_views;
    // Reused by each update, so it allocates nothing once they have grown: the sprite slots that lived before a view
    // created its things, and the slots a view kept before the update.
    std::vector<bool> m_spritesBefore;
    std::vector<MuEditor::Effects::EffectPoolSlot> m_keptBefore;
};

extern BoneThingsPreview g_BoneThingsPreview;
} // namespace MuEditor::Looks

#endif // _EDITOR
