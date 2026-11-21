#include "stdafx.h"

#ifdef _EDITOR

#include "BoneThingsPreview.h"

#include "WorldTimeScope.h"

#include "../MuEditor/UI/EffectBrowser/EffectWorldPreview.h"
#include "../MuEditor/UI/EffectBrowser/PreviewModelPose.h"
#include "../MuEditor/UI/ItemTools/ItemModelPreview.h"
#include "Audio/EditorSoundMute.h"
#include "Camera/CameraState.h"
#include "Core/Globals/_define.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Effects/PreviewSlots.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <cstddef>

namespace MuEditor::Looks
{
static_assert(BoneThingsPreview::ItemModelView == MuEditor::Items::CItemModelPreview::MaxViews,
              "the Item Model window's view comes after the Style Editor's, which are as many as a preview shows");
static_assert(BoneThingsPreview::MaxViews == BoneThingsPreview::ItemModelView + 1,
              "the Item Model window's view is the last");
BoneThingsPreview g_BoneThingsPreview;

namespace
{
using MuEditor::Effects::EffectPool;
using MuEditor::Effects::EffectPools;
using MuEditor::Effects::EffectPoolSlot;
using Render::Effects::PreviewSlots::List;

// The mark of a view's slots: 0 is the game's.
Render::Effects::PreviewSlots::View MarkOf(int view)
{
    return static_cast<Render::Effects::PreviewSlots::View>(view + 1);
}

// The effect with its sprites only: what a view creates while its time is stopped (design L54). Its light stays.
Data::Looks::LookDefinition SpritesOnly(const Data::Looks::LookDefinition& effect)
{
    Data::Looks::LookDefinition sprites = effect;
    sprites.animationSpeed.reset();
    for (Data::Looks::OnBones& entry : sprites.onBones)
    {
        std::erase_if(entry.things, [](const Data::Looks::BoneThing& thing)
                      { return thing.kind != Data::Looks::BoneThingKind::Sprite; });
    }
    std::erase_if(sprites.onBones, [](const Data::Looks::OnBones& entry) { return entry.things.empty(); });
    return sprites;
}

template <typename Object> int CountFree(std::span<Object> pool)
{
    return static_cast<int>(std::ranges::count_if(pool, [](const Object& object) { return !object.Live; }));
}

// The sprite slots live now that weren't before.
void NewSprites(std::span<const OBJECT> sprites, const std::vector<bool>& liveBefore, std::vector<int>& created)
{
    created.clear();
    for (std::size_t i = 0; i < sprites.size(); ++i)
    {
        if (sprites[i].Live && !liveBefore[i])
        {
            created.push_back(static_cast<int>(i));
        }
    }
}

// The list and index of a pool's slot as the marks count them.
std::pair<List, int> MarkedSlot(const EffectPoolSlot& slot)
{
    switch (slot.pool)
    {
    case EffectPool::Effect:
        return {List::Effect, slot.index};
    case EffectPool::SkillEffect:
        // RenderEffects counts the skill effects after the effects.
        return {List::Effect, MAX_EFFECTS + slot.index};
    case EffectPool::Particle:
        return {List::Particle, slot.index};
    case EffectPool::Joint:
        return {List::Joint, slot.index};
    case EffectPool::Sprite:
        break;
    }
    return {List::Sprite, slot.index};
}

void MarkSlot(const EffectPoolSlot& slot, Render::Effects::PreviewSlots::View view)
{
    const auto [list, index] = MarkedSlot(slot);
    Render::Effects::PreviewSlots::Mark(list, index, view);
}

// Takes the view's mark off a slot it no longer keeps: the slot is the game's again. A slot a create call took again
// (PreviewSlots::Born) or another view marked keeps its mark.
void UnmarkSlot(List list, int index, Render::Effects::PreviewSlots::View view)
{
    if (Render::Effects::PreviewSlots::ViewOf(list, index) == view)
    {
        Render::Effects::PreviewSlots::Mark(list, index, 0);
    }
}

void UnmarkSlots(const std::vector<EffectPoolSlot>& slots, Render::Effects::PreviewSlots::View view)
{
    for (const EffectPoolSlot& slot : slots)
    {
        const auto [list, index] = MarkedSlot(slot);
        UnmarkSlot(list, index, view);
    }
}
} // namespace

void BoneThingsPreview::Draw(int view, const BoneThingsShown& shown,
                             const MuEditor::Effects::EffectPreviewCamera& camera)
{
    if (view < 0 || view >= MaxViews)
    {
        return;
    }
    ViewThings& things = m_views[static_cast<std::size_t>(view)];
    things.updatesSinceDraw = 0;
    things.request = shown.request;
    things.turnDegrees = shown.turnDegrees;
    things.stoppedAt = shown.stoppedAt;
    // From the next move on, not one update later: what lives a frame (the Wings of Darkness' lightning) stops as it
    // is instead of ending before the update sees the time stopped.
    Render::Effects::PreviewSlots::SetStopped(MarkOf(view), shown.stoppedAt.has_value());
    // The effect is copied (and its types found) only when it changes, not every frame.
    const bool same = shown.effect == nullptr ? !things.effect.has_value()
                                              : things.effect.has_value() && *things.effect == *shown.effect;
    if (!same)
    {
        ++things.effectVersion;
        things.effect.reset();
        things.types.clear();
        things.spritesOnly = {};
        things.spriteTypes.clear();
        if (shown.effect != nullptr)
        {
            things.effect = *shown.effect;
            things.types = Render::Items::Looks::ResolveBoneThingTypes(*things.effect);
            things.spritesOnly = SpritesOnly(*things.effect);
            things.spriteTypes = Render::Items::Looks::ResolveBoneThingTypes(things.spritesOnly);
        }
    }
    things.shown = {MuEditor::Effects::PreviewItemModelsOf(shown.request).model, shown.request.itemPose,
                    things.effectVersion};
    if (!things.tracker.IsEmpty() || !things.sprites.empty())
    {
        Capture(view, things, camera);
    }
}

void BoneThingsPreview::Update()
{
    const EffectPools pools = MuEditor::Effects::GetGamePools();
    const bool ready = MuEditor::Effects::IsWorldReadyForPreview();
    for (int view = 0; view < MaxViews; ++view)
    {
        ViewThings& things = m_views[static_cast<std::size_t>(view)];
        for (const int sprite : things.sprites)
        {
            UnmarkSlot(List::Sprite, sprite, MarkOf(view));
        }
        things.sprites.clear();
    }
    // Every view forgets what ended before any creates: a slot one view's thing left can be filled by another view's
    // of the same type, which the first would take for its own.
    std::array<bool, MaxViews> creates{};
    for (int view = 0; view < MaxViews; ++view)
    {
        ViewThings& things = m_views[static_cast<std::size_t>(view)];
        const bool drawn = things.updatesSinceDraw < UpdatesWithoutDraw;
        things.updatesSinceDraw = std::min(things.updatesSinceDraw + 1, UpdatesWithoutDraw);
        // A slot whose thing ended and that a create call took again (PreviewSlots::Born) is not the view's any more,
        // even with a thing of the same type in it: the game's own lightning, which the game creates while a view's
        // lightning of a frame ends. It is neither drawn by the view nor removed with its things.
        things.tracker.ForgetSlots(
            [view](const EffectPoolSlot& slot)
            {
                const auto [list, index] = MarkedSlot(slot);
                return Render::Effects::PreviewSlots::ViewOf(list, index) != MarkOf(view);
            });
        if (!ready || !drawn || (things.made && *things.made != things.shown))
        {
            Remove(things, view);
        }
        Render::Effects::PreviewSlots::SetStopped(MarkOf(view), drawn && things.stoppedAt.has_value());
        if (ready && drawn)
        {
            m_keptBefore.assign(things.tracker.GetKept().begin(), things.tracker.GetKept().end());
            things.tracker.Update(pools);
            const std::vector<EffectPoolSlot>& kept = things.tracker.GetKept();
            for (const EffectPoolSlot& slot : m_keptBefore)
            {
                if (std::ranges::find(kept, slot) == kept.end())
                {
                    const auto [list, index] = MarkedSlot(slot);
                    UnmarkSlot(list, index, MarkOf(view));
                }
            }
            creates[static_cast<std::size_t>(view)] = true;
        }
    }
    for (int view = 0; view < MaxViews; ++view)
    {
        if (creates[static_cast<std::size_t>(view)] && HasRoom())
        {
            Create(m_views[static_cast<std::size_t>(view)], pools);
        }
    }
    for (int view = 0; view < MaxViews; ++view)
    {
        const ViewThings& things = m_views[static_cast<std::size_t>(view)];
        for (const EffectPoolSlot& slot : things.tracker.GetKept())
        {
            MarkSlot(slot, MarkOf(view));
        }
        for (const int sprite : things.sprites)
        {
            Render::Effects::PreviewSlots::Mark(List::Sprite, sprite, MarkOf(view));
        }
    }
}

void BoneThingsPreview::OnWorldClearing()
{
    RemoveAll();
}

void BoneThingsPreview::RemoveAll()
{
    for (int view = 0; view < MaxViews; ++view)
    {
        Remove(m_views[static_cast<std::size_t>(view)], view);
    }
    Render::Effects::PreviewSlots::Clear();
}

MuEditor::Effects::EffectPreviewCounts BoneThingsPreview::GetCounts(int view) const
{
    if (view < 0 || view >= MaxViews)
    {
        return {};
    }
    return m_views[static_cast<std::size_t>(view)].tracker.Count();
}

void BoneThingsPreview::Remove(ViewThings& things, int view)
{
    UnmarkSlots(things.tracker.GetKept(), MarkOf(view));
    for (const int sprite : things.sprites)
    {
        UnmarkSlot(List::Sprite, sprite, MarkOf(view));
    }
    if (!things.tracker.IsEmpty())
    {
        things.tracker.RemoveAll(MuEditor::Effects::GetGamePools(), &MuEditor::Effects::RemoveGameEffect);
    }
    for (const int sprite : things.sprites)
    {
        Sprites[sprite].Live = false;
    }
    things.sprites.clear();
    things.made.reset();
}

// The view's item, posed as the view draws it, at the character; its effect creates this frame's things there.
void BoneThingsPreview::Create(ViewThings& things, const EffectPools& pools)
{
    const MuEditor::Effects::PreviewItemModels models = MuEditor::Effects::PreviewItemModelsOf(things.request);
    // While the time is stopped, what the view made stays as it was and only the sprites (made from the clock) come
    // again; a view that has made nothing yet (its item, pose or effect just changed) makes all once, to stop on.
    const bool spritesOnly = things.stoppedAt && things.made;
    const Data::Looks::LookDefinition* effect = spritesOnly ? &things.spritesOnly : nullptr;
    const Render::Items::Looks::BoneThingTypes* types = spritesOnly ? &things.spriteTypes : &things.types;
    if (!spritesOnly && things.effect)
    {
        effect = &*things.effect;
    }
    // Armor is drawn on the character skeleton: the game creates nothing for it on the ground either.
    if (effect == nullptr || Hero == nullptr || !models.CanDraw() || models.skeleton != models.model)
    {
        return;
    }
    if (!things.bones)
    {
        things.bones = std::make_unique<vec34_t[]>(MAX_BONES);
    }
    BMD& b = Models[models.model];
    const MuEditor::Effects::ScopedModelState savedModel(b);
    OBJECT& o = things.object;
    MuEditor::Effects::PosePreviewItem(things.request, models, things.turnDegrees, o, things.bones.get());
    o.Live = true;
    o.Owner = nullptr;
    o.BoneTransform = things.bones.get();
    VectorCopy(Hero->Object.Position, o.Position);
    things.place = {o.Position[0], o.Position[1], o.Position[2]};
    b.BodyScale = o.Scale;
    VectorCopy(o.Position, b.BodyOrigin);

    m_spritesBefore.resize(pools.sprites.size());
    for (std::size_t i = 0; i < pools.sprites.size(); ++i)
    {
        m_spritesBefore[i] = pools.sprites[i].Live;
    }
    {
        // The views play no sound effects, as the world preview.
        const Audio::EditorMute::Scope mute;
        const WorldTimeScope time(things.stoppedAt);
        things.tracker.BeginCreate(pools);
        Render::Items::Looks::SpawnItemEffect(*effect, *types, &b, &o, things.bones.get());
        things.tracker.EndCreate(pools);
    }
    NewSprites(pools.sprites, m_spritesBefore, things.sprites);
    things.made = things.shown;
}

// The view's things, drawn as its camera sees them: the view drew its item at the scene's origin, its things stand
// around the copy at the character, so the camera is moved there. The draw loops draw only the view's slots, with no
// test against the world's view, and the world's per-frame sprite marks are kept.
void BoneThingsPreview::Capture(int view, const ViewThings& things,
                                const MuEditor::Effects::EffectPreviewCamera& camera)
{
    std::array<float, 16> matrix = camera.View();
    for (int row = 0; row < 4; ++row)
    {
        matrix[12 + row] -=
            matrix[row] * things.place[0] + matrix[4 + row] * things.place[1] + matrix[8 + row] * things.place[2];
    }
    mu::IMuRenderer& renderer = mu::GetRenderer();
    renderer.SetMatrixMode(GL_MODELVIEW);
    renderer.PushMatrix();
    renderer.LoadMatrix(matrix.data());
    // Kept here, not in SaveCameraPerspective's one slot, which the game's item windows use.
    const CameraState worldCamera = g_Camera;
    for (int row = 0; row < 3; ++row)
    {
        for (int column = 0; column < 4; ++column)
        {
            g_Camera.Matrix[row][column] = matrix[column * 4 + row];
        }
    }
    const SpriteDrawFlags worldSprites = SpritesDrawnThisFrame();
    {
        const Render::Effects::PreviewSlots::CaptureScope capture(MarkOf(view));
        BeginSpriteDraws();
        RenderJoints();
        RenderEffects();
        BeginSprite();
        RenderSprites();
        RenderParticles();
        EndSprite();
    }
    SetSpritesDrawnThisFrame(worldSprites);
    g_Camera = worldCamera;
    renderer.SetMatrixMode(GL_MODELVIEW);
    renderer.PopMatrix();
}

bool BoneThingsPreview::HasRoom() const
{
    const EffectPools pools = MuEditor::Effects::GetGamePools();
    MuEditor::Effects::EffectPreviewCounts ours;
    int ourSprites = 0;
    for (const ViewThings& things : m_views)
    {
        const MuEditor::Effects::EffectPreviewCounts counts = things.tracker.Count();
        ours.effects += counts.effects;
        ours.particles += counts.particles;
        ours.joints += counts.joints;
        ourSprites += static_cast<int>(things.sprites.size());
    }
    const auto fits = [](int size, int free, int used) { return free >= size / 4 && used < size / 4; };
    return fits(static_cast<int>(pools.effects.size()), CountFree(pools.effects), ours.effects) &&
           fits(static_cast<int>(pools.particles.size()), CountFree(pools.particles), ours.particles) &&
           fits(static_cast<int>(pools.joints.size()), CountFree(pools.joints), ours.joints) &&
           fits(static_cast<int>(pools.sprites.size()), CountFree(pools.sprites), ourSprites);
}
} // namespace MuEditor::Looks

#endif // _EDITOR
