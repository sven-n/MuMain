#pragma once

#include "Core/Math/ZzzMathLib.h"

#include <memory>

class CHARACTER;
struct _PART_t;

// The parts a character's draw attaches to it (weapons, wings, back items, set objects), one slot per
// RenderLinkObject call in draw order, each with the part's own pose.
namespace Engine::Object::Attachments
{
struct Slot
{
    // The RenderLinkObject call this slot is.
    int type = -1;
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
    int level = 0;
    int option1 = 0;
    bool link = false;
    bool translate = false;
    int renderType = 0;
    bool rightHandItem = false;
    // The character's part, or nullptr for a part the draw built for this call only.
    _PART_t* part = nullptr;
    unsigned int drawnFrame = 0;

    std::unique_ptr<vec34_t[]> bones;
};

// Starts a frame's character draws.
void BeginFrame();
// Starts recording the parts drawn for this character.
void BeginCharacter(const CHARACTER* c);
// The next slot of the character, recorded with this draw; nullptr for a character outside CharactersClient, which
// keeps drawing its parts with the shared bones.
Slot* Record(CHARACTER* c, _PART_t* part, int type, float x, float y, float z, int level, int option1, bool link,
             bool translate, int renderType, bool rightHandItem);
// The slot's own bones, MAX_BONES long.
float (*Bones(Slot& slot))[3][4];
} // namespace Engine::Object::Attachments
