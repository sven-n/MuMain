#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/EffectBrowser/EffectPreviewCamera.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewScene.h"

#include <memory>

class OBJECT;

namespace MuEditor::Monsters
{
// What the Monster Preview shows: an animation frame of a monster or NPC
// model, standing on a checkered plane, drawn as RenderObject draws a model
// (the effect preview's pose), into the open capture.
class MonsterPreviewScene
{
public:
    MonsterPreviewScene();
    ~MonsterPreviewScene();

    MonsterPreviewScene(const MonsterPreviewScene&) = delete;
    MonsterPreviewScene& operator=(const MonsterPreviewScene&) = delete;

    // What the camera frames: the model at the first frame of its first
    // animation, with the plane under it.
    Effects::PreviewFraming Frame(int slot);

    // Draws the slot's model at `frame` of `animation`, blended from
    // `priorFrame` as the game blends (BMD::Animation), turned by
    // `turnDegrees`, as `camera` sees it at the aspect width / height of the
    // view. Leaves the game's render state as it was.
    void Draw(int slot, int animation, float frame, float priorFrame, float turnDegrees,
              const Effects::EffectPreviewCamera& camera, float aspect);

private:
    void SetUp(int slot, int animation, float frame, float priorFrame, float turnDegrees);

    std::unique_ptr<OBJECT> m_object;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
