#pragma once

#ifdef _EDITOR

#include "EffectPreviewCamera.h"

#include "Engine/Object/ZzzObject.h"
#include "Render/Models/ZzzBMD.h"

// How the previews pose a model: the effect preview, and the monster preview.
namespace MuEditor::Effects
{
// The fields of a model the preview sets. The game sets them before each of
// its own draws; the preview puts them back anyway.
class ScopedModelState
{
public:
    explicit ScopedModelState(BMD& model)
        : m_model(model), m_lightEnable(model.LightEnable), m_contrastEnable(model.ContrastEnable),
          m_hideSkin(model.HideSkin), m_bodyScale(model.BodyScale), m_bodyHeight(model.BodyHeight),
          m_currentAction(model.CurrentAction)
    {
        VectorCopy(model.BodyLight, m_bodyLight);
        VectorCopy(model.BodyOrigin, m_bodyOrigin);
    }

    ~ScopedModelState()
    {
        m_model.LightEnable = m_lightEnable;
        m_model.ContrastEnable = m_contrastEnable;
        m_model.HideSkin = m_hideSkin;
        m_model.BodyScale = m_bodyScale;
        m_model.BodyHeight = m_bodyHeight;
        m_model.CurrentAction = m_currentAction;
        VectorCopy(m_bodyLight, m_model.BodyLight);
        VectorCopy(m_bodyOrigin, m_model.BodyOrigin);
    }

    ScopedModelState(const ScopedModelState&) = delete;
    ScopedModelState& operator=(const ScopedModelState&) = delete;

private:
    BMD& m_model;
    bool m_lightEnable;
    bool m_contrastEnable;
    bool m_hideSkin;
    float m_bodyScale;
    float m_bodyHeight;
    unsigned short m_currentAction;
    vec3_t m_bodyLight;
    vec3_t m_bodyOrigin;
};

// Places the bones of `o`'s model as RenderObject does before it draws
// (Calc_RenderObject), without the terrain light.
void PoseLikeRenderObject(OBJECT& o, BMD& b);

// The extent of `o`'s model at its first frame, around its origin; false
// for a model without actions, whose bones would stay those of another.
bool MeasureModel(OBJECT& o, PreviewVector& min, PreviewVector& max);
} // namespace MuEditor::Effects

#endif // _EDITOR
