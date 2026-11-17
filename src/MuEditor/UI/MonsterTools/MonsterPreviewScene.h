#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/EffectBrowser/EffectPreviewCamera.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewScene.h"

#include "Data/GameData/MonsterData/MonsterSetup.h"

#include <memory>

class CHARACTER;
class OBJECT;

namespace MuEditor::Monsters
{
// How the preview shows the monster beside its animation (Q-MN33, Q-MN40).
struct MonsterPreviewLook
{
    // As the game draws the monster's character (RenderCharacter): its
    // weapons, gear and skin as `setup` dresses it, the outline while it is
    // pointed at; else the model alone.
    bool asCharacter = false;
    int number = 0;
    int kind = 0;
    Data::Monsters::MonsterSetup setup;
    bool pointedAt = false;
    // The model's own light added to the place's (OBJECT::Light).
    float light = 1.0f;

    bool operator==(const MonsterPreviewLook&) const = default;
};

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

    // A character's size before its setup (CreateCharacterPointer).
    static constexpr float CharacterScale = 0.9f;

    // What the camera frames: the model at the first frame of its first
    // animation at `scale`, with the plane under it.
    Effects::PreviewFraming Frame(int slot, float scale = 1.0f);

    // Draws the slot's model at `frame` of `animation`, blended from
    // `priorFrame` as the game blends (BMD::Animation), turned by
    // `turnDegrees`, as `camera` sees it at the aspect width / height of the
    // view. Leaves the game's render state as it was.
    void Draw(int slot, int animation, float frame, float priorFrame, float turnDegrees,
              const Effects::EffectPreviewCamera& camera, float aspect, const MonsterPreviewLook& look = {});

    // Makes the look's spawn effects again on the character at its next
    // draw (Replay spawn); the ones it has end first.
    void ReplaySpawn();
    // Plays the look's appear on the character at its next draw: its
    // effects, its fade-in and, with `sound`, its sound (Replay appear).
    void ReplayAppear(bool sound);
    // Ends the character's effects; the next draw makes it again. For a
    // preview that is not shown.
    void ReleaseCharacter();

private:
    void SetUp(int slot, int animation, float frame, float priorFrame, float turnDegrees);
    // The character of `look` in the slot at the pose, drawn as the game
    // draws characters, with what the game's tick adds to it; made again,
    // with its spawn effects, when the slot or the look's setup changed.
    void DrawCharacter(int slot, int animation, float frame, float priorFrame, float turnDegrees,
                       const MonsterPreviewLook& look);
    // The game's joints, effects, sprites and particles as the game draws
    // them, seen by `camera`.
    void DrawGameEffects(const Effects::EffectPreviewCamera& camera);

    std::unique_ptr<OBJECT> m_object;
    std::unique_ptr<CHARACTER> m_character;
    int m_characterSlot = -1;
    MonsterPreviewLook m_characterLook;
    bool m_spawnPending = false;
    bool m_appearPending = false;
    bool m_appearSound = false;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
