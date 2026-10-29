#pragma once

#ifdef _EDITOR

#include "MonsterPreviewScene.h"

#include "../MuEditor/UI/EffectBrowser/EffectPreviewCamera.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewTarget.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

namespace MuEditor::Monsters
{
// The Monster Preview tool (monsters design, section 9): the selected
// monster's model in a view with the item preview's camera (the mouse turns
// it, the middle button moves the picture, the wheel zooms). Any animation of
// the model plays, pauses and goes frame by frame; the speeds are the Monster
// Model tool's, also while one is being dragged; the model's slot sounds play
// when their animation starts; the impact frame of attacks 1 and 2 and the
// animation that follows are marked, and the speed factors the game adds can
// be played too. Only in the editor's window (Q-MN17).
class MuMonsterPreviewUI
{
public:
    static MuMonsterPreviewUI& GetInstance();

    void Render(bool* open);

    // Between frames: the texture can only be released then.
    void BeforeFrame()
    {
        m_target.BeforeFrame();
    }

private:
    MuMonsterPreviewUI() = default;

    void RenderPreview(int slot);
    void RenderControls(int slot, const Data::Monsters::MonsterModelDefinition& values);
    void RenderFrameSlider(int slot, int keys);
    void RenderView(int slot, const Data::Monsters::MonsterModelDefinition& values);
    void Advance(int slot, const Data::Monsters::MonsterModelDefinition& values);
    void Restart(int slot);
    float SpeedFactor(int slot) const;

    Effects::EffectPreviewCamera m_camera;
    Effects::EffectPreviewTarget m_target;
    MonsterPreviewScene m_scene;
    // The slot the camera was framed for, and the animation shown.
    int m_framedSlot = -1;
    int m_slot = -1;
    int m_animation = 0;
    float m_frame = 0.0f;
    float m_turnDegrees = 0.0f;
    bool m_playing = true;
    bool m_turn = false;
    bool m_sounds = true;
    bool m_gameFactors = true;
    bool m_transformed = false;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterPreviewUI MuEditor::Monsters::MuMonsterPreviewUI::GetInstance()

#endif // _EDITOR
