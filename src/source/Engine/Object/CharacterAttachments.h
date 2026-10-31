#pragma once

#include "Core/Math/ZzzMathLib.h"

#include <functional>
#include <memory>
#include <optional>

class CHARACTER;
struct _PART_t;

// The parts a character's draw attaches to it (weapons, wings, back items, set objects), one slot per
// RenderLinkObject call in draw order, each with the part's own pose.
namespace Engine::Object::Attachments
{
// The animation state and placement a slot's bones were posed with.
struct Posed
{
    float animationFrame = 0.f;
    float priorAnimationFrame = 0.f;
    unsigned short priorAction = 0;
    unsigned short action = 0;
    float parent[3][4]{};
    vec3_t origin{};
    vec3_t angle{};
    float scale = 0.f;
};

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
    unsigned char linkBone = 0;
    // The character's scale during the call; some draws change it around one part.
    float ownerScale = 1.f;
    // The character's part, or nullptr for a part the draw built for this call only.
    _PART_t* part = nullptr;
    // A copy of a part the draw built for this call only (a weapon on the back, a weapon's shadow).
    std::shared_ptr<_PART_t> copy;
    // Drawn in the shadow pass.
    bool shadow = false;
    unsigned int drawnFrame = 0;
    // The previous draw recorded the same attachment in this slot.
    bool sameAsLast = false;

    // The draw an update advanced and posed the part for (its animation step for the next draw), and how.
    unsigned int advancedFor = 0;
    Posed posed;
    std::unique_ptr<vec34_t[]> bones;
};

// Starts a frame's character draws.
void BeginFrame();
// Starts recording the parts drawn for this character.
void BeginCharacter(const CHARACTER* c);
// The next slot of the character, recorded with this draw; nullptr for a character outside CharactersClient, which
// keeps drawing its parts with the shared bones.
Slot* Record(CHARACTER* c, _PART_t* part, int type, float x, float y, float z, int level, int option1, bool link,
             bool translate, int renderType, bool rightHandItem, unsigned char linkBone, float ownerScale);
// The slot's own bones, MAX_BONES long.
float (*Bones(Slot& slot))[3][4];
// Whether the update since the last draw already advanced and posed this slot's part; the draw then reuses it.
bool WasAdvanced(const Slot& slot);
// Records whether this draw drew the character's weapons in its hands or on its back.
void RecordWeaponsHeld(const CHARACTER* c, bool inHands);
// Whether the character's last draw held its weapons in its hands; nullopt when that draw did not get to them.
std::optional<bool> WeaponsHeldInLastDraw(const CHARACTER* c);
// The bones the character's last draw posed this part of it into, or nullptr.
vec34_t* LastBonesOf(const CHARACTER* c, const _PART_t* part);
// The slots of the character's last draw, for the update to advance once per draw; marks each one it hands out.
void ForEachToAdvance(const CHARACTER* c, const std::function<void(Slot&)>& advance);
// The slots of the character's last draw outside its shadow pass that drew a part built for that call only.
void ForEachCopied(const CHARACTER* c, const std::function<void(Slot&)>& visit);
} // namespace Engine::Object::Attachments
