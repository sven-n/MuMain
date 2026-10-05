#pragma once

#ifdef _EDITOR

#include "EffectPreviewCamera.h"
#include "EffectPreviewSubject.h"

#include "Data/GameData/EffectData/EffectKind.h"

#include <cstdint>
#include <memory>

class OBJECT;

namespace MuEditor::Effects
{
// What the preview shows the type on.
enum class PreviewShowOn
{
    Nothing,
    Plane,
    Cube,
    Item,
};

// The blends a sprite's SubType chooses (zzzeffectsprite.cpp), in its
// order: 0 glow, 1 subtract, 2 alpha test, 3 luminance.
enum class PreviewSpriteBlend
{
    Glow,
    Subtract,
    AlphaTest,
    Luminance,
};

struct EffectPreviewRequest
{
    Data::Effects::EffectKind kind = Data::Effects::EffectKind::Effect;
    int type = 0;
    int subType = 0;
    PreviewSubject subject;
    PreviewShowOn showOn = PreviewShowOn::Nothing;
    // With PreviewShowOn::Item.
    int itemType = -1;
    int itemLevel = 0;
    bool itemExcellent = false;
    bool itemAncient = false;
    PreviewSpriteBlend spriteBlend = PreviewSpriteBlend::Glow;
    bool turn = true;
};

// Where the camera looks and the size of what it shows.
struct PreviewFraming
{
    PreviewVector center{};
    float radius = 0.0f;
};

// Draws the effect preview into the open capture: what the type is shown
// on, the type, and a last pass that makes the picture opaque.
class EffectPreviewScene
{
public:
    EffectPreviewScene();
    ~EffectPreviewScene();

    // Makes the effect's object again when the type or SubType changed, and
    // returns what the camera frames.
    PreviewFraming Prepare(const EffectPreviewRequest& request);

    // Draws `request` as `camera` sees it, at the aspect width / height of
    // the view. Leaves the game's render state as it was.
    void Draw(const EffectPreviewRequest& request, const EffectPreviewCamera& camera, float aspect);

    // The PreviewNote flags of the effect's object.
    std::uint16_t GetObjectNotes() const
    {
        return m_objectNotes;
    }

private:
    struct Extent
    {
        PreviewVector min{};
        PreviewVector max{};
    };

    void RebuildEffectObject(const EffectPreviewRequest& request);
    Extent SubjectExtent(const EffectPreviewRequest& request) const;
    float BaseTop(const EffectPreviewRequest& request, float subjectRadius) const;
    PreviewVector Anchor(const EffectPreviewRequest& request) const;
    float SpriteScale(const EffectPreviewRequest& request) const;

    void DrawBase(const EffectPreviewRequest& request, float subjectRadius);
    void DrawItem(const EffectPreviewRequest& request);
    void DrawSubject(const EffectPreviewRequest& request, const EffectPreviewCamera& camera);
    void DrawModel(const PreviewVector& anchor);
    void DrawSprite(const EffectPreviewRequest& request, const PreviewVector& anchor,
                    const EffectPreviewCamera& camera);
    void DrawGroundDecal(const EffectPreviewRequest& request, const PreviewVector& anchor);
    void CoverAlpha(const EffectPreviewCamera& camera, float aspect);

    // Never in the effect pools.
    std::unique_ptr<OBJECT> m_effect;
    // What the effect's object was made for.
    Data::Effects::EffectKind m_effectKind = Data::Effects::EffectKind::Effect;
    int m_effectType = -1;
    int m_effectSubType = -1;
    PreviewDraw m_effectDraw = PreviewDraw::None;
    float m_creationYaw = 0.0f;
    std::uint16_t m_objectNotes = 0;
    // The model's extent at its first frame, around its origin.
    Extent m_modelExtent;
    float m_turnDegrees = 0.0f;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
